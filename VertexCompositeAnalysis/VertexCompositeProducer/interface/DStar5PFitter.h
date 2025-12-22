// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      DStar5PFitter
// 
/**\class DStar5PFitter DStar5PFitter.h VertexCompositeAnalysis/VertexCompositeProducer/interface/DStar5PFitter.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Class Author:  Soohwan Lee
//
//

#ifndef VertexCompositeAnalysis__DStar_5P_FITTER_H
#define VertexCompositeAnalysis__DStar_5P_FITTER_H

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

// #include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/Math/interface/angle.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"

// DCA
#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "Geometry/CommonDetUnit/interface/TrackingGeometry.h"
//#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
//#include "Geometry/TrackerGeometryBuilder/interface/GluedGeomDet.h"

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

class DStar5PFitter {
 public:
  using CC = pat::CompositeCandidate;
  using CCC = pat::CompositeCandidateCollection;

  DStar5PFitter(const edm::ParameterSet& theParams, edm::ConsumesCollector && iC);
  ~DStar5PFitter();

  void fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  const CCC& getDStar() const;
  const std::vector<float>& getDCAVals() const;
  const std::vector<float>& getDCAErrs() const;
  const std::vector<float>& getMVAVals() const; 
  const std::vector<float>& getDeltaM() const; 

  void resetAll();

 private:
  // Helper function to check if two tracks are the same (avoid double counting)
  bool isSameTrack(const reco::Track* trk1, const reco::Track* trk2, double tolerance = 1e-5) const;
  // STL vector of pat::CompositeCandidate
  CCC theDStars;

  // Tracker geometry for discerning hit positions
  const TrackerGeometry* trackerGeom;

  const MagneticField* magField;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bField_esToken_;

  edm::InputTag recoAlg;
  edm::InputTag vtxAlg;
  edm::EDGetTokenT<reco::TrackCollection> token_tracks;
  edm::EDGetTokenT<reco::VertexCollection> token_vertices;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> token_d0cand;
  edm::EDGetTokenT<edm::ValueMap<reco::DeDxData> > token_dedx;
  edm::EDGetTokenT<std::vector<edm::Ptr<pat::PackedCandidate>>> token_track2pc;
  bool useDeDx_;
  edm::EDGetTokenT<reco::BeamSpot> token_beamSpot;

  // Cuts
  double mPiKCutMin;
  double mPiKCutMax;
  double tkDCACut;
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
  double dStarMassCut;
  double d0MassCut;
  double dauTransImpactSigCut;
  double dauLongImpactSigCut;
  double VtxChiProbCut;
  double dPtCut;
  double alphaCut;
  double alpha2DCut;
  bool   isWrongSign;

  std::vector<reco::TrackBase::TrackQuality> qualities;

  //setup mva selector
  bool useAnyMVA_;
  std::vector<bool> useMVA_;
  std::vector<double> min_MVA_;
  std::string mvaType_;
  std::string forestLabel_;
  GBRForest * forest_;
  bool useForestFromDB_;

  std::vector<float> mvaVals_;

//  auto_ptr<edm::ValueMap<float> >mvaValValueMap;
//  MVACollection mvas; 
  // DCA
  std::vector<float> dcaVals_;
  std::vector<float> dcaErrs_;
  std::vector<float> detlaM_;

  std::string dbFileName_;

};

#endif
