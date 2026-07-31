#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubCounts.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/L1Trigger/interface/P2GTAlgoBlock.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "L1Trigger/DemonstratorTools/interface/codecs/tracks.h"
#include "L1Trigger/L1TTrackStub/interface/GTTStubLinkCodec.h"

#include "TH1D.h"
#include "TH2D.h"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <set>
#include <string>
#include <vector>

namespace l1t {

  class GTTStubValidation : public edm::one::EDAnalyzer<edm::one::SharedResources> {
  public:
    explicit GTTStubValidation(const edm::ParameterSet&);
    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    void analyze(const edm::Event&, const edm::EventSetup&) override;

    edm::EDGetTokenT<GTTStubCounts> countsToken_;
    edm::EDGetTokenT<tt::Streams> framesToken_;
    edm::EDGetTokenT<GTTStubPrimitive> primitiveToken_;
    edm::EDGetTokenT<edm::View<TTTrack<Ref_Phase2TrackerDigi_>>> tracksToken_;
    edm::EDGetTokenT<std::vector<TTTrack<Ref_Phase2TrackerDigi_>>> decodedTracksToken_;
    edm::EDGetTokenT<P2GTAlgoBlockMap> algoBlocksToken_;
    std::string algorithmName_;
    std::vector<unsigned int> regionLinks_;
    unsigned int metadataTrackSlot_;
    uint32_t minStubCount_;
    uint32_t maxStubCount_;
    bool rejectOverflow_;
    bool rejectTrackOverflow_;
    bool strict_;

    TH1D* rawCount_;
    TH1D* acceptedCount_;
    TH1D* lostCount_;
    TH1D* trackCount_;
    TH1D* maxRegionCount_;
    TH1D* trackOverflow_;
    TH1D* decision_;
    TH2D* acceptedVsRaw_;
  };

  GTTStubValidation::GTTStubValidation(const edm::ParameterSet& config)
      : countsToken_(consumes<GTTStubCounts>(config.getParameter<edm::InputTag>("counts"))),
        framesToken_(consumes<tt::Streams>(config.getParameter<edm::InputTag>("frames"))),
        primitiveToken_(consumes<GTTStubPrimitive>(config.getParameter<edm::InputTag>("primitive"))),
        tracksToken_(
            consumes<edm::View<TTTrack<Ref_Phase2TrackerDigi_>>>(config.getParameter<edm::InputTag>("tracks"))),
        decodedTracksToken_(consumes<std::vector<TTTrack<Ref_Phase2TrackerDigi_>>>(
            config.getParameter<edm::InputTag>("decodedTracks"))),
        algoBlocksToken_(consumes<P2GTAlgoBlockMap>(config.getParameter<edm::InputTag>("algoBlocks"))),
        algorithmName_(config.getParameter<std::string>("algorithmName")),
        regionLinks_(config.getParameter<std::vector<unsigned int>>("regionLinks")),
        metadataTrackSlot_(config.getParameter<unsigned int>("metadataTrackSlot")),
        minStubCount_(config.getParameter<unsigned int>("minStubCount")),
        maxStubCount_(config.getParameter<unsigned int>("maxStubCount")),
        rejectOverflow_(config.getParameter<bool>("rejectOverflow")),
        rejectTrackOverflow_(config.getParameter<bool>("rejectTrackOverflow")),
        strict_(config.getParameter<bool>("strict")) {
    const std::set<unsigned int> uniqueLinks(regionLinks_.begin(), regionLinks_.end());
    if (regionLinks_.size() != GTTStubCountWord::kNumRegions || uniqueLinks.size() != regionLinks_.size() ||
        *uniqueLinks.rbegin() >= GTTStubLinkCodec::kNumLinks)
      throw cms::Exception("Configuration") << "regionLinks must contain " << GTTStubCountWord::kNumRegions
                                            << " unique entries smaller than " << GTTStubLinkCodec::kNumLinks << ".";
    if (metadataTrackSlot_ >= GTTStubLinkCodec::kTracksPerLink)
      throw cms::Exception("Configuration")
          << "metadataTrackSlot must be smaller than " << GTTStubLinkCodec::kTracksPerLink << ".";
    if (minStubCount_ > maxStubCount_)
      throw cms::Exception("Configuration") << "minStubCount must not exceed maxStubCount.";

    usesResource(TFileService::kSharedResource);
    edm::Service<TFileService> fs;
    rawCount_ = fs->make<TH1D>("rawStubCount", "Physical accepted TTStub count;stubs;events", 400, 0, 40000);
    acceptedCount_ =
        fs->make<TH1D>("acceptedStubCount", "DTC frames delivered to Track Finder;frames;events", 400, 0, 40000);
    lostCount_ = fs->make<TH1D>("lostStubCount", "DTC-truncated stub frames;frames;events", 400, 0, 40000);
    trackCount_ = fs->make<TH1D>("trackCount", "Track Finder output tracks;tracks;events", 400, 0, 4000);
    maxRegionCount_ =
        fs->make<TH1D>("maxRegionStubCount", "Maximum processing-region occupancy;frames;events", 400, 0, 10000);
    trackOverflow_ =
        fs->make<TH1D>("trackSlotOverflow", "Reserved metadata slot displaced a track;overflow;events", 2, 0, 2);
    decision_ = fs->make<TH1D>("decision", "L1 GTT low-stub-occupancy decision;decision;events", 2, 0, 2);
    acceptedVsRaw_ = fs->make<TH2D>("acceptedVsRaw",
                                    "DTC frame count versus physical TTStubs;physical TTStubs;DTC frames",
                                    200,
                                    0,
                                    40000,
                                    200,
                                    0,
                                    40000);
  }

  void GTTStubValidation::analyze(const edm::Event& event, const edm::EventSetup&) {
    const auto& counts = event.get(countsToken_);
    const auto& frames = event.get(framesToken_);
    const auto& primitive = event.get(primitiveToken_);
    const auto& tracks = event.get(tracksToken_);
    const auto& decodedTracks = event.get(decodedTracksToken_);
    const auto& algoBlocks = event.get(algoBlocksToken_);

    const auto& byRegion = counts.acceptedByRegion();
    if (byRegion.size() != regionLinks_.size())
      throw cms::Exception("LogicError") << "Regional count and link-map sizes differ.";

    const auto maximum = std::max_element(byRegion.begin(), byRegion.end());
    const auto encodedTracks = l1t::demo::codecs::encodeTracks(tracks);
    tt::Streams expectedFrames(GTTStubLinkCodec::kNumLinks);
    for (unsigned int link = 0; link < expectedFrames.size(); ++link) {
      expectedFrames[link].reserve(encodedTracks[link].size());
      for (const auto& frame : encodedTracks[link])
        expectedFrames[link].emplace_back(frame.to_uint64());
    }

    uint32_t expectedRegionMask = 0;
    uint32_t expectedTrackOverflowMask = 0;
    uint32_t expectedDecodedTotal = 0;
    uint32_t expectedMaxRegion = 0;
    uint32_t expectedMaxRegionCount = 0;
    bool expectedOverflow = false;
    bool regionalWordsExact = true;
    for (unsigned int region = 0; region < byRegion.size(); ++region) {
      const unsigned int link = regionLinks_[region];
      if (link >= frames.size() || frames[link].size() != GTTStubLinkCodec::kFramesPerLink)
        throw cms::Exception("ProductNotFound") << "Complete track packet is absent during validation.";
      const bool expectedTrackOverflow = GTTStubLinkCodec::slotOccupied(expectedFrames[link], metadataTrackSlot_);
      const GTTStubCountWord expectedWord =
          GTTStubCountWord::pack(byRegion[region], region, true, expectedTrackOverflow);
      GTTStubLinkCodec::insert(expectedFrames[link], metadataTrackSlot_, expectedWord.word());
      const GTTStubCountWord word(GTTStubLinkCodec::extract(frames[link], metadataTrackSlot_));
      const uint32_t expectedCount = std::min<uint64_t>(byRegion[region], GTTStubCountWord::kCountMask);
      expectedDecodedTotal += expectedCount;
      expectedOverflow |= byRegion[region] > GTTStubCountWord::kCountMask;
      expectedRegionMask |= static_cast<uint32_t>(byRegion[region] != 0) << region;
      expectedTrackOverflowMask |= static_cast<uint32_t>(expectedTrackOverflow) << region;
      if (expectedCount > expectedMaxRegionCount) {
        expectedMaxRegionCount = expectedCount;
        expectedMaxRegion = region;
      }
      regionalWordsExact &= GTTStubLinkCodec::carrierUpperBitsZero(frames[link], metadataTrackSlot_) &&
                            word.structurallyValid() && word.region() == region && word.count() == expectedCount &&
                            word.overflow() == (byRegion[region] > GTTStubCountWord::kCountMask) &&
                            word.trackSlotOverflow() == expectedTrackOverflow;
    }
    const bool trackPayloadExact = frames == expectedFrames;
    bool decodedTracksExact = true;
    unsigned int decodedTrackIndex = 0;
    for (unsigned int link = 0; link < expectedFrames.size(); ++link) {
      std::vector<ap_uint<64>> linkFrames;
      linkFrames.reserve(expectedFrames[link].size());
      for (const auto& frame : expectedFrames[link])
        linkFrames.emplace_back(frame.to_ullong());
      const auto sectors = l1t::demo::codecs::sectorsEtaPhiFromGTTLinkID(link);
      for (const auto& trackWord : l1t::demo::codecs::decodeTracks(linkFrames)) {
        if (!trackWord.getValid())
          continue;
        if (decodedTrackIndex >= decodedTracks.size()) {
          decodedTracksExact = false;
          continue;
        }
        const auto& decodedTrack = decodedTracks[decodedTrackIndex++];
        decodedTracksExact &= decodedTrack.getTrackWord() == trackWord.getTrackWord() &&
                              decodedTrack.etaSector() == sectors.first && decodedTrack.phiSector() == sectors.second;
      }
    }
    decodedTracksExact &= decodedTrackIndex == decodedTracks.size();

    const bool expectedDecision = (!rejectOverflow_ || !expectedOverflow) &&
                                  (!rejectTrackOverflow_ || expectedTrackOverflowMask == 0) &&
                                  expectedDecodedTotal >= minStubCount_ && expectedDecodedTotal <= maxStubCount_;

    const auto algorithm = algoBlocks.find(algorithmName_);
    if (algorithm == algoBlocks.end())
      throw cms::Exception("ProductNotFound") << "P2GT algorithm block '" << algorithmName_ << "' is absent.";

    const uint32_t regionalTotal = std::accumulate(byRegion.begin(), byRegion.end(), uint32_t{0});
    const bool exact =
        regionalWordsExact && trackPayloadExact && decodedTracksExact && primitive.structurallyValid() &&
        primitive.formatVersion() == GTTStubCountWord::kFormatVersion && counts.accepted() == regionalTotal &&
        primitive.acceptedStubCount() == expectedDecodedTotal && primitive.maxRegionCount() == expectedMaxRegionCount &&
        primitive.maxRegion() == expectedMaxRegion && primitive.activeRegionMask() == expectedRegionMask &&
        primitive.trackOverflowMask() == expectedTrackOverflowMask && primitive.overflow() == expectedOverflow &&
        algorithm->second.decisionBeforeBxMaskAndPrescale() == expectedDecision &&
        algorithm->second.decisionBeforePrescale() == expectedDecision &&
        algorithm->second.decisionFinal() == expectedDecision;
    if (strict_ && !exact)
      throw cms::Exception("LogicError") << "Apollo packer, Serenity unpacker, or P2GT decision mismatch.";

    rawCount_->Fill(counts.raw());
    acceptedCount_->Fill(counts.accepted());
    lostCount_->Fill(counts.lost());
    trackCount_->Fill(tracks.size());
    maxRegionCount_->Fill(*maximum);
    trackOverflow_->Fill(primitive.trackOverflow());
    decision_->Fill(algorithm->second.decisionBeforeBxMaskAndPrescale());
    acceptedVsRaw_->Fill(counts.raw(), counts.accepted());
  }

  void GTTStubValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("counts", edm::InputTag("apolloGTTStubCounts"));
    description.add<edm::InputTag>("frames", edm::InputTag("apolloGTTStubPacker"));
    description.add<edm::InputTag>("primitive", edm::InputTag("serenityGTTStubUnpacker"));
    description.add<edm::InputTag>("tracks", edm::InputTag("l1tTTTracksFromTrackletEmulation", "Level1TTTracks"));
    description.add<edm::InputTag>("decodedTracks", edm::InputTag("serenityGTTStubUnpacker", "Level1TTTracks"));
    description.add<edm::InputTag>("algoBlocks", edm::InputTag("gttStubAlgoBlocks"));
    description.add<std::string>("algorithmName", "L1_GTT_LowStubOccupancy");
    description.add<std::vector<unsigned int>>("regionLinks", {0, 2, 4, 6, 8, 10, 12, 14, 16});
    description.add<unsigned int>("metadataTrackSlot", GTTStubLinkCodec::kDefaultMetadataTrackSlot);
    description.add<unsigned int>("minStubCount", 0);
    description.add<unsigned int>("maxStubCount", GTTStubCountWord::kNumRegions * GTTStubCountWord::kCountMask);
    description.add<bool>("rejectOverflow", true);
    description.add<bool>("rejectTrackOverflow", true);
    description.add<bool>("strict", true);
    descriptions.add("gttStubValidation", description);
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GTTStubValidation);
