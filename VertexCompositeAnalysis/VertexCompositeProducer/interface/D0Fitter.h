// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D0Fitter
// 
/**\class D0Fitter D0Fitter.h VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Fitter.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li
//
//

#ifndef VertexCompositeAnalysis__D0_FITTER_H
#define VertexCompositeAnalysis__D0_FITTER_H

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/Common/interface/Ref.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/AdaptiveVertexFit/interface/AdaptiveVertexFitter.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicVertex.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackFromFTSFactory.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "MagneticField/VolumeBasedEngine/interface/VolumeBasedMagneticField.h"

// IP/DCA TOOLS
#include "TrackingTools/GeomPropagators/interface/AnalyticalImpactPointExtrapolator.h"
#include "TrackingTools/PatternTools/interface/TransverseImpactPointExtrapolator.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "RecoVertex/VertexPrimitives/interface/ConvertToFromReco.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"

// #include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/Math/interface/angle.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "Geometry/CommonDetUnit/interface/TrackingGeometry.h"
//#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
//#include "Geometry/TrackerGeometryBuilder/interface/GluedGeomDet.h"
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "CondFormats/GBRForest/interface/GBRForest.h"
#include "CondFormats/DataRecord/interface/GBRWrapperRcd.h"

#include <string>
#include <fstream>
#include <typeinfo>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <map>
using namespace cms::Ort;

class D0Fitter {
 public:
 using CC = pat::CompositeCandidate;
 using CCC = pat::CompositeCandidateCollection;
  D0Fitter(const edm::ParameterSet& theParams, edm::ConsumesCollector && iC, const ONNXRuntime* onnxRuntime = nullptr);
  ~D0Fitter();

  void fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  // Switching to L. Lista's reco::Candidate infrastructure for D0 storage
  const CCC& getD0() const;
  const std::vector<float>& getMVAVals() const; 

//  auto_ptr<edm::ValueMap<float> > getMVAMap() const;
  void resetAll();

 private:
  // STL vector of VertexCompositeCandidate that will be filled with VertexCompositeCandidates by fitAll()
  CCC theD0s;

  // Tracker geometry for discerning hit positions
  const TrackerGeometry* trackerGeom;

  const MagneticField* magField;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bField_esToken_;

  edm::InputTag recoAlg;
  edm::InputTag vtxAlg;
  edm::EDGetTokenT<reco::TrackCollection> token_tracks;
  edm::EDGetTokenT<reco::VertexCollection> token_vertices;
  edm::EDGetTokenT<edm::ValueMap<reco::DeDxData> > token_dedx;
  edm::EDGetTokenT<std::vector<edm::Ptr<pat::PackedCandidate>>> token_track2pc;  // track to PackedCandidate mapping
  bool useDeDx_;  // flag to indicate if dEdx is configured
  edm::EDGetTokenT<reco::BeamSpot> token_beamSpot;
  edm::EDGetTokenT<int> tok_centBinLabel_;

  // Cuts
  double mPiKCutMin;
  double mPiKCutMax;
  double tkDCACut;
  double tkDCACutLow;
  double tkChi2Cut;
  int    tkNhitsCut;
  double tkPtErrCut;
  double tkPtCut;
  double tkEtaCut;
  double tkPtSumCut;
  double tkEtaDiffCut;
  double chi2Cut;
  double rVtxCut;
  double rVtxSigCut;
  double lVtxCut;
  double lVtxSigCut;
  double collinCut2D;
  double collinCut3D;
  double d0MassCut;
  double d0AbsYCut;
  double dauTransImpactSigCut;
  double dauLongImpactSigCut;
  double VtxChiProbCut;
  double dPtCut;
  double alphaCut;
  double alpha2DCut;
  bool   isWrongSign;
  double mvaCut;
  int cBin;

  std::vector<reco::TrackBase::TrackQuality> qualities;
  std::vector<float> mvaVals_;

  //setup mva selector
  bool useAnyMVA_;
  std::vector<bool> useMVA_;
  std::string label_MVA_;
  std::vector<double> min_MVA_;
  std::string mvaType_;
  std::string forestLabel_;
  GBRForest * forest_;
  const ONNXRuntime* onnxRuntime_; 
  bool useForestFromDB_;
  edm::ESGetToken<GBRForest, GBRWrapperRcd> mvaToken_;

//  auto_ptr<edm::ValueMap<float> >mvaValValueMap;
//  MVACollection mvas; 

  std::string dbFileName_;
// Run inference and get outputs
// input_names: list of the names of the input nodes.
// input_values: list of input arrays for each input node. The order of `input_values` must match `input_names`.
// input_shapes: list of `int64_t` arrays specifying the shape of each input node. Can leave empty if the model does not have dynamic axes.
// output_names: names of the output nodes to get outputs from. Empty list means all output nodes.
// batch_size: number of samples in the batch. Each array in `input_values` must have a shape layout of (batch_size, ...).
// Returns: a std::vector<std::vector<float>>, with the order matched to `output_names`.
// When `output_names` is empty, will return all outputs ordered as in `getOutputNames()`.

  std::vector<std::string> input_names_;
  std::vector<std::string> output_names_;
  std::vector<std::vector<int64_t>> input_shapes_;
  FloatArrays data_;

};

#endif
