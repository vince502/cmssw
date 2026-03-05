// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D04PFitter
// 
/**\class D04PFitter D04PFitter.cc VertexCompositeAnalysis/VertexCompositeProducer/src/D04PFitter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
//
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D04PFitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"

#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TVector3.h>
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"


// Constructor and (empty) destructor
D04PFitter::D04PFitter(const edm::ParameterSet& theParameters,  edm::ConsumesCollector && iC) :
    bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>())
{
  using std::string;

  // Get the track reco algorithm from the ParameterSet
  token_beamSpot = iC.consumes<reco::BeamSpot>(edm::InputTag("offlineBeamSpot"));
  token_tracks = iC.consumes<reco::TrackCollection>(theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  token_vertices = iC.consumes<reco::VertexCollection>(theParameters.getParameter<edm::InputTag>("vertexRecoAlgorithm"));
  
  // dEdx source - configurable, empty InputTag means disabled
  edm::InputTag dedxTag = theParameters.getParameter<edm::InputTag>("dedxSrc");
  useDeDx_ = !dedxTag.label().empty();
  if(useDeDx_) {
    token_dedx = iC.consumes<edm::ValueMap<reco::DeDxData> >(dedxTag);
    token_track2pc = iC.consumes<std::vector<edm::Ptr<pat::PackedCandidate>>>(
        theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  }

  // Second, initialize post-fit cuts
  mPiKCutMin = theParameters.getParameter<double>(string("mPiKCutMin"));
  mPiKCutMax = theParameters.getParameter<double>(string("mPiKCutMax"));
  tkDCACut = theParameters.getParameter<double>(string("tkDCACut"));
  tkChi2Cut = theParameters.getParameter<double>(string("tkChi2Cut"));
  tkNhitsCut = theParameters.getParameter<int>(string("tkNhitsCut"));
  tkPtCut = theParameters.getParameter<double>(string("tkPtCut"));
  tkPtErrCut = theParameters.getParameter<double>(string("tkPtErrCut"));
  tkEtaCut = theParameters.getParameter<double>(string("tkEtaCut"));
  tkPtSumCut = theParameters.getParameter<double>(string("tkPtSumCut"));
  tkEtaDiffCut = theParameters.getParameter<double>(string("tkEtaDiffCut"));
  chi2Cut = theParameters.getParameter<double>(string("vtxChi2Cut"));
  rVtxCut = theParameters.getParameter<double>(string("rVtxCut"));
  rVtxSigCut = theParameters.getParameter<double>(string("vtxSignificance2DCut"));
  lVtxCut = theParameters.getParameter<double>(string("lVtxCut"));
  lVtxSigCut = theParameters.getParameter<double>(string("vtxSignificance3DCut"));
  collinCut2D = theParameters.getParameter<double>(string("collinearityCut2D"));
  collinCut3D = theParameters.getParameter<double>(string("collinearityCut3D"));
  d0MassCut = theParameters.getParameter<double>(string("d0MassCut"));
  dauTransImpactSigCut = theParameters.getParameter<double>(string("dauTransImpactSigCut"));
  dauLongImpactSigCut = theParameters.getParameter<double>(string("dauLongImpactSigCut"));
  VtxChiProbCut = theParameters.getParameter<double>(string("VtxChiProbCut"));
  dPtCut = theParameters.getParameter<double>(string("dPtCut"));
  alphaCut = theParameters.getParameter<double>(string("alphaCut"));
  alpha2DCut = theParameters.getParameter<double>(string("alpha2DCut"));
  isWrongSign = theParameters.getParameter<bool>(string("isWrongSign"));
  d0AbsYCut = theParameters.getParameter<double>(string("d0AbsYCut"));


  useAnyMVA_ = false;
  forestLabel_ = "D0InpPb";
  std::string type = "BDT";
  useForestFromDB_ = true;
  dbFileName_ = "";

  forest_ = nullptr;

  if(theParameters.exists("useAnyMVA")) useAnyMVA_ = theParameters.getParameter<bool>("useAnyMVA");

  if(useAnyMVA_){
    if(theParameters.exists("mvaType"))type = theParameters.getParameter<std::string>("mvaType");
    if(theParameters.exists("GBRForestLabel"))forestLabel_ = theParameters.getParameter<std::string>("GBRForestLabel");
    if(theParameters.exists("GBRForestFileName")){
      dbFileName_ = theParameters.getParameter<std::string>("GBRForestFileName");
      useForestFromDB_ = false;
    }

    if(!useForestFromDB_){
      edm::FileInPath fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s",dbFileName_.c_str()));
      TFile gbrfile(fip.fullPath().c_str(),"READ");
      forest_ = (GBRForest*)gbrfile.Get(forestLabel_.c_str());
      gbrfile.Close();
    }

    mvaType_ = type;
    mvaToken_ = iC.esConsumes<GBRForest, GBRWrapperRcd>(edm::ESInputTag("", forestLabel_));
  }

  std::vector<std::string> qual = theParameters.getParameter<std::vector<std::string> >("trackQualities");
  for (unsigned int ndx = 0; ndx < qual.size(); ndx++) {
    qualities.push_back(reco::TrackBase::qualityByName(qual[ndx]));
  }
}

D04PFitter::~D04PFitter() {
  delete forest_;
}

// Method containing the algorithm for vertex reconstruction
void D04PFitter::fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

  using std::vector;
  using std::cout;
  using std::endl;
  using namespace reco;
  using namespace edm;
  using namespace std; 

  typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3> > SMatrixSym3D;
  typedef ROOT::Math::SVector<double, 3> SVector3;

  // Create std::vectors for Tracks and TrackRefs (required for
  //  passing to the KalmanVertexFitter)
  std::vector<TrackRef> theTrackRefsPos;
  std::vector<TransientTrack> theTransTracksPos;

  std::vector<TrackRef> theTrackRefsNeg;
  std::vector<TransientTrack> theTransTracksNeg;

  // Handles for tracks, B-field, and tracker geometry
  Handle<reco::TrackCollection> theTrackHandle;
  Handle<reco::VertexCollection> theVertexHandle;
  Handle<reco::BeamSpot> theBeamSpotHandle;
  ESHandle<MagneticField> bFieldHandle;
  Handle<edm::ValueMap<reco::DeDxData> > dEdxHandle;
  Handle<std::vector<edm::Ptr<pat::PackedCandidate>>> track2pcHandle;

  // Get the tracks, vertices from the event, and get the B-field record
  //  from the EventSetup
  iEvent.getByToken(token_tracks, theTrackHandle); 
  iEvent.getByToken(token_vertices, theVertexHandle);
  iEvent.getByToken(token_beamSpot, theBeamSpotHandle);  
  if(useDeDx_) {
    iEvent.getByToken(token_dedx, dEdxHandle);
    iEvent.getByToken(token_track2pc, track2pcHandle);
  }

  if( !theTrackHandle->size() ) return;
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  magField = bFieldHandle.product();

  // Setup TMVA
//  mvaValValueMap = auto_ptr<edm::ValueMap<float> >(new edm::ValueMap<float>);
//  edm::ValueMap<float>::Filler mvaFiller(*mvaValValueMap);

  [[maybe_unused]] bool isVtxPV = false;
  double xVtx=-99999.0;
  double yVtx=-99999.0;
  double zVtx=-99999.0;
  double xVtxError=-999.0;
  double yVtxError=-999.0;
  double zVtxError=-999.0;
  const reco::VertexCollection vtxCollection = *(theVertexHandle.product());
  reco::VertexCollection::const_iterator vtxPrimary = vtxCollection.begin();
  if(vtxCollection.size()>0 && !vtxPrimary->isFake() && vtxPrimary->tracksSize()>=4){
    isVtxPV = 1;
    xVtx = vtxPrimary->x();
    yVtx = vtxPrimary->y();
    zVtx = vtxPrimary->z();
    xVtxError = vtxPrimary->xError();
    yVtxError = vtxPrimary->yError();
    zVtxError = vtxPrimary->zError();
  }
  else{
    isVtxPV = 0;
    xVtx = theBeamSpotHandle->position().x();
    yVtx = theBeamSpotHandle->position().y();
    zVtx = 0.0;
    xVtxError = theBeamSpotHandle->BeamWidthX();
    yVtxError = theBeamSpotHandle->BeamWidthY();
    zVtxError = 0.0;
  }
  math::XYZPoint bestvtx(xVtx,yVtx,zVtx);

  // Fill vectors of TransientTracks and TrackRefs after applying preselection cuts.
  if(theTrackHandle->size() < 4) return;
  for(unsigned int indx = 0; indx < theTrackHandle->size(); indx++){
    TrackRef tmpRef( theTrackHandle, indx );
    bool quality_ok = true;
    if (qualities.size()!=0) {
      quality_ok = false;
      for (unsigned int ndx_ = 0; ndx_ < qualities.size(); ndx_++){
	      if (tmpRef->quality(qualities[ndx_])){
	        quality_ok = true;
	        break;          
	      }
      }
    }
    if( !quality_ok ) continue;

    if( 
        tmpRef->normalizedChi2() < tkChi2Cut &&
        tmpRef->numberOfValidHits() >= tkNhitsCut &&
        tmpRef->ptError() / tmpRef->pt() < tkPtErrCut &&
        tmpRef->pt() > tkPtCut && fabs(tmpRef->eta()) < tkEtaCut 
      ){
      TransientTrack tmpTk( *tmpRef, magField );

      double dzvtx = tmpRef->dz(bestvtx);
      double dxyvtx = tmpRef->dxy(bestvtx);      
      double dzerror = sqrt(tmpRef->dzError()*tmpRef->dzError()+zVtxError*zVtxError);
      double dxyerror = sqrt(tmpRef->d0Error()*tmpRef->d0Error()+xVtxError*yVtxError);

      double dauLongImpactSig = dzvtx/dzerror;
      double dauTransImpactSig = dxyvtx/dxyerror;

      if( fabs(dauTransImpactSig) > dauTransImpactSigCut && fabs(dauLongImpactSig) > dauLongImpactSigCut ) {
        if( tmpRef->charge() > 0 ){
          theTrackRefsPos.push_back( tmpRef );
          theTransTracksPos.push_back( tmpTk );
        }
        if( tmpRef->charge() < 0 ){
          theTrackRefsNeg.push_back( tmpRef );
          theTransTracksNeg.push_back( tmpTk );
        }
      }
    }
  }
  auto calculateInvariantMassFromTransientTracks = [](
    const std::vector<reco::TransientTrack*> transientTracks,
    const std::vector<double>& masses) {
    if (transientTracks.size() != masses.size()) {
      throw std::runtime_error("Mismatch between the number of tracks and mass hypotheses.");
    }
    // Composite 4-momentum
    math::XYZTLorentzVector compositeVector;
    // Loop over transient tracks and compute the total 4-momentum
    for (size_t i = 0; i < transientTracks.size(); ++i) {
      const reco::TransientTrack& track = *(transientTracks[i]);
      double mass = masses[i];
      // Get momentum vector at the closest approach to the beamline
      auto momentum = track.impactPointTSCP().momentum();
      // Extract momentum components
      double px = momentum.x();
      double py = momentum.y();
      double pz = momentum.z();
      // Compute energy
      double energy = std::sqrt(px * px + py * py + pz * pz + mass * mass);
      // Create a Lorentz vector for this particle
      math::XYZTLorentzVector lv(px, py, pz, energy);
      // Add to the composite Lorentz vector
      compositeVector += lv;
    }
    // Return the invariant mass of the composite system
    return compositeVector.mass();
  };

  int lpcnt = 0;
  int passlpcnt = 0;
  // int passlpcnt1 = 0;
  int passlpcnt2 = 0;
  int passlpcnt3 = 0;
  // Loop over tracks and vertex good charged track pairs
  for(unsigned int itrkpos1 = 0; itrkpos1 < theTrackRefsPos.size(); itrkpos1++) {
    for(unsigned int itrkpos2 = itrkpos1+1; itrkpos2 < theTrackRefsPos.size(); itrkpos2++) {
      TrackRef ptr1, ptr2;
      TransientTrack *posPtr1 = 0, *posPtr2 =0;
      ptr1 = theTrackRefsPos[itrkpos1];
      ptr2 = theTrackRefsPos[itrkpos2];
      posPtr1 = &theTransTracksPos[itrkpos1];
      posPtr2 = &theTransTracksPos[itrkpos2];
      if( fabs(ptr1->eta() - ptr2->eta())> tkEtaDiffCut) continue;
      for(unsigned int itrkneg1 = 0; itrkneg1 < theTrackRefsNeg.size(); itrkneg1++) {
        TrackRef ntr1;
        TransientTrack *negPtr1 = 0;
        ntr1 = theTrackRefsNeg[itrkneg1];
        negPtr1 = &theTransTracksNeg[itrkneg1];
        if( fabs(ptr1->eta() - ntr1->eta())> tkEtaDiffCut) continue;
        if( fabs(ptr2->eta() - ntr1->eta())> tkEtaDiffCut) continue;
        for(unsigned int itrkneg2 = itrkneg1+1; itrkneg2 < theTrackRefsNeg.size(); itrkneg2++) {

          TrackRef ntr2;
          TransientTrack *negPtr2 =0;

          KinematicParticleFactoryFromTransientTrack pFactory;
          ntr2 = theTrackRefsNeg[itrkneg2];
          negPtr2 = &theTransTracksNeg[itrkneg2];

          if( fabs(ptr1->eta() - ntr2->eta())> tkEtaDiffCut) continue;
          if( fabs(ptr2->eta() - ntr2->eta())> tkEtaDiffCut) continue;
          if( fabs(ntr2->eta() - ntr1->eta())> tkEtaDiffCut) continue;

          // Apply pre-cuts BEFORE expensive kinematic fit
          // Pt sum cut
          double ptSum = ptr1->pt() + ptr2->pt() + ntr1->pt() + ntr2->pt();
          if( ptSum < tkPtSumCut) continue;

          // Pre-fit mass estimate (cheap calculation)
          std::vector<TransientTrack> transTracks;
          lpcnt++;
          std::vector<reco::TransientTrack*> transientTracks = {posPtr1, posPtr2, negPtr1, negPtr2};
          std::vector<double> masses = {0.13957, 0.13957, 0.13957, 0.13957};
          double d0Mass = calculateInvariantMassFromTransientTracks(transientTracks, masses);
          
          // Apply tighter mass window BEFORE fit (mPiKCutMin/Max are the final cuts)
          // Use a slightly wider window here to account for fit resolution
          double massWindowMargin = 0.1; // GeV, to account for fit differences
          if( d0Mass < mPiKCutMin - massWindowMargin || d0Mass > mPiKCutMax + massWindowMargin) continue;
          if( d0Mass > 2.4) continue; // Keep upper bound check

          passlpcnt2++;

          float chi = 0.0;
          float ndf = 0.0;

          vector<RefCountedKinematicParticle> d0Particles;
          d0Particles.push_back(pFactory.particle(*posPtr1,kaonMassD04P,chi,ndf,kaonMassD04P_sigma));
          d0Particles.push_back(pFactory.particle(*posPtr2,piMassD04P,chi,ndf,piMassD04P_sigma));
          d0Particles.push_back(pFactory.particle(*negPtr1,piMassD04P,chi,ndf,piMassD04P_sigma));
          d0Particles.push_back(pFactory.particle(*negPtr2,piMassD04P,chi,ndf,piMassD04P_sigma));

          KinematicParticleVertexFitter d0Fitter;
          RefCountedKinematicTree d0Vertex;
          d0Vertex = d0Fitter.fit(d0Particles);
          if( !d0Vertex->isValid() ) continue;

          d0Vertex->movePointerToTheTop(); // D0
          RefCountedKinematicParticle d0Cand = d0Vertex->currentParticle(); // D0
          if (!d0Cand->currentState().isValid()) continue;
          passlpcnt3++;

          RefCountedKinematicVertex d0DecayVertex = d0Vertex->currentDecayVertex(); // D0 Vtx
          if (!d0DecayVertex->vertexIsValid()) continue;
          if ( d0DecayVertex->chiSquared()<0 || d0DecayVertex->chiSquared()>100 ) continue;
	        float d0C2Prob = TMath::Prob(d0DecayVertex->chiSquared(),d0DecayVertex->degreesOfFreedom());
	        if (d0C2Prob < VtxChiProbCut) continue;

          d0Vertex->movePointerToTheFirstChild();
          RefCountedKinematicParticle posCand1 = d0Vertex->currentParticle();
          d0Vertex->movePointerToTheNextChild();
          RefCountedKinematicParticle posCand2 = d0Vertex->currentParticle();
          d0Vertex->movePointerToTheNextChild();
          RefCountedKinematicParticle negCand1 = d0Vertex->currentParticle();
          d0Vertex->movePointerToTheNextChild();
          RefCountedKinematicParticle negCand2 = d0Vertex->currentParticle();

          KinematicParameters posCandKP1 = posCand1->currentState().kinematicParameters();
          KinematicParameters posCandKP2 = posCand2->currentState().kinematicParameters();
          KinematicParameters negCandKP1 = negCand1->currentState().kinematicParameters();
          KinematicParameters negCandKP2 = negCand2->currentState().kinematicParameters();

          GlobalVector d0TotalP = GlobalVector (d0Cand->currentState().globalMomentum().x(),
                                                d0Cand->currentState().globalMomentum().y(),
                                                d0Cand->currentState().globalMomentum().z());

          GlobalVector posCandTotalP1 = GlobalVector(posCandKP1.momentum().x(),posCandKP1.momentum().y(),posCandKP1.momentum().z());
          GlobalVector posCandTotalP2 = GlobalVector(posCandKP2.momentum().x(),posCandKP2.momentum().y(),posCandKP2.momentum().z());
          GlobalVector negCandTotalP1 = GlobalVector(negCandKP1.momentum().x(),negCandKP1.momentum().y(),negCandKP1.momentum().z());
          GlobalVector negCandTotalP2 = GlobalVector(negCandKP2.momentum().x(),negCandKP2.momentum().y(),negCandKP2.momentum().z());

          double posCand1TotalE = sqrt( posCandTotalP1.mag2() + kaonMassD04P * kaonMassD04P );
          double posCand2TotalE = sqrt( posCandTotalP2.mag2() + piMassD04P * piMassD04P );
          double negCand1TotalE = sqrt( negCandTotalP1.mag2() + piMassD04P * piMassD04P );
          double negCand2TotalE = sqrt( negCandTotalP2.mag2() + piMassD04P * piMassD04P );
          double d0TotalE = posCand1TotalE + posCand2TotalE + negCand1TotalE + negCand2TotalE;

          const Particle::LorentzVector d0P4(d0TotalP.x(), d0TotalP.y(), d0TotalP.z(), d0TotalE);

          // Apply mPiKCutMin and mPiKCutMax cuts on the fitted D0 mass
          double d0FittedMass = d0P4.mass();
          if (d0FittedMass < mPiKCutMin || d0FittedMass > mPiKCutMax) continue;

          // Apply rapidity cut on D0 candidate (early cut for efficiency)
          double d0Rapidity = d0P4.Rapidity();
          if (fabs(d0Rapidity) > d0AbsYCut) continue;

          Particle::Point d0Vtx((*d0DecayVertex).position().x(), (*d0DecayVertex).position().y(), (*d0DecayVertex).position().z());
          RecoChargedCandidate thePosCand1(1, Particle::LorentzVector(posCandTotalP1.x(), posCandTotalP1.y(), posCandTotalP1.z(), posCand1TotalE), d0Vtx);
          thePosCand1.setTrack(ptr1);
          RecoChargedCandidate thePosCand2(1, Particle::LorentzVector(posCandTotalP2.x(), posCandTotalP2.y(), posCandTotalP2.z(), posCand2TotalE), d0Vtx);
          thePosCand2.setTrack(ptr2);
          RecoChargedCandidate theNegCand1(-1, Particle::LorentzVector(negCandTotalP1.x(), negCandTotalP1.y(), negCandTotalP1.z(), negCand1TotalE), d0Vtx);
          theNegCand1.setTrack(ntr1);
          RecoChargedCandidate theNegCand2(-1, Particle::LorentzVector(negCandTotalP2.x(), negCandTotalP2.y(), negCandTotalP2.z(), negCand2TotalE), d0Vtx);
          theNegCand2.setTrack(ntr2);

          std::vector<double> d0VtxEVec;
          d0VtxEVec.push_back( d0DecayVertex->error().cxx() );
          d0VtxEVec.push_back( d0DecayVertex->error().cyx() );
          d0VtxEVec.push_back( d0DecayVertex->error().cyy() );
          d0VtxEVec.push_back( d0DecayVertex->error().czx() );
          d0VtxEVec.push_back( d0DecayVertex->error().czy() );
          d0VtxEVec.push_back( d0DecayVertex->error().czz() );
          SMatrixSym3D d0VtxCovMatrix(d0VtxEVec.begin(), d0VtxEVec.end());
          const Vertex::CovarianceMatrix d0VtxCov(d0VtxCovMatrix);
          double d0VtxChi2(d0DecayVertex->chiSquared());
          double d0VtxNdof(d0DecayVertex->degreesOfFreedom());

          std::unique_ptr<CC> theD0 = std::make_unique<CC>();
          theD0->setP4(d0P4);
          theD0->addDaughter(thePosCand1, "posdau1");
          theD0->addDaughter(thePosCand2, "posdau2");
          theD0->addDaughter(theNegCand1, "negdau1");
          theD0->addDaughter(theNegCand2, "negdau2");
          
          reco::Vertex d0VtxObj = *d0DecayVertex;
          theD0->addUserData("Vtx", d0VtxObj);
          theD0->addUserFloat("VtxChi2", d0VtxChi2);
          theD0->addUserFloat("VtxNdof", d0VtxNdof);
          
          theD0s.push_back(*theD0);
          passlpcnt++;

        }
      }
    }
  }















}
// Get methods

const D04PFitter::CCC& D04PFitter::getD0() const {
  return theD0s;
}

const std::vector<float>& D04PFitter::getMVAVals() const {
  return mvaVals_;
}

void D04PFitter::resetAll() {
    theD0s.clear();
    mvaVals_.clear();
}
