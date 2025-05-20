#include <memory>
#include <string>
#include <vector>

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/HiGenData/interface/GenHIEvent.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"



#include "HepMC/GenEvent.h"
#include "HepMC/HeavyIon.h"

// root include file
#include "TFile.h"
#include "TTree.h"

class quickVertexAnalysis : public edm::one::EDAnalyzer<edm::one::WatchRuns>{
public:
    quickVertexAnalysis(const edm::ParameterSet&);
    ~quickVertexAnalysis() override;

private:
  void beginRun(const edm::Run& run, const edm::EventSetup& iSetup) override;
  void endRun(const edm::Run& run, const edm::EventSetup& iSetup) override;
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  edm::EDGetTokenT<edm::SimVertexContainer> g4Label;
  edm::EDGetTokenT<edm::HepMCProduct> hepMCLabel;
  edm::EDGetTokenT<std::vector<reco::Vertex> > recoVtxLabel;
  edm::EDGetTokenT<GenEventInfoProduct> genEvtInfo;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo>> pileupSummaryToken_;

  bool hasReco_;
  bool useGenSim_;

  edm::Service<TFileService> f;

  TH1D* hist_pusummary;
  TH1D* hist_vertexcol;
  TH1D* hist_vertexcountdiff;
};


quickVertexAnalysis::quickVertexAnalysis(const edm::ParameterSet& iConfig){
    hasReco_ = iConfig.getUntrackedParameter<bool>("hasReco", false);
    useGenSim_ = iConfig.getUntrackedParameter<bool>("useGen", false);
    if(useGenSim_){
      g4Label = consumes<edm::SimVertexContainer>(iConfig.getUntrackedParameter<edm::InputTag>("g4Label"));
      hepMCLabel = consumes<edm::HepMCProduct>(iConfig.getUntrackedParameter<edm::InputTag>("hepMCLabel"));
      genEvtInfo = consumes<GenEventInfoProduct>(iConfig.getUntrackedParameter<edm::InputTag>("genEvtInfo"));
    }
    pileupSummaryToken_ = consumes<std::vector<PileupSummaryInfo>>(iConfig.getParameter<edm::InputTag>("pileupInfo"));
    if(hasReco_) 
        recoVtxLabel = consumes<std::vector<reco::Vertex>>(iConfig.getUntrackedParameter<edm::InputTag>("recoVtxLabel"));

};
quickVertexAnalysis::~quickVertexAnalysis(){};


void quickVertexAnalysis::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) {
};
void quickVertexAnalysis::endRun(const edm::Run& run, const edm::EventSetup& iSetup) {};
void quickVertexAnalysis::beginJob() {
//   hydjetTree_ = f->make<TTree>("hi", "Tree of Hi gen Event");
  hist_pusummary = f->make<TH1D>("hist_pusummary", "hist pusummary", 20, 0, 20);
  hist_vertexcol = f->make<TH1D>("hist_vertexcol", "hist vertexcol", 20, 0, 20);
  hist_vertexcountdiff = f->make<TH1D>("hist_vertexcountdiff", "hist vertexcol", 20, 0, 20);

};
void quickVertexAnalysis::endJob() {
    hist_vertexcountdiff = (TH1D*)  hist_vertexcol->Clone("hist_vertexcountdiff");
    hist_vertexcountdiff->Add(hist_pusummary,-1);
};


void quickVertexAnalysis::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace HepMC;
  using std::cout;
  using std::endl;

  edm::Handle<edm::SimVertexContainer> simVertices;
  edm::Handle<edm::HepMCProduct> hepP;
  edm::Handle<std::vector<reco::Vertex>> recoV;

  std::string verb_z = "";
  std::vector<float> hepz = {};
  std::vector<float> recoz = {};
  if(useGenSim_){
    iEvent.getByToken(g4Label, simVertices);
    iEvent.getByToken(hepMCLabel, hepP);
    // edm::SimVertexContainer::const_iterator it = simVertices->begin();
    // if (it != simVertices->end()) {
    //   SimVertex vertex = (*it);
    //   //   vx = vertex.position().x();
    //   //   vy = vertex.position().y();
    //   //   vz = vertex.position().z();
    //   //   vr = vertex.position().rho();
    //     cout << vertex.position().z() << endl;
    // }
    const HepMC::GenEvent* Evt = hepP->GetEvent();
    if (Evt->vertices_begin() != Evt->vertices_end()) {
        auto* first_v = (*Evt->vertices_begin());
        verb_z += Form("HepMC first Vtx z: %.3f ",first_v->point3d().z() / 10.);
	hepz.push_back(first_v->point3d().z() / 10.);

    }

  }
  edm::Handle<std::vector<PileupSummaryInfo>> PupInfo;
  iEvent.getByToken(pileupSummaryToken_, PupInfo);

  float trueNumInteractions;
  int puNumInteractions;
  for (auto PVI : *PupInfo) {
    int BX = PVI.getBunchCrossing();
    //cout << BX << endl;
    if (BX == 0) {
      const std::vector<float>& zpositions = PVI.getPU_zpositions();
      for( auto zp : zpositions){
        verb_z += Form(",%.3f ", zp);
        hepz.push_back(zp);
      }
      
      trueNumInteractions = PVI.getTrueNumInteractions();
      puNumInteractions = PVI.getPU_NumInteractions();
      cout << "trueNumInteractions: " << trueNumInteractions << ", puNumInteractions: " << puNumInteractions << endl; 
      hist_pusummary->Fill(puNumInteractions);

    }
  }

  std::string verb_z_rec = "Reco Vtx z: ";
  if(hasReco_){
    iEvent.getByToken(recoVtxLabel, recoV);
    reco::Vertex const* pv;
    int nVtxCount = 0;
    cout << ", reco vertex collection size: " << recoV->size() << endl;
    for (const reco::Vertex& vtx : *recoV) {
      if (vtx.isFake() || !vtx.isValid()){
        cout << "Vtx is either invalid or fake" << endl;
        continue;
      }
      verb_z_rec += Form("%.3f, ", vtx.z());
      recoz.push_back(vtx.z());
      nVtxCount++;
    }
    hist_vertexcol->Fill(nVtxCount);
  }

  cout << verb_z << endl;
  cout << verb_z_rec << endl;
  cout << "Sorted: " << endl;
  verb_z = "Gen  : ";
  verb_z_rec = "Reco : ";
  std::sort(hepz.begin(), hepz.end());
  std::sort(recoz.begin(), recoz.end());
  for( auto gz : hepz){
    verb_z += Form("%.3f, ", gz);
  }
  for( auto rz : recoz){
    verb_z_rec += Form("%.3f, ", rz);
  }
  cout << verb_z << endl;
  cout << verb_z_rec << endl;
};

DEFINE_FWK_MODULE(quickVertexAnalysis);
