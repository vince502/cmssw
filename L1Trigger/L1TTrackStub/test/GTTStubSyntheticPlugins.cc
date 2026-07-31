#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/L1Trigger/interface/P2GTAlgoBlock.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "L1Trigger/L1TTrackStub/interface/GTTStubLinkCodec.h"

#include <array>
#include <cstdint>
#include <string>
#include <utility>

namespace l1t::test {

  class GTTStubSyntheticStreams : public edm::global::EDProducer<> {
  public:
    explicit GTTStubSyntheticStreams(const edm::ParameterSet&) : putToken_(produces<tt::Streams>()) {}
    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
      edm::ParameterSetDescription description;
      descriptions.add("gttStubSyntheticStreams", description);
    }

  private:
    void produce(edm::StreamID, edm::Event& event, const edm::EventSetup&) const override {
      constexpr std::array<unsigned int, GTTStubCountWord::kNumRegions> regionLinks = {0, 2, 4, 6, 8, 10, 12, 14, 16};
      std::array<uint32_t, GTTStubCountWord::kNumRegions> counts = {};
      bool trackSlotOverflow = false;
      bool malformedNoncarrierLink = false;

      switch (event.id().event()) {
        case 1:
          for (unsigned int region = 0; region < counts.size(); ++region)
            counts[region] = region + 1;
          break;
        case 2:
          counts.fill(100);
          break;
        case 3:
          counts[0] = GTTStubCountWord::kCountMask + 1;
          break;
        case 4:
          counts[0] = 10;
          trackSlotOverflow = true;
          break;
        case 5:
          counts[0] = 10;
          malformedNoncarrierLink = true;
          break;
        default:
          throw cms::Exception("LogicError") << "Unexpected synthetic event " << event.id().event() << ".";
      }

      tt::Streams streams(GTTStubLinkCodec::kNumLinks, tt::Stream(GTTStubLinkCodec::kFramesPerLink));
      for (unsigned int region = 0; region < counts.size(); ++region) {
        const auto word = GTTStubCountWord::pack(counts[region], region, true, trackSlotOverflow && region == 0);
        GTTStubLinkCodec::insert(
            streams[regionLinks[region]], GTTStubLinkCodec::kDefaultMetadataTrackSlot, word.word());
      }
      if (malformedNoncarrierLink)
        streams[1].pop_back();

      event.emplace(putToken_, std::move(streams));
    }

    edm::EDPutTokenT<tt::Streams> putToken_;
  };

  class GTTStubSyntheticValidation : public edm::one::EDAnalyzer<> {
  public:
    explicit GTTStubSyntheticValidation(const edm::ParameterSet& config)
        : primitiveToken_(consumes<GTTStubPrimitive>(config.getParameter<edm::InputTag>("primitive"))),
          algoBlocksToken_(consumes<P2GTAlgoBlockMap>(config.getParameter<edm::InputTag>("algoBlocks"))),
          algorithmName_(config.getParameter<std::string>("algorithmName")) {}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
      edm::ParameterSetDescription description;
      description.add<edm::InputTag>("primitive", edm::InputTag("serenityGTTStubUnpacker"));
      description.add<edm::InputTag>("algoBlocks", edm::InputTag("gttStubAlgoBlocks"));
      description.add<std::string>("algorithmName", "L1_GTT_LowStubOccupancy");
      descriptions.add("gttStubSyntheticValidation", description);
    }

  private:
    struct Expected {
      uint32_t total;
      uint32_t maximum;
      uint32_t maximumRegion;
      uint32_t activeMask;
      uint32_t trackOverflowMask;
      bool countOverflow;
      bool valid;
      bool decision;
    };

    void analyze(const edm::Event& event, const edm::EventSetup&) override {
      Expected expected;
      switch (event.id().event()) {
        case 1:
          expected = {45, 9, 8, GTTStubPrimitive::kActiveRegionMask, 0, false, true, true};
          break;
        case 2:
          expected = {900, 100, 0, GTTStubPrimitive::kActiveRegionMask, 0, false, true, false};
          break;
        case 3:
          expected = {GTTStubCountWord::kCountMask, GTTStubCountWord::kCountMask, 0, 1, 0, true, true, false};
          break;
        case 4:
          expected = {10, 10, 0, 1, 1, false, true, false};
          break;
        case 5:
          expected = {10, 10, 0, 1, 0, false, false, false};
          break;
        default:
          throw cms::Exception("LogicError") << "Unexpected synthetic event " << event.id().event() << ".";
      }

      const auto& primitive = event.get(primitiveToken_);
      const auto& algoBlocks = event.get(algoBlocksToken_);
      const auto algorithm = algoBlocks.find(algorithmName_);
      if (algorithm == algoBlocks.end())
        throw cms::Exception("ProductNotFound") << "P2GT algorithm block '" << algorithmName_ << "' is absent.";

      const bool exact =
          primitive.acceptedStubCount() == expected.total && primitive.maxRegionCount() == expected.maximum &&
          primitive.maxRegion() == expected.maximumRegion && primitive.activeRegionMask() == expected.activeMask &&
          primitive.trackOverflowMask() == expected.trackOverflowMask &&
          primitive.overflow() == expected.countOverflow && primitive.valid() == expected.valid &&
          primitive.structurallyValid() == expected.valid &&
          algorithm->second.decisionBeforeBxMaskAndPrescale() == expected.decision &&
          algorithm->second.decisionBeforePrescale() == expected.decision &&
          algorithm->second.decisionFinal() == expected.decision;
      if (!exact)
        throw cms::Exception("LogicError") << "Synthetic packet branch " << event.id().event()
                                           << " did not produce the expected primitive and P2GT decision.";
    }

    edm::EDGetTokenT<GTTStubPrimitive> primitiveToken_;
    edm::EDGetTokenT<P2GTAlgoBlockMap> algoBlocksToken_;
    std::string algorithmName_;
  };

}  // namespace l1t::test

DEFINE_FWK_MODULE(l1t::test::GTTStubSyntheticStreams);
DEFINE_FWK_MODULE(l1t::test::GTTStubSyntheticValidation);
