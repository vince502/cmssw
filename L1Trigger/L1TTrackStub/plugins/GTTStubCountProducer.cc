#include "DataFormats/L1TrackTrigger/interface/GTTStubCounts.h"
#include "DataFormats/L1TrackTrigger/interface/TTDTC.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace l1t {

  class GTTStubCountProducer : public edm::stream::EDProducer<> {
  public:
    explicit GTTStubCountProducer(const edm::ParameterSet&);
    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    void produce(edm::Event&, const edm::EventSetup&) override;

    struct LinkCounts {
      uint32_t total = 0;
      std::vector<uint32_t> byRegion;
      std::vector<uint32_t> byLink;
    };

    static LinkCounts countLinks(const TTDTC&);

    edm::EDGetTokenT<TTStubDetSetVec> rawToken_;
    edm::EDGetTokenT<TTDTC> acceptedToken_;
    edm::EDGetTokenT<TTDTC> lostToken_;
    edm::EDPutTokenT<GTTStubCounts> putToken_;
  };

  GTTStubCountProducer::GTTStubCountProducer(const edm::ParameterSet& config)
      : rawToken_(consumes<TTStubDetSetVec>(config.getParameter<edm::InputTag>("rawStubs"))),
        acceptedToken_(consumes<TTDTC>(config.getParameter<edm::InputTag>("acceptedStubs"))),
        lostToken_(consumes<TTDTC>(config.getParameter<edm::InputTag>("lostStubs"))),
        putToken_(produces<GTTStubCounts>()) {}

  GTTStubCountProducer::LinkCounts GTTStubCountProducer::countLinks(const TTDTC& dtc) {
    LinkCounts counts;
    counts.byRegion.reserve(dtc.tfpRegions().size());
    counts.byLink.reserve(dtc.tfpRegions().size() * dtc.tfpChannels().size());

    for (const int region : dtc.tfpRegions()) {
      uint32_t regionCount = 0;
      for (const int channel : dtc.tfpChannels()) {
        uint32_t linkCount = 0;
        for (const auto& frame : dtc.stream(region, channel)) {
          // Empty time-multiplexed frames carry a null TTStubRef.
          if (frame.first.isNonnull())
            ++linkCount;
        }
        counts.byLink.push_back(linkCount);
        regionCount += linkCount;
      }
      counts.byRegion.push_back(regionCount);
      counts.total += regionCount;
    }
    return counts;
  }

  void GTTStubCountProducer::produce(edm::Event& event, const edm::EventSetup&) {
    const auto& rawStubs = event.get(rawToken_);
    auto accepted = countLinks(event.get(acceptedToken_));
    auto lost = countLinks(event.get(lostToken_));

    uint32_t rawCount = 0;
    for (const auto& detset : rawStubs)
      rawCount += detset.size();

    event.emplace(putToken_,
                  rawCount,
                  accepted.total,
                  lost.total,
                  std::move(accepted.byRegion),
                  std::move(lost.byRegion),
                  std::move(accepted.byLink),
                  std::move(lost.byLink));
  }

  void GTTStubCountProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("rawStubs", edm::InputTag("TTStubsFromPhase2TrackerDigis", "StubAccepted"));
    description.add<edm::InputTag>("acceptedStubs", edm::InputTag("ProducerDTC", "StubAccepted"));
    description.add<edm::InputTag>("lostStubs", edm::InputTag("ProducerDTC", "StubLost"));
    descriptions.add("gttStubCounts", description);
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GTTStubCountProducer);
