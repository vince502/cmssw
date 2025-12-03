// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiMuMuFitter
// 
/**\class BDiMuMuFitter BDiMuMuFitter.cc VertexCompositeAnalysis/VertexCompositeProducer/src/BDiMuMuFitter.cc

 Description: B and Bc meson reconstruction from dimuons and additional tracks

 Implementation:
     Reconstructs B+ -> J/psi K+, B0 -> J/psi K*0, and Bc+ -> J/psi pi+ decays
*/

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiMuMuFitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"

#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TVector3.h>
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

using namespace std;
using namespace reco;
using namespace edm;

#define DEBUG_BFIT false

// Constructor
BDiMuMuFitter::BDiMuMuFitter(const edm::ParameterSet& theParameters, edm::ConsumesCollector && iC) :
    bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>())
{
  using std::string;

  // Input collections
  token_dimuons = iC.consumes<pat::CompositeCandidateCollection>(theParameters.getParameter<edm::InputTag>("dimuonCollection"));
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

  // Dimuon selection cuts
  dimuonMassMin = theParameters.getParameter<double>("dimuonMassMin");
  dimuonMassMax = theParameters.getParameter<double>("dimuonMassMax");
  dimuonPtCut = theParameters.getParameter<double>("dimuonPtCut");
  dimuonYCut = theParameters.getParameter<double>("dimuonYCut");

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

BDiMuMuFitter::~BDiMuMuFitter() {
}

// Main fitting algorithm
void BDiMuMuFitter::fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

#if DEBUG_BFIT == true
  using std::cout;
  using std::endl;
#endif
  
  typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3>> SMatrixSym3D;
  typedef ROOT::Math::SVector<double, 3> SVector3;

  // Get input collections
  Handle<pat::CompositeCandidateCollection> dimuonHandle;
  Handle<reco::TrackCollection> trackHandle;
  Handle<reco::VertexCollection> vertexHandle;
  Handle<reco::BeamSpot> beamSpotHandle;
  Handle<edm::ValueMap<reco::DeDxData>> dEdxHandle;
  ESHandle<MagneticField> bFieldHandle;

  iEvent.getByToken(token_dimuons, dimuonHandle);
  iEvent.getByToken(token_tracks, trackHandle);
  iEvent.getByToken(token_vertices, vertexHandle);
  iEvent.getByToken(token_beamSpot, beamSpotHandle);
  iEvent.getByToken(token_dedx, dEdxHandle);
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  if (!dimuonHandle.isValid() || !trackHandle.isValid() || !trackHandle->size()) {
#if DEBUG_BFIT == true
    cout << "Some handles are missing!" << endl;
    if(!dimuonHandle.isValid()) cout << "Dimuon!" << endl;
    if(!trackHandle.isValid()) cout << "Track!" << endl;
#endif
    return;
  }
  
  magField = bFieldHandle.product();

  // Primary vertex information
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
  
#if DEBUG_BFIT == true
  cout << "Looping over track handle" << endl;
#endif
  for (unsigned int indx = 0; indx < trackHandle->size(); indx++) {
    reco::TrackRef trackRef(trackHandle, indx);
    
    if (!passTrackCuts(trackRef, bestvtx, xVtxError, yVtxError, zVtxError)) continue;
    
    // Check track quality
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

  // Loop over dimuon candidates
  for (const auto& dimuon : *dimuonHandle) {
    if (!passDimuonCuts(dimuon)) continue;
    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(dimuon.daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(dimuon.daughter("muon2"));
    const reco::Track& muonTrk1 = *muon1->bestTrack();
    const reco::Track& muonTrk2 = *muon2->bestTrack();
    auto muonTTrk1 = reco::TransientTrack(muonTrk1, magField);
    auto muonTTrk2 = reco::TransientTrack(muonTrk2, magField);

    // B+ reconstruction: dimuon + single track (kaon)
    if (doBPlus) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        const auto& kaonTrack = selectedTracks[i];
        
        // Skip tracks that are daughters of the dimuon
        bool skipTrack = false;
        skipTrack = (
                      &muonTrk1 == &(*kaonTrack) || 
                      &muonTrk2 == &(*kaonTrack)
                    );

//        for (size_t j = 0; j < dimuon.numberOfDaughters(); j++) {
//          const auto* daughter = dynamic_cast<const reco::RecoChargedCandidate*>(dimuon.daughter(j));
//          if (daughter && daughter->track().isNonnull() && 
//              daughter->track().key() == kaonTrack.key()) {
//            skipTrack = true;
//            break;
//          }
//        }

#if DEBUG_BFIT == true
        cout << "Track good? " << !skipTrack << endl;
#endif
        if (skipTrack) continue;

        // Get dimuon daughters as transient tracks
        std::vector<reco::TransientTrack> bTracks;
        std::vector<double> bMasses;
 
#if DEBUG_BFIT == true
        cout << "Dimuon basic property (pt, y): "  << ", " << dimuon.pt() << ", "<< dimuon.y() << "with Ndau " << dimuon.numberOfDaughters()<< endl;
#endif
        bTracks.push_back(muonTTrk1);
        bTracks.push_back(muonTTrk2);
        bMasses.push_back(muonMass);
        bMasses.push_back(muonMass);

        bTracks.push_back(transientTracks[i]);
        bMasses.push_back(kaonMass);

#if DEBUG_BFIT == true
        cout << "Checking Transient track sizes: bTracks: " << bTracks.size() << ", bMasses: " << bMasses.size() << endl;
#endif

        // Fit B+ vertex
        TransientVertex bVertex = fitBVertex(bTracks, bMasses);
        if (!bVertex.isValid()) continue;
#if DEBUG_BFIT == true
        cout << "Track fit is success" << endl;
#endif

        // Apply vertex quality cuts
        double vtxChi2 = bVertex.totalChiSquared();
        double vtxNdof = bVertex.degreesOfFreedom();
        double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
        
        if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;
#if DEBUG_BFIT == true
        cout << "Vertex Qual cut is success" << endl;
#endif

        // Create B+ candidate
        pat::CompositeCandidate bPlus = createBPlus(dimuon, kaonTrack, *vtxPrimary, bVertex);
        
        // Apply B+ selection cuts
#if DEBUG_BFIT == true
        cout << "Bmass diff : " << fabs(bPlus.mass() - bPlusMass) << " vs cut : " << bPlusMassCut << endl;
#endif
        if (fabs(bPlus.mass() - bPlusMass) > bPlusMassCut) continue;
        if (bPlus.pt() < bPtCut) continue;
        if (fabs(bPlus.rapidity()) > bYCut) continue;

        theBPlus.push_back(bPlus);
      }
    }

    // Bc+ reconstruction: dimuon + single track (pion)
    if (doBc) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        const auto& pionTrack = selectedTracks[i];
        
        // Skip tracks that are daughters of the dimuon
        bool skipTrack = false;
        skipTrack = (
                      &muonTrk1 == &(*pionTrack) || 
                      &muonTrk2 == &(*pionTrack)
                    );
        if (skipTrack) continue;

        // Get dimuon daughters as transient tracks
        std::vector<reco::TransientTrack> bcTracks;
        std::vector<double> bcMasses;
        
        bcTracks.push_back(muonTTrk1);
        bcTracks.push_back(muonTTrk2);
        bcMasses.push_back(muonMass);
        bcMasses.push_back(muonMass);

        
        // Add pion track
        bcTracks.push_back(transientTracks[i]);
        bcMasses.push_back(pionMass);

        // Fit Bc+ vertex
        TransientVertex bcVertex = fitBVertex(bcTracks, bcMasses);
        if (!bcVertex.isValid()) continue;

        // Apply vertex quality cuts (tighter for Bc due to shorter lifetime)
        double vtxChi2 = bcVertex.totalChiSquared();
        double vtxNdof = bcVertex.degreesOfFreedom();
        double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
        
        if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;

        // Create Bc+ candidate
        pat::CompositeCandidate bc = createBc(dimuon, pionTrack, *vtxPrimary, bcVertex);
        
        // Apply Bc+ selection cuts
        if (fabs(bc.mass() - bcMass) > bcMassCut) continue;
        if (bc.pt() < bcPtCut) continue;
        if (fabs(bc.rapidity()) > bcYCut) continue;

        theBc.push_back(bc);
      }
    }

    // B0 reconstruction: dimuon + two tracks (K*0 -> K+ pi-)
    if (doBZero) {
      for (size_t i = 0; i < selectedTracks.size(); i++) {
        for (size_t j = i + 1; j < selectedTracks.size(); j++) {
          const auto& track1 = selectedTracks[i];
          const auto& track2 = selectedTracks[j];
          
          // Check charge requirement for K*0
          if (!isWrongSign) {
            if (track1->charge() * track2->charge() > 0) continue;
          }
          
          // Skip tracks that are daughters of the dimuon
          bool skipTracks = false;
          skipTracks = (
                        &muonTrk1 == &(*track1) || 
                        &muonTrk2 == &(*track1) ||
                        &muonTrk1 == &(*track2) || 
                        &muonTrk2 == &(*track2)
                      );
          if (skipTracks) continue;

          // Reconstruct K*0 candidate
          math::PtEtaPhiMLorentzVector kaonP4, pionP4;
          reco::TrackRef kaonTrack, pionTrack;
          
          // Assign mass hypotheses (positive track as kaon, negative as pion)
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
          
          // Apply K*0 mass cut
          if (kstarP4.mass() < kstarMassMin || kstarP4.mass() > kstarMassMax) continue;
          if (kstarP4.pt() < kstarPtCut) continue;

          // Get all tracks for B0 vertex fit
          std::vector<reco::TransientTrack> bTracks;
          std::vector<double> bMasses;
          
          bTracks.push_back(muonTTrk1);
          bTracks.push_back(muonTTrk2);
          bMasses.push_back(muonMass);
          bMasses.push_back(muonMass);
          
          bTracks.push_back(transientTracks[i]);
          bTracks.push_back(transientTracks[j]);
          bMasses.push_back(track1->charge() > 0 ? kaonMass : pionMass);
          bMasses.push_back(track1->charge() > 0 ? pionMass : kaonMass);

#if DEBUG_BFIT == true
        cout << "Checking Transient track sizes: bTracks: " << bTracks.size() << ", bMasses: " << bMasses.size() << endl;
#endif

          // Fit B0 vertex
          TransientVertex bVertex = fitBVertex(bTracks, bMasses);
          if (!bVertex.isValid()) continue;

          // Apply vertex quality cuts
          double vtxChi2 = bVertex.totalChiSquared();
          double vtxNdof = bVertex.degreesOfFreedom();
          double vtxProb = TMath::Prob(vtxChi2, vtxNdof);
          
          if (vtxChi2/vtxNdof > vtxChi2Cut || vtxProb < vtxProbCut) continue;

          // Create B0 candidate
          pat::CompositeCandidate bZero = createBZero(dimuon, kaonTrack, pionTrack, *vtxPrimary, bVertex);
          
          // Apply B0 selection cuts
          if (fabs(bZero.mass() - bZeroMass) > bZeroMassCut) continue;
          if (bZero.pt() < bPtCut) continue;
          if (fabs(bZero.rapidity()) > bYCut) continue;

          theBZero.push_back(bZero);
        }
      }
    }
  }
}

// Helper method implementations
bool BDiMuMuFitter::passTrackCuts(const reco::TrackRef& track, const math::XYZPoint& bestvtx,
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

bool BDiMuMuFitter::passDimuonCuts(const pat::CompositeCandidate& dimuon) {
  if (dimuon.mass() < dimuonMassMin || dimuon.mass() > dimuonMassMax) return false;
  if (dimuon.pt() < dimuonPtCut) return false;
  if (fabs(dimuon.rapidity()) > dimuonYCut) return false;
  return true;
}

pat::CompositeCandidate BDiMuMuFitter::createBPlus(const pat::CompositeCandidate& dimuon,
                                                  const reco::TrackRef& kaonTrack,
                                                  const reco::Vertex& vertex,
                                                  const TransientVertex& bVertex) {
  pat::CompositeCandidate bPlus;
  
  // Set the dimuon as first daughter
  bPlus.addDaughter(dimuon, "dimuon");
  
  // Create kaon candidate and add as second daughter
  math::PtEtaPhiMLorentzVector kaonP4;
  kaonP4.SetPt(kaonTrack->pt());
  kaonP4.SetEta(kaonTrack->eta());
  kaonP4.SetPhi(kaonTrack->phi());
  kaonP4.SetM(kaonMass);
  
  // Convert to the required types
  reco::Candidate::LorentzVector kaonP4_converted(kaonP4.Px(), kaonP4.Py(), kaonP4.Pz(), kaonP4.E());
  reco::Candidate::Point vtxPos(bVertex.position().x(), bVertex.position().y(), bVertex.position().z());
  
  reco::RecoChargedCandidate kaonCand(kaonTrack->charge(), kaonP4_converted, vtxPos);
  kaonCand.setTrack(kaonTrack);
  bPlus.addDaughter(kaonCand, "kaon");
  
  // Set B+ four-momentum
  AddFourMomenta addP4;
  addP4.set(bPlus);
  
  // Add vertex information
  bPlus.addUserData("vertex", reco::Vertex(bVertex));
  bPlus.addUserFloat("vtxChi2", bVertex.totalChiSquared());
  bPlus.addUserFloat("vtxNdof", bVertex.degreesOfFreedom());
  bPlus.addUserFloat("vtxProb", TMath::Prob(bVertex.totalChiSquared(), bVertex.degreesOfFreedom()));
  
  // Calculate and store topological variables
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

pat::CompositeCandidate BDiMuMuFitter::createBZero(const pat::CompositeCandidate& dimuon,
                                                  const reco::TrackRef& kaonTrack,
                                                  const reco::TrackRef& pionTrack,
                                                  const reco::Vertex& vertex,
                                                  const TransientVertex& bVertex) {
  pat::CompositeCandidate bZero;
  
  // Set the dimuon as first daughter
  bZero.addDaughter(dimuon, "dimuon");
  
  // Create K*0 candidate
  pat::CompositeCandidate kstar;
  
  // Create kaon candidate
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
  
  // Create pion candidate
  math::PtEtaPhiMLorentzVector pionP4;
  pionP4.SetPt(pionTrack->pt());
  pionP4.SetEta(pionTrack->eta());
  pionP4.SetPhi(pionTrack->phi());
  pionP4.SetM(pionMass);
  
  reco::Candidate::LorentzVector pionP4_converted(pionP4.Px(), pionP4.Py(), pionP4.Pz(), pionP4.E());
  
  reco::RecoChargedCandidate pionCand(pionTrack->charge(), pionP4_converted, vtxPos);
  pionCand.setTrack(pionTrack);
  kstar.addDaughter(pionCand, "pion");
  
  // Set K*0 four-momentum
  AddFourMomenta addP4;
  addP4.set(kstar);
  kstar.addUserFloat("mass", kstar.mass());
  
  // Add K*0 to B0
  bZero.addDaughter(kstar, "kstar");
  
  // Set B0 four-momentum
  addP4.set(bZero);
  
  // Add vertex information
  bZero.addUserData("vertex", reco::Vertex(bVertex));
  bZero.addUserFloat("vtxChi2", bVertex.totalChiSquared());
  bZero.addUserFloat("vtxNdof", bVertex.degreesOfFreedom());
  bZero.addUserFloat("vtxProb", TMath::Prob(bVertex.totalChiSquared(), bVertex.degreesOfFreedom()));
  
  // Calculate and store topological variables
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

pat::CompositeCandidate BDiMuMuFitter::createBc(const pat::CompositeCandidate& dimuon,
                                                const reco::TrackRef& pionTrack,
                                                const reco::Vertex& vertex,
                                                const TransientVertex& bcVertex) {
  pat::CompositeCandidate bc;
  
  // Set the dimuon as first daughter
  bc.addDaughter(dimuon, "dimuon");
  
  // Create pion candidate and add as second daughter
  math::PtEtaPhiMLorentzVector pionP4;
  pionP4.SetPt(pionTrack->pt());
  pionP4.SetEta(pionTrack->eta());
  pionP4.SetPhi(pionTrack->phi());
  pionP4.SetM(pionMass);
  
  // Convert to the required types
  reco::Candidate::LorentzVector pionP4_converted(pionP4.Px(), pionP4.Py(), pionP4.Pz(), pionP4.E());
  reco::Candidate::Point vtxPos(bcVertex.position().x(), bcVertex.position().y(), bcVertex.position().z());
  
  reco::RecoChargedCandidate pionCand(pionTrack->charge(), pionP4_converted, vtxPos);
  pionCand.setTrack(pionTrack);
  bc.addDaughter(pionCand, "pion");
  
  // Set Bc+ four-momentum
  AddFourMomenta addP4;
  addP4.set(bc);
  
  // Add vertex information
  bc.addUserData("vertex", reco::Vertex(bcVertex));
  bc.addUserFloat("vtxChi2", bcVertex.totalChiSquared());
  bc.addUserFloat("vtxNdof", bcVertex.degreesOfFreedom());
  bc.addUserFloat("vtxProb", TMath::Prob(bcVertex.totalChiSquared(), bcVertex.degreesOfFreedom()));
  
  // Calculate and store topological variables
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

TransientVertex BDiMuMuFitter::fitBVertex(const std::vector<reco::TransientTrack>& tracks,
                                         const std::vector<double>& masses) {
  KalmanVertexFitter fitter;
  return fitter.vertex(tracks);
}

double BDiMuMuFitter::calculatePointingAngle(const math::XYZVector& momentum,
                                            const math::XYZVector& displacement) {
  return acos(momentum.Dot(displacement) / (momentum.R() * displacement.R()));
}

// Get methods
const pat::CompositeCandidateCollection& BDiMuMuFitter::getBPlus() const {
  return theBPlus;
}

const pat::CompositeCandidateCollection& BDiMuMuFitter::getBZero() const {
  return theBZero;
}

const pat::CompositeCandidateCollection& BDiMuMuFitter::getBc() const {
  return theBc;
}

const std::vector<float>& BDiMuMuFitter::getMVAVals() const {
  return mvaVals_;
}

void BDiMuMuFitter::resetAll() {
  theBPlus.clear();
  theBZero.clear();
  theBc.clear();
  mvaVals_.clear();
}
