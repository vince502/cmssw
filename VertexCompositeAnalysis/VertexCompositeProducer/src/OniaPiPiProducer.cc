// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      OniaPiPiProducer
//
// Producer for Onia + Pi+ Pi- (X(3872), psi(2S)) and Onia + e+ e- (chi_c conversion)

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/OniaPiPiProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"

#include <TMath.h>
#include <TVector3.h>

OniaPiPiProducer::OniaPiPiProducer(const edm::ParameterSet& iConfig)
    : oniaToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("oniaSrc"))),
      trackToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("trackSrc"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexSrc"))),
      bsToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotSrc"))),
      useDeDx_(false),
      ttbToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      bFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
      ttBuilder_(nullptr),
      magField_(nullptr),
      // Mode selection
      doPiPi_(iConfig.getParameter<bool>("doPiPi")),
      doConversion_(iConfig.getParameter<bool>("doConversion")),
      // Onia cuts
      minOniaPt_(iConfig.getParameter<double>("minOniaPt")),
      maxOniaAbsY_(iConfig.getParameter<double>("maxOniaAbsY")),
      // Track cuts
      tkPtCut_(iConfig.getParameter<double>("tkPtCut")),
      tkEtaCut_(iConfig.getParameter<double>("tkEtaCut")),
      tkChi2Cut_(iConfig.getParameter<double>("tkChi2Cut")),
      tkNhitsCut_(iConfig.getParameter<int>("tkNhitsCut")),
      tkPtErrCut_(iConfig.getParameter<double>("tkPtErrCut")),
      tkDCACut_(iConfig.getParameter<double>("tkDCACut")),
      // Di-track cuts
      ditrackPtCut_(iConfig.getParameter<double>("ditrackPtCut")),
      ditrackMassMin_(iConfig.getParameter<double>("ditrackMassMin")),
      ditrackMassMax_(iConfig.getParameter<double>("ditrackMassMax")),
      // Combined candidate cuts
      candMassMin_(iConfig.getParameter<double>("candMassMin")),
      candMassMax_(iConfig.getParameter<double>("candMassMax")),
      candPtCut_(iConfig.getParameter<double>("candPtCut")),
      candAbsYCut_(iConfig.getParameter<double>("candAbsYCut")),
      // Vertex cuts
      vtxChi2Cut_(iConfig.getParameter<double>("vtxChi2Cut")),
      vtxSignif3DCut_(iConfig.getParameter<double>("vtxSignif3DCut")),
      alphaCut_(iConfig.getParameter<double>("alphaCut")),
      alpha2DCut_(iConfig.getParameter<double>("alpha2DCut")),
      // Conversion mode
      convRadiusMin_(iConfig.getParameter<double>("convRadiusMin")),
      convRadiusMax_(iConfig.getParameter<double>("convRadiusMax")),
      // Wrong-sign
      isWrongSign_(iConfig.getParameter<bool>("isWrongSign"))
{
  // dEdx source - configurable, empty InputTag means disabled
  edm::InputTag dedxTag = iConfig.getParameter<edm::InputTag>("dedxSrc");
  useDeDx_ = !dedxTag.label().empty();
  if (useDeDx_) {
    dedxToken_ = consumes<edm::ValueMap<reco::DeDxData>>(dedxTag);
    // Get track to PackedCandidate mapping (produced by unpackedTracksAndVertices)
    track2pcToken_ = consumes<std::vector<edm::Ptr<pat::PackedCandidate>>>(
        iConfig.getParameter<edm::InputTag>("trackSrc"));
  }

  if (doPiPi_) {
    produces<pat::CompositeCandidateCollection>("OniaPiPi");
  }
  if (doConversion_) {
    produces<pat::CompositeCandidateCollection>("OniaEE");
  }
}

void OniaPiPiProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  
  // Input collections
  desc.add<edm::InputTag>("oniaSrc", edm::InputTag("onia2MuMuPAT"));
  desc.add<edm::InputTag>("trackSrc", edm::InputTag("generalTracks"));
  desc.add<edm::InputTag>("vertexSrc", edm::InputTag("offlinePrimaryVertices"));
  desc.add<edm::InputTag>("beamSpotSrc", edm::InputTag("offlineBeamSpot"));
  desc.add<edm::InputTag>("dedxSrc", edm::InputTag(""));  // empty = disabled
  
  // Mode selection
  desc.add<bool>("doPiPi", true);        // X(3872), psi(2S) -> J/psi pi+ pi-
  desc.add<bool>("doConversion", true);  // chi_c -> J/psi gamma -> J/psi e+ e-
  
  // Onia cuts
  desc.add<double>("minOniaPt", 0.0);
  desc.add<double>("maxOniaAbsY", 2.4);
  
  // Track cuts
  desc.add<double>("tkPtCut", 0.3);
  desc.add<double>("tkEtaCut", 2.4);
  desc.add<double>("tkChi2Cut", 5.0);
  desc.add<int>("tkNhitsCut", 5);
  desc.add<double>("tkPtErrCut", 0.1);
  desc.add<double>("tkDCACut", 1.0);
  
  // Di-track cuts
  desc.add<double>("ditrackPtCut", 0.0);
  desc.add<double>("ditrackMassMin", 0.0);
  desc.add<double>("ditrackMassMax", 2.0);
  
  // Combined candidate cuts
  desc.add<double>("candMassMin", 3.4);
  desc.add<double>("candMassMax", 4.2);
  desc.add<double>("candPtCut", 0.0);
  desc.add<double>("candAbsYCut", 2.4);
  
  // Vertex cuts
  desc.add<double>("vtxChi2Cut", 10.0);
  desc.add<double>("vtxSignif3DCut", 0.0);
  desc.add<double>("alphaCut", 1.0);
  desc.add<double>("alpha2DCut", 1.0);
  
  // Conversion mode specific
  desc.add<double>("convRadiusMin", 0.0);
  desc.add<double>("convRadiusMax", 100.0);
  
  // Wrong-sign
  desc.add<bool>("isWrongSign", false);
  
  descriptions.add("oniaPiPiProducer", desc);
}

bool OniaPiPiProducer::passTrackCuts(const reco::Track& track) {
  if (track.pt() < tkPtCut_) return false;
  if (std::abs(track.eta()) > tkEtaCut_) return false;
  if (track.normalizedChi2() > tkChi2Cut_) return false;
  if (track.numberOfValidHits() < tkNhitsCut_) return false;
  if (track.ptError() / track.pt() > tkPtErrCut_) return false;
  return true;
}

void OniaPiPiProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  auto oniaPiPiColl = std::make_unique<pat::CompositeCandidateCollection>();
  auto oniaEEColl = std::make_unique<pat::CompositeCandidateCollection>();

  // Get TransientTrackBuilder
  ttBuilder_ = &iSetup.getData(ttbToken_);
  magField_ = &iSetup.getData(bFieldToken_);

  // Get input collections
  edm::Handle<pat::CompositeCandidateCollection> oniaHandle;
  iEvent.getByToken(oniaToken_, oniaHandle);
  
  edm::Handle<reco::TrackCollection> trackHandle;
  iEvent.getByToken(trackToken_, trackHandle);
  
  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByToken(pvToken_, pvHandle);
  
  edm::Handle<reco::BeamSpot> bsHandle;
  iEvent.getByToken(bsToken_, bsHandle);

  // Get dEdx handles
  edm::Handle<edm::ValueMap<reco::DeDxData>> dEdxHandle;
  edm::Handle<std::vector<edm::Ptr<pat::PackedCandidate>>> track2pcHandle;
  if (useDeDx_) {
    iEvent.getByToken(dedxToken_, dEdxHandle);
    iEvent.getByToken(track2pcToken_, track2pcHandle);
  }

  if (!oniaHandle.isValid() || !trackHandle.isValid()) {
    if (doPiPi_) iEvent.put(std::move(oniaPiPiColl), "OniaPiPi");
    if (doConversion_) iEvent.put(std::move(oniaEEColl), "OniaEE");
    return;
  }

  // Get primary vertex
  reco::Vertex pv;
  if (pvHandle.isValid() && !pvHandle->empty() && !pvHandle->front().isFake()) {
    pv = pvHandle->front();
  } else if (bsHandle.isValid()) {
    pv = reco::Vertex(bsHandle->position(), bsHandle->covariance3D());
  }
  
  // Helper lambda to get dEdx for a track
  auto getDeDx = [&](const reco::TrackRef& trackRef) -> double {
    if (!useDeDx_ || !dEdxHandle.isValid() || !track2pcHandle.isValid()) return -999.;
    const auto& dEdxMap = *dEdxHandle.product();
    const auto& track2pc = *track2pcHandle.product();
    if (trackRef.key() >= track2pc.size()) return -999.;
    auto pc = track2pc.at(trackRef.key());
    if (pc.isNonnull() && dEdxMap.contains(pc.id()))
      return dEdxMap[pc].dEdx();
    return -999.;
  };
  
  // Separate tracks by charge
  std::vector<reco::TrackRef> positiveTracks;
  std::vector<reco::TrackRef> negativeTracks;
  
  for (size_t i = 0; i < trackHandle->size(); ++i) {
    reco::TrackRef trackRef(trackHandle, i);
    if (!passTrackCuts(*trackRef)) continue;
    
    if (trackRef->charge() > 0) {
      positiveTracks.push_back(trackRef);
    } else {
      negativeTracks.push_back(trackRef);
    }
  }

  // Loop over onia candidates
  for (size_t oniaIdx = 0; oniaIdx < oniaHandle->size(); ++oniaIdx) {
    const auto& oniaCand = (*oniaHandle)[oniaIdx];
    
    // Onia cuts
    if (oniaCand.pt() < minOniaPt_) continue;
    if (std::abs(oniaCand.rapidity()) > maxOniaAbsY_) continue;
    
    // Get onia vertex position
    reco::Candidate::Point oniaVtxPos = oniaCand.vertex();
    if (oniaCand.hasUserData("commonVertex")) {
      const reco::Vertex* oniaVtx = oniaCand.userData<reco::Vertex>("commonVertex");
      if (oniaVtx) oniaVtxPos = oniaVtx->position();
    }
    
    // Get onia daughter tracks (muons) to exclude from pion/electron candidates
    std::set<size_t> oniaDaughterTrackKeys;
    for (unsigned int iDau = 0; iDau < oniaCand.numberOfDaughters(); ++iDau) {
      const reco::Candidate* dau = oniaCand.daughter(iDau);
      const pat::Muon* mu = dynamic_cast<const pat::Muon*>(dau);
      if (mu && mu->innerTrack().isNonnull()) {
        oniaDaughterTrackKeys.insert(mu->innerTrack().key());
      }
    }

    // ========== Pi+ Pi- mode (X(3872), psi(2S)) ==========
    if (doPiPi_) {
      if (!isWrongSign_) {
        // Right-sign: + - pairs
        for (const auto& piPlusRef : positiveTracks) {
          // Skip if track is from onia
          if (oniaDaughterTrackKeys.count(piPlusRef.key())) continue;
          
          for (const auto& piMinusRef : negativeTracks) {
            // Skip if track is from onia
            if (oniaDaughterTrackKeys.count(piMinusRef.key())) continue;
            
            double dedxPlus = getDeDx(piPlusRef);
            double dedxMinus = getDeDx(piMinusRef);
            pat::CompositeCandidate cand = makeOniaPiPiCandidate(
                oniaCand, piPlusRef, piMinusRef, pv, oniaIdx, dedxPlus, dedxMinus);
            
            // Check if candidate was successfully built (has daughters)
            if (cand.numberOfDaughters() > 0) {
              oniaPiPiColl->push_back(cand);
            }
          }
        }
      } else {
        // Wrong-sign: ++ and -- pairs
        for (size_t i = 0; i < positiveTracks.size(); ++i) {
          if (oniaDaughterTrackKeys.count(positiveTracks[i].key())) continue;
          for (size_t j = i + 1; j < positiveTracks.size(); ++j) {
            if (oniaDaughterTrackKeys.count(positiveTracks[j].key())) continue;
            
            double dedx1 = getDeDx(positiveTracks[i]);
            double dedx2 = getDeDx(positiveTracks[j]);
            pat::CompositeCandidate cand = makeOniaPiPiCandidate(
                oniaCand, positiveTracks[i], positiveTracks[j], pv, oniaIdx, dedx1, dedx2);
            if (cand.numberOfDaughters() > 0) oniaPiPiColl->push_back(cand);
          }
        }
        for (size_t i = 0; i < negativeTracks.size(); ++i) {
          if (oniaDaughterTrackKeys.count(negativeTracks[i].key())) continue;
          for (size_t j = i + 1; j < negativeTracks.size(); ++j) {
            if (oniaDaughterTrackKeys.count(negativeTracks[j].key())) continue;
            
            double dedx1 = getDeDx(negativeTracks[i]);
            double dedx2 = getDeDx(negativeTracks[j]);
            pat::CompositeCandidate cand = makeOniaPiPiCandidate(
                oniaCand, negativeTracks[i], negativeTracks[j], pv, oniaIdx, dedx1, dedx2);
            if (cand.numberOfDaughters() > 0) oniaPiPiColl->push_back(cand);
          }
        }
      }
    }

    // ========== e+ e- (conversion) mode (chi_c) ==========
    if (doConversion_) {
      if (!isWrongSign_) {
        // Right-sign: + - pairs
        for (const auto& ePlusRef : positiveTracks) {
          if (oniaDaughterTrackKeys.count(ePlusRef.key())) continue;
          
          for (const auto& eMinusRef : negativeTracks) {
            if (oniaDaughterTrackKeys.count(eMinusRef.key())) continue;
            
            double dedxPlus = getDeDx(ePlusRef);
            double dedxMinus = getDeDx(eMinusRef);
            pat::CompositeCandidate cand = makeOniaEECandidate(
                oniaCand, ePlusRef, eMinusRef, pv, oniaIdx, dedxPlus, dedxMinus);
            
            // Check if candidate was successfully built (has daughters)
            if (cand.numberOfDaughters() > 0) {
              oniaEEColl->push_back(cand);
            }
          }
        }
      } else {
        // Wrong-sign: ++ and -- pairs
        for (size_t i = 0; i < positiveTracks.size(); ++i) {
          if (oniaDaughterTrackKeys.count(positiveTracks[i].key())) continue;
          for (size_t j = i + 1; j < positiveTracks.size(); ++j) {
            if (oniaDaughterTrackKeys.count(positiveTracks[j].key())) continue;
            
            double dedx1 = getDeDx(positiveTracks[i]);
            double dedx2 = getDeDx(positiveTracks[j]);
            pat::CompositeCandidate cand = makeOniaEECandidate(
                oniaCand, positiveTracks[i], positiveTracks[j], pv, oniaIdx, dedx1, dedx2);
            if (cand.numberOfDaughters() > 0) oniaEEColl->push_back(cand);
          }
        }
        for (size_t i = 0; i < negativeTracks.size(); ++i) {
          if (oniaDaughterTrackKeys.count(negativeTracks[i].key())) continue;
          for (size_t j = i + 1; j < negativeTracks.size(); ++j) {
            if (oniaDaughterTrackKeys.count(negativeTracks[j].key())) continue;
            
            double dedx1 = getDeDx(negativeTracks[i]);
            double dedx2 = getDeDx(negativeTracks[j]);
            pat::CompositeCandidate cand = makeOniaEECandidate(
                oniaCand, negativeTracks[i], negativeTracks[j], pv, oniaIdx, dedx1, dedx2);
            if (cand.numberOfDaughters() > 0) oniaEEColl->push_back(cand);
          }
        }
      }
    }
  }

  if (doPiPi_) iEvent.put(std::move(oniaPiPiColl), "OniaPiPi");
  if (doConversion_) iEvent.put(std::move(oniaEEColl), "OniaEE");
}

pat::CompositeCandidate OniaPiPiProducer::makeOniaPiPiCandidate(
    const pat::CompositeCandidate& onia,
    const reco::TrackRef& piPlusTrack,
    const reco::TrackRef& piMinusTrack,
    const reco::Vertex& pv,
    int oniaIdx,
    double dedxPiPlus,
    double dedxPiMinus) {
  
  pat::CompositeCandidate cand;
  cand.setP4(reco::Candidate::LorentzVector(0, 0, 0, -1)); // Invalid by default

  // Build transient tracks
  reco::TransientTrack piPlusTT = ttBuilder_->build(piPlusTrack);
  reco::TransientTrack piMinusTT = ttBuilder_->build(piMinusTrack);
  
  if (!piPlusTT.isValid() || !piMinusTT.isValid()) return cand;

  // Calculate DCA between tracks
  ClosestApproachInRPhi cApp;
  cApp.calculate(piPlusTT.initialFreeState(), piMinusTT.initialFreeState());
  if (!cApp.status()) return cand;
  
  float dca = cApp.distance();
  if (dca > tkDCACut_) return cand;

  // Build pi+ pi- 4-momentum
  double piPlusE = std::sqrt(piPlusTrack->p() * piPlusTrack->p() + OniaPiPi::piMass * OniaPiPi::piMass);
  double piMinusE = std::sqrt(piMinusTrack->p() * piMinusTrack->p() + OniaPiPi::piMass * OniaPiPi::piMass);
  
  reco::Candidate::LorentzVector piPlusP4(
      piPlusTrack->px(), piPlusTrack->py(), piPlusTrack->pz(), piPlusE);
  reco::Candidate::LorentzVector piMinusP4(
      piMinusTrack->px(), piMinusTrack->py(), piMinusTrack->pz(), piMinusE);
  
  reco::Candidate::LorentzVector diPionP4 = piPlusP4 + piMinusP4;
  
  // Di-pion cuts
  if (diPionP4.pt() < ditrackPtCut_) return cand;
  if (diPionP4.M() < ditrackMassMin_ || diPionP4.M() > ditrackMassMax_) return cand;

  // Build combined candidate: J/psi + pi+ pi-
  reco::Candidate::LorentzVector candP4 = onia.p4() + diPionP4;
  
  // Mass cut
  if (candP4.M() < candMassMin_ || candP4.M() > candMassMax_) return cand;
  if (candP4.pt() < candPtCut_) return cand;
  if (std::abs(candP4.Rapidity()) > candAbsYCut_) return cand;

  // Vertex fit with all 4 tracks (2 muons + 2 pions)
  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> allParticles;
  
  float muSigma = OniaPiPi::muMass * 1e-6;
  float piSigma = OniaPiPi::piMass * 1e-6;
  
  // Get muon tracks from onia
  const reco::Candidate* dau1 = onia.daughter(0);
  const reco::Candidate* dau2 = onia.daughter(1);
  
  const pat::Muon* mu1 = dynamic_cast<const pat::Muon*>(dau1);
  const pat::Muon* mu2 = dynamic_cast<const pat::Muon*>(dau2);
  
  if (!mu1 || !mu2) return cand;
  if (mu1->innerTrack().isNull() || mu2->innerTrack().isNull()) return cand;
  
  reco::TransientTrack mu1TT = ttBuilder_->build(mu1->innerTrack());
  reco::TransientTrack mu2TT = ttBuilder_->build(mu2->innerTrack());
  
  allParticles.push_back(pFactory.particle(mu1TT, OniaPiPi::muMass, float(0), float(0), muSigma));
  allParticles.push_back(pFactory.particle(mu2TT, OniaPiPi::muMass, float(0), float(0), muSigma));
  allParticles.push_back(pFactory.particle(piPlusTT, OniaPiPi::piMass, float(0), float(0), piSigma));
  allParticles.push_back(pFactory.particle(piMinusTT, OniaPiPi::piMass, float(0), float(0), piSigma));
  
  KinematicParticleVertexFitter fitter;
  RefCountedKinematicTree vertexTree = fitter.fit(allParticles);
  
  if (!vertexTree->isValid()) return cand;
  
  vertexTree->movePointerToTheTop();
  RefCountedKinematicVertex fitVertex = vertexTree->currentDecayVertex();
  RefCountedKinematicParticle fitParticle = vertexTree->currentParticle();
  
  if (!fitVertex->vertexIsValid()) return cand;
  
  float vtxChi2 = fitVertex->chiSquared();
  float vtxNdof = fitVertex->degreesOfFreedom();
  float vtxProb = ChiSquaredProbability(vtxChi2, vtxNdof);
  
  if (vtxProb < vtxChi2Cut_) return cand;

  // Vertex position
  GlobalPoint vtxPos = fitVertex->position();
  
  // Calculate lifetime-related quantities
  TVector3 vtx3D(vtxPos.x(), vtxPos.y(), vtxPos.z());
  TVector3 pv3D(pv.x(), pv.y(), pv.z());
  TVector3 vtx2D(vtxPos.x(), vtxPos.y(), 0);
  TVector3 pv2D(pv.x(), pv.y(), 0);
  
  TVector3 pperp(candP4.px(), candP4.py(), 0);
  TVector3 pvec(candP4.px(), candP4.py(), candP4.pz());
  
  TVector3 vdiff2D = vtx2D - pv2D;
  TVector3 vdiff3D = vtx3D - pv3D;
  
  double cosAlpha2D = vdiff2D.Dot(pperp) / (vdiff2D.Mag() * pperp.Mag());
  double cosAlpha3D = vdiff3D.Dot(pvec) / (vdiff3D.Mag() * pvec.Mag());
  
  if (std::abs(std::acos(cosAlpha2D)) > alpha2DCut_) return cand;
  if (std::abs(std::acos(cosAlpha3D)) > alphaCut_) return cand;

  // 3D decay length significance
  VertexDistance3D vdist3D;
  reco::Vertex myVertex(*fitVertex);
  Measurement1D dist3D = vdist3D.distance(myVertex, pv);
  
  double decayLengthSignif3D = dist3D.significance();
  if (decayLengthSignif3D < vtxSignif3DCut_) return cand;

  // Build output candidate
  reco::RecoChargedCandidate piPlusCand(1, piPlusP4, reco::Candidate::Point(vtxPos.x(), vtxPos.y(), vtxPos.z()));
  piPlusCand.setTrack(piPlusTrack);
  
  reco::RecoChargedCandidate piMinusCand(-1, piMinusP4, reco::Candidate::Point(vtxPos.x(), vtxPos.y(), vtxPos.z()));
  piMinusCand.setTrack(piMinusTrack);

  cand.addDaughter(onia, "onia");
  cand.addDaughter(piPlusCand, "piPlus");
  cand.addDaughter(piMinusCand, "piMinus");
  
  cand.setP4(candP4);
  cand.setVertex(reco::Candidate::Point(vtxPos.x(), vtxPos.y(), vtxPos.z()));
  cand.setPdgId(isWrongSign_ ? 9920443 : 20443); // X(3872) or chi_c1
  
  // Add user data
  cand.addUserInt("oniaIdx", oniaIdx);
  cand.addUserFloat("VtxChi2", vtxChi2);
  cand.addUserFloat("VtxNdof", vtxNdof);
  cand.addUserFloat("VtxProb", vtxProb);
  cand.addUserFloat("alpha2D", std::acos(cosAlpha2D));
  cand.addUserFloat("alpha3D", std::acos(cosAlpha3D));
  cand.addUserFloat("decayLength3D", dist3D.value());
  cand.addUserFloat("decayLengthSignif3D", decayLengthSignif3D);
  cand.addUserFloat("dca", dca);
  
  // Di-pion info
  cand.addUserFloat("diPionMass", diPionP4.M());
  cand.addUserFloat("diPionPt", diPionP4.pt());
  
  // Onia info
  cand.addUserFloat("oniaMass", onia.mass());
  cand.addUserFloat("oniaPt", onia.pt());
  
  // Delta mass (Q-value)
  double deltaM = candP4.M() - onia.mass();
  cand.addUserFloat("deltaM", deltaM);
  
  // Pion track info
  cand.addUserFloat("piPlus_pt", piPlusTrack->pt());
  cand.addUserFloat("piPlus_eta", piPlusTrack->eta());
  cand.addUserFloat("piPlus_phi", piPlusTrack->phi());
  cand.addUserFloat("piMinus_pt", piMinusTrack->pt());
  cand.addUserFloat("piMinus_eta", piMinusTrack->eta());
  cand.addUserFloat("piMinus_phi", piMinusTrack->phi());
  
  // dEdx info (for ntuplizer: posDauDeDx = piPlus, negDauDeDx = piMinus)
  cand.addUserFloat("posDauDeDx", dedxPiPlus);
  cand.addUserFloat("negDauDeDx", dedxPiMinus);

  return cand;
}

pat::CompositeCandidate OniaPiPiProducer::makeOniaEECandidate(
    const pat::CompositeCandidate& onia,
    const reco::TrackRef& ePlusTrack,
    const reco::TrackRef& eMinusTrack,
    const reco::Vertex& pv,
    int oniaIdx,
    double dedxEPlus,
    double dedxEMinus) {
  
  pat::CompositeCandidate cand;
  cand.setP4(reco::Candidate::LorentzVector(0, 0, 0, -1)); // Invalid by default

  // Build transient tracks
  reco::TransientTrack ePlusTT = ttBuilder_->build(ePlusTrack);
  reco::TransientTrack eMinusTT = ttBuilder_->build(eMinusTrack);
  
  if (!ePlusTT.isValid() || !eMinusTT.isValid()) return cand;

  // Calculate DCA between tracks
  ClosestApproachInRPhi cApp;
  cApp.calculate(ePlusTT.initialFreeState(), eMinusTT.initialFreeState());
  if (!cApp.status()) return cand;
  
  float dca = cApp.distance();
  if (dca > tkDCACut_) return cand;
  
  // Get crossing point (conversion vertex estimate)
  GlobalPoint crossingPoint = cApp.crossingPoint();
  double convRadius = crossingPoint.perp();
  
  // Conversion radius cut
  if (convRadius < convRadiusMin_ || convRadius > convRadiusMax_) return cand;

  // Build e+ e- 4-momentum (with electron mass for photon conversion)
  double ePlusE = std::sqrt(ePlusTrack->p() * ePlusTrack->p() + OniaPiPi::elMass * OniaPiPi::elMass);
  double eMinusE = std::sqrt(eMinusTrack->p() * eMinusTrack->p() + OniaPiPi::elMass * OniaPiPi::elMass);
  
  reco::Candidate::LorentzVector ePlusP4(
      ePlusTrack->px(), ePlusTrack->py(), ePlusTrack->pz(), ePlusE);
  reco::Candidate::LorentzVector eMinusP4(
      eMinusTrack->px(), eMinusTrack->py(), eMinusTrack->pz(), eMinusE);
  
  reco::Candidate::LorentzVector diElectronP4 = ePlusP4 + eMinusP4;
  
  // Di-electron (photon) cuts - for conversion, mass should be near zero
  if (diElectronP4.pt() < ditrackPtCut_) return cand;
  if (diElectronP4.M() < ditrackMassMin_ || diElectronP4.M() > ditrackMassMax_) return cand;

  // Build combined candidate: J/psi + gamma (e+e-)
  reco::Candidate::LorentzVector candP4 = onia.p4() + diElectronP4;
  
  // Mass cut (chi_c mass range)
  if (candP4.M() < candMassMin_ || candP4.M() > candMassMax_) return cand;
  if (candP4.pt() < candPtCut_) return cand;
  if (std::abs(candP4.Rapidity()) > candAbsYCut_) return cand;

  // Fit e+e- vertex (conversion vertex) with photon mass constraint (m=0)
  KinematicParticleFactoryFromTransientTrack pFactory;
  std::vector<RefCountedKinematicParticle> eeParticles;
  
  float elSigma = OniaPiPi::elMass * 1e-6;
  
  eeParticles.push_back(pFactory.particle(ePlusTT, OniaPiPi::elMass, float(0), float(0), elSigma));
  eeParticles.push_back(pFactory.particle(eMinusTT, OniaPiPi::elMass, float(0), float(0), elSigma));
  
  // First do vertex fit
  KinematicParticleVertexFitter eeFitter;
  RefCountedKinematicTree eeTree = eeFitter.fit(eeParticles);
  
  if (!eeTree->isValid()) return cand;
  
  // Apply photon mass constraint (m = 0) to the e+e- system
  const ParticleMass photonMass(0.0);
  float photonMassSigma = 1e-6;
  KinematicParticleFitter csFitter;
  KinematicConstraint* photonMassConstraint = new MassKinematicConstraint(photonMass, photonMassSigma);
  eeTree->movePointerToTheTop();
  eeTree = csFitter.fit(photonMassConstraint, eeTree);
  delete photonMassConstraint;
  
  if (!eeTree->isValid()) return cand;
  
  eeTree->movePointerToTheTop();
  RefCountedKinematicVertex eeVertex = eeTree->currentDecayVertex();
  RefCountedKinematicParticle fitPhoton = eeTree->currentParticle();
  
  if (!eeVertex->vertexIsValid()) return cand;
  
  float eeVtxChi2 = eeVertex->chiSquared();
  float eeVtxNdof = eeVertex->degreesOfFreedom();
  float eeVtxProb = ChiSquaredProbability(eeVtxChi2, eeVtxNdof);

  GlobalPoint eeVtxPos = eeVertex->position();
  double fittedConvRadius = eeVtxPos.perp();
  
  // Updated conversion radius cut after fit
  if (fittedConvRadius < convRadiusMin_ || fittedConvRadius > convRadiusMax_) return cand;
  
  // Get refitted photon 4-momentum (mass-constrained)
  reco::Candidate::LorentzVector refitPhotonP4;
  if (fitPhoton->currentState().isValid()) {
    float photonPx = fitPhoton->currentState().kinematicParameters().momentum().x();
    float photonPy = fitPhoton->currentState().kinematicParameters().momentum().y();
    float photonPz = fitPhoton->currentState().kinematicParameters().momentum().z();
    float photonE = std::sqrt(photonPx*photonPx + photonPy*photonPy + photonPz*photonPz); // m=0
    refitPhotonP4 = reco::Candidate::LorentzVector(photonPx, photonPy, photonPz, photonE);
  } else {
    refitPhotonP4 = diElectronP4; // fallback to unconstrained
  }

  // Get onia vertex position
  reco::Candidate::Point oniaVtxPos = onia.vertex();
  if (onia.hasUserData("commonVertex")) {
    const reco::Vertex* oniaVtx = onia.userData<reco::Vertex>("commonVertex");
    if (oniaVtx) oniaVtxPos = oniaVtx->position();
  }

  // Calculate dz between conversion and onia vertex using refitted photon momentum
  TVector3 eeVtx(eeVtxPos.x(), eeVtxPos.y(), eeVtxPos.z());
  TVector3 oniaVtx(oniaVtxPos.x(), oniaVtxPos.y(), oniaVtxPos.z());
  TVector3 photonMom(refitPhotonP4.px(), refitPhotonP4.py(), refitPhotonP4.pz());
  
  // dz calculation (similar to OniaPhotonProducer)
  double dz = (eeVtx.Z() - oniaVtx.Z()) - 
              ((eeVtx.X() - oniaVtx.X()) * photonMom.X() + 
               (eeVtx.Y() - oniaVtx.Y()) * photonMom.Y()) / 
              photonMom.Perp() * photonMom.Z() / photonMom.Perp();

  // Build chi_c candidate using mass-constrained photon momentum
  reco::Candidate::LorentzVector chiCandP4 = onia.p4() + refitPhotonP4;

  // Build output candidate
  reco::RecoChargedCandidate ePlusCand(1, ePlusP4, reco::Candidate::Point(eeVtxPos.x(), eeVtxPos.y(), eeVtxPos.z()));
  ePlusCand.setTrack(ePlusTrack);
  
  reco::RecoChargedCandidate eMinusCand(-1, eMinusP4, reco::Candidate::Point(eeVtxPos.x(), eeVtxPos.y(), eeVtxPos.z()));
  eMinusCand.setTrack(eMinusTrack);

  // Build photon (e+e-) composite with mass-constrained momentum
  pat::CompositeCandidate photonCand;
  photonCand.addDaughter(ePlusCand, "ePlus");
  photonCand.addDaughter(eMinusCand, "eMinus");
  photonCand.setP4(refitPhotonP4);  // Use mass-constrained photon 4-momentum
  photonCand.setVertex(reco::Candidate::Point(eeVtxPos.x(), eeVtxPos.y(), eeVtxPos.z()));
  photonCand.setPdgId(22); // photon
  
  cand.addDaughter(onia, "onia");
  cand.addDaughter(photonCand, "photon");
  
  cand.setP4(chiCandP4);  // Use mass-constrained chi_c 4-momentum
  cand.setVertex(oniaVtxPos); // Use onia vertex as candidate vertex
  cand.setPdgId(isWrongSign_ ? 9910441 : 10441); // chi_c0 or similar
  
  // Add user data
  cand.addUserInt("oniaIdx", oniaIdx);
  
  // Standard vertex info names (for ntuplizer compatibility)
  cand.addUserFloat("VtxChi2", eeVtxChi2);
  cand.addUserFloat("VtxNdof", eeVtxNdof);
  cand.addUserFloat("VtxProb", eeVtxProb);
  
  // Conversion-specific vertex info
  cand.addUserFloat("convRadius", fittedConvRadius);
  cand.addUserFloat("convVtxX", eeVtxPos.x());
  cand.addUserFloat("convVtxY", eeVtxPos.y());
  cand.addUserFloat("convVtxZ", eeVtxPos.z());
  cand.addUserFloat("dz", dz);
  cand.addUserFloat("dca", dca);
  
  // Di-electron (photon) info - raw (unconstrained) values
  cand.addUserFloat("photonMassRaw", diElectronP4.M());  // Before mass constraint
  cand.addUserFloat("photonMass", refitPhotonP4.M());    // After mass constraint (should be ~0)
  cand.addUserFloat("photonPt", refitPhotonP4.pt());
  cand.addUserFloat("photonEta", refitPhotonP4.eta());
  cand.addUserFloat("photonPhi", refitPhotonP4.phi());
  
  // Onia info
  cand.addUserFloat("oniaMass", onia.mass());
  cand.addUserFloat("oniaPt", onia.pt());
  
  // Delta mass (Q-value) - both raw and constrained
  double deltaM = chiCandP4.M() - onia.mass();        // Using mass-constrained photon
  double deltaMRaw = candP4.M() - onia.mass();        // Using raw e+e- mass
  cand.addUserFloat("deltaM", deltaM);
  cand.addUserFloat("deltaMRaw", deltaMRaw);
  
  // Electron track info
  cand.addUserFloat("ePlus_pt", ePlusTrack->pt());
  cand.addUserFloat("ePlus_eta", ePlusTrack->eta());
  cand.addUserFloat("ePlus_phi", ePlusTrack->phi());
  cand.addUserFloat("ePlus_chi2", ePlusTrack->normalizedChi2());
  cand.addUserInt("ePlus_nHits", ePlusTrack->numberOfValidHits());
  
  cand.addUserFloat("eMinus_pt", eMinusTrack->pt());
  cand.addUserFloat("eMinus_eta", eMinusTrack->eta());
  cand.addUserFloat("eMinus_phi", eMinusTrack->phi());
  cand.addUserFloat("eMinus_chi2", eMinusTrack->normalizedChi2());
  cand.addUserInt("eMinus_nHits", eMinusTrack->numberOfValidHits());
  
  // dEdx info (for ntuplizer: posDauDeDx = ePlus, negDauDeDx = eMinus)
  cand.addUserFloat("posDauDeDx", dedxEPlus);
  cand.addUserFloat("negDauDeDx", dedxEMinus);

  return cand;
}

DEFINE_FWK_MODULE(OniaPiPiProducer);
