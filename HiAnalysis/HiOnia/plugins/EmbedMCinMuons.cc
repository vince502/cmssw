#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

namespace pat {

  class EmbedMCinMuons : public edm::stream::EDProducer<> {
    
  public:
    
    explicit EmbedMCinMuons(const edm::ParameterSet & iConfig):
      muonGenMatchToken_(consumes(iConfig.getParameter<edm::InputTag>("matchedGen"))),
      muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons")))
    {
      produces<pat::MuonCollection>();
    }
    ~EmbedMCinMuons() override {};
    
    void produce(edm::Event&, const edm::EventSetup&) override;
    
    static void fillDescriptions(edm::ConfigurationDescriptions&);
    
  private:
    const edm::EDGetTokenT<edm::Association<reco::GenParticleCollection> > muonGenMatchToken_;
    const edm::EDGetTokenT<pat::MuonCollection> muonToken_;
    
  };
  
}

void pat::EmbedMCinMuons::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // extract MC-muons matching map
  edm::Handle<edm::Association<reco::GenParticleCollection> > matches;
  iEvent.getByToken(muonGenMatchToken_, matches);

  // extract muons from packedCandidate collections
  edm::Handle<pat::MuonCollection> muonCands;
  iEvent.getByToken(muonToken_, muonCands);

  pat::MuonCollection outMuons;

  auto output = std::make_unique<pat::MuonCollection>();
  //loop over muons to grab and add the matched gen muon
  for (size_t i=0; i<muonCands->size(); i++) {
    pat::MuonRef muonref(muonCands, i);
    const auto& genPRef = (*matches)[muonref];

    pat::Muon outmuon = *muonref;
    outmuon.setGenParticleRef(genPRef, true);
    output->push_back(outmuon);
  }

  iEvent.put(std::move(output));
}
  
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void pat::EmbedMCinMuons::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("muons", edm::InputTag("unpackedMuons"))->setComment("muon input collection");
  desc.add<edm::InputTag>("matchedGen", edm::InputTag("muonMatch"))->setComment("matches with gen muons input collection");
  descriptions.add("unpackedMuonsWithGenMatch", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(EmbedMCinMuons);
