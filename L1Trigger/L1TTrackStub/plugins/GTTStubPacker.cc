#include "DataFormats/Common/interface/View.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubCounts.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "L1Trigger/DemonstratorTools/interface/codecs/tracks.h"
#include "L1Trigger/L1TTrackStub/interface/GTTStubLinkCodec.h"

#include <array>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>

namespace l1t {

  class GTTStubPacker : public edm::global::EDProducer<> {
  public:
    explicit GTTStubPacker(const edm::ParameterSet&);
    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

    edm::EDGetTokenT<GTTStubCounts> countsToken_;
    edm::EDGetTokenT<edm::View<TTTrack<Ref_Phase2TrackerDigi_>>> tracksToken_;
    edm::EDPutTokenT<tt::Streams> putToken_;
    std::vector<unsigned int> regionLinks_;
    unsigned int metadataTrackSlot_;
    bool throwOnTrackSlotOccupied_;
  };

  GTTStubPacker::GTTStubPacker(const edm::ParameterSet& config)
      : countsToken_(consumes<GTTStubCounts>(config.getParameter<edm::InputTag>("counts"))),
        tracksToken_(
            consumes<edm::View<TTTrack<Ref_Phase2TrackerDigi_>>>(config.getParameter<edm::InputTag>("tracks"))),
        putToken_(produces<tt::Streams>()),
        regionLinks_(config.getParameter<std::vector<unsigned int>>("regionLinks")),
        metadataTrackSlot_(config.getParameter<unsigned int>("metadataTrackSlot")),
        throwOnTrackSlotOccupied_(config.getParameter<bool>("throwOnTrackSlotOccupied")) {
    const std::set<unsigned int> uniqueLinks(regionLinks_.begin(), regionLinks_.end());
    if (regionLinks_.size() != GTTStubCountWord::kNumRegions || uniqueLinks.size() != regionLinks_.size() ||
        *uniqueLinks.rbegin() >= GTTStubLinkCodec::kNumLinks)
      throw cms::Exception("Configuration")
          << "regionLinks must contain " << GTTStubCountWord::kNumRegions << " unique link indices smaller than "
          << GTTStubLinkCodec::kNumLinks << ".";
    if (metadataTrackSlot_ >= GTTStubLinkCodec::kTracksPerLink)
      throw cms::Exception("Configuration")
          << "metadataTrackSlot must be smaller than " << GTTStubLinkCodec::kTracksPerLink << ".";
  }

  void GTTStubPacker::produce(edm::StreamID, edm::Event& event, const edm::EventSetup&) const {
    const auto& counts = event.get(countsToken_);
    const auto& tracks = event.get(tracksToken_);
    const auto& byRegion = counts.acceptedByRegion();

    if (byRegion.size() != GTTStubCountWord::kNumRegions)
      throw cms::Exception("DataCorruption")
          << "Expected " << GTTStubCountWord::kNumRegions << " Track Finder regions, got " << byRegion.size() << ".";

    std::array<unsigned int, GTTStubLinkCodec::kNumLinks> tracksByLink{};
    for (const auto& track : tracks) {
      const unsigned int link = l1t::demo::codecs::gttLinkID(track);
      if (link >= tracksByLink.size())
        throw cms::Exception("DataCorruption") << "Track maps to invalid GTT link " << link << ".";
      if (++tracksByLink[link] > GTTStubLinkCodec::kTracksPerLink)
        throw cms::Exception("Overflow") << "GTT link " << link << " contains more than "
                                         << GTTStubLinkCodec::kTracksPerLink
                                         << " tracks and cannot fit in one standard packet.";
    }

    const auto encodedTracks = l1t::demo::codecs::encodeTracks(tracks);
    tt::Streams output(GTTStubLinkCodec::kNumLinks);
    for (unsigned int link = 0; link < output.size(); ++link) {
      output[link].reserve(encodedTracks[link].size());
      for (const auto& frame : encodedTracks[link])
        output[link].emplace_back(frame.to_uint64());
    }

    for (unsigned int region = 0; region < byRegion.size(); ++region) {
      auto& stream = output[regionLinks_[region]];
      if (stream.size() != GTTStubLinkCodec::kFramesPerLink)
        throw cms::Exception("LogicError") << "The standard GTT track codec produced " << stream.size()
                                           << " frames instead of " << GTTStubLinkCodec::kFramesPerLink << ".";
      const bool trackSlotOccupied = GTTStubLinkCodec::slotOccupied(stream, metadataTrackSlot_);
      if (throwOnTrackSlotOccupied_ && trackSlotOccupied)
        throw cms::Exception("Overflow")
            << "Reserved metadata track slot " << metadataTrackSlot_ << " on link " << regionLinks_[region]
            << " contains a valid track. The provisional protocol cannot preserve this event without truncation.";
      const auto word = GTTStubCountWord::pack(byRegion[region], region, true, trackSlotOccupied);
      GTTStubLinkCodec::insert(stream, metadataTrackSlot_, word.word());
    }
    event.emplace(putToken_, std::move(output));
  }

  void GTTStubPacker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("counts", edm::InputTag("apolloGTTStubCounts"));
    description.add<edm::InputTag>("tracks", edm::InputTag("l1tTTTracksFromTrackletEmulation", "Level1TTTracks"));
    description.add<std::vector<unsigned int>>("regionLinks", {0, 2, 4, 6, 8, 10, 12, 14, 16});
    description.add<unsigned int>("metadataTrackSlot", GTTStubLinkCodec::kDefaultMetadataTrackSlot);
    description.add<bool>("throwOnTrackSlotOccupied", true);
    descriptions.add("gttStubPacker", description);
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GTTStubPacker);
