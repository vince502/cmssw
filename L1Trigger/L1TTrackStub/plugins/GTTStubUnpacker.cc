#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h"
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

#include <cstdint>
#include <iomanip>
#include <limits>
#include <set>
#include <utility>
#include <vector>

namespace l1t {

  class GTTStubUnpacker : public edm::global::EDProducer<> {
  public:
    explicit GTTStubUnpacker(const edm::ParameterSet&);
    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    static constexpr unsigned int kNumFitParameters = 4;
    static constexpr double kMagneticFieldTesla = 3.8;

    void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

    edm::EDGetTokenT<tt::Streams> inputToken_;
    edm::EDPutTokenT<GTTStubPrimitive> putToken_;
    edm::EDPutTokenT<std::vector<TTTrack<Ref_Phase2TrackerDigi_>>> tracksPutToken_;
    std::vector<unsigned int> regionLinks_;
    unsigned int metadataTrackSlot_;
    bool throwOnMalformed_;
  };

  GTTStubUnpacker::GTTStubUnpacker(const edm::ParameterSet& config)
      : inputToken_(consumes<tt::Streams>(config.getParameter<edm::InputTag>("src"))),
        putToken_(produces<GTTStubPrimitive>()),
        tracksPutToken_(produces<std::vector<TTTrack<Ref_Phase2TrackerDigi_>>>("Level1TTTracks")),
        regionLinks_(config.getParameter<std::vector<unsigned int>>("regionLinks")),
        metadataTrackSlot_(config.getParameter<unsigned int>("metadataTrackSlot")),
        throwOnMalformed_(config.getParameter<bool>("throwOnMalformed")) {
    const std::set<unsigned int> uniqueLinks(regionLinks_.begin(), regionLinks_.end());
    if (regionLinks_.size() != GTTStubCountWord::kNumRegions || uniqueLinks.size() != regionLinks_.size() ||
        *uniqueLinks.rbegin() >= GTTStubLinkCodec::kNumLinks)
      throw cms::Exception("Configuration") << "regionLinks must contain " << GTTStubCountWord::kNumRegions
                                            << " unique entries smaller than " << GTTStubLinkCodec::kNumLinks << ".";
    if (metadataTrackSlot_ >= GTTStubLinkCodec::kTracksPerLink)
      throw cms::Exception("Configuration")
          << "metadataTrackSlot must be smaller than " << GTTStubLinkCodec::kTracksPerLink << ".";
  }

  void GTTStubUnpacker::produce(edm::StreamID, edm::Event& event, const edm::EventSetup&) const {
    const auto& streams = event.get(inputToken_);
    const bool completePacket = GTTStubLinkCodec::packetStructurallyValid(streams);
    if (throwOnMalformed_ && !completePacket)
      throw cms::Exception("ProductNotFound")
          << "The Apollo track packet must contain exactly " << GTTStubLinkCodec::kNumLinks << " links with exactly "
          << GTTStubLinkCodec::kFramesPerLink << " frames per link.";

    uint32_t total = 0;
    uint32_t maxRegionCount = 0;
    uint32_t maxRegion = 0;
    uint32_t activeRegionMask = 0;
    uint32_t trackOverflowMask = 0;
    bool overflow = false;
    bool valid = completePacket;
    unsigned int formatVersion = GTTStubCountWord::kFormatVersion;
    std::vector<TTTrack<Ref_Phase2TrackerDigi_>> decodedTracks;

    if (completePacket) {
      for (unsigned int link = 0; link < streams.size(); ++link) {
        std::vector<ap_uint<64>> frames;
        frames.reserve(streams[link].size());
        for (const auto& frame : streams[link])
          frames.emplace_back(frame.to_ullong());

        for (const auto& trackWord : l1t::demo::codecs::decodeTracks(frames)) {
          if (!trackWord.getValid())
            continue;
          const double decodedRinv = trackWord.getRinv();
          TTTrack<Ref_Phase2TrackerDigi_> track(
              decodedRinv == 0. ? std::numeric_limits<double>::epsilon() : decodedRinv,
              trackWord.getPhi(),
              trackWord.getTanl(),
              trackWord.getZ0(),
              trackWord.getD0(),
              trackWord.getChi2RPhi(),
              trackWord.getChi2RZ(),
              trackWord.getMVAQuality(),
              trackWord.getMVAOther(),
              0.,
              trackWord.getHitPattern(),
              kNumFitParameters,
              kMagneticFieldTesla);
          track.setChi2BendRed(trackWord.getBendChi2());
          static_cast<TTTrack_TrackWord&>(track) = trackWord;
          const auto sectors = l1t::demo::codecs::sectorsEtaPhiFromGTTLinkID(link);
          track.setEtaSector(sectors.first);
          track.setPhiSector(sectors.second);
          decodedTracks.push_back(std::move(track));
        }
      }
    }

    for (unsigned int expectedRegion = 0; expectedRegion < regionLinks_.size(); ++expectedRegion) {
      const unsigned int link = regionLinks_[expectedRegion];
      if (link >= streams.size() || streams[link].size() != GTTStubLinkCodec::kFramesPerLink) {
        valid = false;
        if (throwOnMalformed_)
          throw cms::Exception("ProductNotFound")
              << "The complete Apollo track packet on link " << link << " is absent.";
        continue;
      }

      const GTTStubCountWord word(GTTStubLinkCodec::extract(streams[link], metadataTrackSlot_));
      const bool wordValid = GTTStubLinkCodec::carrierUpperBitsZero(streams[link], metadataTrackSlot_) &&
                             word.structurallyValid() && word.region() == expectedRegion;
      valid &= wordValid;
      if (throwOnMalformed_ && !wordValid)
        throw cms::Exception("DataCorruption") << "Malformed regional Apollo-to-Serenity GTT stub word 0x" << std::hex
                                               << word.word() << std::dec << " on link " << link << ".";
      if (!wordValid)
        continue;

      total += word.count();
      overflow |= word.overflow();
      formatVersion = word.formatVersion();
      activeRegionMask |= static_cast<uint32_t>(word.count() != 0) << expectedRegion;
      trackOverflowMask |= static_cast<uint32_t>(word.trackSlotOverflow()) << expectedRegion;
      if (word.count() > maxRegionCount) {
        maxRegionCount = word.count();
        maxRegion = expectedRegion;
      }
    }

    event.emplace(putToken_,
                  total,
                  maxRegionCount,
                  maxRegion,
                  activeRegionMask,
                  trackOverflowMask,
                  overflow,
                  valid,
                  formatVersion);
    event.emplace(tracksPutToken_, std::move(decodedTracks));
  }

  void GTTStubUnpacker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("src", edm::InputTag("apolloGTTStubPacker"));
    description.add<std::vector<unsigned int>>("regionLinks", {0, 2, 4, 6, 8, 10, 12, 14, 16});
    description.add<unsigned int>("metadataTrackSlot", GTTStubLinkCodec::kDefaultMetadataTrackSlot);
    description.add<bool>("throwOnMalformed", true);
    descriptions.add("gttStubUnpacker", description);
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GTTStubUnpacker);
