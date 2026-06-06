#include <vector>

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

class DileptonEventFilter : public edm::stream::EDFilter<> {
public:
  explicit DileptonEventFilter(const edm::ParameterSet& iConfig)
      : minNumber_(iConfig.getParameter<unsigned int>("minNumber")) {
    const auto sources = iConfig.getParameter<std::vector<edm::InputTag> >("src");
    tokens_.reserve(sources.size());
    for (const auto& source : sources) {
      tokens_.push_back(consumes<edm::View<reco::Candidate> >(source));
    }
  }

  bool filter(edm::Event& iEvent, const edm::EventSetup&) override {
    unsigned int nCandidates = 0;
    for (const auto& token : tokens_) {
      edm::Handle<edm::View<reco::Candidate> > candidates;
      iEvent.getByToken(token, candidates);
      if (candidates.isValid()) {
        nCandidates += candidates->size();
      }
    }
    return nCandidates >= minNumber_;
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::vector<edm::InputTag> >("src");
    desc.add<unsigned int>("minNumber", 1);
    descriptions.add("dileptonEventFilter", desc);
  }

private:
  std::vector<edm::EDGetTokenT<edm::View<reco::Candidate> > > tokens_;
  unsigned int minNumber_;
};

DEFINE_FWK_MODULE(DileptonEventFilter);
