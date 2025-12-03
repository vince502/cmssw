// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      DStarFitter
// 
/**\class DStarFitter DStarFitter.cc VertexCompositeAnalysis/VertexCompositeProducer/src/DStarFitter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
//
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DStarFitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"

#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

// for DCA
#include "TrackingTools/GeomPropagators/interface/AnalyticalImpactPointExtrapolator.h"
#include "TrackingTools/PatternTools/interface/TransverseImpactPointExtrapolator.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackFromFTSFactory.h"

#include "TrackingTools/IPTools/interface/IPTools.h"
#include "RecoVertex/VertexPrimitives/interface/ConvertToFromReco.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"
#include "DataFormats/PatCandidates/interface/GenericParticle.h"


#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TVector3.h>
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "CondFormats/DataRecord/interface/GBRWrapperRcd.h"

static const float piMassDStar = 0.13957018;
static const float piMassDStarSquared = piMassDStar*piMassDStar;
static const float dStarMassDStar = 2.010000;
static float piMassDStar_sigma = 3.5E-7f;
static float D0MassD0_sigma = 1.6E-4f;
static float dStarMassDStar_sigma = dStarMassDStar*1.e-6;


// Constructor and (empty) destructor
DStarFitter::DStarFitter(const edm::ParameterSet& theParameters,  edm::ConsumesCollector && iC) :
    bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>())
{
  using std::string;

  // Get the track reco algorithm from the ParameterSet
  token_beamSpot = iC.consumes<reco::BeamSpot>(edm::InputTag("offlineBeamSpot"));
  token_d0cand = iC.consumes<CCC>(theParameters.getParameter<edm::InputTag>("d0Collection"));
  token_tracks = iC.consumes<reco::TrackCollection>(theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  token_vertices = iC.consumes<reco::VertexCollection>(theParameters.getParameter<edm::InputTag>("vertexRecoAlgorithm"));
  token_dedx = iC.consumes<edm::ValueMap<reco::DeDxData> >(edm::InputTag("dedxHarmonic2"));

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
  dStarMassCut = theParameters.getParameter<double>(string("dStarMassCut"));
  dauTransImpactSigCut = theParameters.getParameter<double>(string("dauTransImpactSigCut"));
  dauLongImpactSigCut = theParameters.getParameter<double>(string("dauLongImpactSigCut"));
  VtxChiProbCut = theParameters.getParameter<double>(string("VtxChiProbCut"));
  dPtCut = theParameters.getParameter<double>(string("dPtCut"));
  alphaCut = theParameters.getParameter<double>(string("alphaCut"));
  alpha2DCut = theParameters.getParameter<double>(string("alpha2DCut"));
  isWrongSign = theParameters.getParameter<bool>(string("isWrongSign"));


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
  }

  std::vector<std::string> qual = theParameters.getParameter<std::vector<std::string> >("trackQualities");
  for (unsigned int ndx = 0; ndx < qual.size(); ndx++) {
    qualities.push_back(reco::TrackBase::qualityByName(qual[ndx]));
  }
}

DStarFitter::~DStarFitter() {
  delete forest_;
}

// Method containing the algorithm for vertex reconstruction
void DStarFitter::fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

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
  std::vector<pat::GenericParticleRef> theD0CandRefs;

  // Handles for tracks, B-field, and tracker geometry
  Handle<reco::TrackCollection> theTrackHandle;
  Handle<reco::VertexCollection> theVertexHandle;
  Handle<CCC> theD0Handle;
  Handle<reco::BeamSpot> theBeamSpotHandle;
  ESHandle<MagneticField> bFieldHandle;
  Handle<edm::ValueMap<reco::DeDxData> > dEdxHandle;

  // Get the tracks, vertices from the event, and get the B-field record
  //  from the EventSetup
  iEvent.getByToken(token_tracks, theTrackHandle); 
  iEvent.getByToken(token_vertices, theVertexHandle);
  iEvent.getByToken(token_d0cand, theD0Handle);
  iEvent.getByToken(token_beamSpot, theBeamSpotHandle);  
  iEvent.getByToken(token_dedx, dEdxHandle);


  if( !theTrackHandle->size() ) return;
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  magField = bFieldHandle.product();

  //needed for IP error
  AnalyticalImpactPointExtrapolator extrapolator(magField);
  TrajectoryStateOnSurface tsos;

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
  // for(unsigned int idx = 0; idx < theD0Handle->size(); idx ++){
  //   pat::GenericParticleRef tmpRef ( theD0Handle, idx);
  //   theD0CandRefs.push_back( tmpRef );
  // }

  //float posCandMass[2] = {piMassDStar, kaonMassD0};
  //float negCandMass[2] = {kaonMassD0, piMassDStar};
  //float posCandMass_sigma[2] = {piMassDStar_sigma, kaonMassD0_sigma};
  //float negCandMass_sigma[2] = {kaonMassD0_sigma, piMassDStar_sigma};
  //int   pdg_id[2] = {421, -421};

  // Loop over tracks and vertex good charged track pairs
  for(unsigned int didx1 = 0; didx1 < theD0Handle->size(); didx1++) {

    for(unsigned int trdx1 = 0; trdx1 < theTrackRefs.size(); trdx1++) {

      // Not using this on Dstar fit (1)
      // if( (theTrackRefs[didx1]->pt() + theTrackRefs[trdx2]->pt()) < tkPtSumCut) continue;
      // if( abs(theTrackRefs[didx1]->eta() - theTrackRefs[trdx2]->eta()) > tkEtaDiffCut) continue;

      //This vector holds the 3 tracks (K + pi) +pi to be vertexed
      std::vector<TransientTrack> transTracks;

      TrackRef pionTrackRef = theTrackRefs[trdx1];
      TransientTrack* pionTransTkPtr = 0;
      pionTransTkPtr = &theTransTracks[trdx1];
      CC theD0 = (*theD0Handle)[didx1];

      // if( !pionTransTkPtr->impactPointStateAvailable()) continue;
      const auto& D0Vec = theD0.p4();
      const reco::Track& thePiTrack = pionTransTkPtr->track();
      math::PtEtaPhiMLorentzVector pPi(thePiTrack.pt(), thePiTrack.eta(), thePiTrack.phi(), piMassDStar);
      double theDStarcandMass = (D0Vec + pPi).M();
      // std::cout << "D* - D0 mass : " << theDStarcandMass << ", " << D0Vec.M() << std::endl;
      if(theDStarcandMass - D0Vec.M() >0.16) continue;
      

      // Calculate DCA of two daughters
//      double dzvtx_pos = positiveTrackRef->dz(bestvtx);
//      double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
//      double dzerror_pos = sqrt(positiveTrackRef->dzError()*positiveTrackRef->dzError()+zVtxError*zVtxError);
//      double dxyerror_pos = sqrt(positiveTrackRef->d0Error()*positiveTrackRef->d0Error()+xVtxError*yVtxError);
//      double dauLongImpactSig_pos = dzvtx_pos/dzerror_pos;
//      double dauTransImpactSig_pos = dxyvtx_pos/dxyerror_pos;
//
//      double dzvtx_neg = negativeTrackRef->dz(bestvtx);
//      double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
//      double dzerror_neg = sqrt(negativeTrackRef->dzError()*negativeTrackRef->dzError()+zVtxError*zVtxError);
//      double dxyerror_neg = sqrt(negativeTrackRef->d0Error()*negativeTrackRef->d0Error()+xVtxError*yVtxError);
//      double dauLongImpactSig_neg = dzvtx_neg/dzerror_neg;
//      double dauTransImpactSig_neg = dxyvtx_neg/dxyerror_neg;
//
//      double nhits_pos = positiveTrackRef->numberOfValidHits();
//      double nhits_neg = negativeTrackRef->numberOfValidHits(); 
//    
//      double ptErr_pos = positiveTrackRef->ptError();
//      double ptErr_neg = negativeTrackRef->ptError();
//
      // Extract dEdx for slow pion
      double dedx_slowPion = -999.;
      if(dEdxHandle.isValid()){
        const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle.product();
        dedx_slowPion = dEdxTrack[pionTrackRef].dEdx();
      }

//      // Fill the vector of TransientTracks to send to KVF
//      transTracks.push_back(*posTransTkPtr);
//      transTracks.push_back(*negTransTkPtr);

      // Trajectory states to calculate DCA for the 2 tracks
//      FreeTrajectoryState posState = posTransTkPtr->impactPointTSCP().theState();
//      FreeTrajectoryState negState = pionTransTkPtr->impactPointTSCP().theState();
//
//      if( !posTransTkPtr->impactPointTSCP().isValid() || !negTransTkPtr->impactPointTSCP().isValid() ) continue;
//
//      // Measure distance between tracks at their closest approach
//      ClosestApproachInRPhi cApp;
//      cApp.calculate(posState, negState);
//      if( !cApp.status() ) continue;
//      float dca = fabs( cApp.distance() );
//      GlobalPoint cxPt = cApp.crossingPoint();
//
//      if (dca < 0. || dca > tkDCACut) continue;
//
//      // Get trajectory states for the tracks at POCA for later cuts
//      TrajectoryStateClosestToPoint posTSCP = posTransTkPtr->trajectoryStateClosestToPoint( cxPt );
//      TrajectoryStateClosestToPoint negTSCP = negTransTkPtr->trajectoryStateClosestToPoint( cxPt );
//
//      if( !posTSCP.isValid() || !negTSCP.isValid() ) continue;
//
//      if( (mass1 > mPiKCutMax || mass1 < mPiKCutMin) && (mass2 > mPiKCutMax || mass2 < mPiKCutMin)) continue;
//      if( totalPt < dPtCut ) continue;


       float chi = 0.0;
       float ndf = 0.0;

       //Creating a KinematicParticleFactory
       KinematicParticleFactoryFromTransientTrack pFactory;
       vector<RefCountedKinematicParticle> d0Daus;
       reco::Candidate* dau0 = theD0.daughter(0);
       reco::Candidate* dau1 = theD0.daughter(1);
       int slowPionCharge = pionTrackRef->charge();

       reco::Candidate* kaonCand = nullptr;
       [[maybe_unused]] reco::Candidate* pionCand = nullptr;


       if (dau0->mass() > dau1->mass()) {
               kaonCand = dau0;
               pionCand = dau1;
       } else {
               kaonCand = dau1;
               pionCand = dau0;
       }
       

       

       // For D*+: K- pi+ followed by slow pi+
       // For D*-: K+ pi- followed by slow pi-
       if (slowPionCharge > 0) { // D*+ case
               if (kaonCand->charge() > 0) continue;
       } else { 
               if (kaonCand->charge() < 0) continue;
       }



       reco::TransientTrack ttk0(*dau0->bestTrack(), magField);
       reco::TransientTrack ttk1(*dau1->bestTrack(), magField);
       float dau0mass =  dau0->mass();
       float dau1mass =  dau1->mass();
       d0Daus.push_back(pFactory.particle(ttk0,dau0mass,chi,ndf,D0MassD0_sigma));
       d0Daus.push_back(pFactory.particle(ttk1,dau1mass,chi,ndf,D0MassD0_sigma));

       KinematicParticleVertexFitter kpvFitter;
       RefCountedKinematicTree d0Tree =  kpvFitter.fit(d0Daus);
      if( !d0Tree->isValid() ) continue;

       d0Tree->movePointerToTheTop();

       vector<RefCountedKinematicParticle> dStarParticles;
       dStarParticles.push_back(d0Tree->currentParticle());
       dStarParticles.push_back(pFactory.particle(*pionTransTkPtr,piMassDStar,chi,ndf,piMassDStar_sigma));

       KinematicParticleVertexFitter dStarFitter;
       RefCountedKinematicTree dStarVertex;
       dStarVertex = dStarFitter.fit(dStarParticles);

       if( !dStarVertex->isValid() ) continue;

       dStarVertex->movePointerToTheTop();
       RefCountedKinematicParticle dStarCand = dStarVertex->currentParticle();
       if (!dStarCand->currentState().isValid()) continue;

       RefCountedKinematicVertex dStarDecayVertex = dStarVertex->currentDecayVertex();
       if (!dStarDecayVertex->vertexIsValid()) continue;

	     float dStarC2Prob = TMath::Prob(dStarDecayVertex->chiSquared(),dStarDecayVertex->degreesOfFreedom());
	     if (dStarC2Prob < VtxChiProbCut) continue;

       dStarVertex->movePointerToTheFirstChild();
       RefCountedKinematicParticle posCand = dStarVertex->currentParticle();
       dStarVertex->movePointerToTheNextChild();
       RefCountedKinematicParticle negCand = dStarVertex->currentParticle();

       if(!posCand->currentState().isValid() || !negCand->currentState().isValid()) continue;

       KinematicParameters posCandKP = posCand->currentState().kinematicParameters();
       KinematicParameters negCandKP = negCand->currentState().kinematicParameters();

       TwoTrackMinimumDistance minDistCalculator;
       //minDistCalculator.calculate( posCand->currentState().trajectoryParameters(),negCand->currentState().trajectoryParameter() );
       //float dca = minDistCalculator.distance();
       //GlobalPoint cxPt = minDistCalculator.crossingPoint();
       //GlobalError posErr = posCand->currentState().trajectoryParameters().cartesianError().position();
       //GlobalError negErr = negCand->currentState().trajectoryParameters().cartesianError().position();

       // DCA error propagation
       //double sigma_x2 = posErr.cxx() + negErr.cxx();
       //double sigma_y2 = posErr.cyy() + negErr.cyy();
       //float dcaError = sqrt(sigma_x2 * cxPt.x() * cxPt.x() +
       //                sigma_y2 * cxPt.y() * cxPt.y()) / dca;
    
       //cout << "dca : " << dca << "dcaerr : " << dcaError << endl;




       GlobalVector dStarTotalP = GlobalVector (dStarCand->currentState().globalMomentum().x(),
                       dStarCand->currentState().globalMomentum().y(),
                       dStarCand->currentState().globalMomentum().z());

       GlobalVector posCandTotalP = GlobalVector(posCandKP.momentum().x(),posCandKP.momentum().y(),posCandKP.momentum().z());
       GlobalVector negCandTotalP = GlobalVector(negCandKP.momentum().x(),negCandKP.momentum().y(),negCandKP.momentum().z());

       float posCandTotalE = sqrt( posCandTotalP.mag2() + theD0.mass()*theD0.mass() );
       float negCandTotalE = sqrt( negCandTotalP.mag2() + piMassDStar*piMassDStar );
       float dStarTotalE = posCandTotalE + negCandTotalE;

       const Particle::LorentzVector dStarP4(dStarTotalP.x(), dStarTotalP.y(), dStarTotalP.z(), dStarTotalE);

       Particle::Point dStarVtx((*dStarDecayVertex).position().x(), (*dStarDecayVertex).position().y(), (*dStarDecayVertex).position().z());
       std::vector<double> dStarVtxEVec;
       dStarVtxEVec.push_back( dStarDecayVertex->error().cxx() );
       dStarVtxEVec.push_back( dStarDecayVertex->error().cyx() );
       dStarVtxEVec.push_back( dStarDecayVertex->error().cyy() );
       dStarVtxEVec.push_back( dStarDecayVertex->error().czx() );
       dStarVtxEVec.push_back( dStarDecayVertex->error().czy() );
       dStarVtxEVec.push_back( dStarDecayVertex->error().czz() );
       SMatrixSym3D dStarVtxCovMatrix(dStarVtxEVec.begin(), dStarVtxEVec.end());
       const Vertex::CovarianceMatrix dStarVtxCov(dStarVtxCovMatrix);
       double dStarVtxChi2(dStarDecayVertex->chiSquared());
       double dStarVtxNdof(dStarDecayVertex->degreesOfFreedom());
       double dStarNormalizedChi2 = dStarVtxChi2/dStarVtxNdof;

       double rVtxMag = 99999.0; 
       double lVtxMag = 99999.0;
       double sigmaRvtxMag = 999.0;
       double sigmaLvtxMag = 999.0;
       double dStarAngle3D = -100.0;
       double dStarAngle2D = -100.0;

       GlobalVector dStarLineOfFlight = GlobalVector (dStarVtx.x() - xVtx,
                                                   dStarVtx.y() - yVtx,
                                                   dStarVtx.z() - zVtx);

       SMatrixSym3D dStarTotalCov;
       if(isVtxPV) dStarTotalCov = dStarVtxCovMatrix + vtxPrimary->covariance();
       else dStarTotalCov = dStarVtxCovMatrix + theBeamSpotHandle->rotatedCovariance3D();

       SVector3 distanceVector3D(dStarLineOfFlight.x(), dStarLineOfFlight.y(), dStarLineOfFlight.z());
       SVector3 distanceVector2D(dStarLineOfFlight.x(), dStarLineOfFlight.y(), 0.0);

       dStarAngle3D = angle(dStarLineOfFlight.x(), dStarLineOfFlight.y(), dStarLineOfFlight.z(),
                       dStarTotalP.x(), dStarTotalP.y(), dStarTotalP.z());
       dStarAngle2D = angle(dStarLineOfFlight.x(), dStarLineOfFlight.y(), (float)0.0,
                       dStarTotalP.x(), dStarTotalP.y(), (float)0.0);

       lVtxMag = dStarLineOfFlight.mag();
       rVtxMag = dStarLineOfFlight.perp();
       sigmaLvtxMag = sqrt(ROOT::Math::Similarity(dStarTotalCov, distanceVector3D)) / lVtxMag;
       sigmaRvtxMag = sqrt(ROOT::Math::Similarity(dStarTotalCov, distanceVector2D)) / rVtxMag;

       // DCA error
       tsos = extrapolator.extrapolate(dStarCand->currentState().freeTrajectoryState(), RecoVertex::convertPos(vtxPrimary->position()));
       Measurement1D cur3DIP;
       VertexDistance3D a3d;
       GlobalPoint refPoint          = tsos.globalPosition();
       GlobalError refPointErr       = tsos.cartesianError().position();
       GlobalPoint vertexPosition    = RecoVertex::convertPos(vtxPrimary->position());
       GlobalError vertexPositionErr = RecoVertex::convertError(vtxPrimary->error());
       cur3DIP =  (a3d.distance(VertexState(vertexPosition,vertexPositionErr), VertexState(refPoint, refPointErr)));

       if( dStarNormalizedChi2 > chi2Cut ||
           rVtxMag < rVtxCut ||
           rVtxMag / sigmaRvtxMag < rVtxSigCut ||
           lVtxMag < lVtxCut ||
           lVtxMag / sigmaLvtxMag < lVtxSigCut ||
           cos(dStarAngle3D) < collinCut3D || cos(dStarAngle2D) < collinCut2D || dStarAngle3D > alphaCut || dStarAngle2D > alpha2DCut
       ) continue;



       CC* theDStar = 0;
      //  theDStar = new VertexCompositeCandidate(theTrackRefs[trdx1]->charge(), dStarP4, dStarVtx, dStarVtxCov, dStarVtxChi2, dStarVtxNdof);
      theDStar = new CC();
      theDStar->setP4(dStarP4);

       RecoChargedCandidate
         theNegCand(theTrackRefs[trdx1]->charge(), Particle::LorentzVector(negCandTotalP.x(),
                                                  negCandTotalP.y(), negCandTotalP.z(),
                                                  negCandTotalE), dStarVtx);
       theNegCand.setTrack(pionTrackRef);




      //  AddFourMomenta addp4;
       theDStar->addDaughter(theD0, "D0");
       theDStar->addDaughter(theNegCand, "pion");
       int pdgId = (int) theTrackRefs[trdx1]->charge() * 413;
       theDStar->setPdgId(pdgId);
        reco::Vertex dStarVtxObj = *dStarDecayVertex;
        theDStar->addUserData("Vtx", dStarVtxObj);
        theDStar->addUserFloat("VtxChi2", dStarVtxChi2 );
        theDStar->addUserFloat("VtxNdof", dStarVtxNdof );
        theDStar->addUserFloat("alpha2D", dStarAngle2D );
        theDStar->addUserFloat("alpha3D", dStarAngle3D );
        theDStar->addUserFloat("decaylength2D", rVtxMag);
        theDStar->addUserFloat("decaylength3D", lVtxMag );
        theDStar->addUserFloat("decaylengthsignif2D", rVtxMag/sigmaRvtxMag);
        theDStar->addUserFloat("decaylengthsignif3D", lVtxMag/sigmaLvtxMag );
        theDStar->addUserFloat("dca3D", cur3DIP.value());
        theDStar->addUserFloat("dca3DErr", cur3DIP.error());
        theDStar->addUserFloat("slowPionDeDx", dedx_slowPion);
        if(theD0.hasUserFloat("mva")) theDStar->addUserFloat("D0mva", theD0.userFloat("mva"));
        if(theD0.hasUserFloat("posDauDeDx")) theDStar->addUserFloat("D0posDauDeDx", theD0.userFloat("posDauDeDx"));
        if(theD0.hasUserFloat("negDauDeDx")) theDStar->addUserFloat("D0negDauDeDx", theD0.userFloat("negDauDeDx"));
//        theDStar->addUserFloat("D03DDCA", dca);
//        theDStar->addUserFloat("D03DDCAErr", dcaError);
      //  addp4.set( *theDStar );
       if( theDStar->mass() < dStarMassDStar + dStarMassCut &&
           theDStar->mass() > dStarMassDStar - dStarMassCut ) 
       {
         theDStars.push_back( *theDStar );
         dcaVals_.push_back(cur3DIP.value());
         dcaErrs_.push_back(cur3DIP.error());
//if(theDStar->pt()<4){cout <<"Dstar pt : " <<theDStar->pt()<<endl;}

// per//form MVA evaluation
         if(useAnyMVA_)
         {
      //    //   float gbrVals_[20];
      //    //   gbrVals_[0] = d0P4.Pt();
      //    //   gbrVals_[1] = d0P4.Eta();
      //    //   gbrVals_[2] = d0C2Prob;
      //    //   gbrVals_[3] = lVtxMag / sigmaLvtxMag;
      //    //   gbrVals_[4] = rVtxMag / sigmaRvtxMag;
      //    //   gbrVals_[5] = lVtxMag;
      //    //   gbrVals_[6] = d0Angle3D;
      //    //   gbrVals_[7] = d0Angle2D;
      //    //   gbrVals_[8] = dauLongImpactSig_pos;
      //    //   gbrVals_[9] = dauLongImpactSig_neg;
      //    //   gbrVals_[10] = dauTransImpactSig_pos;
      //    //   gbrVals_[11] = dauTransImpactSig_neg;
      //    //   gbrVals_[12] = nhits_pos;
      //    //   gbrVals_[13] = nhits_neg;
      //    //   gbrVals_[14] = ptErr_pos;
      //    //   gbrVals_[15] = ptErr_neg;
      //    //   gbrVals_[16] = posCandTotalP.perp();
      //    //   gbrVals_[17] = negCandTotalP.perp();
      //    //   gbrVals_[18] = posCandTotalP.eta();
      //    //   gbrVals_[19] = negCandTotalP.eta();

      //    //   GBRForest const * forest = forest_;
      //    //   if(useForestFromDB_){
      //    //     edm::ESHandle<GBRForest> forestHandle;
      //    //     iSetup.get<GBRWrapperRcd>().get(forestLabel_,forestHandle);
      //    //     forest = forestHandle.product();
      //    //   }

      //    //   auto gbrVal = forest->GetClassifier(gbrVals_);
      //    //   mvaVals_.push_back(gbrVal);
         }
       }

       if(theDStar) delete theDStar;
      }
  }

//  mvaFiller.insert(theDStars,mvaVals_.begin(),mvaVals_.end());
//  mvaFiller.fill();
//  mvas = std::make_unique<MVACollection>(mvaVals_.begin(),mvaVals_.end());

}
// Get methods

const pat::CompositeCandidateCollection& DStarFitter::getDStar() const {
  return theDStars;
}

const std::vector<float>& DStarFitter::getDCAVals() const{
  return dcaVals_;
}

const std::vector<float>& DStarFitter::getDCAErrs() const{
  return dcaErrs_;
}

const std::vector<float>& DStarFitter::getMVAVals() const {
  return mvaVals_;
}

/*
auto_ptr<edm::ValueMap<float> > DStarFitter::getMVAMap() const {
  return mvaValValueMap;
}
*/

void DStarFitter::resetAll() {
    theDStars.clear();
    mvaVals_.clear();
    dcaVals_.clear();
    dcaErrs_.clear();
}
