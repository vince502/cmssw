// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      OniaPhotonProducer
//
// Implementation of Onia + Photon candidate producer
// Based on chi-analysis-miniaod by Alberto Sanchez-Hernandez

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/OniaPhotonProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include <TMath.h>
#include <TVector3.h>
#include <Math/VectorUtil.h>

OniaPhotonProducer::OniaPhotonProducer(const edm::ParameterSet& iConfig)
    : oniaToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("oniaSrc"))),
      conversionToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("conversionSrc"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("pvSrc"))),
      bsToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotSrc"))),
      ttbToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      ttBuilder_(nullptr),
      // Conversion cuts
      minConvPt_(iConfig.getParameter<double>("minConvPt")),
      minConvRho_(iConfig.getParameter<double>("minConvRho")),
      rejectPi0_(iConfig.getParameter<bool>("rejectPi0")),
      // Onia cuts
      minOniaPt_(iConfig.getParameter<double>("minOniaPt")),
      maxOniaAbsY_(iConfig.getParameter<double>("maxOniaAbsY")),
      requireTriggerMatch_(iConfig.getParameter<bool>("requireTriggerMatch")),
      // Matching cuts
      dzMax_(iConfig.getParameter<double>("dzMax")),
      deltaMassRange_(iConfig.getParameter<std::vector<double>>("deltaMassRange")),
      // Combined candidate cuts
      minCandPt_(iConfig.getParameter<double>("minCandPt")),
      maxCandAbsY_(iConfig.getParameter<double>("maxCandAbsY")),
      // Kinematic refit
      doKinematicRefit_(iConfig.getParameter<bool>("doKinematicRefit")),
      constraintMass_(iConfig.getParameter<double>("constraintMass")),
      // Parent type
      parentType_(iConfig.getParameter<std::string>("parentType")),
      // Counters
      nCandidates_(0),
      nDeltaMassFail_(0),
      nDzFail_(0),
      nPi0Fail_(0)
{
  produces<pat::CompositeCandidateCollection>();
  if (doKinematicRefit_) {
    produces<pat::CompositeCandidateCollection>("refit");
  }
}

void OniaPhotonProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  
  // Input collections
  desc.add<edm::InputTag>("oniaSrc", edm::InputTag("onia2MuMuPAT"));
  desc.add<edm::InputTag>("conversionSrc", edm::InputTag("oniaPhotonCandidates", "conversions"));
  desc.add<edm::InputTag>("pvSrc", edm::InputTag("offlinePrimaryVertices"));
  desc.add<edm::InputTag>("beamSpotSrc", edm::InputTag("offlineBeamSpot"));
  
  // Conversion cuts
  desc.add<double>("minConvPt", 0.0);
  desc.add<double>("minConvRho", 0.0);      // conversion radius cut (applied in OniaPhotonConversionProducer)
  desc.add<bool>("rejectPi0", false);        // use pi0 rejection flag from conversion
  
  // Onia cuts
  desc.add<double>("minOniaPt", 0.0);
  desc.add<double>("maxOniaAbsY", 2.4);
  desc.add<bool>("requireTriggerMatch", false);
  
  // Matching cuts
  desc.add<double>("dzMax", 1.0);            // |dz| between conversion and onia vertex
  desc.add<std::vector<double>>("deltaMassRange", {0.0, 2.0});  // deltaM = M(chi) - M(onia)
  
  // Combined candidate cuts
  desc.add<double>("minCandPt", 0.0);
  desc.add<double>("maxCandAbsY", 2.4);
  
  // Kinematic refit
  desc.add<bool>("doKinematicRefit", false);
  desc.add<double>("constraintMass", 3.0969);  // J/psi mass for constraint
  
  // Parent type: "jpsi", "upsilon1S", "upsilon2S", "upsilon3S", "d0"
  desc.add<std::string>("parentType", "jpsi");
  
  descriptions.add("oniaPhotonProducer", desc);
}

void OniaPhotonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  auto chiCandColl = std::make_unique<pat::CompositeCandidateCollection>();
  auto refitCandColl = std::make_unique<pat::CompositeCandidateCollection>();

  // Get transient track builder for kinematic refit
  if (doKinematicRefit_) {
    ttBuilder_ = &iSetup.getData(ttbToken_);
  }

  // Get input collections
  edm::Handle<pat::CompositeCandidateCollection> oniaHandle;
  iEvent.getByToken(oniaToken_, oniaHandle);
  
  edm::Handle<pat::CompositeCandidateCollection> convHandle;
  iEvent.getByToken(conversionToken_, convHandle);
  
  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByToken(pvToken_, pvHandle);
  
  edm::Handle<reco::BeamSpot> bsHandle;
  iEvent.getByToken(bsToken_, bsHandle);

  if (!oniaHandle.isValid() || !convHandle.isValid()) {
    iEvent.put(std::move(chiCandColl));
    if (doKinematicRefit_) {
      iEvent.put(std::move(refitCandColl), "refit");
    }
    return;
  }

  // Get primary vertex
  reco::Vertex pv;
  if (pvHandle.isValid() && !pvHandle->empty() && !pvHandle->front().isFake()) {
    pv = pvHandle->front();
  } else if (bsHandle.isValid()) {
    pv = reco::Vertex(bsHandle->position(), bsHandle->covariance3D());
  }

  // Loop over onia candidates (sorted by decreasing vProb)
  for (size_t oniaIdx = 0; oniaIdx < oniaHandle->size(); ++oniaIdx) {
    const auto& oniaCand = (*oniaHandle)[oniaIdx];
    
    // Onia cuts
    if (oniaCand.pt() < minOniaPt_) continue;
    if (std::abs(oniaCand.rapidity()) > maxOniaAbsY_) continue;
    
    // Trigger match requirement
    if (requireTriggerMatch_) {
      if (oniaCand.hasUserInt("isTriggerMatched") && !oniaCand.userInt("isTriggerMatched")) {
        continue;
      }
    }

    // Get onia vertex position
    reco::Candidate::Point oniaVtxPos;
    if (oniaCand.hasUserData("commonVertex")) {
      const reco::Vertex* oniaVtx = oniaCand.userData<reco::Vertex>("commonVertex");
      if (oniaVtx) oniaVtxPos = oniaVtx->position();
    } else {
      oniaVtxPos = oniaCand.vertex();
    }

    // Loop over conversion candidates
    for (size_t convIdx = 0; convIdx < convHandle->size(); ++convIdx) {
      const auto& convCand = (*convHandle)[convIdx];
      
      // Conversion cuts
      if (!passConversionCuts(convCand)) continue;
      
      // Make chi candidate (simple combination)
      pat::CompositeCandidate chiCand = makeChiCandidate(oniaCand, convCand, oniaIdx, convIdx);
      
      // Delta mass cut
      if (!passDeltaMassCut(chiCand, oniaCand)) {
        nDeltaMassFail_++;
        continue;
      }
      
      // Dz cut
      float dz = computeDz(convCand, oniaVtxPos);
      chiCand.addUserFloat("dz", dz);
      
      if (std::abs(dz) > dzMax_) {
        nDzFail_++;
        continue;
      }
      
      // Pi0 rejection
      if (rejectPi0_ && convCand.hasUserInt("flags")) {
        int flags = convCand.userInt("flags") % 32;  // lower 5 bits
        bool pi0Fail = flags & 8;  // bit 3 = pi0 rejected
        if (pi0Fail) {
          nPi0Fail_++;
          continue;
        }
      }
      
      // Combined candidate cuts
      if (chiCand.pt() < minCandPt_) continue;
      if (std::abs(chiCand.rapidity()) > maxCandAbsY_) continue;
      
      // Store conversion photon info
      chiCand.addUserFloat("convPt", convCand.pt());
      chiCand.addUserFloat("convEta", convCand.eta());
      chiCand.addUserFloat("convPhi", convCand.phi());
      chiCand.addUserFloat("convMass", convCand.mass());
      
      // Conversion vertex info
      chiCand.addUserFloat("convVtxX", convCand.vertex().x());
      chiCand.addUserFloat("convVtxY", convCand.vertex().y());
      chiCand.addUserFloat("convVtxZ", convCand.vertex().z());
      chiCand.addUserFloat("convVtxRho", convCand.vertex().rho());
      
      // Conversion flags (from OniaPhotonConversionProducer)
      if (convCand.hasUserInt("flags")) {
        chiCand.addUserInt("convFlags", convCand.userInt("flags"));
      }
      
      // Get conversion electron tracks (stored as userData in pat::CompositeCandidate)
      const reco::Track* convTk0 = convCand.userData<reco::Track>("track0");
      const reco::Track* convTk1 = convCand.userData<reco::Track>("track1");
      
      if (convTk0 && convTk1) {
        // Electron 1 (higher pT)
        const reco::Track* eleHigher = (convTk0->pt() > convTk1->pt()) ? convTk0 : convTk1;
        const reco::Track* eleLower = (convTk0->pt() > convTk1->pt()) ? convTk1 : convTk0;
        
        chiCand.addUserFloat("ele1_pt", eleHigher->pt());
        chiCand.addUserFloat("ele1_eta", eleHigher->eta());
        chiCand.addUserFloat("ele1_phi", eleHigher->phi());
        chiCand.addUserInt("ele1_charge", eleHigher->charge());
        chiCand.addUserFloat("ele1_chi2", eleHigher->normalizedChi2());
        chiCand.addUserInt("ele1_nHits", eleHigher->numberOfValidHits());
        chiCand.addUserFloat("ele1_dxy", eleHigher->dxy(oniaVtxPos));
        chiCand.addUserFloat("ele1_dz", eleHigher->dz(oniaVtxPos));
        
        // Electron 2 (lower pT)
        chiCand.addUserFloat("ele2_pt", eleLower->pt());
        chiCand.addUserFloat("ele2_eta", eleLower->eta());
        chiCand.addUserFloat("ele2_phi", eleLower->phi());
        chiCand.addUserInt("ele2_charge", eleLower->charge());
        chiCand.addUserFloat("ele2_chi2", eleLower->normalizedChi2());
        chiCand.addUserInt("ele2_nHits", eleLower->numberOfValidHits());
        chiCand.addUserFloat("ele2_dxy", eleLower->dxy(oniaVtxPos));
        chiCand.addUserFloat("ele2_dz", eleLower->dz(oniaVtxPos));
      }
      
      // Store onia info
      chiCand.addUserFloat("oniaMass", oniaCand.mass());
      chiCand.addUserFloat("oniaPt", oniaCand.pt());
      chiCand.addUserFloat("oniaEta", oniaCand.eta());
      chiCand.addUserFloat("oniaPhi", oniaCand.phi());
      chiCand.addUserFloat("oniaY", oniaCand.rapidity());
      if (oniaCand.hasUserFloat("vProb")) {
        chiCand.addUserFloat("oniaVProb", oniaCand.userFloat("vProb"));
      }
      
      // Delta mass (Q-value)
      double deltaM = chiCand.mass() - oniaCand.mass();
      chiCand.addUserFloat("deltaM", deltaM);
      
      // Corrected mass using PDG value
      double correctedMass = deltaM + constraintMass_;
      chiCand.addUserFloat("correctedMass", correctedMass);
      
      chiCandColl->push_back(chiCand);
      nCandidates_++;
      
      // Kinematic refit
      if (doKinematicRefit_) {
        pat::CompositeCandidate refitCand = doKinematicRefit(chiCand, oniaCand, convCand, pv, oniaIdx);
        if (refitCand.mass() > 0) {
          refitCand.addUserInt("rawIndex", chiCandColl->size() - 1);
          refitCandColl->push_back(refitCand);
        }
      }
    }
  }

  iEvent.put(std::move(chiCandColl));
  if (doKinematicRefit_) {
    iEvent.put(std::move(refitCandColl), "refit");
  }
}

pat::CompositeCandidate OniaPhotonProducer::makeChiCandidate(
    const pat::CompositeCandidate& onia,
    const pat::CompositeCandidate& photon,
    int oniaIdx, int convIdx) {
  
  pat::CompositeCandidate chiCand;
  chiCand.addDaughter(onia, "dimuon");
  chiCand.addDaughter(photon, "photon");
  
  // Set vertex from onia
  reco::Candidate::Point vtxPos;
  if (onia.hasUserData("commonVertex")) {
    const reco::Vertex* oniaVtx = onia.userData<reco::Vertex>("commonVertex");
    if (oniaVtx) vtxPos = oniaVtx->position();
  } else {
    vtxPos = onia.vertex();
  }
  chiCand.setVertex(vtxPos);
  
  // Set 4-momentum
  reco::Candidate::LorentzVector chiP4 = onia.p4() + photon.p4();
  chiCand.setP4(chiP4);
  
  // Store indices
  chiCand.addUserInt("oniaIdx", oniaIdx);
  chiCand.addUserInt("convIdx", convIdx);
  
  return chiCand;
}

float OniaPhotonProducer::computeDz(const pat::CompositeCandidate& conv, 
                                    const reco::Candidate::Point& vtxPos) {
  // Compute dz using the formula from the original code
  // dz = (vz - pz) - ((vx - px)*px_mom + (vy - py)*py_mom)/pT * pz_mom/pT
  
  const reco::Candidate::LorentzVector& mom = conv.p4();
  const reco::Candidate::Point& convVtx = conv.vertex();
  
  double dz = (convVtx.Z() - vtxPos.Z()) - 
              ((convVtx.X() - vtxPos.X()) * mom.X() + (convVtx.Y() - vtxPos.Y()) * mom.Y()) / 
              mom.Rho() * mom.Z() / mom.Rho();
  
  return static_cast<float>(dz);
}

bool OniaPhotonProducer::passDeltaMassCut(const pat::CompositeCandidate& chiCand,
                                          const pat::CompositeCandidate& oniaCand) {
  double deltaM = chiCand.mass() - oniaCand.mass();
  return (deltaM > deltaMassRange_[0] && deltaM < deltaMassRange_[1]);
}

bool OniaPhotonProducer::passConversionCuts(const pat::CompositeCandidate& conv) {
  if (conv.pt() < minConvPt_) return false;
  if (conv.vertex().rho() < minConvRho_) return false;
  return true;
}

pat::CompositeCandidate OniaPhotonProducer::doKinematicRefit(
    const pat::CompositeCandidate& chiCand,
    const pat::CompositeCandidate& oniaCand,
    const pat::CompositeCandidate& photonCand,
    const reco::Vertex& pv,
    int oniaIdx) {
  
  pat::CompositeCandidate refitCand;
  refitCand.setP4(reco::Candidate::LorentzVector(0, 0, 0, -1));  // Invalid by default

  // Get muon tracks from onia
  reco::TrackRef muTk1, muTk2;
  const reco::Candidate* dau1 = oniaCand.daughter("muon1");
  const reco::Candidate* dau2 = oniaCand.daughter("muon2");
  
  if (!dau1 || !dau2) {
    // Try without role names
    if (oniaCand.numberOfDaughters() >= 2) {
      dau1 = oniaCand.daughter(0);
      dau2 = oniaCand.daughter(1);
    }
  }
  
  if (!dau1 || !dau2) return refitCand;
  
  const pat::Muon* mu1 = dynamic_cast<const pat::Muon*>(dau1);
  const pat::Muon* mu2 = dynamic_cast<const pat::Muon*>(dau2);
  
  if (!mu1 || !mu2) return refitCand;
  
  muTk1 = mu1->innerTrack();
  muTk2 = mu2->innerTrack();
  
  if (muTk1.isNull() || muTk2.isNull()) return refitCand;

  // Get conversion tracks
  const reco::Track* convTk0 = photonCand.userData<reco::Track>("track0");
  const reco::Track* convTk1 = photonCand.userData<reco::Track>("track1");
  
  if (!convTk0 || !convTk1) return refitCand;

  // Build transient tracks
  std::vector<reco::TransientTrack> mumuTT;
  mumuTT.push_back(ttBuilder_->build(&muTk1));
  mumuTT.push_back(ttBuilder_->build(&muTk2));
  
  std::vector<reco::TransientTrack> eeTT;
  eeTT.push_back(ttBuilder_->build(*convTk0));
  eeTT.push_back(ttBuilder_->build(*convTk1));

  // Kinematic fit for photon (e+e- with mass = 0 constraint)
  const ParticleMass zeroMass(0);
  float zeroSigma = 1e-6;
  const ParticleMass eleMass(OniaPhoton::elMass);
  float eleSigma = 1e-6;
  
  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> photonParticles;
  photonParticles.push_back(pFactory.particle(eeTT[0], eleMass, float(0), float(0), eleSigma));
  photonParticles.push_back(pFactory.particle(eeTT[1], eleMass, float(0), float(0), eleSigma));
  
  KinematicParticleVertexFitter fitter;
  RefCountedKinematicTree photonTree = fitter.fit(photonParticles);
  
  if (!photonTree->isValid()) {
    // Try with relaxed parameters
    edm::ParameterSet pSet;
    pSet.addParameter<double>("maxDistance", 3);
    pSet.addParameter<int>("maxNbrOfIterations", 10000);
    KinematicParticleVertexFitter fitter2(pSet);
    photonTree = fitter2.fit(photonParticles);
  }
  
  if (!photonTree->isValid()) return refitCand;

  // Apply photon mass constraint (m = 0)
  KinematicParticleFitter csFitter;
  KinematicConstraint* photonConstraint = new MassKinematicConstraint(zeroMass, zeroSigma);
  photonTree->movePointerToTheTop();
  photonTree = csFitter.fit(photonConstraint, photonTree);
  
  if (!photonTree->isValid()) {
    delete photonConstraint;
    return refitCand;
  }
  delete photonConstraint;

  // Get fitted photon
  photonTree->movePointerToTheTop();
  RefCountedKinematicParticle fitPhoton = photonTree->currentParticle();
  
  // Build chi candidate: mu+ mu- + gamma
  const ParticleMass muonMass(OniaPhoton::muMass);
  float muonSigma = muonMass * 1e-6;
  
  std::vector<RefCountedKinematicParticle> chiDaughters;
  chiDaughters.push_back(pFactory.particle(mumuTT[0], muonMass, float(0), float(0), muonSigma));
  chiDaughters.push_back(pFactory.particle(mumuTT[1], muonMass, float(0), float(0), muonSigma));
  chiDaughters.push_back(fitPhoton);
  
  // Constrained vertex fit with onia mass constraint
  KinematicConstrainedVertexFitter constFitter;
  MultiTrackKinematicConstraint* oniaMassConstraint = new TwoTrackMassKinematicConstraint(constraintMass_);
  RefCountedKinematicTree chiTree = constFitter.fit(chiDaughters, oniaMassConstraint);
  delete oniaMassConstraint;
  
  if (chiTree->isEmpty() || !chiTree->isValid()) return refitCand;
  
  chiTree->movePointerToTheTop();
  RefCountedKinematicParticle fitChi = chiTree->currentParticle();
  RefCountedKinematicVertex chiVertex = chiTree->currentDecayVertex();
  
  if (!fitChi->currentState().isValid()) return refitCand;

  // Extract fitted quantities
  float chiM = fitChi->currentState().mass();
  float chiPx = fitChi->currentState().kinematicParameters().momentum().x();
  float chiPy = fitChi->currentState().kinematicParameters().momentum().y();
  float chiPz = fitChi->currentState().kinematicParameters().momentum().z();
  float chiVtxX = chiVertex->position().x();
  float chiVtxY = chiVertex->position().y();
  float chiVtxZ = chiVertex->position().z();
  float chiVProb = ChiSquaredProbability(chiVertex->chiSquared(), chiVertex->degreesOfFreedom());
  
  // Create refitted candidate
  float chiE = std::sqrt(chiM * chiM + chiPx * chiPx + chiPy * chiPy + chiPz * chiPz);
  reco::CompositeCandidate recoRefitCand(0, 
                                         math::XYZTLorentzVector(chiPx, chiPy, chiPz, chiE),
                                         math::XYZPoint(chiVtxX, chiVtxY, chiVtxZ), 
                                         50551);  // chi_b0 PDG ID (placeholder)
  
  refitCand = pat::CompositeCandidate(recoRefitCand);
  refitCand.addUserFloat("vProb", chiVProb);
  refitCand.addUserInt("oniaIdx", oniaIdx);
  
  // Compute lifetime quantities
  TVector3 vtx(chiVtxX, chiVtxY, 0);
  TVector3 pvtx(pv.x(), pv.y(), 0);
  TVector3 pperp(chiPx, chiPy, 0);
  
  TVector3 vdiff = vtx - pvtx;
  double cosAlpha = vdiff.Dot(pperp) / (vdiff.Perp() * pperp.Perp());
  
  VertexDistanceXY vdistXY;
  reco::Vertex myVertex(*chiVertex);
  Measurement1D distXY = vdistXY.distance(myVertex, pv);
  
  double ctauPV = distXY.value() * cosAlpha * chiM / pperp.Perp();
  
  // Error calculation
  AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0);
  GlobalError v1e = myVertex.error();
  GlobalError v2e = pv.error();
  AlgebraicSymMatrix33 vXYe = v1e.matrix() + v2e.matrix();
  double ctauErrPV = std::sqrt(ROOT::Math::Similarity(vpperp, vXYe)) * chiM / (pperp.Perp2());
  
  refitCand.addUserFloat("ctauPV", ctauPV);
  refitCand.addUserFloat("ctauErrPV", ctauErrPV);
  refitCand.addUserFloat("cosAlpha", cosAlpha);
  
  // Extract fitted daughters
  // Muon 1
  bool child = chiTree->movePointerToTheFirstChild();
  if (!child) return refitCand;
  RefCountedKinematicParticle fitMu1 = chiTree->currentParticle();
  
  float mu1M = fitMu1->currentState().mass();
  float mu1Q = fitMu1->currentState().particleCharge();
  float mu1Px = fitMu1->currentState().kinematicParameters().momentum().x();
  float mu1Py = fitMu1->currentState().kinematicParameters().momentum().y();
  float mu1Pz = fitMu1->currentState().kinematicParameters().momentum().z();
  float mu1E = std::sqrt(mu1M * mu1M + mu1Px * mu1Px + mu1Py * mu1Py + mu1Pz * mu1Pz);
  
  reco::CompositeCandidate recoMu1(mu1Q, math::XYZTLorentzVector(mu1Px, mu1Py, mu1Pz, mu1E),
                                   math::XYZPoint(chiVtxX, chiVtxY, chiVtxZ), 13);
  pat::CompositeCandidate patMu1(recoMu1);
  
  // Muon 2
  child = chiTree->movePointerToTheNextChild();
  if (!child) return refitCand;
  RefCountedKinematicParticle fitMu2 = chiTree->currentParticle();
  
  float mu2M = fitMu2->currentState().mass();
  float mu2Q = fitMu2->currentState().particleCharge();
  float mu2Px = fitMu2->currentState().kinematicParameters().momentum().x();
  float mu2Py = fitMu2->currentState().kinematicParameters().momentum().y();
  float mu2Pz = fitMu2->currentState().kinematicParameters().momentum().z();
  float mu2E = std::sqrt(mu2M * mu2M + mu2Px * mu2Px + mu2Py * mu2Py + mu2Pz * mu2Pz);
  
  reco::CompositeCandidate recoMu2(mu2Q, math::XYZTLorentzVector(mu2Px, mu2Py, mu2Pz, mu2E),
                                   math::XYZPoint(chiVtxX, chiVtxY, chiVtxZ), 13);
  pat::CompositeCandidate patMu2(recoMu2);
  
  // Build refitted dimuon
  pat::CompositeCandidate dimuon;
  dimuon.addDaughter(patMu1, "muon1");
  dimuon.addDaughter(patMu2, "muon2");
  dimuon.setP4(patMu1.p4() + patMu2.p4());
  
  // Photon
  child = chiTree->movePointerToTheNextChild();
  if (!child) return refitCand;
  RefCountedKinematicParticle fitGamma = chiTree->currentParticle();
  
  float gammaM = fitGamma->currentState().mass();
  float gammaPx = fitGamma->currentState().kinematicParameters().momentum().x();
  float gammaPy = fitGamma->currentState().kinematicParameters().momentum().y();
  float gammaPz = fitGamma->currentState().kinematicParameters().momentum().z();
  float gammaE = std::sqrt(gammaM * gammaM + gammaPx * gammaPx + gammaPy * gammaPy + gammaPz * gammaPz);
  
  reco::CompositeCandidate recoGamma(0, math::XYZTLorentzVector(gammaPx, gammaPy, gammaPz, gammaE),
                                     math::XYZPoint(chiVtxX, chiVtxY, chiVtxZ), 22);
  pat::CompositeCandidate patGamma(recoGamma);
  
  refitCand.addDaughter(dimuon, "dimuon");
  refitCand.addDaughter(patGamma, "photon");
  
  return refitCand;
}

DEFINE_FWK_MODULE(OniaPhotonProducer);
