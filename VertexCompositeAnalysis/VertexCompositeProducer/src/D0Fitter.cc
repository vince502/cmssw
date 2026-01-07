// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D0Fitter
//
/**\class D0Fitter D0Fitter.cc
 VertexCompositeAnalysis/VertexCompositeProducer/src/D0Fitter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
//
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Fitter.h"
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/GlobalError.h"

#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include <Math/Functions.h>
#include <Math/SMatrix.h>
#include <Math/SVector.h>
#include <TMath.h>
#include <TVector3.h>
#include <algorithm>
#include <numeric>

const size_t VARSIZE = 17; // Number of ONNX input features
const float piMassD0 = 0.13957018;
const float piMassD0Squared = piMassD0 * piMassD0;
const float kaonMassD0 = 0.493677;
const float kaonMassD0Squared = kaonMassD0 * kaonMassD0;
const float d0MassD0 = 1.86484;
float piMassD0_sigma = 3.5E-7f;
float kaonMassD0_sigma = 1.6E-5f;
float d0MassD0_sigma = d0MassD0 * 1.e-6;

using CC = pat::CompositeCandidate;
using CCC = pat::CompositeCandidateCollection;
// using cms;

// Constructor and (empty) destructor
D0Fitter::D0Fitter(const edm::ParameterSet &theParameters,
                   edm::ConsumesCollector &&iC, const ONNXRuntime *onnxRuntime)
    : bField_esToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>()),
      onnxRuntime_(onnxRuntime), input_shapes_() {
  //		   const edm::Event& iEvent, const edm::EventSetup& iSetup,
  // edm::ConsumesCollector && iC) {
  using std::string;

  // Get the track reco algorithm from the ParameterSet
  token_beamSpot =
      iC.consumes<reco::BeamSpot>(edm::InputTag("offlineBeamSpot"));
  token_tracks = iC.consumes<reco::TrackCollection>(
      theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  token_vertices = iC.consumes<reco::VertexCollection>(
      theParameters.getParameter<edm::InputTag>("vertexRecoAlgorithm"));

  // dEdx source - configurable, empty InputTag means disabled
  // Also need track->PackedCandidate mapping for MiniAOD dEdx lookup
  edm::InputTag dedxTag = theParameters.getParameter<edm::InputTag>("dedxSrc");
  useDeDx_ = !dedxTag.label().empty();
  if (useDeDx_) {
    token_dedx = iC.consumes<edm::ValueMap<reco::DeDxData>>(dedxTag);
    // Get track to PackedCandidate mapping (produced by
    // unpackedTracksAndVertices)
    token_track2pc = iC.consumes<std::vector<edm::Ptr<pat::PackedCandidate>>>(
        theParameters.getParameter<edm::InputTag>("trackRecoAlgorithm"));
  }
  tok_centBinLabel_ =
      iC.consumes<int>(edm::InputTag("centralityBin", "HFtowers"));

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
  rVtxSigCut =
      theParameters.getParameter<double>(string("vtxSignificance2DCut"));
  lVtxCut = theParameters.getParameter<double>(string("lVtxCut"));
  lVtxSigCut =
      theParameters.getParameter<double>(string("vtxSignificance3DCut"));
  collinCut2D = theParameters.getParameter<double>(string("collinearityCut2D"));
  collinCut3D = theParameters.getParameter<double>(string("collinearityCut3D"));
  d0MassCut = theParameters.getParameter<double>(string("d0MassCut"));
  d0AbsYCut = theParameters.getParameter<double>(string("d0AbsYCut"));
  dauTransImpactSigCut =
      theParameters.getParameter<double>(string("dauTransImpactSigCut"));
  dauLongImpactSigCut =
      theParameters.getParameter<double>(string("dauLongImpactSigCut"));
  VtxChiProbCut = theParameters.getParameter<double>(string("VtxChiProbCut"));
  dPtCut = theParameters.getParameter<double>(string("dPtCut"));
  alphaCut = theParameters.getParameter<double>(string("alphaCut"));
  alpha2DCut = theParameters.getParameter<double>(string("alpha2DCut"));
  isWrongSign = theParameters.getParameter<bool>(string("isWrongSign"));
  mvaCut = theParameters.getParameter<double>(string("mvaCut"));

  useAnyMVA_ = false;
  forestLabel_ = "D0InPbPb";
  label_MVA_ = ""; 
  std::string type = "BDT";
  useForestFromDB_ = true;
  dbFileName_ = "";

  forest_ = nullptr;

  if (theParameters.exists("useAnyMVA")){
    useAnyMVA_ = theParameters.getParameter<bool>("useAnyMVA");
    if (theParameters.exists("label_MVA"))
      label_MVA_ = theParameters.getParameter<std::string>("label_MVA");
  }

  // if(useAnyMVA_){
  //   if(theParameters.exists("mvaType"))type =
  //   theParameters.getParameter<std::string>("mvaType");
  //   if(theParameters.exists("GBRForestLabel"))forestLabel_ =
  //   theParameters.getParameter<std::string>("GBRForestLabel");
  //   if(theParameters.exists("GBRForestFileName")){
  //     dbFileName_ =
  //     theParameters.getParameter<std::string>("GBRForestFileName");
  //     useForestFromDB_ = false;
  //   }

  //  if(!useForestFromDB_){
  //    edm::FileInPath
  //    fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s",dbFileName_.c_str()));
  //    TFile gbrfile(fip.fullPath().c_str(),"READ");
  //    forest_ = (GBRForest*)gbrfile.Get(forestLabel_.c_str());
  //    gbrfile.Close();
  //  }

  //  mvaType_ = type;
  //  mvaToken_ = iC.esConsumes<GBRForest, GBRWrapperRcd>(edm::ESInputTag("",
  //  forestLabel_));
  //}
  if (useAnyMVA_) {
    if (theParameters.exists("input_names") ||
        theParameters.exists("output_names")) {
      input_names_ =
          theParameters.getParameter<std::vector<std::string>>("input_names");
      output_names_ =
          theParameters.getParameter<std::vector<std::string>>("output_names");
    } else {
      throw cms::Exception("Configuration")
          << "onnxModelFileName not provided in ParameterSet";
    }
    //  Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "D0Fitter");
    //    Ort::SessionOptions sessionOptions;
    //    sessionOptions.SetIntraOpNumThreads(1); // Single-threaded for
    //    simplicity
    // edm::FileInPath
    // fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s",onnxModelPath_.c_str()));
    // // Path relative to CMSSW_BASE onnxModel_ =
    // std::make_unique<cms::Ort::ONNXRuntime>(fip.fullPath()); Retrieve input
    // and output names
    // Ort::AllocatorWithDefaultOptions allocator;
    // auto numInputNodes = onnxSession_->GetInputCount();
    // for (size_t i = 0; i < numInputNodes; i++) {
    //  auto inputName = onnxSession_->GetInputNameAllocated(i, allocator);
    //  inputNames_.push_back(inputName.get());
    //}
    // auto numOutputNodes = onnxSession_->GetOutputCount();
    // for (size_t i = 0; i < numOutputNodes; i++) {
    //  auto outputName = onnxSession_->GetOutputNameAllocated(i, allocator);
    //  outputNames_.push_back(outputName.get());
    //}
  }

  std::vector<std::string> qual =
      theParameters.getParameter<std::vector<std::string>>("trackQualities");
  for (unsigned int ndx = 0; ndx < qual.size(); ndx++) {
    qualities.push_back(reco::TrackBase::qualityByName(qual[ndx]));
  }
}

D0Fitter::~D0Fitter() { delete forest_; }

// Method containing the algorithm for vertex reconstruction
void D0Fitter::fitAll(const edm::Event &iEvent, const edm::EventSetup &iSetup) {

  using std::cout;
  using std::endl;
  using std::vector;
  using namespace reco;
  using namespace edm;
  using namespace std;

  typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3>>
      SMatrixSym3D;
  typedef ROOT::Math::SVector<double, 3> SVector3;

  // Create std::vectors for Tracks and TrackRefs (required for
  //  passing to the KalmanVertexFitter)
  // Separate into positive and negative charge tracks for efficiency
  std::vector<TrackRef> theTrackRefsPos;
  std::vector<TransientTrack> theTransTracksPos;
  std::vector<TrackRef> theTrackRefsNeg;
  std::vector<TransientTrack> theTransTracksNeg;

  // Handles for tracks, B-field, and tracker geometry
  Handle<reco::TrackCollection> theTrackHandle;
  Handle<reco::VertexCollection> theVertexHandle;
  Handle<reco::BeamSpot> theBeamSpotHandle;
  ESHandle<MagneticField> bFieldHandle;
  Handle<edm::ValueMap<reco::DeDxData>> dEdxHandle;
  Handle<std::vector<edm::Ptr<pat::PackedCandidate>>> track2pcHandle;
  Handle<int> cbin;
  [[maybe_unused]] int centrality = -1;
  if (useAnyMVA_) {
    iEvent.getByToken(tok_centBinLabel_, cbin);
    centrality = (cbin.isValid() ? *cbin : -1);
  }

  // Get the tracks, vertices from the event, and get the B-field record
  //  from the EventSetup
  iEvent.getByToken(token_tracks, theTrackHandle);
  iEvent.getByToken(token_vertices, theVertexHandle);
  iEvent.getByToken(token_beamSpot, theBeamSpotHandle);
  if (useDeDx_) {
    iEvent.getByToken(token_dedx, dEdxHandle);
    iEvent.getByToken(token_track2pc, track2pcHandle);
  }

  if (!theTrackHandle->size())
    return;
  bFieldHandle = iSetup.getHandle(bField_esToken_);

  magField = bFieldHandle.product();

  // Setup TMVA
  //  mvaValValueMap = auto_ptr<edm::ValueMap<float> >(new
  //  edm::ValueMap<float>); edm::ValueMap<float>::Filler
  //  mvaFiller(*mvaValValueMap);

  bool isVtxPV = 0;
  double xVtx = -99999.0;
  double yVtx = -99999.0;
  double zVtx = -99999.0;
  double xVtxError = -999.0;
  double yVtxError = -999.0;
  double zVtxError = -999.0;
  const reco::VertexCollection vtxCollection = *(theVertexHandle.product());
  reco::VertexCollection::const_iterator vtxPrimary = vtxCollection.begin();
  if (vtxCollection.size() > 0 && !vtxPrimary->isFake() &&
      vtxPrimary->tracksSize() >= 2) {
    isVtxPV = 1;
    xVtx = vtxPrimary->x();
    yVtx = vtxPrimary->y();
    zVtx = vtxPrimary->z();
    xVtxError = vtxPrimary->xError();
    yVtxError = vtxPrimary->yError();
    zVtxError = vtxPrimary->zError();
  } else {
    isVtxPV = 0;
    xVtx = theBeamSpotHandle->position().x();
    yVtx = theBeamSpotHandle->position().y();
    zVtx = 0.0;
    xVtxError = theBeamSpotHandle->BeamWidthX();
    yVtxError = theBeamSpotHandle->BeamWidthY();
    zVtxError = 0.0;
  }
  math::XYZPoint bestvtx(xVtx, yVtx, zVtx);

  // Fill vectors of TransientTracks and TrackRefs after applying preselection
  // cuts.
  for (unsigned int indx = 0; indx < theTrackHandle->size(); indx++) {
    TrackRef tmpRef(theTrackHandle, indx);
    bool quality_ok = true;
    if (qualities.size() != 0) {
      quality_ok = false;
      for (unsigned int ndx_ = 0; ndx_ < qualities.size(); ndx_++) {
        if (tmpRef->quality(qualities[ndx_])) {
          quality_ok = true;
          break;
        }
      }
    }
    if (!quality_ok)
      continue;

    if (tmpRef->normalizedChi2() < tkChi2Cut &&
        tmpRef->numberOfValidHits() >= tkNhitsCut &&
        tmpRef->ptError() / tmpRef->pt() < tkPtErrCut &&
        tmpRef->pt() > tkPtCut && fabs(tmpRef->eta()) < tkEtaCut) {
      //      TransientTrack tmpTk( *tmpRef, &(*bFieldHandle), globTkGeomHandle
      //      );
      TransientTrack tmpTk(*tmpRef, magField);

      double dzvtx = tmpRef->dz(bestvtx);
      double dxyvtx = tmpRef->dxy(bestvtx);
      double dzerror =
          sqrt(tmpRef->dzError() * tmpRef->dzError() + zVtxError * zVtxError);
      double dxyerror =
          sqrt(tmpRef->d0Error() * tmpRef->d0Error() + xVtxError * yVtxError);

      double dauLongImpactSig = dzvtx / dzerror;
      double dauTransImpactSig = dxyvtx / dxyerror;

      if (fabs(dauTransImpactSig) > dauTransImpactSigCut &&
          fabs(dauLongImpactSig) > dauLongImpactSigCut) {
        if (tmpRef->charge() > 0) {
          theTrackRefsPos.push_back(tmpRef);
          theTransTracksPos.push_back(tmpTk);
        }
        if (tmpRef->charge() < 0) {
          theTrackRefsNeg.push_back(tmpRef);
          theTransTracksNeg.push_back(tmpTk);
        }
      }
    }
  }

  float posCandMass[2] = {piMassD0, kaonMassD0};
  float negCandMass[2] = {kaonMassD0, piMassD0};
  float posCandMass_sigma[2] = {piMassD0_sigma, kaonMassD0_sigma};
  float negCandMass_sigma[2] = {kaonMassD0_sigma, piMassD0_sigma};
  int pdg_id[2] = {421, -421};

  // Pre-filter: remove tracks that can never form valid pairs (pt too low)
  // A track needs at least (tkPtSumCut - maxPt) to form a valid pair
  // Since we don't know maxPt yet, we'll sort by pt and use early breaks
  // instead

  // Create index vectors for sorting by pt (descending) for early break
  // optimization
  std::vector<size_t> posIndices(theTrackRefsPos.size());
  std::vector<size_t> negIndices(theTrackRefsNeg.size());
  std::iota(posIndices.begin(), posIndices.end(), 0);
  std::iota(negIndices.begin(), negIndices.end(), 0);

  std::sort(posIndices.begin(), posIndices.end(), [&](size_t i, size_t j) {
    return theTrackRefsPos[i]->pt() > theTrackRefsPos[j]->pt();
  });
  std::sort(negIndices.begin(), negIndices.end(), [&](size_t i, size_t j) {
    return theTrackRefsNeg[i]->pt() > theTrackRefsNeg[j]->pt();
  });

  // Loop over tracks and vertex good charged track pairs
  // Now loop over separated positive and negative tracks for efficiency

  if (!isWrongSign) {
    // Early exit if no tracks in either category
    if (theTrackRefsPos.empty() || theTrackRefsNeg.empty())
      return;

    // Right-sign: loop over pos and neg tracks separately (sorted by pt)
    for (unsigned int ipos = 0; ipos < posIndices.size(); ipos++) {
      size_t itrkpos = posIndices[ipos];
      TrackRef positiveTrackRef = theTrackRefsPos[itrkpos];
      TransientTrack *posTransTkPtr = &theTransTracksPos[itrkpos];

      // Early break: if this track's pt + smallest neg track pt < cut, no more
      // valid pairs
      if (positiveTrackRef->pt() + theTrackRefsNeg[negIndices.back()]->pt() <
          tkPtSumCut)
        break;

      for (unsigned int ineg = 0; ineg < negIndices.size(); ineg++) {
        size_t itrkneg = negIndices[ineg];
        TrackRef negativeTrackRef = theTrackRefsNeg[itrkneg];
        TransientTrack *negTransTkPtr = &theTransTracksNeg[itrkneg];

        // Early break: if pt sum is too small, remaining tracks (smaller pt)
        // won't work
        if ((positiveTrackRef->pt() + negativeTrackRef->pt()) < tkPtSumCut)
          break;
        if (abs(positiveTrackRef->eta() - negativeTrackRef->eta()) >
            tkEtaDiffCut)
          continue;

        // This vector holds the pair of oppositely-charged tracks to be
        // vertexed
        std::vector<TransientTrack> transTracks;

        // Calculate DCA of two daughters
        double dzvtx_pos = positiveTrackRef->dz(bestvtx);
        double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
        double dzerror_pos =
            sqrt(positiveTrackRef->dzError() * positiveTrackRef->dzError() +
                 zVtxError * zVtxError);
        double dxyerror_pos =
            sqrt(positiveTrackRef->d0Error() * positiveTrackRef->d0Error() +
                 xVtxError * yVtxError);
        double dauLongImpactSig_pos = dzvtx_pos / dzerror_pos;
        double dauTransImpactSig_pos = dxyvtx_pos / dxyerror_pos;

        double dzvtx_neg = negativeTrackRef->dz(bestvtx);
        double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
        double dzerror_neg =
            sqrt(negativeTrackRef->dzError() * negativeTrackRef->dzError() +
                 zVtxError * zVtxError);
        double dxyerror_neg =
            sqrt(negativeTrackRef->d0Error() * negativeTrackRef->d0Error() +
                 xVtxError * yVtxError);
        double dauLongImpactSig_neg = dzvtx_neg / dzerror_neg;
        double dauTransImpactSig_neg = dxyvtx_neg / dxyerror_neg;

        double nhits_pos = positiveTrackRef->numberOfValidHits();
        double nhits_neg = negativeTrackRef->numberOfValidHits();

        double ptErr_pos = positiveTrackRef->ptError();
        double ptErr_neg = negativeTrackRef->ptError();

        // dEdx extraction - uses PackedCandidate mapping for MiniAOD
        double dedx_pos = -999.;
        double dedx_neg = -999.;
        if (dEdxHandle.isValid() && track2pcHandle.isValid()) {
          const edm::ValueMap<reco::DeDxData> &dEdxMap = *dEdxHandle.product();
          const auto &track2pc = *track2pcHandle.product();
          auto posPC = track2pc.at(positiveTrackRef.key());
          auto negPC = track2pc.at(negativeTrackRef.key());
          if (posPC.isNonnull() && dEdxMap.contains(posPC.id()))
            dedx_pos = dEdxMap[posPC].dEdx();
          if (negPC.isNonnull() && dEdxMap.contains(negPC.id()))
            dedx_neg = dEdxMap[negPC].dEdx();
        }

        // Fill the vector of TransientTracks to send to KVF
        transTracks.push_back(*posTransTkPtr);
        transTracks.push_back(*negTransTkPtr);

        // Trajectory states to calculate DCA for the 2 tracks
        FreeTrajectoryState posState =
            posTransTkPtr->impactPointTSCP().theState();
        FreeTrajectoryState negState =
            negTransTkPtr->impactPointTSCP().theState();

        if (!posTransTkPtr->impactPointTSCP().isValid() ||
            !negTransTkPtr->impactPointTSCP().isValid())
          continue;

        // Measure distance between tracks at their closest approach
        ClosestApproachInRPhi cApp;
        cApp.calculate(posState, negState);
        if (!cApp.status())
          continue;
        float dca = cApp.distance();
        GlobalPoint cxPt = cApp.crossingPoint();

        // TrajectoryStateClosestToPoint posTsctp =
        // posTransTkPtr->trajectoryStateClosestToPoint(bestvtx);
        // TrajectoryStateClosestToPoint negTsctp =
        // negTransTkPtr->trajectoryStateClosestToPoint(bestvtx);

        // GlobalVector pospT = posTransTkPtr.impactPointTSCP().momentum();
        // GlobalVector negpT = negTransTkPtr.impactPointTSCP().momentum();
        // math::XYZVector deltaP(pospT.x() - negpT.x(), pospT.y() - negpT.y(),
        // 0);

        // TwoTrackMinimumDistanceHelixHelix minDistCalculator;
        // minDistCalculator.calculate(posState.parameters(),
        // negState.parameters());
        TwoTrackMinimumDistance minDistCalculator;
        minDistCalculator.calculate(posState, negState);
        dca = minDistCalculator.distance();
        // std::cout << "(pca,dca) : " << minDistCalculator.distance() << ", "
        // << dca << std::endl;

        if (dca < tkDCACutLow || dca > tkDCACut)
          continue;
        if (dca < 0)
          std::cout << "Negative DCA : " << dca << std::endl;
        //      if (sqrt( cxPt.x()*cxPt.x() + cxPt.y()*cxPt.y() ) > 120.
        //          || std::abs(cxPt.z()) > 300.) continue;

        // Get trajectory states for the tracks at POCA for later cuts
        TrajectoryStateClosestToPoint posTSCP =
            posTransTkPtr->trajectoryStateClosestToPoint(cxPt);
        TrajectoryStateClosestToPoint negTSCP =
            negTransTkPtr->trajectoryStateClosestToPoint(cxPt);

        if (!posTSCP.isValid() || !negTSCP.isValid())
          continue;

        double totalE1 = sqrt(posTSCP.momentum().mag2() + kaonMassD0Squared) +
                         sqrt(negTSCP.momentum().mag2() + piMassD0Squared);
        double totalE1Sq = totalE1 * totalE1;

        double totalE2 = sqrt(posTSCP.momentum().mag2() + piMassD0Squared) +
                         sqrt(negTSCP.momentum().mag2() + kaonMassD0Squared);
        double totalE2Sq = totalE2 * totalE2;

        double totalPSq = (posTSCP.momentum() + negTSCP.momentum()).mag2();

        auto sumMom = (posTSCP.momentum() + negTSCP.momentum());
        double totalPt = sumMom.perp();

        double mass1 = sqrt(totalE1Sq - totalPSq);
        double mass2 = sqrt(totalE2Sq - totalPSq);

        if ((mass1 > mPiKCutMax || mass1 < mPiKCutMin) &&
            (mass2 > mPiKCutMax || mass2 < mPiKCutMin))
          continue;
        if (totalPt < dPtCut)
          continue;
        double totalY1 =
            0.5 * log((totalE1 + totalPt * TMath::SinH(sumMom.eta())) /
                      (totalE1 - totalPt * TMath::SinH(sumMom.eta())));
        double totalY2 =
            0.5 * log((totalE2 + totalPt * TMath::SinH(sumMom.eta())) /
                      (totalE2 - totalPt * TMath::SinH(sumMom.eta())));
        if (fabs(totalY1) > d0AbsYCut && fabs(totalY2) > d0AbsYCut)
          continue;

        // Create the vertex fitter object and vertex the tracks

        float posCandTotalE[2] = {0.0};
        float negCandTotalE[2] = {0.0};
        float d0TotalE[2] = {0.0};

        for (int i = 0; i < 2; i++) {
          // Creating a KinematicParticleFactory
          KinematicParticleFactoryFromTransientTrack pFactory;

          float chi = 0.0;
          float ndf = 0.0;

          vector<RefCountedKinematicParticle> d0Particles;
          d0Particles.push_back(pFactory.particle(
              *posTransTkPtr, posCandMass[i], chi, ndf, posCandMass_sigma[i]));
          d0Particles.push_back(pFactory.particle(
              *negTransTkPtr, negCandMass[i], chi, ndf, negCandMass_sigma[i]));

          KinematicParticleVertexFitter d0Fitter;
          RefCountedKinematicTree d0Vertex;
          d0Vertex = d0Fitter.fit(d0Particles);

          if (!d0Vertex->isValid())
            continue;

          d0Vertex->movePointerToTheTop();
          RefCountedKinematicParticle d0Cand = d0Vertex->currentParticle();
          if (!d0Cand->currentState().isValid())
            continue;

          RefCountedKinematicVertex d0DecayVertex =
              d0Vertex->currentDecayVertex();
          if (!d0DecayVertex->vertexIsValid())
            continue;

          // if ( d0DecayVertex->chiSquared()<0 ||
          // d0DecayVertex->chiSquared()>1000 ) continue;

          // float d0C2Prob =
          //    ChiSquaredProbability((double)(d0DecayVertex->chiSquared()),(double)(d0DecayVertex->degreesOfFreedom()));
          // if (d0C2Prob < 0.0001) continue;

          float d0C2Prob = TMath::Prob(d0DecayVertex->chiSquared(),
                                       d0DecayVertex->degreesOfFreedom());
          if (d0C2Prob < VtxChiProbCut)
            continue;

          // if ( d0Cand->currentState().mass() > 2.5 ||
          // d0Cand->currentState().mass() < 1.0) continue;

          d0Vertex->movePointerToTheFirstChild();
          RefCountedKinematicParticle posCand = d0Vertex->currentParticle();
          d0Vertex->movePointerToTheNextChild();
          RefCountedKinematicParticle negCand = d0Vertex->currentParticle();

          if (!posCand->currentState().isValid() ||
              !negCand->currentState().isValid())
            continue;

          KinematicParameters posCandKP =
              posCand->currentState().kinematicParameters();
          KinematicParameters negCandKP =
              negCand->currentState().kinematicParameters();

          GlobalVector d0TotalP =
              GlobalVector(d0Cand->currentState().globalMomentum().x(),
                           d0Cand->currentState().globalMomentum().y(),
                           d0Cand->currentState().globalMomentum().z());

          GlobalVector posCandTotalP =
              GlobalVector(posCandKP.momentum().x(), posCandKP.momentum().y(),
                           posCandKP.momentum().z());
          GlobalVector negCandTotalP =
              GlobalVector(negCandKP.momentum().x(), negCandKP.momentum().y(),
                           negCandKP.momentum().z());

          posCandTotalE[i] =
              sqrt(posCandTotalP.mag2() + posCandMass[i] * posCandMass[i]);
          negCandTotalE[i] =
              sqrt(negCandTotalP.mag2() + negCandMass[i] * negCandMass[i]);
          d0TotalE[i] = posCandTotalE[i] + negCandTotalE[i];

          const Particle::LorentzVector d0P4(d0TotalP.x(), d0TotalP.y(),
                                             d0TotalP.z(), d0TotalE[i]);

          Particle::Point d0Vtx((*d0DecayVertex).position().x(),
                                (*d0DecayVertex).position().y(),
                                (*d0DecayVertex).position().z());
          std::vector<double> d0VtxEVec;
          d0VtxEVec.push_back(d0DecayVertex->error().cxx());
          d0VtxEVec.push_back(d0DecayVertex->error().cyx());
          d0VtxEVec.push_back(d0DecayVertex->error().cyy());
          d0VtxEVec.push_back(d0DecayVertex->error().czx());
          d0VtxEVec.push_back(d0DecayVertex->error().czy());
          d0VtxEVec.push_back(d0DecayVertex->error().czz());
          SMatrixSym3D d0VtxCovMatrix(d0VtxEVec.begin(), d0VtxEVec.end());
          const Vertex::CovarianceMatrix d0VtxCov(d0VtxCovMatrix);
          double d0VtxChi2(d0DecayVertex->chiSquared());
          double d0VtxNdof(d0DecayVertex->degreesOfFreedom());
          double d0NormalizedChi2 = d0VtxChi2 / d0VtxNdof;

          double rVtxMag = 99999.0;
          double lVtxMag = 99999.0;
          double sigmaRvtxMag = 999.0;
          double sigmaLvtxMag = 999.0;
          double d0Angle3D = -100.0;
          double d0Angle2D = -100.0;

          GlobalVector d0LineOfFlight = GlobalVector(
              d0Vtx.x() - xVtx, d0Vtx.y() - yVtx, d0Vtx.z() - zVtx);

          SMatrixSym3D d0TotalCov;
          if (isVtxPV)
            d0TotalCov = d0VtxCovMatrix + vtxPrimary->covariance();
          else
            d0TotalCov =
                d0VtxCovMatrix + theBeamSpotHandle->rotatedCovariance3D();

          SVector3 distanceVector3D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                    d0LineOfFlight.z());
          SVector3 distanceVector2D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                    0.0);

          d0Angle3D =
              angle(d0LineOfFlight.x(), d0LineOfFlight.y(), d0LineOfFlight.z(),
                    d0TotalP.x(), d0TotalP.y(), d0TotalP.z());
          d0Angle2D = angle(d0LineOfFlight.x(), d0LineOfFlight.y(), (float)0.0,
                            d0TotalP.x(), d0TotalP.y(), (float)0.0);

          lVtxMag = d0LineOfFlight.mag();
          rVtxMag = d0LineOfFlight.perp();
          sigmaLvtxMag =
              sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector3D)) /
              lVtxMag;
          sigmaRvtxMag =
              sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector2D)) /
              rVtxMag;

          if (d0NormalizedChi2 > chi2Cut || rVtxMag < rVtxCut ||
              rVtxMag / sigmaRvtxMag < rVtxSigCut || lVtxMag < lVtxCut ||
              lVtxMag / sigmaLvtxMag < lVtxSigCut ||
              cos(d0Angle3D) < collinCut3D || cos(d0Angle2D) < collinCut2D ||
              d0Angle3D > alphaCut || d0Angle2D > alpha2DCut)
            continue;
          AnalyticalImpactPointExtrapolator extrapolator(magField);
          TrajectoryStateOnSurface tsos = extrapolator.extrapolate(
              d0Cand->currentState().freeTrajectoryState(),
              RecoVertex::convertPos(vtxPrimary->position()));
          ;

          if (!tsos.isValid())
            continue;
          Measurement1D cur3DIP;
          VertexDistance3D a3d;
          GlobalPoint refPoint = tsos.globalPosition();
          GlobalError refPointErr = tsos.cartesianError().position();
          GlobalPoint vertexPosition =
              RecoVertex::convertPos(vtxPrimary->position());
          GlobalError vertexPositionErr =
              RecoVertex::convertError(vtxPrimary->error());
          cur3DIP =
              (a3d.distance(VertexState(vertexPosition, vertexPositionErr),
                            VertexState(refPoint, refPointErr)));
          // // Debugging part : cur3DIP and sin(alpha) * decaylength value is
          // equal but the error different std::cout << "By cur3DIP " <<
          // cur3DIP.value() << " +/- " << cur3DIP.error() <<std::endl;
          // std::cout << "By decay length and alpha " <<
          // std::sin(d0Angle3D)*lVtxMag << " +/- " << sigmaLvtxMag *
          // std::sin(d0Angle3D) <<std::endl; std::cout << "By decay length " <<
          // lVtxMag << " +/- " << sigmaLvtxMag * lVtxMag <<std::endl;

          FreeTrajectoryState posStateNew =
              posTransTkPtr->impactPointTSCP().theState();
          FreeTrajectoryState negStateNew =
              negTransTkPtr->impactPointTSCP().theState();
          ClosestApproachInRPhi cApp;
          cApp.calculate(posStateNew, negStateNew);
          if (!cApp.status())
            continue;
          float dca = fabs(cApp.distance());
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
                                 sigma_y2 * cxPt.y() * cxPt.y()) /
                            dca;

          std::unique_ptr<CC> theD0 = std::make_unique<CC>();

          theD0->setP4(d0P4);
          // theD0 = new VertexCompositeCandidate(0, d0P4, d0Vtx, d0VtxCov,
          // d0VtxChi2, d0VtxNdof);

          RecoChargedCandidate thePosCand(
              1,
              Particle::LorentzVector(posCandTotalP.x(), posCandTotalP.y(),
                                      posCandTotalP.z(), posCandTotalE[i]),
              d0Vtx);
          thePosCand.setTrack(positiveTrackRef);

          RecoChargedCandidate theNegCand(
              -1,
              Particle::LorentzVector(negCandTotalP.x(), negCandTotalP.y(),
                                      negCandTotalP.z(), negCandTotalE[i]),
              d0Vtx);
          theNegCand.setTrack(negativeTrackRef);

          if (isWrongSign) {
            thePosCand.setCharge(positiveTrackRef->charge());
            theNegCand.setCharge(positiveTrackRef->charge());
          }

          AddFourMomenta addp4;
          theD0->addDaughter(thePosCand, "posdau");
          theD0->addDaughter(theNegCand, "negdau");
          theD0->setPdgId(pdg_id[i]);
          reco::Vertex d0VtxObj = *d0DecayVertex;
          theD0->addUserData("Vtx", d0VtxObj);
          theD0->addUserFloat("VtxChi2", d0VtxChi2);
          theD0->addUserFloat("VtxNdof", d0VtxNdof);
          theD0->addUserFloat("alpha2D", d0Angle2D);
          theD0->addUserFloat("alpha3D", d0Angle3D);
          theD0->addUserFloat("decaylength2D", rVtxMag);
          theD0->addUserFloat("decaylength3D", lVtxMag);
          theD0->addUserFloat("decaylengthsignif2D", rVtxMag / sigmaRvtxMag);
          theD0->addUserFloat("decaylengthsignif3D", lVtxMag / sigmaLvtxMag);
          theD0->addUserFloat("dca3D", cur3DIP.value());
          theD0->addUserFloat("dca3DErr", cur3DIP.error());
          theD0->addUserFloat("track3DDCA", dca);
          theD0->addUserFloat("track3DDCAErr", dcaError);
          theD0->addUserFloat("posDauDeDx", dedx_pos);
          theD0->addUserFloat("negDauDeDx", dedx_neg);

          addp4.set(*theD0);
          if (theD0->mass() < d0MassD0 + d0MassCut &&
              theD0->mass() > d0MassD0 - d0MassCut) //&&
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
              // Prepare input data for new 17-feature BDT model
              // Features in order:
              // 0: VtxProb, 1: decayLengthSig3D, 2: alpha3D, 3: pTD1, 4: pTD2,
              // 5: etaD1, 6: etaD2, 7: pT, 8: y, 9: dca3D, 10: trackDCA,
              // 11: DeltaEta, 12: DeltaPhi, 13: DeltaR, 14: pTRatio,
              // 15: dca3DSig, 16: trackDCASig
              cms::Ort::FloatArrays data_;
              auto safe = [](float val) -> float {
                return (std::isnan(val) || std::isinf(val)) ? 0.0f : val;
              };

              if( label_MVA_ == std::string("ppref_pr")){
                data_.emplace_back(19, 0);
                std::vector<float> &onnxVals_ = data_[0];

                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                // Re-calculate daughter IP sigs relative to best PV
                double dzvtx_pos = positiveTrackRef->dz(bestvtx);
                double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
                double dzerror_pos = sqrt(positiveTrackRef->dzError() *
                                              positiveTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_pos = sqrt(positiveTrackRef->d0Error() *
                                               positiveTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD1 = safe(dzvtx_pos / dzerror_pos);
                double dxySigD1 = safe(dxyvtx_pos / dxyerror_pos);

                double dzvtx_neg = negativeTrackRef->dz(bestvtx);
                double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
                double dzerror_neg = sqrt(negativeTrackRef->dzError() *
                                              negativeTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_neg = sqrt(negativeTrackRef->d0Error() *
                                               negativeTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD2 = safe(dzvtx_neg / dzerror_neg);
                double dxySigD2 = safe(dxyvtx_neg / dxyerror_neg);
                double trkChi2D1 = safe(positiveTrackRef->normalizedChi2());
                double trkChi2D2 = safe(negativeTrackRef->normalizedChi2());
                double nhitD1 = safe(positiveTrackRef->numberOfValidHits());
                double nhitD2 = safe(negativeTrackRef->numberOfValidHits());
                if(pTD1 < pTD2){
                    std::swap(etaD1, etaD2);
                    std::swap(phiD1, phiD2);
                    std::swap(pTD1, pTD2);
                    std::swap(dzSigD1, dzSigD2);
                    std::swap(dxySigD1, dxySigD2);
                    std::swap(nhitD1, nhitD2);
                    std::swap(trkChi2D1, trkChi2D2);
                }

                // Calculate dEta_dau and dPhi_dau AFTER swap to match training (D1 always has higher pT)
                float dEta_dau = etaD1 - etaD2;
                float dPhi_dau = phiD1 - phiD2;
                while (dPhi_dau > M_PI)
                  dPhi_dau -= 2.0 * M_PI;
                while (dPhi_dau <= -M_PI)
                  dPhi_dau += 2.0 * M_PI;

//                onnxVals_[0] = safe(d0C2Prob);
//                onnxVals_[1] = safe(d0Angle3D);
//                onnxVals_[2] = safe(d0Angle2D);
//                onnxVals_[3] = safe(lVtxMag / sigmaLvtxMag);
//                onnxVals_[4] = safe(rVtxMag / sigmaRvtxMag);
//                onnxVals_[5] = safe(cur3DIP.value());
//                onnxVals_[6] = safe(dca);
//                onnxVals_[7] = safe(rVtxMag);
//                onnxVals_[8] = safe(d0LineOfFlight.z());
//                onnxVals_[9] = safe(dxySigD1);
//                onnxVals_[10] = safe(dzSigD1);
//                onnxVals_[11] = safe(dxySigD2);
//                onnxVals_[12] = safe(dzSigD2);
//                onnxVals_[13] = safe(pTD1);
//                onnxVals_[14] = safe(pTD2);
//                onnxVals_[15] = safe(theD0->pt());
//                onnxVals_[16] = safe(theD0->eta());
//                onnxVals_[17] = safe(etaD1);
//                onnxVals_[18] = safe(etaD2);
//                onnxVals_[19] = safe(dEta_dau);

                onnxVals_[0]  = safe(d0C2Prob);                // VtxProb
                onnxVals_[1]  = safe(d0Angle3D);               // alpha3D
                onnxVals_[2]  = safe(d0Angle2D);               // alpha2D
                onnxVals_[3]  = safe(lVtxMag / sigmaLvtxMag);  // decayLengthSig3D
                onnxVals_[4]  = safe(rVtxMag / sigmaRvtxMag);  // decayLengthSig2D
                onnxVals_[5]  = safe(cur3DIP.value());         // dca3D
                onnxVals_[6]  = safe(dca);                     // trackDCA
                onnxVals_[7]  = safe(pTD1);                    // pTD1
                onnxVals_[8]  = safe(pTD2);                    // pTD2
                onnxVals_[9]  = safe(theD0->pt());             // pT
                onnxVals_[10] = safe(theD0->eta());            // eta
                onnxVals_[11] = safe(etaD1);                   // etaD1
                onnxVals_[12] = safe(etaD2);                   // etaD2
                onnxVals_[13] = safe(nhitD1);                  // nhitD1
                onnxVals_[14] = safe(nhitD2);                  // nhitD2
                onnxVals_[15] = safe(trkChi2D1);               // trkChi2D1
                onnxVals_[16] = safe(trkChi2D2);               // trkChi2D2
                onnxVals_[17] = safe(dEta_dau);                // dEta_dau
                onnxVals_[18] = safe(dPhi_dau);                // dPhi_dau


                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
              } else {
                data_.emplace_back(17, 0);
                std::vector<float> &onnxVals_ = data_[0];

                // Compute derived quantities
                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                // DeltaEta
                float DeltaEta = std::abs(etaD1 - etaD2);

                // DeltaPhi (folded to [0, pi])
                float DeltaPhi = std::abs(phiD1 - phiD2);
                if (DeltaPhi > M_PI)
                  DeltaPhi = 2.0 * M_PI - DeltaPhi;

                // DeltaR
                float DeltaR =
                    std::sqrt(DeltaEta * DeltaEta + DeltaPhi * DeltaPhi);

                // pTRatio = min(pTD1, pTD2) / max(pTD1, pTD2)
                float pTMax = std::max(pTD1, pTD2);
                float pTRatio =
                    (pTMax > 0.0f) ? std::min(pTD1, pTD2) / pTMax : 0.0f;

                // dca3DSig = dca3D / dca3DErr
                float dca3D_val = cur3DIP.value();
                float dca3DErr_val = cur3DIP.error();
                float dca3DSig =
                    (dca3DErr_val > 0.0f) ? dca3D_val / dca3DErr_val : 0.0f;

                // trackDCASig = trackDCA / trackDCAErr
                float trackDCASig = (dcaError > 0.0f) ? dca / dcaError : 0.0f;

                // Fill ONNX input array (17 features) with NaN protection
                onnxVals_[0] = safe(d0C2Prob);               // VtxProb
                onnxVals_[1] = safe(lVtxMag / sigmaLvtxMag); // decayLengthSig3D
                onnxVals_[2] = safe(d0Angle3D);              // alpha3D
                onnxVals_[3] = safe(pTD1);                   // pTD1
                onnxVals_[4] = safe(pTD2);                   // pTD2
                onnxVals_[5] = safe(etaD1);                  // etaD1
                onnxVals_[6] = safe(etaD2);                  // etaD2
                onnxVals_[7] = safe(theD0->pt());            // pT
                onnxVals_[8] = safe(theD0->y());             // y
                onnxVals_[9] = safe(dca3D_val);              // dca3D
                onnxVals_[10] = safe(dca);                   // trackDCA
                onnxVals_[11] = safe(DeltaEta);              // DeltaEta
                onnxVals_[12] = safe(DeltaPhi);              // DeltaPhi
                onnxVals_[13] = safe(DeltaR);                // DeltaR
                onnxVals_[14] = safe(pTRatio);               // pTRatio
                onnxVals_[15] = safe(dca3DSig);              // dca3DSig
                onnxVals_[16] = safe(trackDCASig);           // trackDCASig

                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
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
            //     forestHandle = iSetup.getHandle<GBRForest,
            //     GBRWrapperRcd>(mvaToken_); forest = forestHandle.product();
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
  } else {
    // Wrong-sign: loop over same-charge pairs (pos-pos and neg-neg)
    // Process pos-pos pairs (sorted by pt)
    if (theTrackRefsPos.size() >= 2) {
      for (unsigned int ipos1 = 0; ipos1 < posIndices.size(); ipos1++) {
        size_t itrkpos1 = posIndices[ipos1];
        TrackRef positiveTrackRef = theTrackRefsPos[itrkpos1];
        TransientTrack *posTransTkPtr = &theTransTracksPos[itrkpos1];

        // Early break: if this track's pt + smallest remaining pos track pt <
        // cut, no more valid pairs
        if (ipos1 + 1 < posIndices.size() &&
            positiveTrackRef->pt() + theTrackRefsPos[posIndices.back()]->pt() <
                tkPtSumCut)
          break;

        for (unsigned int ipos2 = ipos1 + 1; ipos2 < posIndices.size();
             ipos2++) {
          size_t itrkpos2 = posIndices[ipos2];
          TrackRef negativeTrackRef =
              theTrackRefsPos[itrkpos2]; // Wrong-sign: both are positive
          TransientTrack *negTransTkPtr = &theTransTracksPos[itrkpos2];

          // Early break: if pt sum is too small, remaining tracks (smaller pt)
          // won't work
          if ((positiveTrackRef->pt() + negativeTrackRef->pt()) < tkPtSumCut)
            break;
          if (abs(positiveTrackRef->eta() - negativeTrackRef->eta()) >
              tkEtaDiffCut)
            continue;

          // This vector holds the pair of same-charged tracks to be vertexed
          // (wrong-sign)
          std::vector<TransientTrack> transTracks;

          // Calculate DCA of two daughters (same logic as right-sign)
          double dzvtx_pos = positiveTrackRef->dz(bestvtx);
          double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
          double dzerror_pos =
              sqrt(positiveTrackRef->dzError() * positiveTrackRef->dzError() +
                   zVtxError * zVtxError);
          double dxyerror_pos =
              sqrt(positiveTrackRef->d0Error() * positiveTrackRef->d0Error() +
                   xVtxError * yVtxError);
          double dauLongImpactSig_pos = dzvtx_pos / dzerror_pos;
          double dauTransImpactSig_pos = dxyvtx_pos / dxyerror_pos;

          double dzvtx_neg = negativeTrackRef->dz(bestvtx);
          double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
          double dzerror_neg =
              sqrt(negativeTrackRef->dzError() * negativeTrackRef->dzError() +
                   zVtxError * zVtxError);
          double dxyerror_neg =
              sqrt(negativeTrackRef->d0Error() * negativeTrackRef->d0Error() +
                   xVtxError * yVtxError);
          double dauLongImpactSig_neg = dzvtx_neg / dzerror_neg;
          double dauTransImpactSig_neg = dxyvtx_neg / dxyerror_neg;

          double nhits_pos = positiveTrackRef->numberOfValidHits();
          double nhits_neg = negativeTrackRef->numberOfValidHits();

          double ptErr_pos = positiveTrackRef->ptError();
          double ptErr_neg = negativeTrackRef->ptError();

          // dEdx extraction - uses PackedCandidate mapping for MiniAOD
          double dedx_pos = -999.;
          double dedx_neg = -999.;
          if (dEdxHandle.isValid() && track2pcHandle.isValid()) {
            const edm::ValueMap<reco::DeDxData> &dEdxMap =
                *dEdxHandle.product();
            const auto &track2pc = *track2pcHandle.product();
            auto posPC = track2pc.at(positiveTrackRef.key());
            auto negPC = track2pc.at(negativeTrackRef.key());
            if (posPC.isNonnull() && dEdxMap.contains(posPC.id()))
              dedx_pos = dEdxMap[posPC].dEdx();
            if (negPC.isNonnull() && dEdxMap.contains(negPC.id()))
              dedx_neg = dEdxMap[negPC].dEdx();
          }

          // Fill the vector of TransientTracks to send to KVF
          transTracks.push_back(*posTransTkPtr);
          transTracks.push_back(*negTransTkPtr);

          // Trajectory states to calculate DCA for the 2 tracks
          FreeTrajectoryState posState =
              posTransTkPtr->impactPointTSCP().theState();
          FreeTrajectoryState negState =
              negTransTkPtr->impactPointTSCP().theState();

          if (!posTransTkPtr->impactPointTSCP().isValid() ||
              !negTransTkPtr->impactPointTSCP().isValid())
            continue;

          // Measure distance between tracks at their closest approach
          ClosestApproachInRPhi cApp;
          cApp.calculate(posState, negState);
          if (!cApp.status())
            continue;
          float dca = cApp.distance();
          GlobalPoint cxPt = cApp.crossingPoint();

          TwoTrackMinimumDistance minDistCalculator;
          minDistCalculator.calculate(posState, negState);
          dca = minDistCalculator.distance();

          if (dca < tkDCACutLow || dca > tkDCACut)
            continue;
          if (dca < 0)
            std::cout << "Negative DCA : " << dca << std::endl;

          // Get trajectory states for the tracks at POCA for later cuts
          TrajectoryStateClosestToPoint posTSCP =
              posTransTkPtr->trajectoryStateClosestToPoint(cxPt);
          TrajectoryStateClosestToPoint negTSCP =
              negTransTkPtr->trajectoryStateClosestToPoint(cxPt);

          if (!posTSCP.isValid() || !negTSCP.isValid())
            continue;

          double totalE1 = sqrt(posTSCP.momentum().mag2() + kaonMassD0Squared) +
                           sqrt(negTSCP.momentum().mag2() + piMassD0Squared);
          double totalE1Sq = totalE1 * totalE1;

          double totalE2 = sqrt(posTSCP.momentum().mag2() + piMassD0Squared) +
                           sqrt(negTSCP.momentum().mag2() + kaonMassD0Squared);
          double totalE2Sq = totalE2 * totalE2;

          double totalPSq = (posTSCP.momentum() + negTSCP.momentum()).mag2();

          auto sumMom = (posTSCP.momentum() + negTSCP.momentum());
          double totalPt = sumMom.perp();

          double mass1 = sqrt(totalE1Sq - totalPSq);
          double mass2 = sqrt(totalE2Sq - totalPSq);

          if ((mass1 > mPiKCutMax || mass1 < mPiKCutMin) &&
              (mass2 > mPiKCutMax || mass2 < mPiKCutMin))
            continue;
          if (totalPt < dPtCut)
            continue;
          double totalY1 =
              0.5 * log((totalE1 + totalPt * TMath::SinH(sumMom.eta())) /
                        (totalE1 - totalPt * TMath::SinH(sumMom.eta())));
          double totalY2 =
              0.5 * log((totalE2 + totalPt * TMath::SinH(sumMom.eta())) /
                        (totalE2 - totalPt * TMath::SinH(sumMom.eta())));
          if (fabs(totalY1) > d0AbsYCut && fabs(totalY2) > d0AbsYCut)
            continue;

          // Create the vertex fitter object and vertex the tracks

          float posCandTotalE[2] = {0.0};
          float negCandTotalE[2] = {0.0};
          float d0TotalE[2] = {0.0};

          for (int i = 0; i < 2; i++) {
            // Creating a KinematicParticleFactory
            KinematicParticleFactoryFromTransientTrack pFactory;

            float chi = 0.0;
            float ndf = 0.0;

            vector<RefCountedKinematicParticle> d0Particles;
            d0Particles.push_back(pFactory.particle(*posTransTkPtr,
                                                    posCandMass[i], chi, ndf,
                                                    posCandMass_sigma[i]));
            d0Particles.push_back(pFactory.particle(*negTransTkPtr,
                                                    negCandMass[i], chi, ndf,
                                                    negCandMass_sigma[i]));

            KinematicParticleVertexFitter d0Fitter;
            RefCountedKinematicTree d0Vertex;
            d0Vertex = d0Fitter.fit(d0Particles);

            if (!d0Vertex->isValid())
              continue;

            d0Vertex->movePointerToTheTop();
            RefCountedKinematicParticle d0Cand = d0Vertex->currentParticle();
            if (!d0Cand->currentState().isValid())
              continue;

            RefCountedKinematicVertex d0DecayVertex =
                d0Vertex->currentDecayVertex();
            if (!d0DecayVertex->vertexIsValid())
              continue;

            float d0C2Prob = TMath::Prob(d0DecayVertex->chiSquared(),
                                         d0DecayVertex->degreesOfFreedom());
            if (d0C2Prob < VtxChiProbCut)
              continue;

            d0Vertex->movePointerToTheFirstChild();
            RefCountedKinematicParticle posCand = d0Vertex->currentParticle();
            d0Vertex->movePointerToTheNextChild();
            RefCountedKinematicParticle negCand = d0Vertex->currentParticle();

            if (!posCand->currentState().isValid() ||
                !negCand->currentState().isValid())
              continue;

            KinematicParameters posCandKP =
                posCand->currentState().kinematicParameters();
            KinematicParameters negCandKP =
                negCand->currentState().kinematicParameters();

            GlobalVector d0TotalP =
                GlobalVector(d0Cand->currentState().globalMomentum().x(),
                             d0Cand->currentState().globalMomentum().y(),
                             d0Cand->currentState().globalMomentum().z());

            GlobalVector posCandTotalP =
                GlobalVector(posCandKP.momentum().x(), posCandKP.momentum().y(),
                             posCandKP.momentum().z());
            GlobalVector negCandTotalP =
                GlobalVector(negCandKP.momentum().x(), negCandKP.momentum().y(),
                             negCandKP.momentum().z());

            posCandTotalE[i] =
                sqrt(posCandTotalP.mag2() + posCandMass[i] * posCandMass[i]);
            negCandTotalE[i] =
                sqrt(negCandTotalP.mag2() + negCandMass[i] * negCandMass[i]);
            d0TotalE[i] = posCandTotalE[i] + negCandTotalE[i];

            const Particle::LorentzVector d0P4(d0TotalP.x(), d0TotalP.y(),
                                               d0TotalP.z(), d0TotalE[i]);

            Particle::Point d0Vtx((*d0DecayVertex).position().x(),
                                  (*d0DecayVertex).position().y(),
                                  (*d0DecayVertex).position().z());
            std::vector<double> d0VtxEVec;
            d0VtxEVec.push_back(d0DecayVertex->error().cxx());
            d0VtxEVec.push_back(d0DecayVertex->error().cyx());
            d0VtxEVec.push_back(d0DecayVertex->error().cyy());
            d0VtxEVec.push_back(d0DecayVertex->error().czx());
            d0VtxEVec.push_back(d0DecayVertex->error().czy());
            d0VtxEVec.push_back(d0DecayVertex->error().czz());
            SMatrixSym3D d0VtxCovMatrix(d0VtxEVec.begin(), d0VtxEVec.end());
            const Vertex::CovarianceMatrix d0VtxCov(d0VtxCovMatrix);
            double d0VtxChi2(d0DecayVertex->chiSquared());
            double d0VtxNdof(d0DecayVertex->degreesOfFreedom());
            double d0NormalizedChi2 = d0VtxChi2 / d0VtxNdof;

            double rVtxMag = 99999.0;
            double lVtxMag = 99999.0;
            double sigmaRvtxMag = 999.0;
            double sigmaLvtxMag = 999.0;
            double d0Angle3D = -100.0;
            double d0Angle2D = -100.0;

            GlobalVector d0LineOfFlight = GlobalVector(
                d0Vtx.x() - xVtx, d0Vtx.y() - yVtx, d0Vtx.z() - zVtx);

            SMatrixSym3D d0TotalCov;
            if (isVtxPV)
              d0TotalCov = d0VtxCovMatrix + vtxPrimary->covariance();
            else
              d0TotalCov =
                  d0VtxCovMatrix + theBeamSpotHandle->rotatedCovariance3D();

            SVector3 distanceVector3D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                      d0LineOfFlight.z());
            SVector3 distanceVector2D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                      0.0);

            d0Angle3D = angle(d0LineOfFlight.x(), d0LineOfFlight.y(),
                              d0LineOfFlight.z(), d0TotalP.x(), d0TotalP.y(),
                              d0TotalP.z());
            d0Angle2D =
                angle(d0LineOfFlight.x(), d0LineOfFlight.y(), (float)0.0,
                      d0TotalP.x(), d0TotalP.y(), (float)0.0);

            lVtxMag = d0LineOfFlight.mag();
            rVtxMag = d0LineOfFlight.perp();
            sigmaLvtxMag =
                sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector3D)) /
                lVtxMag;
            sigmaRvtxMag =
                sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector2D)) /
                rVtxMag;

            if (d0NormalizedChi2 > chi2Cut || rVtxMag < rVtxCut ||
                rVtxMag / sigmaRvtxMag < rVtxSigCut || lVtxMag < lVtxCut ||
                lVtxMag / sigmaLvtxMag < lVtxSigCut ||
                cos(d0Angle3D) < collinCut3D || cos(d0Angle2D) < collinCut2D ||
                d0Angle3D > alphaCut || d0Angle2D > alpha2DCut)
              continue;
            AnalyticalImpactPointExtrapolator extrapolator(magField);
            TrajectoryStateOnSurface tsos = extrapolator.extrapolate(
                d0Cand->currentState().freeTrajectoryState(),
                RecoVertex::convertPos(vtxPrimary->position()));
            ;

            if (!tsos.isValid())
              continue;
            Measurement1D cur3DIP;
            VertexDistance3D a3d;
            GlobalPoint refPoint = tsos.globalPosition();
            GlobalError refPointErr = tsos.cartesianError().position();
            GlobalPoint vertexPosition =
                RecoVertex::convertPos(vtxPrimary->position());
            GlobalError vertexPositionErr =
                RecoVertex::convertError(vtxPrimary->error());
            cur3DIP =
                (a3d.distance(VertexState(vertexPosition, vertexPositionErr),
                              VertexState(refPoint, refPointErr)));

            FreeTrajectoryState posStateNew =
                posTransTkPtr->impactPointTSCP().theState();
            FreeTrajectoryState negStateNew =
                negTransTkPtr->impactPointTSCP().theState();
            ClosestApproachInRPhi cApp;
            cApp.calculate(posStateNew, negStateNew);
            if (!cApp.status())
              continue;
            float dca = fabs(cApp.distance());
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
                                   sigma_y2 * cxPt.y() * cxPt.y()) /
                              dca;

            std::unique_ptr<CC> theD0 = std::make_unique<CC>();

            theD0->setP4(d0P4);

            RecoChargedCandidate thePosCand(
                1,
                Particle::LorentzVector(posCandTotalP.x(), posCandTotalP.y(),
                                        posCandTotalP.z(), posCandTotalE[i]),
                d0Vtx);
            thePosCand.setTrack(positiveTrackRef);

            RecoChargedCandidate theNegCand(
                -1,
                Particle::LorentzVector(negCandTotalP.x(), negCandTotalP.y(),
                                        negCandTotalP.z(), negCandTotalE[i]),
                d0Vtx);
            theNegCand.setTrack(negativeTrackRef);

            // Wrong-sign: set charges to match the actual track charges
            thePosCand.setCharge(positiveTrackRef->charge());
            theNegCand.setCharge(positiveTrackRef->charge());

            AddFourMomenta addp4;
            theD0->addDaughter(thePosCand, "posdau");
            theD0->addDaughter(theNegCand, "negdau");
            theD0->setPdgId(pdg_id[i]);
            reco::Vertex d0VtxObj = *d0DecayVertex;
            theD0->addUserData("Vtx", d0VtxObj);
            theD0->addUserFloat("VtxChi2", d0VtxChi2);
            theD0->addUserFloat("VtxNdof", d0VtxNdof);
            theD0->addUserFloat("alpha2D", d0Angle2D);
            theD0->addUserFloat("alpha3D", d0Angle3D);
            theD0->addUserFloat("decaylength2D", rVtxMag);
            theD0->addUserFloat("decaylength3D", lVtxMag);
            theD0->addUserFloat("decaylengthsignif2D", rVtxMag / sigmaRvtxMag);
            theD0->addUserFloat("decaylengthsignif3D", lVtxMag / sigmaLvtxMag);
            theD0->addUserFloat("dca3D", cur3DIP.value());
            theD0->addUserFloat("dca3DErr", cur3DIP.error());
            theD0->addUserFloat("track3DDCA", dca);
            theD0->addUserFloat("track3DDCAErr", dcaError);
            theD0->addUserFloat("posDauDeDx", dedx_pos);
            theD0->addUserFloat("negDauDeDx", dedx_neg);

            addp4.set(*theD0);
            if (theD0->mass() < d0MassD0 + d0MassCut &&
                theD0->mass() > d0MassD0 - d0MassCut) {
            if (useAnyMVA_ && onnxRuntime_) {
              // Prepare input data for new 17-feature BDT model
              // Features in order:
              // 0: VtxProb, 1: decayLengthSig3D, 2: alpha3D, 3: pTD1, 4: pTD2,
              // 5: etaD1, 6: etaD2, 7: pT, 8: y, 9: dca3D, 10: trackDCA,
              // 11: DeltaEta, 12: DeltaPhi, 13: DeltaR, 14: pTRatio,
              // 15: dca3DSig, 16: trackDCASig
              cms::Ort::FloatArrays data_;
              auto safe = [](float val) -> float {
                return (std::isnan(val) || std::isinf(val)) ? 0.0f : val;
              };

              if( label_MVA_ == std::string("ppref_pr")){
                data_.emplace_back(21, 0);
                std::vector<float> &onnxVals_ = data_[0];

                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                float dEta_dau = etaD1 - etaD2;
                float dPhi_dau = phiD1 - phiD2;
                while (dPhi_dau > M_PI)
                  dPhi_dau -= 2.0 * M_PI;
                while (dPhi_dau <= -M_PI)
                  dPhi_dau += 2.0 * M_PI;

                // Re-calculate daughter IP sigs relative to best PV
                double dzvtx_pos = positiveTrackRef->dz(bestvtx);
                double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
                double dzerror_pos = sqrt(positiveTrackRef->dzError() *
                                              positiveTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_pos = sqrt(positiveTrackRef->d0Error() *
                                               positiveTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD1 = safe(dzvtx_pos / dzerror_pos);
                double dxySigD1 = safe(dxyvtx_pos / dxyerror_pos);

                double dzvtx_neg = negativeTrackRef->dz(bestvtx);
                double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
                double dzerror_neg = sqrt(negativeTrackRef->dzError() *
                                              negativeTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_neg = sqrt(negativeTrackRef->d0Error() *
                                               negativeTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD2 = safe(dzvtx_neg / dzerror_neg);
                double dxySigD2 = safe(dxyvtx_neg / dxyerror_neg);

                onnxVals_[0] = safe(d0C2Prob);
                onnxVals_[1] = safe(d0Angle3D);
                onnxVals_[2] = safe(d0Angle2D);
                onnxVals_[3] = safe(lVtxMag / sigmaLvtxMag);
                onnxVals_[4] = safe(rVtxMag / sigmaRvtxMag);
                onnxVals_[5] = safe(cur3DIP.value());
                onnxVals_[6] = safe(dca);
                onnxVals_[7] = safe(rVtxMag);
                onnxVals_[8] = safe(d0LineOfFlight.z());
                onnxVals_[9] = safe(dxySigD1);
                onnxVals_[10] = safe(dzSigD1);
                onnxVals_[11] = safe(dxySigD2);
                onnxVals_[12] = safe(dzSigD2);
                onnxVals_[13] = safe(pTD1);
                onnxVals_[14] = safe(pTD2);
                onnxVals_[15] = safe(theD0->pt());
                onnxVals_[16] = safe(theD0->eta());
                onnxVals_[17] = safe(etaD1);
                onnxVals_[18] = safe(etaD2);
                onnxVals_[19] = safe(dEta_dau);

                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
              } else {
                data_.emplace_back(17, 0);
                std::vector<float> &onnxVals_ = data_[0];

                // Compute derived quantities
                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                // DeltaEta
                float DeltaEta = std::abs(etaD1 - etaD2);

                // DeltaPhi (folded to [0, pi])
                float DeltaPhi = std::abs(phiD1 - phiD2);
                if (DeltaPhi > M_PI)
                  DeltaPhi = 2.0 * M_PI - DeltaPhi;

                // DeltaR
                float DeltaR =
                    std::sqrt(DeltaEta * DeltaEta + DeltaPhi * DeltaPhi);

                // pTRatio = min(pTD1, pTD2) / max(pTD1, pTD2)
                float pTMax = std::max(pTD1, pTD2);
                float pTRatio =
                    (pTMax > 0.0f) ? std::min(pTD1, pTD2) / pTMax : 0.0f;

                // dca3DSig = dca3D / dca3DErr
                float dca3D_val = cur3DIP.value();
                float dca3DErr_val = cur3DIP.error();
                float dca3DSig =
                    (dca3DErr_val > 0.0f) ? dca3D_val / dca3DErr_val : 0.0f;

                // trackDCASig = trackDCA / trackDCAErr
                float trackDCASig = (dcaError > 0.0f) ? dca / dcaError : 0.0f;

                // Fill ONNX input array (17 features) with NaN protection
                onnxVals_[0] = safe(d0C2Prob);               // VtxProb
                onnxVals_[1] = safe(lVtxMag / sigmaLvtxMag); // decayLengthSig3D
                onnxVals_[2] = safe(d0Angle3D);              // alpha3D
                onnxVals_[3] = safe(pTD1);                   // pTD1
                onnxVals_[4] = safe(pTD2);                   // pTD2
                onnxVals_[5] = safe(etaD1);                  // etaD1
                onnxVals_[6] = safe(etaD2);                  // etaD2
                onnxVals_[7] = safe(theD0->pt());            // pT
                onnxVals_[8] = safe(theD0->y());             // y
                onnxVals_[9] = safe(dca3D_val);              // dca3D
                onnxVals_[10] = safe(dca);                   // trackDCA
                onnxVals_[11] = safe(DeltaEta);              // DeltaEta
                onnxVals_[12] = safe(DeltaPhi);              // DeltaPhi
                onnxVals_[13] = safe(DeltaR);                // DeltaR
                onnxVals_[14] = safe(pTRatio);               // pTRatio
                onnxVals_[15] = safe(dca3DSig);              // dca3DSig
                onnxVals_[16] = safe(trackDCASig);           // trackDCASig

                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
              }
            } else { 
                theD0s.push_back(*theD0);
              }
            }
          }
        }
      }
    }
    // Process neg-neg pairs for wrong-sign (sorted by pt)
    if (theTrackRefsNeg.size() >= 2) {
      for (unsigned int ineg1 = 0; ineg1 < negIndices.size(); ineg1++) {
        size_t itrkneg1 = negIndices[ineg1];
        TrackRef positiveTrackRef =
            theTrackRefsNeg[itrkneg1]; // Wrong-sign: both are negative
        TransientTrack *posTransTkPtr = &theTransTracksNeg[itrkneg1];

        // Early break: if this track's pt + smallest remaining neg track pt <
        // cut, no more valid pairs
        if (ineg1 + 1 < negIndices.size() &&
            positiveTrackRef->pt() + theTrackRefsNeg[negIndices.back()]->pt() <
                tkPtSumCut)
          break;

        for (unsigned int ineg2 = ineg1 + 1; ineg2 < negIndices.size();
             ineg2++) {
          size_t itrkneg2 = negIndices[ineg2];
          TrackRef negativeTrackRef = theTrackRefsNeg[itrkneg2];
          TransientTrack *negTransTkPtr = &theTransTracksNeg[itrkneg2];

          // Early break: if pt sum is too small, remaining tracks (smaller pt)
          // won't work
          if ((positiveTrackRef->pt() + negativeTrackRef->pt()) < tkPtSumCut)
            break;
          if (abs(positiveTrackRef->eta() - negativeTrackRef->eta()) >
              tkEtaDiffCut)
            continue;

          // This vector holds the pair of same-charged tracks to be vertexed
          // (wrong-sign)
          std::vector<TransientTrack> transTracks;

          // Calculate DCA of two daughters (same logic as right-sign)
          double dzvtx_pos = positiveTrackRef->dz(bestvtx);
          double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
          double dzerror_pos =
              sqrt(positiveTrackRef->dzError() * positiveTrackRef->dzError() +
                   zVtxError * zVtxError);
          double dxyerror_pos =
              sqrt(positiveTrackRef->d0Error() * positiveTrackRef->d0Error() +
                   xVtxError * yVtxError);
          double dauLongImpactSig_pos = dzvtx_pos / dzerror_pos;
          double dauTransImpactSig_pos = dxyvtx_pos / dxyerror_pos;

          double dzvtx_neg = negativeTrackRef->dz(bestvtx);
          double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
          double dzerror_neg =
              sqrt(negativeTrackRef->dzError() * negativeTrackRef->dzError() +
                   zVtxError * zVtxError);
          double dxyerror_neg =
              sqrt(negativeTrackRef->d0Error() * negativeTrackRef->d0Error() +
                   xVtxError * yVtxError);
          double dauLongImpactSig_neg = dzvtx_neg / dzerror_neg;
          double dauTransImpactSig_neg = dxyvtx_neg / dxyerror_neg;

          double nhits_pos = positiveTrackRef->numberOfValidHits();
          double nhits_neg = negativeTrackRef->numberOfValidHits();

          double ptErr_pos = positiveTrackRef->ptError();
          double ptErr_neg = negativeTrackRef->ptError();

          // dEdx extraction - uses PackedCandidate mapping for MiniAOD
          double dedx_pos = -999.;
          double dedx_neg = -999.;
          if (dEdxHandle.isValid() && track2pcHandle.isValid()) {
            const edm::ValueMap<reco::DeDxData> &dEdxMap =
                *dEdxHandle.product();
            const auto &track2pc = *track2pcHandle.product();
            auto posPC = track2pc.at(positiveTrackRef.key());
            auto negPC = track2pc.at(negativeTrackRef.key());
            if (posPC.isNonnull() && dEdxMap.contains(posPC.id()))
              dedx_pos = dEdxMap[posPC].dEdx();
            if (negPC.isNonnull() && dEdxMap.contains(negPC.id()))
              dedx_neg = dEdxMap[negPC].dEdx();
          }

          // Fill the vector of TransientTracks to send to KVF
          transTracks.push_back(*posTransTkPtr);
          transTracks.push_back(*negTransTkPtr);

          // Trajectory states to calculate DCA for the 2 tracks
          FreeTrajectoryState posState =
              posTransTkPtr->impactPointTSCP().theState();
          FreeTrajectoryState negState =
              negTransTkPtr->impactPointTSCP().theState();

          if (!posTransTkPtr->impactPointTSCP().isValid() ||
              !negTransTkPtr->impactPointTSCP().isValid())
            continue;

          // Measure distance between tracks at their closest approach
          ClosestApproachInRPhi cApp;
          cApp.calculate(posState, negState);
          if (!cApp.status())
            continue;
          float dca = cApp.distance();
          GlobalPoint cxPt = cApp.crossingPoint();

          TwoTrackMinimumDistance minDistCalculator;
          minDistCalculator.calculate(posState, negState);
          dca = minDistCalculator.distance();

          if (dca < tkDCACutLow || dca > tkDCACut)
            continue;
          if (dca < 0)
            std::cout << "Negative DCA : " << dca << std::endl;

          // Get trajectory states for the tracks at POCA for later cuts
          TrajectoryStateClosestToPoint posTSCP =
              posTransTkPtr->trajectoryStateClosestToPoint(cxPt);
          TrajectoryStateClosestToPoint negTSCP =
              negTransTkPtr->trajectoryStateClosestToPoint(cxPt);

          if (!posTSCP.isValid() || !negTSCP.isValid())
            continue;

          double totalE1 = sqrt(posTSCP.momentum().mag2() + kaonMassD0Squared) +
                           sqrt(negTSCP.momentum().mag2() + piMassD0Squared);
          double totalE1Sq = totalE1 * totalE1;

          double totalE2 = sqrt(posTSCP.momentum().mag2() + piMassD0Squared) +
                           sqrt(negTSCP.momentum().mag2() + kaonMassD0Squared);
          double totalE2Sq = totalE2 * totalE2;

          double totalPSq = (posTSCP.momentum() + negTSCP.momentum()).mag2();

          auto sumMom = (posTSCP.momentum() + negTSCP.momentum());
          double totalPt = sumMom.perp();

          double mass1 = sqrt(totalE1Sq - totalPSq);
          double mass2 = sqrt(totalE2Sq - totalPSq);

          if ((mass1 > mPiKCutMax || mass1 < mPiKCutMin) &&
              (mass2 > mPiKCutMax || mass2 < mPiKCutMin))
            continue;
          if (totalPt < dPtCut)
            continue;
          double totalY1 =
              0.5 * log((totalE1 + totalPt * TMath::SinH(sumMom.eta())) /
                        (totalE1 - totalPt * TMath::SinH(sumMom.eta())));
          double totalY2 =
              0.5 * log((totalE2 + totalPt * TMath::SinH(sumMom.eta())) /
                        (totalE2 - totalPt * TMath::SinH(sumMom.eta())));
          if (fabs(totalY1) > d0AbsYCut && fabs(totalY2) > d0AbsYCut)
            continue;

          // Create the vertex fitter object and vertex the tracks

          float posCandTotalE[2] = {0.0};
          float negCandTotalE[2] = {0.0};
          float d0TotalE[2] = {0.0};

          for (int i = 0; i < 2; i++) {
            // Creating a KinematicParticleFactory
            KinematicParticleFactoryFromTransientTrack pFactory;

            float chi = 0.0;
            float ndf = 0.0;

            vector<RefCountedKinematicParticle> d0Particles;
            d0Particles.push_back(pFactory.particle(*posTransTkPtr,
                                                    posCandMass[i], chi, ndf,
                                                    posCandMass_sigma[i]));
            d0Particles.push_back(pFactory.particle(*negTransTkPtr,
                                                    negCandMass[i], chi, ndf,
                                                    negCandMass_sigma[i]));

            KinematicParticleVertexFitter d0Fitter;
            RefCountedKinematicTree d0Vertex;
            d0Vertex = d0Fitter.fit(d0Particles);

            if (!d0Vertex->isValid())
              continue;

            d0Vertex->movePointerToTheTop();
            RefCountedKinematicParticle d0Cand = d0Vertex->currentParticle();
            if (!d0Cand->currentState().isValid())
              continue;

            RefCountedKinematicVertex d0DecayVertex =
                d0Vertex->currentDecayVertex();
            if (!d0DecayVertex->vertexIsValid())
              continue;

            float d0C2Prob = TMath::Prob(d0DecayVertex->chiSquared(),
                                         d0DecayVertex->degreesOfFreedom());
            if (d0C2Prob < VtxChiProbCut)
              continue;

            d0Vertex->movePointerToTheFirstChild();
            RefCountedKinematicParticle posCand = d0Vertex->currentParticle();
            d0Vertex->movePointerToTheNextChild();
            RefCountedKinematicParticle negCand = d0Vertex->currentParticle();

            if (!posCand->currentState().isValid() ||
                !negCand->currentState().isValid())
              continue;

            KinematicParameters posCandKP =
                posCand->currentState().kinematicParameters();
            KinematicParameters negCandKP =
                negCand->currentState().kinematicParameters();

            GlobalVector d0TotalP =
                GlobalVector(d0Cand->currentState().globalMomentum().x(),
                             d0Cand->currentState().globalMomentum().y(),
                             d0Cand->currentState().globalMomentum().z());

            GlobalVector posCandTotalP =
                GlobalVector(posCandKP.momentum().x(), posCandKP.momentum().y(),
                             posCandKP.momentum().z());
            GlobalVector negCandTotalP =
                GlobalVector(negCandKP.momentum().x(), negCandKP.momentum().y(),
                             negCandKP.momentum().z());

            posCandTotalE[i] =
                sqrt(posCandTotalP.mag2() + posCandMass[i] * posCandMass[i]);
            negCandTotalE[i] =
                sqrt(negCandTotalP.mag2() + negCandMass[i] * negCandMass[i]);
            d0TotalE[i] = posCandTotalE[i] + negCandTotalE[i];

            const Particle::LorentzVector d0P4(d0TotalP.x(), d0TotalP.y(),
                                               d0TotalP.z(), d0TotalE[i]);

            Particle::Point d0Vtx((*d0DecayVertex).position().x(),
                                  (*d0DecayVertex).position().y(),
                                  (*d0DecayVertex).position().z());
            std::vector<double> d0VtxEVec;
            d0VtxEVec.push_back(d0DecayVertex->error().cxx());
            d0VtxEVec.push_back(d0DecayVertex->error().cyx());
            d0VtxEVec.push_back(d0DecayVertex->error().cyy());
            d0VtxEVec.push_back(d0DecayVertex->error().czx());
            d0VtxEVec.push_back(d0DecayVertex->error().czy());
            d0VtxEVec.push_back(d0DecayVertex->error().czz());
            SMatrixSym3D d0VtxCovMatrix(d0VtxEVec.begin(), d0VtxEVec.end());
            const Vertex::CovarianceMatrix d0VtxCov(d0VtxCovMatrix);
            double d0VtxChi2(d0DecayVertex->chiSquared());
            double d0VtxNdof(d0DecayVertex->degreesOfFreedom());
            double d0NormalizedChi2 = d0VtxChi2 / d0VtxNdof;

            double rVtxMag = 99999.0;
            double lVtxMag = 99999.0;
            double sigmaRvtxMag = 999.0;
            double sigmaLvtxMag = 999.0;
            double d0Angle3D = -100.0;
            double d0Angle2D = -100.0;

            GlobalVector d0LineOfFlight = GlobalVector(
                d0Vtx.x() - xVtx, d0Vtx.y() - yVtx, d0Vtx.z() - zVtx);

            SMatrixSym3D d0TotalCov;
            if (isVtxPV)
              d0TotalCov = d0VtxCovMatrix + vtxPrimary->covariance();
            else
              d0TotalCov =
                  d0VtxCovMatrix + theBeamSpotHandle->rotatedCovariance3D();

            SVector3 distanceVector3D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                      d0LineOfFlight.z());
            SVector3 distanceVector2D(d0LineOfFlight.x(), d0LineOfFlight.y(),
                                      0.0);

            d0Angle3D = angle(d0LineOfFlight.x(), d0LineOfFlight.y(),
                              d0LineOfFlight.z(), d0TotalP.x(), d0TotalP.y(),
                              d0TotalP.z());
            d0Angle2D =
                angle(d0LineOfFlight.x(), d0LineOfFlight.y(), (float)0.0,
                      d0TotalP.x(), d0TotalP.y(), (float)0.0);

            lVtxMag = d0LineOfFlight.mag();
            rVtxMag = d0LineOfFlight.perp();
            sigmaLvtxMag =
                sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector3D)) /
                lVtxMag;
            sigmaRvtxMag =
                sqrt(ROOT::Math::Similarity(d0TotalCov, distanceVector2D)) /
                rVtxMag;

            if (d0NormalizedChi2 > chi2Cut || rVtxMag < rVtxCut ||
                rVtxMag / sigmaRvtxMag < rVtxSigCut || lVtxMag < lVtxCut ||
                lVtxMag / sigmaLvtxMag < lVtxSigCut ||
                cos(d0Angle3D) < collinCut3D || cos(d0Angle2D) < collinCut2D ||
                d0Angle3D > alphaCut || d0Angle2D > alpha2DCut)
              continue;
            AnalyticalImpactPointExtrapolator extrapolator(magField);
            TrajectoryStateOnSurface tsos = extrapolator.extrapolate(
                d0Cand->currentState().freeTrajectoryState(),
                RecoVertex::convertPos(vtxPrimary->position()));
            ;

            if (!tsos.isValid())
              continue;
            Measurement1D cur3DIP;
            VertexDistance3D a3d;
            GlobalPoint refPoint = tsos.globalPosition();
            GlobalError refPointErr = tsos.cartesianError().position();
            GlobalPoint vertexPosition =
                RecoVertex::convertPos(vtxPrimary->position());
            GlobalError vertexPositionErr =
                RecoVertex::convertError(vtxPrimary->error());
            cur3DIP =
                (a3d.distance(VertexState(vertexPosition, vertexPositionErr),
                              VertexState(refPoint, refPointErr)));

            FreeTrajectoryState posStateNew =
                posTransTkPtr->impactPointTSCP().theState();
            FreeTrajectoryState negStateNew =
                negTransTkPtr->impactPointTSCP().theState();
            ClosestApproachInRPhi cApp;
            cApp.calculate(posStateNew, negStateNew);
            if (!cApp.status())
              continue;
            float dca = fabs(cApp.distance());
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
                                   sigma_y2 * cxPt.y() * cxPt.y()) /
                              dca;

            std::unique_ptr<CC> theD0 = std::make_unique<CC>();

            theD0->setP4(d0P4);

            RecoChargedCandidate thePosCand(
                1,
                Particle::LorentzVector(posCandTotalP.x(), posCandTotalP.y(),
                                        posCandTotalP.z(), posCandTotalE[i]),
                d0Vtx);
            thePosCand.setTrack(positiveTrackRef);

            RecoChargedCandidate theNegCand(
                -1,
                Particle::LorentzVector(negCandTotalP.x(), negCandTotalP.y(),
                                        negCandTotalP.z(), negCandTotalE[i]),
                d0Vtx);
            theNegCand.setTrack(negativeTrackRef);

            // Wrong-sign: set charges to match the actual track charges
            thePosCand.setCharge(positiveTrackRef->charge());
            theNegCand.setCharge(positiveTrackRef->charge());

            AddFourMomenta addp4;
            theD0->addDaughter(thePosCand, "posdau");
            theD0->addDaughter(theNegCand, "negdau");
            theD0->setPdgId(pdg_id[i]);
            reco::Vertex d0VtxObj = *d0DecayVertex;
            theD0->addUserData("Vtx", d0VtxObj);
            theD0->addUserFloat("VtxChi2", d0VtxChi2);
            theD0->addUserFloat("VtxNdof", d0VtxNdof);
            theD0->addUserFloat("alpha2D", d0Angle2D);
            theD0->addUserFloat("alpha3D", d0Angle3D);
            theD0->addUserFloat("decaylength2D", rVtxMag);
            theD0->addUserFloat("decaylength3D", lVtxMag);
            theD0->addUserFloat("decaylengthsignif2D", rVtxMag / sigmaRvtxMag);
            theD0->addUserFloat("decaylengthsignif3D", lVtxMag / sigmaLvtxMag);
            theD0->addUserFloat("dca3D", cur3DIP.value());
            theD0->addUserFloat("dca3DErr", cur3DIP.error());
            theD0->addUserFloat("track3DDCA", dca);
            theD0->addUserFloat("track3DDCAErr", dcaError);
            theD0->addUserFloat("posDauDeDx", dedx_pos);
            theD0->addUserFloat("negDauDeDx", dedx_neg);

            addp4.set(*theD0);
            if (theD0->mass() < d0MassD0 + d0MassCut &&
                theD0->mass() > d0MassD0 - d0MassCut) {
            if (useAnyMVA_ && onnxRuntime_) {
              // Prepare input data for new 17-feature BDT model
              // Features in order:
              // 0: VtxProb, 1: decayLengthSig3D, 2: alpha3D, 3: pTD1, 4: pTD2,
              // 5: etaD1, 6: etaD2, 7: pT, 8: y, 9: dca3D, 10: trackDCA,
              // 11: DeltaEta, 12: DeltaPhi, 13: DeltaR, 14: pTRatio,
              // 15: dca3DSig, 16: trackDCASig
              cms::Ort::FloatArrays data_;
              auto safe = [](float val) -> float {
                return (std::isnan(val) || std::isinf(val)) ? 0.0f : val;
              };

              if( label_MVA_ == std::string("ppref_pr")){
                data_.emplace_back(21, 0);
                std::vector<float> &onnxVals_ = data_[0];

                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                float dEta_dau = etaD1 - etaD2;
                float dPhi_dau = phiD1 - phiD2;
                while (dPhi_dau > M_PI)
                  dPhi_dau -= 2.0 * M_PI;
                while (dPhi_dau <= -M_PI)
                  dPhi_dau += 2.0 * M_PI;

                // Re-calculate daughter IP sigs relative to best PV
                double dzvtx_pos = positiveTrackRef->dz(bestvtx);
                double dxyvtx_pos = positiveTrackRef->dxy(bestvtx);
                double dzerror_pos = sqrt(positiveTrackRef->dzError() *
                                              positiveTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_pos = sqrt(positiveTrackRef->d0Error() *
                                               positiveTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD1 = safe(dzvtx_pos / dzerror_pos);
                double dxySigD1 = safe(dxyvtx_pos / dxyerror_pos);

                double dzvtx_neg = negativeTrackRef->dz(bestvtx);
                double dxyvtx_neg = negativeTrackRef->dxy(bestvtx);
                double dzerror_neg = sqrt(negativeTrackRef->dzError() *
                                              negativeTrackRef->dzError() +
                                          zVtxError * zVtxError);
                double dxyerror_neg = sqrt(negativeTrackRef->d0Error() *
                                               negativeTrackRef->d0Error() +
                                           xVtxError * yVtxError);
                double dzSigD2 = safe(dzvtx_neg / dzerror_neg);
                double dxySigD2 = safe(dxyvtx_neg / dxyerror_neg);

                onnxVals_[0] = safe(d0C2Prob);
                onnxVals_[1] = safe(d0Angle3D);
                onnxVals_[2] = safe(d0Angle2D);
                onnxVals_[3] = safe(lVtxMag / sigmaLvtxMag);
                onnxVals_[4] = safe(rVtxMag / sigmaRvtxMag);
                onnxVals_[5] = safe(cur3DIP.value());
                onnxVals_[6] = safe(dca);
                onnxVals_[7] = safe(rVtxMag);
                onnxVals_[8] = safe(d0LineOfFlight.z());
                onnxVals_[9] = safe(dxySigD1);
                onnxVals_[10] = safe(dzSigD1);
                onnxVals_[11] = safe(dxySigD2);
                onnxVals_[12] = safe(dzSigD2);
                onnxVals_[13] = safe(pTD1);
                onnxVals_[14] = safe(pTD2);
                onnxVals_[15] = safe(theD0->pt());
                onnxVals_[16] = safe(theD0->eta());
                onnxVals_[17] = safe(etaD1);
                onnxVals_[18] = safe(etaD2);
                onnxVals_[19] = safe(dEta_dau);

                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
              } else {
                data_.emplace_back(17, 0);
                std::vector<float> &onnxVals_ = data_[0];

                // Compute derived quantities
                float pTD1 = posCandTotalP.perp();
                float pTD2 = negCandTotalP.perp();
                float etaD1 = posCandTotalP.eta();
                float etaD2 = negCandTotalP.eta();
                float phiD1 = posCandTotalP.phi();
                float phiD2 = negCandTotalP.phi();

                // DeltaEta
                float DeltaEta = std::abs(etaD1 - etaD2);

                // DeltaPhi (folded to [0, pi])
                float DeltaPhi = std::abs(phiD1 - phiD2);
                if (DeltaPhi > M_PI)
                  DeltaPhi = 2.0 * M_PI - DeltaPhi;

                // DeltaR
                float DeltaR =
                    std::sqrt(DeltaEta * DeltaEta + DeltaPhi * DeltaPhi);

                // pTRatio = min(pTD1, pTD2) / max(pTD1, pTD2)
                float pTMax = std::max(pTD1, pTD2);
                float pTRatio =
                    (pTMax > 0.0f) ? std::min(pTD1, pTD2) / pTMax : 0.0f;

                // dca3DSig = dca3D / dca3DErr
                float dca3D_val = cur3DIP.value();
                float dca3DErr_val = cur3DIP.error();
                float dca3DSig =
                    (dca3DErr_val > 0.0f) ? dca3D_val / dca3DErr_val : 0.0f;

                // trackDCASig = trackDCA / trackDCAErr
                float trackDCASig = (dcaError > 0.0f) ? dca / dcaError : 0.0f;

                // Fill ONNX input array (17 features) with NaN protection
                onnxVals_[0] = safe(d0C2Prob);               // VtxProb
                onnxVals_[1] = safe(lVtxMag / sigmaLvtxMag); // decayLengthSig3D
                onnxVals_[2] = safe(d0Angle3D);              // alpha3D
                onnxVals_[3] = safe(pTD1);                   // pTD1
                onnxVals_[4] = safe(pTD2);                   // pTD2
                onnxVals_[5] = safe(etaD1);                  // etaD1
                onnxVals_[6] = safe(etaD2);                  // etaD2
                onnxVals_[7] = safe(theD0->pt());            // pT
                onnxVals_[8] = safe(theD0->y());             // y
                onnxVals_[9] = safe(dca3D_val);              // dca3D
                onnxVals_[10] = safe(dca);                   // trackDCA
                onnxVals_[11] = safe(DeltaEta);              // DeltaEta
                onnxVals_[12] = safe(DeltaPhi);              // DeltaPhi
                onnxVals_[13] = safe(DeltaR);                // DeltaR
                onnxVals_[14] = safe(pTRatio);               // pTRatio
                onnxVals_[15] = safe(dca3DSig);              // dca3DSig
                onnxVals_[16] = safe(trackDCASig);           // trackDCASig

                // Run ONNX inference
                std::vector<float> outputs = onnxRuntime_->run(
                    input_names_, data_, input_shapes_, output_names_)[1];
                
                float onnxVal = outputs[1]; // Probability of signal class
                // Final NaN check on output
                if (std::isnan(onnxVal) || std::isinf(onnxVal))
                  onnxVal = 0.0f;

                if (onnxVal > mvaCut) {
                  theD0->addUserFloat("mva", onnxVal);
                  mvaVals_.push_back(onnxVal);
                  theD0s.push_back(std::move(*theD0));
                }
              }
            } else { 
                theD0s.push_back(*theD0);
              }
            }
          }
        }
      }
    } // end if (theTrackRefsNeg.size() >= 2)
  } // end else (wrong-sign)
}
// Get methods

const CCC &D0Fitter::getD0() const { return theD0s; }

const std::vector<float> &D0Fitter::getMVAVals() const { return mvaVals_; }

/*
auto_ptr<edm::ValueMap<float> > D0Fitter::getMVAMap() const {
  return mvaValValueMap;
}
*/

void D0Fitter::resetAll() {
  theD0s.clear();
  mvaVals_.clear();
}
