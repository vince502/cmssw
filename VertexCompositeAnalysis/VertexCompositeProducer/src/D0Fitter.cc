// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D0Fitter
// 
/**\class D0Fitter D0Fitter.cc VertexCompositeAnalysis/VertexCompositeProducer/src/D0Fitter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
//
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Fitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"


#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"


#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TVector3.h>
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"

#include <chrono>
using namespace std::chrono;


const size_t VARSIZE = 19;
const float piMassD0 = 0.13957018;
const float piMassD0Squared = piMassD0*piMassD0;
const float kaonMassD0 = 0.493677;
const float kaonMassD0Squared = kaonMassD0*kaonMassD0;
const float d0MassD0 = 1.86484;
float piMassD0_sigma = 3.5E-7f;
float kaonMassD0_sigma = 1.6E-5f;
float d0MassD0_sigma = d0MassD0*1.e-6;

using CC = pat::CompositeCandidate;
using CCC = pat::CompositeCandidateCollection;
// using cms;

// Constructor and (empty) destructor
D0Fitter::D0Fitter(const edm::ParameterSet& theParameters,  edm::ConsumesCollector && iC, const ONNXRuntime* onnxRuntime) :
    bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>()), onnxRuntime_(onnxRuntime),input_shapes_()
{
//		   const edm::Event& iEvent, const edm::EventSetup& iSetup, edm::ConsumesCollector && iC) {
  using std::string;

  // Get the track reco algorithm from the ParameterSet
  token_beamSpot = iC.consumes<reco::BeamSpot>(edm::InputTag("offlineBeamSpot"));
  token_tracks = iC.consumes<reco::TrackCollection>(theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  token_vertices = iC.consumes<reco::VertexCollection>(theParameters.getParameter<edm::InputTag>("vertexRecoAlgorithm"));
  token_dedx = iC.consumes<edm::ValueMap<reco::DeDxData> >(edm::InputTag("dedxHarmonic2"));
  tok_centBinLabel_ = iC.consumes<int>(edm::InputTag("centralityBin","HFtowers"));

  // Second, initialize post-fit cuts
  mPiKCutMin = theParameters.getParameter<double>(string("mPiKCutMin"));
  mPiKCutMax = theParameters.getParameter<double>(string("mPiKCutMax"));
  tkDCACut = theParameters.getParameter<double>(string("tkDCACut"));
  tkDCACutLow = theParameters.getParameter<double>(string("tkDCACutLow"));
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
  d0AbsYCut = theParameters.getParameter<double>(string("d0AbsYCut"));
  dauTransImpactSigCut = theParameters.getParameter<double>(string("dauTransImpactSigCut"));
  dauLongImpactSigCut = theParameters.getParameter<double>(string("dauLongImpactSigCut"));
  VtxChiProbCut = theParameters.getParameter<double>(string("VtxChiProbCut"));
  dPtCut = theParameters.getParameter<double>(string("dPtCut"));
  alphaCut = theParameters.getParameter<double>(string("alphaCut"));
  alpha2DCut = theParameters.getParameter<double>(string("alpha2DCut"));
  isWrongSign = theParameters.getParameter<bool>(string("isWrongSign"));
  mvaCut = theParameters.getParameter<double>(string("mvaCut"));


  useAnyMVA_ = false;
  forestLabel_ = "D0InPbPb";
  std::string type = "BDT";
  useForestFromDB_ = true;
  dbFileName_ = "";

  forest_ = nullptr;

  if(theParameters.exists("useAnyMVA")) useAnyMVA_ = theParameters.getParameter<bool>("useAnyMVA");

  //if(useAnyMVA_){
  //  if(theParameters.exists("mvaType"))type = theParameters.getParameter<std::string>("mvaType");
  //  if(theParameters.exists("GBRForestLabel"))forestLabel_ = theParameters.getParameter<std::string>("GBRForestLabel");
  //  if(theParameters.exists("GBRForestFileName")){
  //    dbFileName_ = theParameters.getParameter<std::string>("GBRForestFileName");
  //    useForestFromDB_ = false;
  //  }

  //  if(!useForestFromDB_){
  //    edm::FileInPath fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s",dbFileName_.c_str()));
  //    TFile gbrfile(fip.fullPath().c_str(),"READ");
  //    forest_ = (GBRForest*)gbrfile.Get(forestLabel_.c_str());
  //    gbrfile.Close();
  //  }

  //  mvaType_ = type;
  //  mvaToken_ = iC.esConsumes<GBRForest, GBRWrapperRcd>(edm::ESInputTag("", forestLabel_));
  //}
  if (useAnyMVA_) {
    if (theParameters.exists("input_names")||theParameters.exists("output_names")) {
      input_names_ = theParameters.getParameter<std::vector<std::string>>("input_names");
      output_names_ = theParameters.getParameter<std::vector<std::string>>("output_names");
    } else {
      throw cms::Exception("Configuration") << "onnxModelFileName not provided in ParameterSet";
    }
//  Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "D0Fitter");
//    Ort::SessionOptions sessionOptions;
//    sessionOptions.SetIntraOpNumThreads(1); // Single-threaded for simplicity
    // edm::FileInPath fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s",onnxModelPath_.c_str()));    // Path relative to CMSSW_BASE
    // onnxModel_ = std::make_unique<cms::Ort::ONNXRuntime>(fip.fullPath());
    // Retrieve input and output names
    //Ort::AllocatorWithDefaultOptions allocator;
    //auto numInputNodes = onnxSession_->GetInputCount();
    //for (size_t i = 0; i < numInputNodes; i++) {
    //  auto inputName = onnxSession_->GetInputNameAllocated(i, allocator);
    //  inputNames_.push_back(inputName.get());
    //}
    //auto numOutputNodes = onnxSession_->GetOutputCount();
    //for (size_t i = 0; i < numOutputNodes; i++) {
    //  auto outputName = onnxSession_->GetOutputNameAllocated(i, allocator);
    //  outputNames_.push_back(outputName.get());
    //}
  }

  std::vector<std::string> qual = theParameters.getParameter<std::vector<std::string> >("trackQualities");
  for (unsigned int ndx = 0; ndx < qual.size(); ndx++) {
    qualities.push_back(reco::TrackBase::qualityByName(qual[ndx]));
  }
}

D0Fitter::~D0Fitter() {
  delete forest_;
}

// Method containing the algorithm for vertex reconstruction
void D0Fitter::fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
 
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
  std::vector<TrackRef> theTrackRefs;
  std::vector<TransientTrack> theTransTracks;

  // Handles for tracks, B-field, and tracker geometry
  Handle<reco::TrackCollection> theTrackHandle;
  Handle<reco::VertexCollection> theVertexHandle;
  Handle<reco::BeamSpot> theBeamSpotHandle;
  ESHandle<MagneticField> bFieldHandle;
  Handle<edm::ValueMap<reco::DeDxData> > dEdxHandle;
  Handle<int> cbin;
  int centrality = -1;
  if(useAnyMVA_){
    iEvent.getByToken(tok_centBinLabel_, cbin);
    centrality = (cbin.isValid() ? *cbin : -1);
  }

  // Get the tracks, vertices from the event, and get the B-field record
  //  from the EventSetup
  iEvent.getByToken(token_tracks, theTrackHandle); 
  iEvent.getByToken(token_vertices, theVertexHandle);
  iEvent.getByToken(token_beamSpot, theBeamSpotHandle);  
  iEvent.getByToken(token_dedx, dEdxHandle);


  if( !theTrackHandle->size() ) return;
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  magField = bFieldHandle.product();

  // Setup TMVA
//  mvaValValueMap = auto_ptr<edm::ValueMap<float> >(new edm::ValueMap<float>);
//  edm::ValueMap<float>::Filler mvaFiller(*mvaValValueMap);

  bool isVtxPV = 0;
  double xVtx=-99999.0;
  double yVtx=-99999.0;
  double zVtx=-99999.0;
  double xVtxError=-999.0;
  double yVtxError=-999.0;
  double zVtxError=-999.0;
  const reco::VertexCollection vtxCollection = *(theVertexHandle.product());
  reco::VertexCollection::const_iterator vtxPrimary = vtxCollection.begin();
  if(vtxCollection.size()>0 && !vtxPrimary->isFake() && vtxPrimary->tracksSize()>=2)
  {
    isVtxPV = 1;
    xVtx = vtxPrimary->x();
    yVtx = vtxPrimary->y();
    zVtx = vtxPrimary->z();
    xVtxError = vtxPrimary->xError();
    yVtxError = vtxPrimary->yError();
    zVtxError = vtxPrimary->zError();
  }
  else {
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
  for(unsigned int indx = 0; indx < theTrackHandle->size(); indx++) {
    TrackRef tmpRef( theTrackHandle, indx );
    bool quality_ok = true;
    if (qualities.size()!=0) {
      quality_ok = false;
      for (unsigned int ndx_ = 0; ndx_ < qualities.size(); ndx_++) {
	if (tmpRef->quality(qualities[ndx_])){
	  quality_ok = true;
	  break;          
	}
      }
    }
    if( !quality_ok ) continue;

    if( tmpRef->normalizedChi2() < tkChi2Cut &&
        tmpRef->numberOfValidHits() >= tkNhitsCut &&
        tmpRef->ptError() / tmpRef->pt() < tkPtErrCut &&
        tmpRef->pt() > tkPtCut && fabs(tmpRef->eta()) < tkEtaCut ) {
//      TransientTrack tmpTk( *tmpRef, &(*bFieldHandle), globTkGeomHandle );
      TransientTrack tmpTk( *tmpRef, magField );

      double dzvtx = tmpRef->dz(bestvtx);
      double dxyvtx = tmpRef->dxy(bestvtx);      
      double dzerror = sqrt(tmpRef->dzError()*tmpRef->dzError()+zVtxError*zVtxError);
      double dxyerror = sqrt(tmpRef->d0Error()*tmpRef->d0Error()+xVtxError*yVtxError);

      double dauLongImpactSig = dzvtx/dzerror;
      double dauTransImpactSig = dxyvtx/dxyerror;

      if( fabs(dauTransImpactSig) > dauTransImpactSigCut && fabs(dauLongImpactSig) > dauLongImpactSigCut ) {
        theTrackRefs.push_back( tmpRef );
        theTransTracks.push_back( tmpTk );
      }
    }
  }

  float posCandMass[2] = {piMassD0, kaonMassD0};
  float negCandMass[2] = {kaonMassD0, piMassD0};
  float posCandMass_sigma[2] = {piMassD0_sigma, kaonMassD0_sigma};
  float negCandMass_sigma[2] = {kaonMassD0_sigma, piMassD0_sigma};
  int   pdg_id[2] = {421, -421};

  double totaltime = 0.0;
  int nloop = 0;
  // Loop over tracks and vertex good charged track pairs
  // std::vector<CC*> tmpD0s;
  // cms::Ort::FloatArrays data_(1);
  // data_.reserve( 50000);
  for(unsigned int trdx1 = 0; trdx1 < theTrackRefs.size(); trdx1++) {

    for(unsigned int trdx2 = trdx1 + 1; trdx2 < theTrackRefs.size(); trdx2++) {

      if( (theTrackRefs[trdx1]->pt() + theTrackRefs[trdx2]->pt()) < tkPtSumCut) continue;
      if( abs(theTrackRefs[trdx1]->eta() - theTrackRefs[trdx2]->eta()) > tkEtaDiffCut) continue;

      //This vector holds the pair of oppositely-charged tracks to be vertexed
      std::vector<TransientTrack> transTracks;

      TrackRef positiveTrackRef;
      TrackRef negativeTrackRef;
      TransientTrack* posTransTkPtr = 0;
      TransientTrack* negTransTkPtr = 0;

      // Look at the two tracks we're looping over.  If they're oppositely
      //  charged, load them into the hypothesized positive and negative tracks
      //  and references to be sent to the KalmanVertexFitter
      if(!isWrongSign && theTrackRefs[trdx1]->charge() < 0. && 
	 theTrackRefs[trdx2]->charge() > 0.) {
	negativeTrackRef = theTrackRefs[trdx1];
	positiveTrackRef = theTrackRefs[trdx2];
	negTransTkPtr = &theTransTracks[trdx1];
	posTransTkPtr = &theTransTracks[trdx2];
      }
      else if(!isWrongSign && theTrackRefs[trdx1]->charge() > 0. &&
	      theTrackRefs[trdx2]->charge() < 0.) {
	negativeTrackRef = theTrackRefs[trdx2];
	positiveTrackRef = theTrackRefs[trdx1];
	negTransTkPtr = &theTransTracks[trdx2];
	posTransTkPtr = &theTransTracks[trdx1];
      }
      else if(isWrongSign && theTrackRefs[trdx1]->charge() > 0. &&
              theTrackRefs[trdx2]->charge() > 0.) { 
        negativeTrackRef = theTrackRefs[trdx2];
        positiveTrackRef = theTrackRefs[trdx1];
        negTransTkPtr = &theTransTracks[trdx2];
        posTransTkPtr = &theTransTracks[trdx1];
      }
      else if(isWrongSign && theTrackRefs[trdx1]->charge() < 0. &&
              theTrackRefs[trdx2]->charge() < 0.) { 
        negativeTrackRef = theTrackRefs[trdx1];
        positiveTrackRef = theTrackRefs[trdx2];
        negTransTkPtr = &theTransTracks[trdx1];
        posTransTkPtr = &theTransTracks[trdx2];
      }

      // If they're not 2 oppositely charged tracks, loop back to the
      //  beginning and try the next pair.
      else continue;

      // Calculate DCA of two daughters
      double dzvtx_pos = positiveTrackRef->dz(bestvtx);
      double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
      double dzerror_pos = sqrt(positiveTrackRef->dzError()*positiveTrackRef->dzError()+zVtxError*zVtxError);
      double dxyerror_pos = sqrt(positiveTrackRef->d0Error()*positiveTrackRef->d0Error()+xVtxError*yVtxError);
      double dauLongImpactSig_pos = dzvtx_pos/dzerror_pos;
      double dauTransImpactSig_pos = dxyvtx_pos/dxyerror_pos;

      double dzvtx_neg = negativeTrackRef->dz(bestvtx);
      double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
      double dzerror_neg = sqrt(negativeTrackRef->dzError()*negativeTrackRef->dzError()+zVtxError*zVtxError);
      double dxyerror_neg = sqrt(negativeTrackRef->d0Error()*negativeTrackRef->d0Error()+xVtxError*yVtxError);
      double dauLongImpactSig_neg = dzvtx_neg/dzerror_neg;
      double dauTransImpactSig_neg = dxyvtx_neg/dxyerror_neg;

      double nhits_pos = positiveTrackRef->numberOfValidHits();
      double nhits_neg = negativeTrackRef->numberOfValidHits(); 
    
      double ptErr_pos = positiveTrackRef->ptError();
      double ptErr_neg = negativeTrackRef->ptError();

      double dedx_pos=-999.;
      double dedx_neg=-999.;
      // Extract dEdx
      if(dEdxHandle.isValid()){
        const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle.product();
        dedx_pos = dEdxTrack[positiveTrackRef].dEdx();
        dedx_neg = dEdxTrack[negativeTrackRef].dEdx();
      } 
      dedx_pos = dedx_pos;
      dedx_neg = dedx_neg;

      // Fill the vector of TransientTracks to send to KVF
      transTracks.push_back(*posTransTkPtr);
      transTracks.push_back(*negTransTkPtr);

      // Trajectory states to calculate DCA for the 2 tracks
      FreeTrajectoryState posState = posTransTkPtr->impactPointTSCP().theState();
      FreeTrajectoryState negState = negTransTkPtr->impactPointTSCP().theState();

      if( !posTransTkPtr->impactPointTSCP().isValid() || !negTransTkPtr->impactPointTSCP().isValid() ) continue;

      // Measure distance between tracks at their closest approach
      ClosestApproachInRPhi cApp;
      cApp.calculate(posState, negState);
      if( !cApp.status() ) continue;
      float dca =  cApp.distance();
      GlobalPoint cxPt = cApp.crossingPoint();

      // TrajectoryStateClosestToPoint posTsctp = posTransTkPtr->trajectoryStateClosestToPoint(bestvtx);
      // TrajectoryStateClosestToPoint negTsctp = negTransTkPtr->trajectoryStateClosestToPoint(bestvtx);

      // GlobalVector pospT = posTransTkPtr.impactPointTSCP().momentum();
      // GlobalVector negpT = negTransTkPtr.impactPointTSCP().momentum();
      // math::XYZVector deltaP(pospT.x() - negpT.x(), pospT.y() - negpT.y(), 0);

      // TwoTrackMinimumDistanceHelixHelix minDistCalculator;
      // minDistCalculator.calculate(posState.parameters(), negState.parameters());
      TwoTrackMinimumDistance minDistCalculator;
      minDistCalculator.calculate(posState, negState);
      dca = minDistCalculator.distance(); 
      // std::cout << "(pca,dca) : " << minDistCalculator.distance() << ", " << dca << std::endl;
      

      if (dca < tkDCACutLow || dca > tkDCACut) continue;
      if( dca < 0 ) std::cout << "Negative DCA : " << dca << std::endl;
//      if (sqrt( cxPt.x()*cxPt.x() + cxPt.y()*cxPt.y() ) > 120. 
//          || std::abs(cxPt.z()) > 300.) continue;

      // Get trajectory states for the tracks at POCA for later cuts
      TrajectoryStateClosestToPoint posTSCP =
        posTransTkPtr->trajectoryStateClosestToPoint( cxPt );
      TrajectoryStateClosestToPoint negTSCP =
        negTransTkPtr->trajectoryStateClosestToPoint( cxPt );

      if( !posTSCP.isValid() || !negTSCP.isValid() ) continue;

      double totalE1 = sqrt( posTSCP.momentum().mag2() + kaonMassD0Squared ) +
                      sqrt( negTSCP.momentum().mag2() + piMassD0Squared );
      double totalE1Sq = totalE1*totalE1;

      double totalE2 = sqrt( posTSCP.momentum().mag2() + piMassD0Squared ) +
                      sqrt( negTSCP.momentum().mag2() + kaonMassD0Squared );
      double totalE2Sq = totalE2*totalE2;

      double totalPSq =
        ( posTSCP.momentum() + negTSCP.momentum() ).mag2();

      auto sumMom = ( posTSCP.momentum() + negTSCP.momentum() );
      double totalPt = sumMom.perp();

      double mass1 = sqrt( totalE1Sq - totalPSq);
      double mass2 = sqrt( totalE2Sq - totalPSq);

      if( (mass1 > mPiKCutMax || mass1 < mPiKCutMin) && (mass2 > mPiKCutMax || mass2 < mPiKCutMin)) continue;
      if( totalPt < dPtCut ) continue;
      double totalY1 = 0.5 * log((totalE1 + totalPt* TMath::SinH(sumMom.eta()) )/(totalE1 - totalPt*TMath::SinH(sumMom.eta())));
      double totalY2 = 0.5 * log((totalE2 + totalPt* TMath::SinH(sumMom.eta()) )/(totalE2 - totalPt*TMath::SinH(sumMom.eta())));
      if( fabs(totalY1) > d0AbsYCut && fabs(totalY2) > d0AbsYCut ) continue;



      // Create the vertex fitter object and vertex the tracks
    
      float posCandTotalE[2]={0.0};
      float negCandTotalE[2]={0.0};
      float d0TotalE[2]={0.0};

      for(int i=0;i<2;i++)
      {
        //Creating a KinematicParticleFactory
        KinematicParticleFactoryFromTransientTrack pFactory;
        
        float chi = 0.0;
        float ndf = 0.0;

        vector<RefCountedKinematicParticle> d0Particles;
        d0Particles.push_back(pFactory.particle(*posTransTkPtr,posCandMass[i],chi,ndf,posCandMass_sigma[i]));
        d0Particles.push_back(pFactory.particle(*negTransTkPtr,negCandMass[i],chi,ndf,negCandMass_sigma[i]));

        KinematicParticleVertexFitter d0Fitter;
        RefCountedKinematicTree d0Vertex;
        d0Vertex = d0Fitter.fit(d0Particles);

        if( !d0Vertex->isValid() ) continue;

        d0Vertex->movePointerToTheTop();
        RefCountedKinematicParticle d0Cand = d0Vertex->currentParticle();
        if (!d0Cand->currentState().isValid()) continue;

        RefCountedKinematicVertex d0DecayVertex = d0Vertex->currentDecayVertex();
        if (!d0DecayVertex->vertexIsValid()) continue;

        //if ( d0DecayVertex->chiSquared()<0 || d0DecayVertex->chiSquared()>1000 ) continue;

        //float d0C2Prob =
        //   ChiSquaredProbability((double)(d0DecayVertex->chiSquared()),(double)(d0DecayVertex->degreesOfFreedom()));
        //if (d0C2Prob < 0.0001) continue;

	      float d0C2Prob = TMath::Prob(d0DecayVertex->chiSquared(),d0DecayVertex->degreesOfFreedom());
	      if (d0C2Prob < VtxChiProbCut) continue;

        //if ( d0Cand->currentState().mass() > 2.5 || d0Cand->currentState().mass() < 1.0) continue;

        d0Vertex->movePointerToTheFirstChild();
        RefCountedKinematicParticle posCand = d0Vertex->currentParticle();
        d0Vertex->movePointerToTheNextChild();
        RefCountedKinematicParticle negCand = d0Vertex->currentParticle();

        if(!posCand->currentState().isValid() || !negCand->currentState().isValid()) continue;

        KinematicParameters posCandKP = posCand->currentState().kinematicParameters();
        KinematicParameters negCandKP = negCand->currentState().kinematicParameters();

        GlobalVector d0TotalP = GlobalVector (d0Cand->currentState().globalMomentum().x(),
                                              d0Cand->currentState().globalMomentum().y(),
                                              d0Cand->currentState().globalMomentum().z());

        GlobalVector posCandTotalP = GlobalVector(posCandKP.momentum().x(),posCandKP.momentum().y(),posCandKP.momentum().z());
        GlobalVector negCandTotalP = GlobalVector(negCandKP.momentum().x(),negCandKP.momentum().y(),negCandKP.momentum().z());

        posCandTotalE[i] = sqrt( posCandTotalP.mag2() + posCandMass[i]*posCandMass[i] );
        negCandTotalE[i] = sqrt( negCandTotalP.mag2() + negCandMass[i]*negCandMass[i] );
        d0TotalE[i] = posCandTotalE[i] + negCandTotalE[i];

        const Particle::LorentzVector d0P4(d0TotalP.x(), d0TotalP.y(), d0TotalP.z(), d0TotalE[i]);

        Particle::Point d0Vtx((*d0DecayVertex).position().x(), (*d0DecayVertex).position().y(), (*d0DecayVertex).position().z());
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
        double d0NormalizedChi2 = d0VtxChi2/d0VtxNdof;

        double rVtxMag = 99999.0; 
        double lVtxMag = 99999.0;
        double sigmaRvtxMag = 999.0;
        double sigmaLvtxMag = 999.0;
        double d0Angle3D = -100.0;
        double d0Angle2D = -100.0;

        GlobalVector d0LineOfFlight = GlobalVector (d0Vtx.x() - xVtx,
                                                    d0Vtx.y() - yVtx,
                                                    d0Vtx.z() - zVtx);

        SMatrixSym3D d0TotalCov;
        if(isVtxPV) d0TotalCov = d0VtxCovMatrix + vtxPrimary->covariance();
        else d0TotalCov = d0VtxCovMatrix + theBeamSpotHandle->rotatedCovariance3D();

        SVector3 distanceVector3D(d0LineOfFlight.x(), d0LineOfFlight.y(), d0LineOfFlight.z());
        SVector3 distanceVector2D(d0LineOfFlight.x(), d0LineOfFlight.y(), 0.0);

        d0Angle3D = angle(d0LineOfFlight.x(), d0LineOfFlight.y(), d0LineOfFlight.z(),
                        d0TotalP.x(), d0TotalP.y(), d0TotalP.z());
        d0Angle2D = angle(d0LineOfFlight.x(), d0LineOfFlight.y(), (float)0.0,
                        d0TotalP.x(), d0TotalP.y(), (float)0.0);

        lVtxMag = d0LineOfFlight.mag();
        rVtxMag = d0LineOfFlight.perp();
        sigmaLvtxMag = sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector3D)) / lVtxMag;
        sigmaRvtxMag = sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector2D)) / rVtxMag;

        if( d0NormalizedChi2 > chi2Cut ||
            rVtxMag < rVtxCut ||
            rVtxMag / sigmaRvtxMag < rVtxSigCut ||
            lVtxMag < lVtxCut ||
            lVtxMag / sigmaLvtxMag < lVtxSigCut ||
            cos(d0Angle3D) < collinCut3D || cos(d0Angle2D) < collinCut2D || d0Angle3D > alphaCut || d0Angle2D > alpha2DCut
        ) continue;
        AnalyticalImpactPointExtrapolator extrapolator(magField);
        TrajectoryStateOnSurface tsos = extrapolator.extrapolate(d0Cand->currentState().freeTrajectoryState(), RecoVertex::convertPos(vtxPrimary->position()));;

	      if( !tsos.isValid() ) continue;
        Measurement1D cur3DIP;
        VertexDistance3D a3d;
        GlobalPoint refPoint          = tsos.globalPosition();
        GlobalError refPointErr       = tsos.cartesianError().position();
        GlobalPoint vertexPosition    = RecoVertex::convertPos(vtxPrimary->position());
        GlobalError vertexPositionErr = RecoVertex::convertError(vtxPrimary->error());
        cur3DIP =  (a3d.distance(VertexState(vertexPosition,vertexPositionErr), VertexState(refPoint, refPointErr)));
        // // Debugging part : cur3DIP and sin(alpha) * decaylength value is equal but the error different
        // std::cout << "By cur3DIP " << cur3DIP.value() << " +/- " << cur3DIP.error() <<std::endl;
        // std::cout << "By decay length and alpha " << std::sin(d0Angle3D)*lVtxMag << " +/- " << sigmaLvtxMag * std::sin(d0Angle3D) <<std::endl;
        // std::cout << "By decay length " << lVtxMag << " +/- " << sigmaLvtxMag * lVtxMag <<std::endl;

        FreeTrajectoryState posStateNew = posTransTkPtr->impactPointTSCP().theState();
        FreeTrajectoryState negStateNew = negTransTkPtr->impactPointTSCP().theState();
        ClosestApproachInRPhi cApp;
        cApp.calculate(posStateNew, negStateNew);
        if( !cApp.status() ) continue;
        float dca = fabs( cApp.distance() );
        TwoTrackMinimumDistance minDistCalculator;
        minDistCalculator.calculate(posState, negState);
        dca = minDistCalculator.distance(); 
        cxPt = minDistCalculator.crossingPoint();
        GlobalError posErr = posStateNew.cartesianError().position();
        GlobalError negErr = negStateNew.cartesianError().position();

        // DCA error propagation
        double sigma_x2 = posErr.cxx() + negErr.cxx();
        double sigma_y2 = posErr.cyy() + negErr.cyy();
    
        // Error in transverse plane (r-phi)
        double dcaError = sqrt(sigma_x2 * cxPt.x() * cxPt.x() + 
                               sigma_y2 * cxPt.y() * cxPt.y()) / dca;

        std::unique_ptr<CC> theD0 = std::make_unique<CC>();

        theD0->setP4(d0P4);
        // theD0 = new VertexCompositeCandidate(0, d0P4, d0Vtx, d0VtxCov, d0VtxChi2, d0VtxNdof);

        RecoChargedCandidate
          thePosCand(1, Particle::LorentzVector(posCandTotalP.x(),
                                                   posCandTotalP.y(), posCandTotalP.z(),
                                                   posCandTotalE[i]), d0Vtx);
        thePosCand.setTrack(positiveTrackRef);

        RecoChargedCandidate
          theNegCand(-1, Particle::LorentzVector(negCandTotalP.x(),
                                                   negCandTotalP.y(), negCandTotalP.z(),
                                                   negCandTotalE[i]), d0Vtx);
        theNegCand.setTrack(negativeTrackRef);

        if(isWrongSign)
        {
          thePosCand.setCharge(theTrackRefs[trdx1]->charge());
          theNegCand.setCharge(theTrackRefs[trdx1]->charge());
        }

        AddFourMomenta addp4;
        theD0->addDaughter(thePosCand, "posdau" );
        theD0->addDaughter(theNegCand, "negdau" );
        theD0->setPdgId(pdg_id[i]);
        reco::Vertex d0VtxObj = *d0DecayVertex;
        theD0->addUserData("Vtx", d0VtxObj);
        theD0->addUserFloat("VtxChi2", d0VtxChi2 );
        theD0->addUserFloat("VtxNdof", d0VtxNdof );
        theD0->addUserFloat("alpha2D", d0Angle2D );
        theD0->addUserFloat("alpha3D", d0Angle3D );
        theD0->addUserFloat("decaylength2D", rVtxMag);
        theD0->addUserFloat("decaylength3D", lVtxMag );
        theD0->addUserFloat("decaylengthsignif2D", rVtxMag/sigmaRvtxMag);
        theD0->addUserFloat("decaylengthsignif3D", lVtxMag/sigmaLvtxMag );
        theD0->addUserFloat("dca3D", cur3DIP.value());
        theD0->addUserFloat("dca3DErr", cur3DIP.error());
        theD0->addUserFloat("track3DDCA", dca);
        theD0->addUserFloat("track3DDCAErr", dcaError);

        addp4.set( *theD0 );
        if( theD0->mass() < d0MassD0 + d0MassCut &&
            theD0->mass() > d0MassD0 - d0MassCut ) //&&
        {

// perform MVA evaluation
          // GBR METHOD START

  //           //gbrVals_[0] = d0C2Prob;
  //           //gbrVals_[1] = cos(d0Angle3D);
  //           //gbrVals_[2] = d0Angle3D;
  //           //gbrVals_[3] = cos(d0Angle2D);
  //           //gbrVals_[4] = d0Angle2D;
  //           //gbrVals_[5] = lVtxMag;
  //           //gbrVals_[6] = lVtxMag / sigmaLvtxMag;
  //           //gbrVals_[7] = rVtxMag;
  //           //gbrVals_[8] = rVtxMag / sigmaRvtxMag;
  //           //gbrVals_[9] = posCandTotalP.perp();
  //           //gbrVals_[10] = posCandTotalP.eta();
  //           //gbrVals_[11] = negCandTotalP.perp();
  //           //gbrVals_[12] = negCandTotalP.eta();
  //           //gbrVals_[13] = ptErr_pos;
  //           //gbrVals_[14] = ptErr_neg;

  // GBR METHOD END
                if (useAnyMVA_ && onnxRuntime_) {
          // Prepare input data
          cms::Ort::FloatArrays data_;

          // auto start = high_resolution_clock::now();
          data_.emplace_back(19, 0);
          std::vector<float> &onnxVals_=data_[0];
          onnxVals_[0] = theD0->pt();
          onnxVals_[1] = theD0->y();;
          onnxVals_[2] = d0C2Prob;
          onnxVals_[3] = centrality;
          onnxVals_[4] = cos(d0Angle3D);
          onnxVals_[5] = d0Angle3D;
          onnxVals_[6] = cos(d0Angle2D);
          onnxVals_[7] = d0Angle2D;
          onnxVals_[8] = lVtxMag;
          onnxVals_[9] = lVtxMag / sigmaLvtxMag;
          onnxVals_[10] = rVtxMag;
          onnxVals_[11] = rVtxMag / sigmaRvtxMag;
          onnxVals_[12] = posCandTotalP.perp();
          onnxVals_[13] = posCandTotalP.eta();
          onnxVals_[14] = negCandTotalP.perp();
          onnxVals_[15] = negCandTotalP.eta();
          onnxVals_[16] = ptErr_pos;
          onnxVals_[17] = ptErr_neg;
          onnxVals_[18] = dca;

          // onnxVals_.push_back(theD0->pt());
          // onnxVals_.push_back(theD0->y());
          // onnxVals_.push_back(d0C2Prob);
          // onnxVals_.push_back(centrality);
          // onnxVals_.push_back(cos(d0Angle3D));
          // onnxVals_.push_back(d0Angle3D);
          // onnxVals_.push_back(cos(d0Angle2D));
          // onnxVals_.push_back(d0Angle2D);
          // onnxVals_.push_back(lVtxMag);
          // onnxVals_.push_back(lVtxMag / sigmaLvtxMag);
          // onnxVals_.push_back(rVtxMag);
          // onnxVals_.push_back(rVtxMag / sigmaRvtxMag);
          // onnxVals_.push_back(posCandTotalP.perp());
          // onnxVals_.push_back(posCandTotalP.eta());
          // onnxVals_.push_back(negCandTotalP.perp());
          // onnxVals_.push_back(negCandTotalP.eta());
          // onnxVals_.push_back(ptErr_pos);
          // onnxVals_.push_back(ptErr_neg);
          // onnxVals_.push_back(dca);
          // tmpD0s.push_back(theD0);

          // Create ONNX input tensor
        // cout << unput_names_.size() << " " << data_.size() << endl;
          std::vector<float> outputs = onnxRuntime_->run(input_names_, data_, input_shapes_,output_names_)[0];

          float onnxVal = outputs[1]; // Adjust if your model has multiple outputs
          //     std::cout << "input data -> ";
          // for (auto &i: onnxVals_) { std::cout << i << " "; }
          // std::cout << std::endl << "output data -> ";
          // for (auto &i: outputs) { std::cout << i << " "; }
          // std::cout << std::endl;
          // auto stop = high_resolution_clock::now();
          // auto duration = duration_cast<microseconds>(stop - start);
          // totaltime += duration.count();
          // nloop++;
          if (onnxVal > mvaCut) {
            theD0->addUserFloat("mva", onnxVal);
            mvaVals_.push_back(onnxVal);
            theD0s.push_back(std::move(*theD0));
          }
        }
  // if (useAnyMVA_ && !onnxRuntime_) {
  //           float gbrVals_[19];
  //           gbrVals_[0] = theD0->pt();
  //           gbrVals_[1] = theD0->y();;
  //           gbrVals_[2] = d0C2Prob;
  //           gbrVals_[3] = centrality;
  //           gbrVals_[4] = cos(d0Angle3D);
  //           gbrVals_[5] = d0Angle3D;
  //           gbrVals_[6] = cos(d0Angle2D);
  //           gbrVals_[7] = d0Angle2D;
  //           gbrVals_[8] = lVtxMag;
  //           gbrVals_[9] = lVtxMag / sigmaLvtxMag;
  //           gbrVals_[10] = rVtxMag;
  //           gbrVals_[11] = rVtxMag / sigmaRvtxMag;
  //           gbrVals_[12] = posCandTotalP.perp();
  //           gbrVals_[13] = posCandTotalP.eta();
  //           gbrVals_[14] = negCandTotalP.perp();
  //           gbrVals_[15] = negCandTotalP.eta();
  //           gbrVals_[16] = ptErr_pos;
  //           gbrVals_[17] = ptErr_neg;
  //           gbrVals_[18] = dca;
  //   theD0s.push_back(*theD0);

  //   GBRForest const * forest = forest_;
  //   if(useForestFromDB_){
  //     edm::ESHandle<GBRForest> forestHandle;
  //     forestHandle = iSetup.getHandle<GBRForest, GBRWrapperRcd>(mvaToken_);
  //     forest = forestHandle.product();
  //   }

  //   auto gbrVal = forest->GetClassifier(gbrVals_);

  //   float gbrresponse = gbrForest_->GetResponse(gbrVals_);
  //   if (gbrresponse > mvaCut) {
  //     theD0->addUserFloat("mva", onnxVal);
  //     mvaVals_.push_back(onnxVal);
  //     theD0s.push_back(*theD0);
  //   }
  // }
        else {
          theD0s.push_back(*theD0);
          }
        }
      }
    }
  }
  // if (useAnyMVA_ && onnxRuntime_ && data_[0].size() > 0) {
  //   // auto start = high_resolution_clock::now();
  //   // cout << input_names_.size() << " " << data_.size() << " " << input_shapes_.size() << " " << output_names_.size() << endl;
  //   int nInput = data_[0].size()/VARSIZE;
  //   input_shapes_[0] ={nInput, VARSIZE};
  //   auto fullOut = onnxRuntime_->run(input_names_, data_, input_shapes_,output_names_,nInput );
  //   // std::cout << "fullOut size: " << fullOut.size() << std::endl;
  //   // std::vector<float> fullOut_ =  fullOut[0];
  //   for( size_t idx = 0; idx < fullOut.size(); idx++){
  //     float& mvaVal = fullOut[idx][1];
  //     if(mvaVal > mvaCut) {
  //       tmpD0s[idx]->addUserFloat("mva", mvaVal);
  //       mvaVals_.push_back(mvaVal);
  //       theD0s.push_back(std::move(*tmpD0s[idx]));
  //     }
  //     if(tmpD0s[idx]) delete tmpD0s[idx];
  //   }
  //   // auto stop = high_resolution_clock::now();
  //   // auto duration = duration_cast<microseconds>(stop - start);
  //   // cout << "onnxRuntime_->run() time: " << duration.count() << " microseconds" << endl;
  // }
    // float onnxVal = outputs[1]; // Adjust if your model has multiple outputs

  // std::cout << "Total time for MVA: " << totaltime << " microseconds" << std::endl;
  // std::cout << "Number of loops: " << nloop << std::endl;
  // std::cout << "Average time per loop: " << (totaltime/nloop) << " microseconds" << std::endl;

//  mvaFiller.insert(theD0s,mvaVals_.begin(),mvaVals_.end());
//  mvaFiller.fill();
//  mvas = std::make_unique<MVACollection>(mvaVals_.begin(),mvaVals_.end());

}
// Get methods

const CCC& D0Fitter::getD0() const {
  return theD0s;
}

const std::vector<float>& D0Fitter::getMVAVals() const {
  return mvaVals_;
}

/*
auto_ptr<edm::ValueMap<float> > D0Fitter::getMVAMap() const {
  return mvaValValueMap;
}
*/

void D0Fitter::resetAll() {
    theD0s.clear();
    mvaVals_.clear();
}
