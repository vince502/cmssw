#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDFilter.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <cstdint>

namespace l1t {

  class GTTStubDecisionFilter : public edm::global::EDFilter<> {
  public:
    explicit GTTStubDecisionFilter(const edm::ParameterSet&);
    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    bool filter(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

    edm::EDGetTokenT<GTTStubPrimitive> primitiveToken_;
    uint32_t minStubCount_;
    uint32_t maxStubCount_;
    bool rejectOverflow_;
    bool rejectTrackOverflow_;
  };

  GTTStubDecisionFilter::GTTStubDecisionFilter(const edm::ParameterSet& config)
      : primitiveToken_(consumes<GTTStubPrimitive>(config.getParameter<edm::InputTag>("src"))),
        minStubCount_(config.getParameter<unsigned int>("minStubCount")),
        maxStubCount_(config.getParameter<unsigned int>("maxStubCount")),
        rejectOverflow_(config.getParameter<bool>("rejectOverflow")),
        rejectTrackOverflow_(config.getParameter<bool>("rejectTrackOverflow")) {
    if (minStubCount_ > maxStubCount_)
      throw cms::Exception("Configuration") << "minStubCount must not exceed maxStubCount.";
  }

  bool GTTStubDecisionFilter::filter(edm::StreamID, edm::Event& event, const edm::EventSetup&) const {
    const auto& primitive = event.get(primitiveToken_);
    if (!primitive.structurallyValid())
      return false;
    if (rejectOverflow_ && primitive.overflow())
      return false;
    if (rejectTrackOverflow_ && primitive.trackOverflow())
      return false;
    return primitive.acceptedStubCount() >= minStubCount_ && primitive.acceptedStubCount() <= maxStubCount_;
  }

  void GTTStubDecisionFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription description;
    description.add<edm::InputTag>("src", edm::InputTag("serenityGTTStubUnpacker"));
    description.add<unsigned int>("minStubCount", 0);
    description.add<unsigned int>("maxStubCount", GTTStubCountWord::kNumRegions * GTTStubCountWord::kCountMask);
    description.add<bool>("rejectOverflow", true);
    description.add<bool>("rejectTrackOverflow", true);
    descriptions.add("gttStubDecisionFilter", description);
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GTTStubDecisionFilter);
