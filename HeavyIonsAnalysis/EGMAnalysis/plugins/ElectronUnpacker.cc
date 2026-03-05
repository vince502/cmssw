#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Math/interface/deltaR.h"

namespace pat {

  class ElectronUnpacker : public edm::global::EDProducer<> {
  public:
    explicit ElectronUnpacker(const edm::ParameterSet& iConfig)
        : primaryToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("primaryElectrons"))),
          secondaryToken_(
              consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("secondaryElectrons"))),
          maxDeltaR_(iConfig.getParameter<double>("maxDeltaR")),
          maxRelPtDiff_(iConfig.getParameter<double>("maxRelPtDiff")),
          preferPrimary_(iConfig.getParameter<bool>("preferPrimary")),
          putToken_(produces<pat::ElectronCollection>()) {}
    ~ElectronUnpacker() override = default;

    void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const override;

    static void fillDescriptions(edm::ConfigurationDescriptions&);

  private:
    const edm::EDGetTokenT<pat::ElectronCollection> primaryToken_;
    const edm::EDGetTokenT<pat::ElectronCollection> secondaryToken_;
    const double maxDeltaR_;
    const double maxRelPtDiff_;
    const bool preferPrimary_;
    const edm::EDPutTokenT<pat::ElectronCollection> putToken_;

    bool isDuplicate(const pat::Electron& a, const pat::Electron& b) const;
  };

}  // namespace pat

bool pat::ElectronUnpacker::isDuplicate(const pat::Electron& a, const pat::Electron& b) const {
  const auto& aRef = a.originalObjectRef();
  const auto& bRef = b.originalObjectRef();
  if (aRef.isNonnull() && bRef.isNonnull() && aRef.id() == bRef.id() && aRef.key() == bRef.key()) {
    return true;
  }

  if (a.gsfTrack().isNonnull() && b.gsfTrack().isNonnull() && a.gsfTrack().id() == b.gsfTrack().id() &&
      a.gsfTrack().key() == b.gsfTrack().key()) {
    return true;
  }

  if (a.charge() != b.charge()) {
    return false;
  }

  if (reco::deltaR2(a.eta(), a.phi(), b.eta(), b.phi()) > maxDeltaR_ * maxDeltaR_) {
    return false;
  }

  const double denom = std::max(1e-6, a.pt());
  const double relPtDiff = std::abs(a.pt() - b.pt()) / denom;
  return relPtDiff < maxRelPtDiff_;
}

void pat::ElectronUnpacker::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const {
  const auto& primary = iEvent.get(primaryToken_);
  const auto& secondary = iEvent.get(secondaryToken_);

  pat::ElectronCollection output;
  output.reserve(primary.size() + secondary.size());

  if (preferPrimary_) {
    output.insert(output.end(), primary.begin(), primary.end());
    for (const auto& ele : secondary) {
      bool duplicated = false;
      for (const auto& kept : output) {
        if (isDuplicate(kept, ele)) {
          duplicated = true;
          break;
        }
      }
      if (!duplicated) {
        output.push_back(ele);
      }
    }
  } else {
    output.insert(output.end(), secondary.begin(), secondary.end());
    for (const auto& ele : primary) {
      bool duplicated = false;
      for (const auto& kept : output) {
        if (isDuplicate(kept, ele)) {
          duplicated = true;
          break;
        }
      }
      if (!duplicated) {
        output.push_back(ele);
      }
    }
  }

  iEvent.emplace(putToken_, std::move(output));
}

void pat::ElectronUnpacker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("primaryElectrons", edm::InputTag("slimmedElectrons"))
      ->setComment("Primary electron input collection (kept on overlap by default)");
  desc.add<edm::InputTag>("secondaryElectrons", edm::InputTag("slimmedLowPtElectrons"))
      ->setComment("Secondary electron input collection (added only when non-duplicate)");
  desc.add<double>("maxDeltaR", 1e-4)->setComment("Fallback geometric matching threshold for de-duplication");
  desc.add<double>("maxRelPtDiff", 0.01)->setComment("Fallback relative pT difference threshold");
  desc.add<bool>("preferPrimary", true)->setComment("If true, keep primary object on duplicates");
  descriptions.add("unpackedElectrons", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(ElectronUnpacker);
