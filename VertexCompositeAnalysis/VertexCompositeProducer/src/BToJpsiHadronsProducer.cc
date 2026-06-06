// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BToJpsiHadronsProducer
//
// Unified B meson producer for multiple decay modes
// See header file for detailed documentation

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BToJpsiHadronsProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include <TMath.h>
#include <TVector3.h>

BToJpsiHadronsProducer::BToJpsiHadronsProducer(const edm::ParameterSet& iConfig)
    : jpsiToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("jpsiSrc"))),
      trackToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("trackSrc"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("pvSrc"))),
      bsToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotSrc"))),
      bFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
      ttbToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      bField_(nullptr),
      ttBuilder_(nullptr),
      // Track cuts
      minTrackPt_(iConfig.getParameter<double>("minTrackPt")),
      maxTrackEta_(iConfig.getParameter<double>("maxTrackEta")),
      minTrackDxySig_(iConfig.getParameter<double>("minTrackDxySig")),
      minTrackDzSig_(iConfig.getParameter<double>("minTrackDzSig")),
      minTrackNhits_(iConfig.getParameter<int>("minTrackNhits")),
      requireHighPurity_(iConfig.getParameter<bool>("requireHighPurity")),
      // J/psi cuts
      minJpsiPt_(iConfig.getParameter<double>("minJpsiPt")),
      maxJpsiAbsY_(iConfig.getParameter<double>("maxJpsiAbsY")),
      jpsiMassMin_(iConfig.getParameter<double>("jpsiMassMin")),
      jpsiMassMax_(iConfig.getParameter<double>("jpsiMassMax")),
      // B cuts
      minBPt_(iConfig.getParameter<double>("minBPt")),
      maxBAbsY_(iConfig.getParameter<double>("maxBAbsY")),
      minVtxProb_(iConfig.getParameter<double>("minVtxProb")),
      // Resonance windows
      phiMassWindow_(iConfig.getParameter<double>("phiMassWindow")),
      kstarMassWindow_(iConfig.getParameter<double>("kstarMassWindow")),
      xPiPiMassMin_(iConfig.getParameter<double>("xPiPiMassMin")),
      xPiPiMassMax_(iConfig.getParameter<double>("xPiPiMassMax")),
      // B mass windows
      bPlusMassWindow_(iConfig.getParameter<double>("bPlusMassWindow")),
      b0MassWindow_(iConfig.getParameter<double>("b0MassWindow")),
      bsMassWindow_(iConfig.getParameter<double>("bsMassWindow")),
      bcMassWindow_(iConfig.getParameter<double>("bcMassWindow")),
      xMassMin_(iConfig.getParameter<double>("xMassMin")),
      xMassMax_(iConfig.getParameter<double>("xMassMax")),
      // Mode flags
      doBplusToJpsiK_(iConfig.getParameter<bool>("doBplusToJpsiK")),
      doBplusToJpsiPi_(iConfig.getParameter<bool>("doBplusToJpsiPi")),
      doB0ToJpsiKPi_(iConfig.getParameter<bool>("doB0ToJpsiKPi")),
      doBsToJpsiKK_(iConfig.getParameter<bool>("doBsToJpsiKK")),
      doBcToJpsiPi_(iConfig.getParameter<bool>("doBcToJpsiPi")),
      doXToJpsiPiPi_(iConfig.getParameter<bool>("doXToJpsiPiPi")),
      // Options
      doVertexFit_(iConfig.getParameter<bool>("doVertexFit")),
      maxDeltaR_overlap_(iConfig.getParameter<double>("maxDeltaR_overlap"))
{
  produces<pat::CompositeCandidateCollection>("BToJpsiHadrons");
}

void BToJpsiHadronsProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  
  // Input tags
  desc.add<edm::InputTag>("jpsiSrc", edm::InputTag("onia2MuMuPAT"));
  desc.add<edm::InputTag>("trackSrc", edm::InputTag("packedPFCandidates"));
  desc.add<edm::InputTag>("pvSrc", edm::InputTag("offlineSlimmedPrimaryVertices"));
  desc.add<edm::InputTag>("beamSpotSrc", edm::InputTag("offlineBeamSpot"));
  
  // Track cuts
  desc.add<double>("minTrackPt", 0.8);
  desc.add<double>("maxTrackEta", 2.4);
  desc.add<double>("minTrackDxySig", 0.0);
  desc.add<double>("minTrackDzSig", 0.0);
  desc.add<int>("minTrackNhits", 3);
  desc.add<bool>("requireHighPurity", true);
  
  // J/psi cuts
  desc.add<double>("minJpsiPt", 3.0);
  desc.add<double>("maxJpsiAbsY", 2.4);
  desc.add<double>("jpsiMassMin", 2.7);  // Wide window to save combinatorics time
  desc.add<double>("jpsiMassMax", 3.4);
  
  // B cuts
  desc.add<double>("minBPt", 5.0);
  desc.add<double>("maxBAbsY", 2.4);
  desc.add<double>("minVtxProb", 0.001);
  
  // Resonance mass windows
  desc.add<double>("phiMassWindow", 0.015);     // ~15 MeV for phi
  desc.add<double>("kstarMassWindow", 0.100);   // ~100 MeV for K*
  desc.add<double>("xPiPiMassMin", 0.0);        // min m(pipi) for X (Q2 cut)
  desc.add<double>("xPiPiMassMax", 1.5);        // max m(pipi) for X
  
  // B mass windows (from PDG mass)
  desc.add<double>("bPlusMassWindow", 0.5);
  desc.add<double>("b0MassWindow", 0.5);
  desc.add<double>("bsMassWindow", 0.5);
  desc.add<double>("bcMassWindow", 0.5);
  desc.add<double>("xMassMin", 3.5);
  desc.add<double>("xMassMax", 10.0);
  
  // Mode flags
  desc.add<bool>("doBplusToJpsiK", true);
  desc.add<bool>("doBplusToJpsiPi", false);
  desc.add<bool>("doB0ToJpsiKPi", true);
  desc.add<bool>("doBsToJpsiKK", true);
  desc.add<bool>("doBcToJpsiPi", false);
  desc.add<bool>("doXToJpsiPiPi", false);
  
  // Options
  desc.add<bool>("doVertexFit", true);
  desc.add<double>("maxDeltaR_overlap", 0.01);
  
  descriptions.add("bToJpsiHadronsProducer", desc);
}

void BToJpsiHadronsProducer::beginRun(edm::Run const&, edm::EventSetup const& iSetup) {
  // Get magnetic field and transient track builder at run start
}

void BToJpsiHadronsProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  auto output = std::make_unique<pat::CompositeCandidateCollection>();
  
  // Get magnetic field and transient track builder
  bField_ = &iSetup.getData(bFieldToken_);
  ttBuilder_ = &iSetup.getData(ttbToken_);
  
  // Get input collections
  edm::Handle<pat::CompositeCandidateCollection> jpsiHandle;
  iEvent.getByToken(jpsiToken_, jpsiHandle);
  if (!jpsiHandle.isValid() || jpsiHandle->empty()) {
    iEvent.put(std::move(output), "BToJpsiHadrons");
    return;
  }
  
  edm::Handle<pat::PackedCandidateCollection> trackHandle;
  iEvent.getByToken(trackToken_, trackHandle);
  if (!trackHandle.isValid() || trackHandle->empty()) {
    iEvent.put(std::move(output), "BToJpsiHadrons");
    return;
  }
  
  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByToken(pvToken_, pvHandle);
  
  edm::Handle<reco::BeamSpot> bsHandle;
  iEvent.getByToken(bsToken_, bsHandle);
  
  // Get primary vertex (use beamspot if no valid PV)
  reco::Vertex pv;
  if (pvHandle.isValid() && !pvHandle->empty() && !pvHandle->front().isFake()) {
    pv = pvHandle->front();
  } else if (bsHandle.isValid()) {
    pv = reco::Vertex(bsHandle->position(), bsHandle->covariance3D());
  } else {
    iEvent.put(std::move(output), "BToJpsiHadrons");
    return;
  }
  
  // Loop over J/psi candidates
  for (size_t jpsiIdx = 0; jpsiIdx < jpsiHandle->size(); ++jpsiIdx) {
    const auto& jpsi = (*jpsiHandle)[jpsiIdx];
    
    if (!passJpsiCuts(jpsi)) continue;
    
    // Select tracks for this J/psi (excluding overlap with J/psi daughters)
    std::vector<TrackInfo> tracks = selectTracks(trackHandle, jpsi);
    if (tracks.empty()) continue;
    
    // === 1-track modes ===
    
    // Mode 0: B+ -> J/psi K+
    if (doBplusToJpsiK_) {
      for (const auto& trk : tracks) {
        math::XYZTLorentzVector bP4 = jpsi.p4() + trk.p4_K;
        if (!passBCuts(bP4)) continue;
        if (std::abs(bP4.mass() - BDecay::bPlusMass) > bPlusMassWindow_) continue;
        
        auto cand = buildCandidate(jpsi, jpsiIdx, trk, nullptr, BDecay::BplusToJpsiK, pv);
        if (cand.mass() > 0) output->push_back(cand);
      }
    }
    
    // Mode 1: B+ -> J/psi pi+
    if (doBplusToJpsiPi_) {
      for (const auto& trk : tracks) {
        math::XYZTLorentzVector bP4 = jpsi.p4() + trk.p4_pi;
        if (!passBCuts(bP4)) continue;
        if (std::abs(bP4.mass() - BDecay::bPlusMass) > bPlusMassWindow_) continue;
        
        auto cand = buildCandidate(jpsi, jpsiIdx, trk, nullptr, BDecay::BplusToJpsiPi, pv);
        if (cand.mass() > 0) output->push_back(cand);
      }
    }
    
    // Mode 4: Bc+ -> J/psi pi+
    if (doBcToJpsiPi_) {
      for (const auto& trk : tracks) {
        math::XYZTLorentzVector bP4 = jpsi.p4() + trk.p4_pi;
        if (!passBCuts(bP4)) continue;
        if (std::abs(bP4.mass() - BDecay::bcMass) > bcMassWindow_) continue;
        
        auto cand = buildCandidate(jpsi, jpsiIdx, trk, nullptr, BDecay::BcToJpsiPi, pv);
        if (cand.mass() > 0) output->push_back(cand);
      }
    }
    
    // === 2-track modes ===
    
    for (size_t i = 0; i < tracks.size(); ++i) {
      for (size_t j = i + 1; j < tracks.size(); ++j) {
        const auto& trk1 = tracks[i];
        const auto& trk2 = tracks[j];
        
        // Skip if tracks overlap
        if (isOverlapping(trk1, trk2)) continue;
        
        // Mode 2: B0 -> J/psi K* (K pi), opposite sign K-pi
        if (doB0ToJpsiKPi_) {
          if (trk1.charge * trk2.charge < 0) {  // Opposite sign
            // Try both assignments: (K, pi) and (pi, K)
            for (int assign = 0; assign < 2; ++assign) {
              math::XYZTLorentzVector kP4 = (assign == 0) ? trk1.p4_K : trk2.p4_K;
              math::XYZTLorentzVector piP4 = (assign == 0) ? trk2.p4_pi : trk1.p4_pi;
              const TrackInfo& kTrk = (assign == 0) ? trk1 : trk2;
              const TrackInfo& piTrk = (assign == 0) ? trk2 : trk1;
              
              // K* mass cut
              double kstarMass = (kP4 + piP4).mass();
              if (std::abs(kstarMass - BDecay::kstarMass) > kstarMassWindow_) continue;
              
              // B0 candidate
              math::XYZTLorentzVector bP4 = jpsi.p4() + kP4 + piP4;
              if (!passBCuts(bP4)) continue;
              if (std::abs(bP4.mass() - BDecay::b0Mass) > b0MassWindow_) continue;
              
              auto cand = buildCandidate(jpsi, jpsiIdx, kTrk, &piTrk, BDecay::B0ToJpsiKPi, pv);
              if (cand.mass() > 0) {
                cand.addUserFloat("kstarMass", kstarMass);
                output->push_back(cand);
              }
            }
          }
        }
        
        // Mode 3: Bs -> J/psi phi (K K), opposite sign K-K
        if (doBsToJpsiKK_) {
          if (trk1.charge * trk2.charge < 0) {  // Opposite sign for phi -> K+K-
            math::XYZTLorentzVector kkP4 = trk1.p4_K + trk2.p4_K;
            double phiMass = kkP4.mass();
            if (std::abs(phiMass - BDecay::phiMass) > phiMassWindow_) continue;
            
            math::XYZTLorentzVector bP4 = jpsi.p4() + kkP4;
            if (!passBCuts(bP4)) continue;
            if (std::abs(bP4.mass() - BDecay::bsMass) > bsMassWindow_) continue;
            
            auto cand = buildCandidate(jpsi, jpsiIdx, trk1, &trk2, BDecay::BsToJpsiKK, pv);
            if (cand.mass() > 0) {
              cand.addUserFloat("phiMass", phiMass);
              output->push_back(cand);
            }
          }
        }
        
        // Mode 5: X -> J/psi pi pi
        if (doXToJpsiPiPi_) {
          if (trk1.charge * trk2.charge < 0) {  // Opposite sign pi+pi-
            math::XYZTLorentzVector pipiP4 = trk1.p4_pi + trk2.p4_pi;
            double pipiMass = pipiP4.mass();
            if (pipiMass < xPiPiMassMin_ || pipiMass > xPiPiMassMax_) continue;
            
            math::XYZTLorentzVector xP4 = jpsi.p4() + pipiP4;
            if (!passBCuts(xP4)) continue;
            if (xP4.mass() < xMassMin_ || xP4.mass() > xMassMax_) continue;
            
            auto cand = buildCandidate(jpsi, jpsiIdx, trk1, &trk2, BDecay::XToJpsiPiPi, pv);
            if (cand.mass() > 0) {
              cand.addUserFloat("pipiMass", pipiMass);
              output->push_back(cand);
            }
          }
        }
      }
    }
  }
  
  iEvent.put(std::move(output), "BToJpsiHadrons");
}

bool BToJpsiHadronsProducer::passTrackCuts(const pat::PackedCandidate& cand) const {
  if (!cand.hasTrackDetails()) return false;
  if (cand.charge() == 0) return false;
  if (cand.pt() < minTrackPt_) return false;
  if (std::abs(cand.eta()) > maxTrackEta_) return false;
  
  const auto& trk = cand.pseudoTrack();
  if (trk.numberOfValidHits() < minTrackNhits_) return false;
  if (requireHighPurity_ && !trk.quality(reco::TrackBase::highPurity)) return false;
  
  return true;
}

bool BToJpsiHadronsProducer::passJpsiCuts(const pat::CompositeCandidate& jpsi) const {
  if (jpsi.pt() < minJpsiPt_) return false;
  if (std::abs(jpsi.rapidity()) > maxJpsiAbsY_) return false;
  if (jpsi.mass() < jpsiMassMin_ || jpsi.mass() > jpsiMassMax_) return false;
  return true;
}

bool BToJpsiHadronsProducer::passBCuts(const math::XYZTLorentzVector& p4) const {
  if (p4.pt() < minBPt_) return false;
  double y = 0.5 * std::log((p4.E() + p4.pz()) / (p4.E() - p4.pz()));
  if (std::abs(y) > maxBAbsY_) return false;
  return true;
}

std::vector<BToJpsiHadronsProducer::TrackInfo> 
BToJpsiHadronsProducer::selectTracks(const edm::Handle<pat::PackedCandidateCollection>& tracks,
                                      const pat::CompositeCandidate& jpsi) {
  std::vector<TrackInfo> result;
  
  for (size_t i = 0; i < tracks->size(); ++i) {
    const auto& cand = (*tracks)[i];
    
    if (!passTrackCuts(cand)) continue;
    
    TrackInfo info;
    info.index = i;
    info.charge = cand.charge();
    info.track = &cand.pseudoTrack();
    info.valid = true;
    
    // Build 4-vectors with different mass hypotheses
    double px = cand.px();
    double py = cand.py();
    double pz = cand.pz();
    double p2 = px*px + py*py + pz*pz;
    
    double E_pi = std::sqrt(p2 + BDecay::piMass * BDecay::piMass);
    double E_K  = std::sqrt(p2 + BDecay::kMass * BDecay::kMass);
    
    info.p4_pi = math::XYZTLorentzVector(px, py, pz, E_pi);
    info.p4_K  = math::XYZTLorentzVector(px, py, pz, E_K);
    
    // Build transient track
    try {
      info.transTrack = ttBuilder_->build(cand.pseudoTrack());
    } catch (...) {
      info.valid = false;
    }
    
    // Check overlap with J/psi daughters
    TrackInfo tempInfo = info;  // Create copy to pass to isOverlapping
    if (isOverlapping(jpsi, tempInfo)) continue;
    
    if (info.valid) {
      result.push_back(info);
    }
  }
  
  return result;
}

TransientVertex BToJpsiHadronsProducer::fitVertex(const std::vector<reco::TransientTrack>& tracks) {
  KalmanVertexFitter fitter;
  TransientVertex vertex;
  
  try {
    vertex = fitter.vertex(tracks);
  } catch (...) {
    // Return invalid vertex on failure
  }
  
  return vertex;
}

void BToJpsiHadronsProducer::computeVertexQuantities(const TransientVertex& vertex,
                                                      const math::XYZTLorentzVector& p4,
                                                      const reco::Vertex& pv,
                                                      float& lxy, float& lxySig, float& cosAlpha,
                                                      float& l3D, float& l3DSig) const {
  lxy = lxySig = cosAlpha = l3D = l3DSig = -99.f;
  
  if (!vertex.isValid()) return;
  
  GlobalPoint vtxPos = vertex.position();
  
  // Displacement vector
  double dx = vtxPos.x() - pv.x();
  double dy = vtxPos.y() - pv.y();
  double dz = vtxPos.z() - pv.z();
  
  // Transverse decay length
  lxy = std::sqrt(dx*dx + dy*dy);
  
  // 3D decay length
  l3D = std::sqrt(dx*dx + dy*dy + dz*dz);
  
  // Error on decay length (simplified)
  GlobalError vtxErr = vertex.positionError();
  double lxyErr = std::sqrt(vtxErr.cxx() + vtxErr.cyy() + pv.xError()*pv.xError() + pv.yError()*pv.yError());
  double l3DErr = std::sqrt(vtxErr.cxx() + vtxErr.cyy() + vtxErr.czz() + 
                            pv.xError()*pv.xError() + pv.yError()*pv.yError() + pv.zError()*pv.zError());
  
  if (lxyErr > 0) lxySig = lxy / lxyErr;
  if (l3DErr > 0) l3DSig = l3D / l3DErr;
  
  // Pointing angle (cosine of angle between momentum and displacement)
  TVector3 pVec(p4.px(), p4.py(), p4.pz());
  TVector3 dVec(dx, dy, dz);
  
  if (pVec.Mag() > 0 && dVec.Mag() > 0) {
    cosAlpha = pVec.Dot(dVec) / (pVec.Mag() * dVec.Mag());
  }
}

pat::CompositeCandidate BToJpsiHadronsProducer::buildCandidate(
    const pat::CompositeCandidate& jpsi,
    int jpsiIdx,
    const TrackInfo& trk1,
    const TrackInfo* trk2,
    BDecay::DecayId decayId,
    const reco::Vertex& pv) {
  
  pat::CompositeCandidate cand;
  cand.setP4(reco::Candidate::LorentzVector(0, 0, 0, 0));  // Will be set below
  
  // Determine mass hypotheses based on decay mode
  math::XYZTLorentzVector trk1P4, trk2P4;
  double trk1Mass = 0, trk2Mass = 0;
  
  switch (decayId) {
    case BDecay::BplusToJpsiK:
      trk1P4 = trk1.p4_K;
      trk1Mass = BDecay::kMass;
      break;
    case BDecay::BplusToJpsiPi:
    case BDecay::BcToJpsiPi:
      trk1P4 = trk1.p4_pi;
      trk1Mass = BDecay::piMass;
      break;
    case BDecay::B0ToJpsiKPi:
      trk1P4 = trk1.p4_K;  // First track is K
      trk1Mass = BDecay::kMass;
      if (trk2) {
        trk2P4 = trk2->p4_pi;  // Second track is pi
        trk2Mass = BDecay::piMass;
      }
      break;
    case BDecay::BsToJpsiKK:
      trk1P4 = trk1.p4_K;
      trk1Mass = BDecay::kMass;
      if (trk2) {
        trk2P4 = trk2->p4_K;
        trk2Mass = BDecay::kMass;
      }
      break;
    case BDecay::XToJpsiPiPi:
      trk1P4 = trk1.p4_pi;
      trk1Mass = BDecay::piMass;
      if (trk2) {
        trk2P4 = trk2->p4_pi;
        trk2Mass = BDecay::piMass;
      }
      break;
    default:
      return cand;  // Return empty candidate for unknown mode
  }
  
  // Build B 4-momentum
  math::XYZTLorentzVector bP4 = jpsi.p4() + trk1P4;
  if (trk2) bP4 += trk2P4;
  
  // Vertex fit
  float vtxChi2 = -1, vtxNdof = -1, vtxProb = -1;
  float lxy = -99, lxySig = -99, cosAlpha = -99, l3D = -99, l3DSig = -99;
  float vtxX = -99, vtxY = -99, vtxZ = -99;
  
  if (doVertexFit_ && trk1.valid && (!trk2 || trk2->valid)) {
    // Get J/psi lepton tracks
    std::vector<reco::TransientTrack> fitTracks;
    
    // Add lepton tracks from J/psi
    for (size_t iDau = 0; iDau < jpsi.numberOfDaughters(); ++iDau) {
      const reco::Candidate* dau = jpsi.daughter(iDau);
      if (!dau) continue;
      
      const reco::Track* lepTrack = nullptr;
      
      // Try muon
      const pat::Muon* muon = dynamic_cast<const pat::Muon*>(dau);
      if (muon && muon->bestTrack()) {
        lepTrack = muon->bestTrack();
      }
      
      // Try electron
      if (!lepTrack) {
        const pat::Electron* electron = dynamic_cast<const pat::Electron*>(dau);
        if (electron && electron->gsfTrack().isNonnull()) {
          lepTrack = electron->gsfTrack().get();
        }
      }
      
      if (lepTrack) {
        try {
          fitTracks.push_back(ttBuilder_->build(*lepTrack));
        } catch (...) {
          // Skip this track
        }
      }
    }
    
    // Add hadron tracks
    fitTracks.push_back(trk1.transTrack);
    if (trk2) fitTracks.push_back(trk2->transTrack);
    
    // Perform vertex fit
    if (fitTracks.size() >= 3) {
      TransientVertex vertex = fitVertex(fitTracks);
      
      if (vertex.isValid()) {
        vtxChi2 = vertex.totalChiSquared();
        vtxNdof = vertex.degreesOfFreedom();
        vtxProb = TMath::Prob(vtxChi2, static_cast<int>(vtxNdof));
        
        GlobalPoint vtxPos = vertex.position();
        vtxX = vtxPos.x();
        vtxY = vtxPos.y();
        vtxZ = vtxPos.z();
        
        // Compute decay length and pointing angle
        computeVertexQuantities(vertex, bP4, pv, lxy, lxySig, cosAlpha, l3D, l3DSig);
        
        // Apply vertex probability cut
        if (vtxProb < minVtxProb_) {
          cand.setP4(reco::Candidate::LorentzVector(0, 0, 0, -1));  // Mark as failed
          return cand;
        }
      }
    }
  }
  
  // Set candidate 4-momentum
  cand.setP4(reco::Candidate::LorentzVector(bP4.px(), bP4.py(), bP4.pz(), bP4.E()));
  cand.setCharge(trk1.charge + (trk2 ? trk2->charge : 0));
  cand.setVertex(reco::Candidate::Point(vtxX, vtxY, vtxZ));
  
  // Store J/psi kinematics as userFloats (more reliable than daughter role lookup)
  cand.addUserFloat("jpsiMass", jpsi.mass());
  cand.addUserFloat("jpsiPt", jpsi.pt());
  cand.addUserFloat("jpsiEta", jpsi.eta());
  cand.addUserFloat("jpsiPhi", jpsi.phi());
  cand.addUserFloat("jpsiY", jpsi.rapidity());
  
  // Add user data
  cand.addUserInt("decayId", static_cast<int>(decayId));
  cand.addUserInt("nTracks", trk2 ? 2 : 1);
  cand.addUserInt("hasDaughter2", trk2 ? 1 : 0);
  cand.addUserInt("jpsiIdx", jpsiIdx);
  
  // Track 1 info (stored as userFloat/Int for reliability)
  cand.addUserFloat("trk1Pt", trk1P4.pt());
  cand.addUserFloat("trk1Eta", trk1P4.eta());
  cand.addUserFloat("trk1Phi", trk1P4.phi());
  cand.addUserFloat("trk1Mass", trk1Mass);
  cand.addUserInt("trk1Charge", trk1.charge);
  cand.addUserInt("trk1Idx", trk1.index);
  
  // Track 2 info
  if (trk2) {
    cand.addUserFloat("trk2Pt", trk2P4.pt());
    cand.addUserFloat("trk2Eta", trk2P4.eta());
    cand.addUserFloat("trk2Phi", trk2P4.phi());
    cand.addUserFloat("trk2Mass", trk2Mass);
    cand.addUserInt("trk2Charge", trk2->charge);
    cand.addUserInt("trk2Idx", trk2->index);
  } else {
    cand.addUserFloat("trk2Pt", -999.f);
    cand.addUserFloat("trk2Eta", -999.f);
    cand.addUserFloat("trk2Phi", -999.f);
    cand.addUserFloat("trk2Mass", -999.f);
    cand.addUserInt("trk2Charge", 0);
    cand.addUserInt("trk2Idx", -1);
  }
  
  // Vertex info
  cand.addUserFloat("vtxChi2", vtxChi2);
  cand.addUserFloat("vtxNdof", vtxNdof);
  cand.addUserFloat("vtxProb", vtxProb);
  cand.addUserFloat("vtxX", vtxX);
  cand.addUserFloat("vtxY", vtxY);
  cand.addUserFloat("vtxZ", vtxZ);
  cand.addUserFloat("lxy", lxy);
  cand.addUserFloat("lxySig", lxySig);
  cand.addUserFloat("l3D", l3D);
  cand.addUserFloat("l3DSig", l3DSig);
  cand.addUserFloat("cosAlpha", cosAlpha);
  
  return cand;
}

bool BToJpsiHadronsProducer::isOverlapping(const pat::CompositeCandidate& jpsi, const TrackInfo& trk) const {
  for (size_t i = 0; i < jpsi.numberOfDaughters(); ++i) {
    const reco::Candidate* dau = jpsi.daughter(i);
    if (!dau) continue;
    
    double dR = reco::deltaR(dau->eta(), dau->phi(), trk.p4_pi.eta(), trk.p4_pi.phi());
    if (dR < maxDeltaR_overlap_) return true;
  }
  return false;
}

bool BToJpsiHadronsProducer::isOverlapping(const TrackInfo& trk1, const TrackInfo& trk2) const {
  if (trk1.index == trk2.index) return true;
  double dR = reco::deltaR(trk1.p4_pi.eta(), trk1.p4_pi.phi(), trk2.p4_pi.eta(), trk2.p4_pi.phi());
  return dR < maxDeltaR_overlap_;
}

DEFINE_FWK_MODULE(BToJpsiHadronsProducer);
