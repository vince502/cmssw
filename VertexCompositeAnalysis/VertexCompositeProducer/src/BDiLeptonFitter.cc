// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiLeptonFitter
// 
/**\class BDiLeptonFitter BDiLeptonFitter.cc VertexCompositeAnalysis/VertexCompositeProducer/src/BDiLeptonFitter.cc

 Description: B and Bc meson reconstruction from dileptons (dimuon or dielectron) and additional tracks
              Unified fitter supporting both HiOnia2MuMuPAT and HiOnia2EEPAT inputs

 Implementation:
     Reconstructs B+ -> J/psi K+, B0 -> J/psi K*0, and Bc+ -> J/psi pi+ decays
     Automatically detects lepton channel from dilepton candidate structure
*/

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiLeptonFitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"

#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"

#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TVector3.h>

using namespace std;
using namespace reco;
using namespace edm;

BDiLeptonFitter::BDiLeptonFitter(const edm::ParameterSet& theParameters, edm::ConsumesCollector && iC) :
    bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>())
{
  // Input collections
  token_dileptons = iC.consumes<pat::CompositeCandidateCollection>(theParameters.getParameter<edm::InputTag>("dileptonCollection"));
  token_tracks = iC.consumes<reco::TrackCollection>(theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  token_vertices = iC.consumes<reco::VertexCollection>(theParameters.getParameter<edm::InputTag>("vertexRecoAlgorithm"));
  token_beamSpot = iC.consumes<reco::BeamSpot>(edm::InputTag("offlineBeamSpot"));
  token_dedx = iC.consumes<edm::ValueMap<reco::DeDxData>>(edm::InputTag("dedxHarmonic2"));

  // Track selection cuts
  tkChi2Cut = theParameters.getParameter<double>("tkChi2Cut");
  tkNhitsCut = theParameters.getParameter<int>("tkNhitsCut");
  tkPtErrCut = theParameters.getParameter<double>("tkPtErrCut");
  tkPtCut = theParameters.getParameter<double>("tkPtCut");
  tkEtaCut = theParameters.getParameter<double>("tkEtaCut");
  tkDCACut = theParameters.getParameter<double>("tkDCACut");
  tkDCACutLow = theParameters.getParameter<double>("tkDCACutLow");

  // Dilepton selection cuts
  dileptonMassMin = theParameters.getParameter<double>("dileptonMassMin");
  dileptonMassMax = theParameters.getParameter<double>("dileptonMassMax");
  dileptonPtCut = theParameters.getParameter<double>("dileptonPtCut");
  dileptonYCut = theParameters.getParameter<double>("dileptonYCut");

  // B meson selection cuts
  bPlusMassCut = theParameters.getParameter<double>("bPlusMassCut");
  bZeroMassCut = theParameters.getParameter<double>("bZeroMassCut");
  bPtCut = theParameters.getParameter<double>("bPtCut");
  bYCut = theParameters.getParameter<double>("bYCut");

  // Bc meson selection cuts
  bcMassCut = theParameters.getParameter<double>("bcMassCut");
  bcPtCut = theParameters.getParameter<double>("bcPtCut");
  bcYCut = theParameters.getParameter<double>("bcYCut");

  // Vertex quality cuts
  vtxChi2Cut = theParameters.getParameter<double>("vtxChi2Cut");
  vtxProbCut = theParameters.getParameter<double>("vtxProbCut");
  rVtxCut = theParameters.getParameter<double>("rVtxCut");
  rVtxSigCut = theParameters.getParameter<double>("rVtxSigCut");
  lVtxCut = theParameters.getParameter<double>("lVtxCut");
  lVtxSigCut = theParameters.getParameter<double>("lVtxSigCut");
  collinCut2D = theParameters.getParameter<double>("collinCut2D");
  collinCut3D = theParameters.getParameter<double>("collinCut3D");
  alphaCut = theParameters.getParameter<double>("alphaCut");
  alpha2DCut = theParameters.getParameter<double>("alpha2DCut");

  // Impact parameter cuts
  dauTransImpactSigCut = theParameters.getParameter<double>("dauTransImpactSigCut");
  dauLongImpactSigCut = theParameters.getParameter<double>("dauLongImpactSigCut");

  // K*0 reconstruction cuts
  kstarMassMin = theParameters.getParameter<double>("kstarMassMin");
  kstarMassMax = theParameters.getParameter<double>("kstarMassMax");
  kstarPtCut = theParameters.getParameter<double>("kstarPtCut");

  // Decay mode flags
  isWrongSign = theParameters.getParameter<bool>("isWrongSign");
  doJPsi = theParameters.getParameter<bool>("doJPsi");
  doPsi2S = theParameters.getParameter<bool>("doPsi2S");
  doUpsilon = theParameters.getParameter<bool>("doUpsilon");
  doBPlus = theParameters.getParameter<bool>("doBPlus");
  doBZero = theParameters.getParameter<bool>("doBZero");
  doBc = theParameters.getParameter<bool>("doBc");

  // Track quality requirements
  std::vector<std::string> qual = theParameters.getParameter<std::vector<std::string>>("trackQualities");
  for (unsigned int ndx = 0; ndx < qual.size(); ndx++) {
    qualities.push_back(reco::TrackBase::qualityByName(qual[ndx]));
  }
}

BDiLeptonFitter::~BDiLeptonFitter() {
}

BDiLeptonFitter::LeptonChannel BDiLeptonFitter::detectLeptonChannel(const pat::CompositeCandidate& dilepton) {
  // Check for muon channel - iterate over daughters to find muons
  int nMuons = 0;
  int nElectrons = 0;
  
  for (size_t i = 0; i < dilepton.numberOfDaughters(); i++) {
    const reco::Candidate* dau = dilepton.daughter(i);
    if (!dau) continue;
    
    if (dynamic_cast<const pat::Muon*>(dau) != nullptr) {
      nMuons++;
    } else if (dynamic_cast<const pat::Electron*>(dau) != nullptr) {
      nElectrons++;
    }
  }
  
  if (nMuons >= 2) {
    return LeptonChannel::Muon;
  }
  
  if (nElectrons >= 2) {
    return LeptonChannel::Electron;
  }
  
  return LeptonChannel::Unknown;
}

double BDiLeptonFitter::getLeptonMass(LeptonChannel channel) {
  switch (channel) {
    case LeptonChannel::Muon: return muonMass;
    case LeptonChannel::Electron: return electronMass;
    default: return muonMass;
  }
}

bool BDiLeptonFitter::getLeptonTracks(const pat::CompositeCandidate& dilepton, 
                                      LeptonChannel channel,
                                      const reco::Track*& trk1, 
                                      const reco::Track*& trk2) {
  trk1 = nullptr;
  trk2 = nullptr;
  
  if (channel == LeptonChannel::Muon) {
    // Find muons by iterating over daughters
    const pat::Muon* mu1 = nullptr;
    const pat::Muon* mu2 = nullptr;
    for (size_t i = 0; i < dilepton.numberOfDaughters(); i++) {
      const pat::Muon* mu = dynamic_cast<const pat::Muon*>(dilepton.daughter(i));
      if (mu) {
        if (!mu1) mu1 = mu;
        else if (!mu2) mu2 = mu;
      }
    }
    if (mu1 && mu2 && mu1->bestTrack() && mu2->bestTrack()) {
      trk1 = mu1->bestTrack();
      trk2 = mu2->bestTrack();
      return true;
    }
  } else if (channel == LeptonChannel::Electron) {
    // Find electrons by iterating over daughters
    const pat::Electron* e1 = nullptr;
    const pat::Electron* e2 = nullptr;
    for (size_t i = 0; i < dilepton.numberOfDaughters(); i++) {
      const pat::Electron* e = dynamic_cast<const pat::Electron*>(dilepton.daughter(i));
      if (e) {
        if (!e1) e1 = e;
        else if (!e2) e2 = e;
      }
    }
    if (e1 && e2 && e1->gsfTrack().isNonnull() && e2->gsfTrack().isNonnull()) {
      trk1 = e1->gsfTrack().get();
      trk2 = e2->gsfTrack().get();
      return true;
    }
  }
  
  return false;
}

void BDiLeptonFitter::fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3>> SMatrixSym3D;
  typedef ROOT::Math::SVector<double, 3> SVector3;

  Handle<pat::CompositeCandidateCollection> dileptonHandle;
  Handle<reco::TrackCollection> trackHandle;
  Handle<reco::VertexCollection> vertexHandle;
  Handle<reco::BeamSpot> beamSpotHandle;
  Handle<edm::ValueMap<reco::DeDxData>> dEdxHandle;
  ESHandle<MagneticField> bFieldHandle;

  iEvent.getByToken(token_dileptons, dileptonHandle);
  iEvent.getByToken(token_tracks, trackHandle);
  iEvent.getByToken(token_vertices, vertexHandle);
  iEvent.getByToken(token_beamSpot, beamSpotHandle);
  iEvent.getByToken(token_dedx, dEdxHandle);
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  if (!dileptonHandle.isValid() || !trackHandle.isValid() || !trackHandle->size()) {
    return;
  }
  
  magField = bFieldHandle.product();

  double xVtx = -99999.0, yVtx = -99999.0, zVtx = -99999.0;
  double xVtxError = -999.0, yVtxError = -999.0, zVtxError = -999.0;
  
  const reco::VertexCollection& vtxCollection = *vertexHandle;
  auto vtxPrimary = vtxCollection.begin();
  
  if (vtxCollection.size() > 0 && !vtxPrimary->isFake() && vtxPrimary->tracksSize() >= 2) {
    xVtx = vtxPrimary->x();
    yVtx = vtxPrimary->y();
    zVtx = vtxPrimary->z();
    xVtxError = vtxPrimary->xError();
    yVtxError = vtxPrimary->yError();
    zVtxError = vtxPrimary->zError();
  } else {
    xVtx = beamSpotHandle->position().x();
    yVtx = beamSpotHandle->position().y();
    zVtx = 0.0;
    xVtxError = beamSpotHandle->BeamWidthX();
    yVtxError = beamSpotHandle->BeamWidthY();
    zVtxError = 0.0;
  }
  
  math::XYZPoint bestvtx(xVtx, yVtx, zVtx);

  // Preselect tracks
  std::vector<reco::TrackRef> selectedTracks;
  std::vector<reco::TransientTrack> transientTracks;
  
  for (unsigned int indx = 0; indx < trackHandle->size(); indx++) {
    reco::TrackRef trackRef(trackHandle, indx);
    
    if (!passTrackCuts(trackRef, bestvtx, xVtxError, yVtxError, zVtxError)) continue;
    
    bool quality_ok = true;
    if (qualities.size() != 0) {
      quality_ok = false;
      for (unsigned int ndx_ = 0; ndx_ < qualities.size(); ndx_++) {
        if (trackRef->quality(qualities[ndx_])) {
          quality_ok = true;
          break;
        }
      }
    }
    if (!quality_ok) continue;
    
    selectedTracks.push_back(trackRef);
    reco::TransientTrack transTrack(*trackRef, magField);
    transientTracks.push_back(transTrack);
  }

  // Loop over dilepton candidates with index tracking
  for (size_t dileptonIdx = 0; dileptonIdx < dileptonHandle->size(); dileptonIdx++) {
    const auto& dilepton = (*dileptonHandle)[dileptonIdx];
    
    if (!passDileptonCuts(dilepton)) continue;
    
    LeptonChannel channel = detectLeptonChannel(dilepton);
    if (channel == LeptonChannel::Unknown) {
      continue;
    }
    
    const reco::Track* lepTrk1 = nullptr;
    const reco::Track* lepTrk2 = nullptr;
    if (!getLeptonTracks(dilepton, channel, lepTrk1, lepTrk2)) {
      continue;
    }
    
    double leptonMass = getLeptonMass(channel);
    auto lepTTrk1 = reco::TransientTrack(*lepTrk1, magField);
    auto lepTTrk2 = reco::TransientTrack(*lepTrk2, magField);

    // B+ reconstruction: dilepton + single track (kaon)
    if (doBPlus) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        const auto& kaonTrack = selectedTracks[i];
        
        // Skip if track is same as lepton track (avoid self-fitting)
        if (isSameTrack(lepTrk1, &(*kaonTrack)) || isSameTrack(lepTrk2, &(*kaonTrack))) continue;

        std::vector<reco::TransientTrack> bTracks;
        std::vector<double> bMasses;
 
        bTracks.push_back(lepTTrk1);
        bTracks.push_back(lepTTrk2);
        bMasses.push_back(leptonMass);
        bMasses.push_back(leptonMass);

        bTracks.push_back(transientTracks[i]);
        bMasses.push_back(kaonMass);

        TransientVertex bVertex = fitBVertex(bTracks, bMasses);
        if (!bVertex.isValid()) continue;

        double vtxChi2 = bVertex.totalChiSquared();
        double vtxNdof = bVertex.degreesOfFreedom();
        double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
        
        if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;

        pat::CompositeCandidate bPlus = createBPlus(dilepton, kaonTrack, *vtxPrimary, bVertex, channel);
        
        // Store the dilepton index for matching with hionia tree
        bPlus.addUserInt("jpsiIdx", static_cast<int>(dileptonIdx));
        
        if (fabs(bPlus.mass() - bPlusMass) > bPlusMassCut) continue;
        if (bPlus.pt() < bPtCut) continue;
        if (fabs(bPlus.rapidity()) > bYCut) continue;

        theBPlus.push_back(bPlus);
      }
    }

    // Bc+ reconstruction: dilepton + single track (pion)
    if (doBc) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        const auto& pionTrack = selectedTracks[i];
        
        // Skip if track is same as lepton track (avoid self-fitting)
        if (isSameTrack(lepTrk1, &(*pionTrack)) || isSameTrack(lepTrk2, &(*pionTrack))) continue;

        std::vector<reco::TransientTrack> bcTracks;
        std::vector<double> bcMasses;
        
        bcTracks.push_back(lepTTrk1);
        bcTracks.push_back(lepTTrk2);
        bcMasses.push_back(leptonMass);
        bcMasses.push_back(leptonMass);
        
        bcTracks.push_back(transientTracks[i]);
        bcMasses.push_back(pionMass);

        TransientVertex bcVertex = fitBVertex(bcTracks, bcMasses);
        if (!bcVertex.isValid()) continue;

        double vtxChi2 = bcVertex.totalChiSquared();
        double vtxNdof = bcVertex.degreesOfFreedom();
        double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
        
        if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;

        pat::CompositeCandidate bc = createBc(dilepton, pionTrack, *vtxPrimary, bcVertex, channel);
        
        // Store the dilepton index for matching with hionia tree
        bc.addUserInt("jpsiIdx", static_cast<int>(dileptonIdx));
        
        if (fabs(bc.mass() - bcMass) > bcMassCut) continue;
        if (bc.pt() < bcPtCut) continue;
        if (fabs(bc.rapidity()) > bcYCut) continue;

        theBc.push_back(bc);
      }
    }

    // B0 reconstruction: dilepton + two tracks (K*0 -> K+ pi-)
    if (doBZero) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        for (size_t j = i + 1; j < selectedTracks.size(); j++) {
          const auto& track1 = selectedTracks[i];
          const auto& track2 = selectedTracks[j];
          
          if (!isWrongSign) {
            if (track1->charge() * track2->charge() > 0) continue;
          }
          
          // Skip if any track is same as lepton track (avoid self-fitting)
          if (isSameTrack(lepTrk1, &(*track1)) || isSameTrack(lepTrk2, &(*track1)) ||
              isSameTrack(lepTrk1, &(*track2)) || isSameTrack(lepTrk2, &(*track2))) continue;

          math::PtEtaPhiMLorentzVector kaonP4, pionP4;
          reco::TrackRef kaonTrack, pionTrack;
          
          if (track1->charge() > 0) {
            kaonTrack = track1;
            pionTrack = track2;
          } else {
            kaonTrack = track2;
            pionTrack = track1;
          }
          
          kaonP4.SetPt(kaonTrack->pt());
          kaonP4.SetEta(kaonTrack->eta());
          kaonP4.SetPhi(kaonTrack->phi());
          kaonP4.SetM(kaonMass);
          
          pionP4.SetPt(pionTrack->pt());
          pionP4.SetEta(pionTrack->eta());
          pionP4.SetPhi(pionTrack->phi());
          pionP4.SetM(pionMass);
          
          auto kstarP4 = kaonP4 + pionP4;
          
          if (kstarP4.mass() < kstarMassMin || kstarP4.mass() > kstarMassMax) continue;
          if (kstarP4.pt() < kstarPtCut) continue;

          std::vector<reco::TransientTrack> bTracks;
          std::vector<double> bMasses;
          
          bTracks.push_back(lepTTrk1);
          bTracks.push_back(lepTTrk2);
          bMasses.push_back(leptonMass);
          bMasses.push_back(leptonMass);
          
          bTracks.push_back(transientTracks[i]);
          bTracks.push_back(transientTracks[j]);
          bMasses.push_back(track1->charge() > 0 ? kaonMass : pionMass);
          bMasses.push_back(track1->charge() > 0 ? pionMass : kaonMass);

          TransientVertex bVertex = fitBVertex(bTracks, bMasses);
          if (!bVertex.isValid()) continue;

          double vtxChi2 = bVertex.totalChiSquared();
          double vtxNdof = bVertex.degreesOfFreedom();
          double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
          
          if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;

          pat::CompositeCandidate bZero = createBZero(dilepton, kaonTrack, pionTrack, *vtxPrimary, bVertex, channel);
          
          // Store the dilepton index for matching with hionia tree
          bZero.addUserInt("jpsiIdx", static_cast<int>(dileptonIdx));
          
          if (fabs(bZero.mass() - bZeroMass) > bZeroMassCut) continue;
          if (bZero.pt() < bPtCut) continue;
          if (fabs(bZero.rapidity()) > bYCut) continue;

          theBZero.push_back(bZero);
        }
      }
    }
  }
}

bool BDiLeptonFitter::passTrackCuts(const reco::TrackRef& track, const math::XYZPoint& bestvtx,
                                 double xVtxError, double yVtxError, double zVtxError) {
  if (track->normalizedChi2() >= tkChi2Cut) return false;
  if (track->numberOfValidHits() < tkNhitsCut) return false;
  if (track->ptError() / track->pt() >= tkPtErrCut) return false;
  if (track->pt() <= tkPtCut) return false;
  if (fabs(track->eta()) >= tkEtaCut) return false;

  double dzvtx = track->dz(bestvtx);
  double dxyvtx = track->dxy(bestvtx);
  double dzerror = sqrt(track->dzError()*track->dzError() + zVtxError*zVtxError);
  double dxyerror = sqrt(track->d0Error()*track->d0Error() + xVtxError*yVtxError);

  double dauLongImpactSig = dzvtx/dzerror;
  double dauTransImpactSig = dxyvtx/dxyerror;

  if (fabs(dauTransImpactSig) <= dauTransImpactSigCut) return false;
  if (fabs(dauLongImpactSig) <= dauLongImpactSigCut) return false;

  return true;
}

bool BDiLeptonFitter::passDileptonCuts(const pat::CompositeCandidate& dilepton) {
  if (dilepton.mass() < dileptonMassMin || dilepton.mass() > dileptonMassMax) return false;
  if (dilepton.pt() < dileptonPtCut) return false;
  if (fabs(dilepton.rapidity()) > dileptonYCut) return false;
  return true;
}

pat::CompositeCandidate BDiLeptonFitter::createBPlus(const pat::CompositeCandidate& dilepton,
                                                  const reco::TrackRef& kaonTrack,
                                                  const reco::Vertex& vertex,
                                                  const TransientVertex& bVertex,
                                                  LeptonChannel channel) {
  pat::CompositeCandidate bPlus;
  
  bPlus.addDaughter(dilepton, "dilepton");
  
  math::PtEtaPhiMLorentzVector kaonP4;
  kaonP4.SetPt(kaonTrack->pt());
  kaonP4.SetEta(kaonTrack->eta());
  kaonP4.SetPhi(kaonTrack->phi());
  kaonP4.SetM(kaonMass);
  
  reco::Candidate::LorentzVector kaonP4_converted(kaonP4.Px(), kaonP4.Py(), kaonP4.Pz(), kaonP4.E());
  reco::Candidate::Point vtxPos(bVertex.position().x(), bVertex.position().y(), bVertex.position().z());
  
  reco::RecoChargedCandidate kaonCand(kaonTrack->charge(), kaonP4_converted, vtxPos);
  kaonCand.setTrack(kaonTrack);
  bPlus.addDaughter(kaonCand, "kaon");
  
  AddFourMomenta addP4;
  addP4.set(bPlus);
  
  bPlus.addUserData("vertex", reco::Vertex(bVertex));
  bPlus.addUserFloat("vtxChi2", bVertex.totalChiSquared());
  bPlus.addUserFloat("vtxNdof", bVertex.degreesOfFreedom());
  bPlus.addUserFloat("vtxProb", TMath::Prob(bVertex.totalChiSquared(), bVertex.degreesOfFreedom()));
  
  bPlus.addUserInt("leptonChannel", static_cast<int>(channel));
  
  GlobalPoint bVtxPos = bVertex.position();
  reco::Vertex::Point primVtxPos = vertex.position();
  math::XYZVector displacement(bVtxPos.x() - primVtxPos.x(), 
                              bVtxPos.y() - primVtxPos.y(), 
                              bVtxPos.z() - primVtxPos.z());
  double pointingAngle2D = calculatePointingAngle(math::XYZVector(bPlus.px(), bPlus.py(), 0),
                                                 math::XYZVector(displacement.x(), displacement.y(), 0));
  double pointingAngle3D = calculatePointingAngle(math::XYZVector(bPlus.px(), bPlus.py(), bPlus.pz()),
                                                 displacement);
  
  bPlus.addUserFloat("pointingAngle2D", pointingAngle2D);
  bPlus.addUserFloat("pointingAngle3D", pointingAngle3D);
  bPlus.addUserFloat("decayLength2D", displacement.Rho());
  bPlus.addUserFloat("decayLength3D", displacement.R());
  
  return bPlus;
}

pat::CompositeCandidate BDiLeptonFitter::createBZero(const pat::CompositeCandidate& dilepton,
                                                  const reco::TrackRef& kaonTrack,
                                                  const reco::TrackRef& pionTrack,
                                                  const reco::Vertex& vertex,
                                                  const TransientVertex& bVertex,
                                                  LeptonChannel channel) {
  pat::CompositeCandidate bZero;
  
  bZero.addDaughter(dilepton, "dilepton");
  
  pat::CompositeCandidate kstar;
  
  math::PtEtaPhiMLorentzVector kaonP4;
  kaonP4.SetPt(kaonTrack->pt());
  kaonP4.SetEta(kaonTrack->eta());
  kaonP4.SetPhi(kaonTrack->phi());
  kaonP4.SetM(kaonMass);
  
  reco::Candidate::LorentzVector kaonP4_converted(kaonP4.Px(), kaonP4.Py(), kaonP4.Pz(), kaonP4.E());
  reco::Candidate::Point vtxPos(bVertex.position().x(), bVertex.position().y(), bVertex.position().z());
  
  reco::RecoChargedCandidate kaonCand(kaonTrack->charge(), kaonP4_converted, vtxPos);
  kaonCand.setTrack(kaonTrack);
  kstar.addDaughter(kaonCand, "kaon");
  
  math::PtEtaPhiMLorentzVector pionP4;
  pionP4.SetPt(pionTrack->pt());
  pionP4.SetEta(pionTrack->eta());
  pionP4.SetPhi(pionTrack->phi());
  pionP4.SetM(pionMass);
  
  reco::Candidate::LorentzVector pionP4_converted(pionP4.Px(), pionP4.Py(), pionP4.Pz(), pionP4.E());
  
  reco::RecoChargedCandidate pionCand(pionTrack->charge(), pionP4_converted, vtxPos);
  pionCand.setTrack(pionTrack);
  kstar.addDaughter(pionCand, "pion");
  
  AddFourMomenta addP4;
  addP4.set(kstar);
  kstar.addUserFloat("mass", kstar.mass());
  
  bZero.addDaughter(kstar, "kstar");
  
  addP4.set(bZero);
  
  bZero.addUserData("vertex", reco::Vertex(bVertex));
  bZero.addUserFloat("vtxChi2", bVertex.totalChiSquared());
  bZero.addUserFloat("vtxNdof", bVertex.degreesOfFreedom());
  bZero.addUserFloat("vtxProb", TMath::Prob(bVertex.totalChiSquared(), bVertex.degreesOfFreedom()));
  
  bZero.addUserInt("leptonChannel", static_cast<int>(channel));
  
  GlobalPoint bVtxPos = bVertex.position();
  reco::Vertex::Point primVtxPos = vertex.position();
  math::XYZVector displacement(bVtxPos.x() - primVtxPos.x(), 
                              bVtxPos.y() - primVtxPos.y(), 
                              bVtxPos.z() - primVtxPos.z());
  double pointingAngle2D = calculatePointingAngle(math::XYZVector(bZero.px(), bZero.py(), 0),
                                                 math::XYZVector(displacement.x(), displacement.y(), 0));
  double pointingAngle3D = calculatePointingAngle(math::XYZVector(bZero.px(), bZero.py(), bZero.pz()),
                                                 displacement);
  
  bZero.addUserFloat("pointingAngle2D", pointingAngle2D);
  bZero.addUserFloat("pointingAngle3D", pointingAngle3D);
  bZero.addUserFloat("decayLength2D", displacement.Rho());
  bZero.addUserFloat("decayLength3D", displacement.R());
  
  return bZero;
}

pat::CompositeCandidate BDiLeptonFitter::createBc(const pat::CompositeCandidate& dilepton,
                                                const reco::TrackRef& pionTrack,
                                                const reco::Vertex& vertex,
                                                const TransientVertex& bcVertex,
                                                LeptonChannel channel) {
  pat::CompositeCandidate bc;
  
  bc.addDaughter(dilepton, "dilepton");
  
  math::PtEtaPhiMLorentzVector pionP4;
  pionP4.SetPt(pionTrack->pt());
  pionP4.SetEta(pionTrack->eta());
  pionP4.SetPhi(pionTrack->phi());
  pionP4.SetM(pionMass);
  
  reco::Candidate::LorentzVector pionP4_converted(pionP4.Px(), pionP4.Py(), pionP4.Pz(), pionP4.E());
  reco::Candidate::Point vtxPos(bcVertex.position().x(), bcVertex.position().y(), bcVertex.position().z());
  
  reco::RecoChargedCandidate pionCand(pionTrack->charge(), pionP4_converted, vtxPos);
  pionCand.setTrack(pionTrack);
  bc.addDaughter(pionCand, "pion");
  
  AddFourMomenta addP4;
  addP4.set(bc);
  
  bc.addUserData("vertex", reco::Vertex(bcVertex));
  bc.addUserFloat("vtxChi2", bcVertex.totalChiSquared());
  bc.addUserFloat("vtxNdof", bcVertex.degreesOfFreedom());
  bc.addUserFloat("vtxProb", TMath::Prob(bcVertex.totalChiSquared(), bcVertex.degreesOfFreedom()));
  
  bc.addUserInt("leptonChannel", static_cast<int>(channel));
  
  GlobalPoint bcVtxPos = bcVertex.position();
  reco::Vertex::Point primVtxPos = vertex.position();
  math::XYZVector displacement(bcVtxPos.x() - primVtxPos.x(), 
                              bcVtxPos.y() - primVtxPos.y(), 
                              bcVtxPos.z() - primVtxPos.z());
  double pointingAngle2D = calculatePointingAngle(math::XYZVector(bc.px(), bc.py(), 0),
                                                 math::XYZVector(displacement.x(), displacement.y(), 0));
  double pointingAngle3D = calculatePointingAngle(math::XYZVector(bc.px(), bc.py(), bc.pz()),
                                                 displacement);
  
  bc.addUserFloat("pointingAngle2D", pointingAngle2D);
  bc.addUserFloat("pointingAngle3D", pointingAngle3D);
  bc.addUserFloat("decayLength2D", displacement.Rho());
  bc.addUserFloat("decayLength3D", displacement.R());
  
  return bc;
}

TransientVertex BDiLeptonFitter::fitBVertex(const std::vector<reco::TransientTrack>& tracks,
                                         const std::vector<double>& masses) {
  KalmanVertexFitter fitter;
  return fitter.vertex(tracks);
}

double BDiLeptonFitter::calculatePointingAngle(const math::XYZVector& momentum,
                                            const math::XYZVector& displacement) {
  return acos(momentum.Dot(displacement) / (momentum.R() * displacement.R()));
}

bool BDiLeptonFitter::isSameTrack(const reco::Track* trk1, const reco::Track* trk2, double tolerance) {
  // Compare tracks by their parameters to avoid self-fitting
  // This is more robust than pointer comparison since lepton tracks come from different collections
  if (!trk1 || !trk2) return false;
  
  // Check if tracks have same charge
  if (trk1->charge() != trk2->charge()) return false;
  
  // Compare kinematic parameters with tolerance
  if (fabs(trk1->pt() - trk2->pt()) > tolerance * trk1->pt()) return false;
  if (fabs(trk1->eta() - trk2->eta()) > tolerance) return false;
  
  // Compare phi with wrapping
  double dphi = trk1->phi() - trk2->phi();
  while (dphi > M_PI) dphi -= 2*M_PI;
  while (dphi < -M_PI) dphi += 2*M_PI;
  if (fabs(dphi) > tolerance) return false;
  
  return true;
}

const pat::CompositeCandidateCollection& BDiLeptonFitter::getBPlus() const {
  return theBPlus;
}

const pat::CompositeCandidateCollection& BDiLeptonFitter::getBZero() const {
  return theBZero;
}

const pat::CompositeCandidateCollection& BDiLeptonFitter::getBc() const {
  return theBc;
}

const std::vector<float>& BDiLeptonFitter::getMVAVals() const {
  return mvaVals_;
}

void BDiLeptonFitter::resetAll() {
  theBPlus.clear();
  theBZero.clear();
  theBc.clear();
  mvaVals_.clear();
}
