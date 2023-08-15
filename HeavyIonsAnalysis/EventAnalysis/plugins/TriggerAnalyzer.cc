// system include files
#include "TTree.h"
#include "TRegexp.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

//
// class declaration
//

class TriggerAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  TriggerAnalyzer(edm::ParameterSet const& ps);
  ~TriggerAnalyzer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) override;
  void beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void endRun(edm::Run const&, edm::EventSetup const&) {};

  // ----------member data ---------------------------

  std::string processName_;
  std::map<std::string, std::string> hltNamesInMenu_;
  std::set<std::string> l1TriggersInMenu_;
  std::map< std::string, L1TUtmAlgorithm > l1tAlgoMap_;

  TTree* tree_;
  ULong64_t fEvent_;
  int fLumiBlock_;
  int fRun_;
  int fBx_;
  int fOrbit_;

  std::vector<std::pair<bool, float> > hltInfo_;
  std::vector<std::pair<bool, float> > l1tInfo_;

  std::vector<std::string> hltNames_;
  std::vector<std::string> l1tNames_;
  edm::EDGetTokenT<edm::TriggerResults> hltResultsToken_;
  edm::ESGetToken<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd> l1tMenuToken_;
  HLTPrescaleProvider hltPrescaleProvider_;
};

//
// constructors and destructor
//
TriggerAnalyzer::TriggerAnalyzer(edm::ParameterSet const& ps):
  fEvent_(0),
  fLumiBlock_(-1),
  fRun_(-1),
  fBx_(-1),
  fOrbit_(-1),
  hltNames_(ps.getParameter<std::vector<std::string>>("hltDummyBranches")),
  l1tNames_(ps.getParameter<std::vector<std::string>>("l1tDummyBranches")),
  hltResultsToken_(consumes<edm::TriggerResults>(ps.getParameter<edm::InputTag>("hltResults"))),
  l1tMenuToken_{esConsumes<edm::Transition::BeginRun>()},
  hltPrescaleProvider_(ps, consumesCollector(), *this)
{
  // find process name
  EDConsumerBase::Labels hltResultsLabel;
  EDConsumerBase::labelsForToken(hltResultsToken_, hltResultsLabel);
  processName_ = hltResultsLabel.process;

  // open the tree file and initialize the tree
  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("HltTree", "");
  tree_->Branch("Event", &fEvent_, "Event/l");
  tree_->Branch("LumiBlock", &fLumiBlock_, "LumiBlock/I");
  tree_->Branch("Run", &fRun_, "Run/I");
  tree_->Branch("Bx", &fBx_, "Bx/I");
  tree_->Branch("Orbit", &fOrbit_, "Orbit/I");
}

TriggerAnalyzer::~TriggerAnalyzer()
{
}

//
// member functions
//

// ------------ method called for each event  ------------
void
TriggerAnalyzer::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup)
{
  // fill event information
  fEvent_ = iEvent.id().event();
  fLumiBlock_ = iEvent.luminosityBlock();
  fRun_ = iEvent.id().run();
  fBx_ = iEvent.bunchCrossing();
  fOrbit_ = iEvent.orbitNumber();

  // clear HLT information
  for (auto& hlt : hltInfo_) {
    hlt.first = false;
    hlt.second = -1;
  }

  // fill HLT information
  const auto& hltResults = iEvent.getHandle(hltResultsToken_);
  const auto& hltConfig = hltPrescaleProvider_.hltConfigProvider();
  const auto& pCol = hltPrescaleProvider_.prescaleSet(iEvent, iSetup);
  
  if (hltResults.isValid() && hltResults->size()>0) {
    for (size_t i = 0; i < hltNames_.size(); i++) {
      const auto& hltName = hltNames_[i];
      const auto& hltNamesInMenuItr = hltNamesInMenu_.find(hltName);
      if (hltNamesInMenuItr==hltNamesInMenu_.end()) continue;
      const auto& hltNameInMenu = hltNamesInMenuItr->second;
      const auto& hltIndex = hltConfig.triggerIndex(hltNameInMenu);
      hltInfo_[i].first = hltResults->accept(hltIndex);
      hltInfo_[i].second = pCol>=0 ? hltConfig.prescaleValue<double>(pCol, hltNameInMenu) : -1;
    }
  }
  else edm::LogInfo("TriggerAnalyzer") << "No HLT results!" << std::endl;

  // clear L1T information
  for (auto& l1t : l1tInfo_) {
    l1t.first = false;
    l1t.second = -1;
  }

  // fill L1T information
  auto& l1tGlobalUtil = *const_cast<l1t::L1TGlobalUtil*>(&hltPrescaleProvider_.l1tGlobalUtil());

  if (l1tGlobalUtil.valid() && !l1tGlobalUtil.decisionsFinal().empty()) {
    for (size_t i = 0; i < l1tNames_.size(); i++) {
      const auto& l1tName = l1tNames_[i];
      if (l1TriggersInMenu_.find(l1tName)==l1TriggersInMenu_.end()) continue;
      const auto& l1tIndex = l1tAlgoMap_[l1tName].getIndex();
      l1tInfo_[i].first = l1tGlobalUtil.decisionsFinal()[l1tIndex].second;
      l1tInfo_[i].second = pCol>=0 ? l1tGlobalUtil.prescales()[l1tIndex].second : -1;
    }
  }
  else edm::LogWarning("TriggerAnalyzer") << "No L1T results!" << std::endl;

  // fill tree
  tree_->Fill();
}

// ------------ method called when starting to processes a run  ------------
void
TriggerAnalyzer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup)
{
  // initialize HLT prescale provider
  bool hltMenuChanged(true); 
  if (!hltPrescaleProvider_.init(iRun, iSetup, processName_, hltMenuChanged)) throw(cms::Exception("TriggerAnalyzer") << "HLT prescale provider failed initialization!");
  if (!hltMenuChanged) return;
  
  // initialize HLT information
  const auto& hltConfig = hltPrescaleProvider_.hltConfigProvider();
  const auto& hltPaths = hltConfig.triggerNames();

  if (hltInfo_.empty()) {
    if (!hltNames_.empty() && hltNames_[0]=="@ALL") {
      hltNames_.clear();
      hltNames_.reserve(hltPaths.size());
      for (const auto& hltPath : hltPaths) {
        if (hltPath.rfind("_v")==std::string::npos) continue;
        hltNames_.emplace_back(hltPath.substr(0, hltPath.find("_v")+2));
      }
    }
    hltInfo_.resize(hltNames_.size());
    for (size_t i = 0; i < hltNames_.size(); i++) {
      const TString& hltName = hltNames_[i].data();
      tree_->Branch(hltName, &(hltInfo_[i].first), hltName + "/O");
      tree_->Branch(hltName + "_Prescl", &(hltInfo_[i].second), hltName + "_Prescl/F");
    }
  }
 
  hltNamesInMenu_.clear();
  for (const auto& hltName : hltNames_) {
    bool found(false);
    for (const auto& hltPath : hltPaths) {
      found = TString(hltPath).Contains(TRegexp(TString(hltName)));
      if (!found) continue;
      hltNamesInMenu_[hltName] = hltPath;
      break;
    }
    if (!found) edm::LogWarning("TriggerAnalyzer") << "Trigger " << hltName << " not found in HLT menu. Skipping..." << std::endl;
  }

  // initialize L1T information
  const auto& l1tMenu = iSetup.getData(l1tMenuToken_);
  l1tAlgoMap_ = l1tMenu.getAlgorithmMap();
  
  if (l1tInfo_.empty()) {
    if (!l1tNames_.empty() && l1tNames_[0]=="@ALL") {
      l1tNames_.clear();
      l1tNames_.reserve(l1tAlgoMap_.size());
      for (const auto& l1tAlgo : l1tAlgoMap_) {
        l1tNames_.emplace_back(l1tAlgo.second.getName());
      }
    }
    l1tInfo_.resize(l1tNames_.size());
    for (size_t i = 0; i < l1tNames_.size(); i++) {
      const TString& l1tName = l1tNames_[i].data();
      tree_->Branch(l1tName, &(l1tInfo_[i].first), l1tName + "/O");
      tree_->Branch(l1tName + "_Prescl", &(l1tInfo_[i].second), l1tName + "_Prescl/F");
    }
  }

  l1TriggersInMenu_.clear();
  for (const auto& l1tName : l1tNames_) {
    if (l1tAlgoMap_.find(l1tName)!=l1tAlgoMap_.end()) {
      l1TriggersInMenu_.insert(l1tName);
    }
    else edm::LogWarning("TriggerAnalyzer") << "Trigger " << l1tName << " not found in L1T menu. Skipping..." << std::endl;
  }
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
TriggerAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::vector<std::string> >("hltDummyBranches", {"@ALL"});
  desc.add<std::vector<std::string> >("l1tDummyBranches", {"@ALL"});
  desc.add<edm::InputTag>("hltResults", edm::InputTag("TriggerResults::HLT"));
  desc.add<edm::InputTag>("l1tAlgBlkInputTag", edm::InputTag("gtStage2Digis"));
  desc.add<edm::InputTag>("l1tExtBlkInputTag", edm::InputTag("gtStage2Digis"));
  desc.add<unsigned>("stageL1Trigger", 2U);
  descriptions.add("hltanalysis", desc);
}

// declare this class as a framework plugin
DEFINE_FWK_MODULE(TriggerAnalyzer);
