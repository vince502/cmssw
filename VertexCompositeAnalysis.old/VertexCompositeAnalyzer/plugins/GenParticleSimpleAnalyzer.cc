// user include files
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm> 



class GenParticleSimpleAnalyzer : public edm::one::EDAnalyzer<>{
public:
    explicit GenParticleSimpleAnalyzer(const edm::ParameterSet &);
    ~GenParticleSimpleAnalyzer(){};
private:
    virtual void beginJob();
    virtual void analyze(const edm::Event &, const edm::EventSetup &);
    virtual void endJob();


    std::vector<int> wanted;
    edm::EDGetTokenT<reco::GenParticleCollection> tok_genParticle_;
    
};

GenParticleSimpleAnalyzer::GenParticleSimpleAnalyzer(const edm::ParameterSet& iConfig){
    tok_genParticle_ = consumes<reco::GenParticleCollection>( edm::InputTag(iConfig.getUntrackedParameter<edm::InputTag>("GenParticleCollection")));
    wanted = iConfig.getUntrackedParameter<std::vector<int> >("pdgIDs");
};


void GenParticleSimpleAnalyzer::beginJob(){};
void GenParticleSimpleAnalyzer::endJob(){};

void GenParticleSimpleAnalyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSteup){
    using namespace std;
    edm::Handle<reco::GenParticleCollection> genpars;
    iEvent.getByToken(tok_genParticle_, genpars);
    if(!genpars.isValid()){
        cout << "Faulty gen collection.. I'm out" << endl;
        return;
    }

    auto const nGens = genpars->size();
    cout << "nGens : " << nGens << endl;
    for(unsigned int igen=0; igen < nGens; igen++){
        const reco::GenParticle &trk = (*genpars)[igen];
        auto hasP = std::find(wanted.begin(), wanted.end(), trk.pdgId() );
        if(hasP == wanted.end()) continue;
        cout << trk.pdgId();
        cout << Form(" -> %zu (",trk.numberOfDaughters());
        string dauInfo = "    Dau : ";
        for(unsigned int idau=0; idau <trk.numberOfDaughters(); ++idau){
            auto const& dau = trk.daughter(idau);
            cout << dau->pdgId() << " ";
            dauInfo += Form("(%.3f, %.3f) ", dau->pt(), dau->eta());
        }
        cout << ")" << endl;
        cout << dauInfo << endl;
    }
};

DEFINE_FWK_MODULE(GenParticleSimpleAnalyzer);