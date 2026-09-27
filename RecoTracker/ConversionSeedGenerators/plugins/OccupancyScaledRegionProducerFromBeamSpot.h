#ifndef RecoTracker_TkTrackingRegions_OccupancyScaledRegionProducerFromBeamSpot_H
#define RecoTracker_TkTrackingRegions_OccupancyScaledRegionProducerFromBeamSpot_H

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"

#include <algorithm>
#include "FWCore/Framework/interface/Event.h"
#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducer.h"
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/DescriptionCloner.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "RecoTracker/Record/interface/TrackerMultipleScatteringRecord.h"
#include "RecoTracker/TkMSParametrization/interface/MultipleScatteringParametrisationMaker.h"

class OccupancyScaledRegionProducerFromBeamSpot : public TrackingRegionProducer {
public:
  OccupancyScaledRegionProducerFromBeamSpot(const edm::ParameterSet& cfg, edm::ConsumesCollector&& iC)
      : OccupancyScaledRegionProducerFromBeamSpot(cfg, iC) {}
  OccupancyScaledRegionProducerFromBeamSpot(const edm::ParameterSet& cfg, edm::ConsumesCollector& iC) {
    edm::ParameterSet regionPSet = cfg.getParameter<edm::ParameterSet>("RegionPSet");
    thePtMin = regionPSet.getParameter<double>("ptMin");
    theOriginRadius = regionPSet.getParameter<double>("originRadius");
    theOriginRScaling = regionPSet.existsAs<bool>("originRScaling4BigEvts") &&
                        regionPSet.getParameter<bool>("originRScaling4BigEvts");
    if (theOriginRScaling) {
      theMinOriginR = regionPSet.getParameter<double>("minOriginR");
      theScalingStart = regionPSet.getParameter<double>("scalingStartNPix");
      theScalingEnd = regionPSet.getParameter<double>("scalingEndNPix");
      if (theScalingEnd <= theScalingStart || theMinOriginR < 0 || theMinOriginR > theOriginRadius) {
        throw cms::Exception("Configuration") << "Invalid origin-radius occupancy scaling parameters";
      }
      token_pixelClusters = iC.consumes<edmNew::DetSetVector<SiPixelCluster>>(
          regionPSet.getParameter<edm::InputTag>("pixelClustersForScaling"));
    }
    if (!regionPSet.existsAs<double>("nSigmaZ") && !regionPSet.existsAs<double>("originHalfLength")) {
      throw cms::Exception("Configuration") << "OccupancyScaledRegionProducerFromBeamSpot: at least one of nSigmaZ, "
                                               "originHalfLength must be present in the cfg.\n";
    }
    theNSigmaZ = (regionPSet.existsAs<double>("nSigmaZ") ? regionPSet.getParameter<double>("nSigmaZ") : 0.0);
    theOriginHalfLength =
        (regionPSet.existsAs<double>("originHalfLength") ? regionPSet.getParameter<double>("originHalfLength") : 0.0);
    token_beamSpot = iC.consumes<reco::BeamSpot>(regionPSet.getParameter<edm::InputTag>("beamSpot"));
    thePrecise = regionPSet.getParameter<bool>("precise");
    theUseMS =
        (regionPSet.existsAs<bool>("useMultipleScattering") ? regionPSet.getParameter<bool>("useMultipleScattering")
                                                            : false);
    if (theUseMS) {
      token_msmaker = iC.esConsumes();
    }
  }

  ~OccupancyScaledRegionProducerFromBeamSpot() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    {
      edm::ParameterSetDescription desc;

      desc.add<bool>("precise", true);
      desc.add<bool>("useMultipleScattering", false);
      desc.add<double>("nSigmaZ", 4.0);
      desc.add<double>("originHalfLength", 0.0);  // this is the default in constructor
      desc.add<double>("originRadius", 0.2);
      desc.add<bool>("originRScaling4BigEvts", false);
      desc.add<double>("minOriginR", 0.0);
      desc.add<double>("scalingStartNPix", 20000.0);
      desc.add<double>("scalingEndNPix", 35000.0);
      desc.add<edm::InputTag>("pixelClustersForScaling", edm::InputTag("siPixelClusters"));
      desc.add<double>("ptMin", 0.9);
      desc.add<edm::InputTag>("beamSpot", edm::InputTag("offlineBeamSpot"));

      // Only for backwards-compatibility
      edm::ParameterSetDescription descRegion;
      descRegion.add<edm::ParameterSetDescription>("RegionPSet", desc);

      descriptions.add("globalTrackingRegionFromBeamSpot", descRegion);
    }

    {
      edm::DescriptionCloner clone;

      clone.set<double>("RegionPSet.nSigmaZ", 0.0);  // this is the default in constructor
      clone.set<double>("RegionPSet.originHalfLength", 21.2);
      descriptions.add("globalTrackingRegionFromBeamSpotFixedZ", clone);
    }
  }

  std::vector<std::unique_ptr<TrackingRegion> > regions(const edm::Event& ev,
                                                        const edm::EventSetup& es) const override {
    std::vector<std::unique_ptr<TrackingRegion> > result;
    edm::Handle<reco::BeamSpot> bsHandle;
    ev.getByToken(token_beamSpot, bsHandle);
    if (bsHandle.isValid()) {
      const reco::BeamSpot& bs = *bsHandle;

      GlobalPoint origin(bs.x0(), bs.y0(), bs.z0());

      const MultipleScatteringParametrisationMaker* msmaker = nullptr;
      if (theUseMS) {
        msmaker = &es.getData(token_msmaker);
      }

      double originRadius = theOriginRadius;
      if (theOriginRScaling) {
        const auto& pixelClusters = ev.get(token_pixelClusters);
        const double fraction = std::clamp((pixelClusters.dataSize() - theScalingStart) /
                                               (theScalingEnd - theScalingStart),
                                           0.0,
                                           1.0);
        originRadius -= (theOriginRadius - theMinOriginR) * fraction;
        // As in the HI MixedTriplet region: no seeds when the scaled radius reaches zero.
        if (originRadius == 0.0)
          return result;
      }

      result.push_back(std::make_unique<GlobalTrackingRegion>(thePtMin,
                                                              origin,
                                                              originRadius,
                                                              std::max(theNSigmaZ * bs.sigmaZ(), theOriginHalfLength),
                                                              thePrecise,
                                                              theUseMS,
                                                              msmaker));
    }
    return result;
  }

private:
  double thePtMin;
  double theOriginRadius;
  bool theOriginRScaling = false;
  double theMinOriginR = 0.0;
  double theScalingStart = 0.0;
  double theScalingEnd = 0.0;
  edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster>> token_pixelClusters;
  double theOriginHalfLength;
  double theNSigmaZ;
  edm::EDGetTokenT<reco::BeamSpot> token_beamSpot;
  edm::ESGetToken<MultipleScatteringParametrisationMaker, TrackerMultipleScatteringRecord> token_msmaker;
  bool thePrecise;
  bool theUseMS;
};

#endif
