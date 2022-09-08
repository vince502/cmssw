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
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

//
// class declaration
//

class TriggerObjectAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  TriggerObjectAnalyzer(edm::ParameterSet const& ps);
  ~TriggerObjectAnalyzer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) override;
  void beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void endRun(edm::Run const&, edm::EventSetup const&) {};

  // ----------member data ---------------------------

  std::string processName_;
  std::vector<std::string> triggerNames_;
  const edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  const edm::EDGetTokenT<trigger::TriggerEvent> triggerEventToken_;
  const edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  HLTConfigProvider hltConfig_;

  std::set<std::string> filtersInMenu_;
  std::map<std::string, std::string> triggerNamesInMenu_;
  std::map<std::string, std::string> triggerFiltersInMenu_;

  struct OBJ {
    TTree*             tree;
    std::vector<short> id;
    std::vector<float> pt;
    std::vector<float> eta;
    std::vector<float> phi;
    std::vector<float> mass;
  };

  std::vector<OBJ> triggerInfo_;
};

//
// constructors and destructor
//
TriggerObjectAnalyzer::TriggerObjectAnalyzer(edm::ParameterSet const& ps):
  triggerNames_(ps.getParameter<std::vector<std::string> >("triggerNames")),
  triggerResultsToken_(consumes<edm::TriggerResults>(ps.getParameter<edm::InputTag>("triggerResults"))),
  triggerEventToken_(consumes<trigger::TriggerEvent>(ps.getParameter<edm::InputTag>("triggerEvent"))),
  triggerObjectsToken_(consumes<pat::TriggerObjectStandAloneCollection>(ps.getParameter<edm::InputTag>("triggerObjects")))
{
  // find process name
  EDConsumerBase::Labels triggerResultsLabel;
  EDConsumerBase::labelsForToken(triggerResultsToken_, triggerResultsLabel);
  processName_ = triggerResultsLabel.process;
}

TriggerObjectAnalyzer::~TriggerObjectAnalyzer()
{
}

//
// member functions
//

// ------------ method called for each event  ------------
void
TriggerObjectAnalyzer::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup)
{
  if (hltConfig_.size() <= 0) return;

  const auto& triggerResults = iEvent.get(triggerResultsToken_);
  const auto& triggerEvent = iEvent.getHandle(triggerEventToken_);
  const auto& triggerObjects = iEvent.getHandle(triggerObjectsToken_);

  // map trigger objects to filters
  std::map<std::string, pat::TriggerObjectStandAloneCollection> triggerObjectsFilterMap;
  if (triggerEvent.isValid()) {
    for (const auto& filter : filtersInMenu_) {
      const auto& filterIndex = triggerEvent->filterIndex(edm::InputTag(filter, "", processName_));
      if (filterIndex>=triggerEvent->sizeFilters()) continue;
      for (const auto& key : triggerEvent->filterKeys(filterIndex)) {
        triggerObjectsFilterMap[filter].emplace_back(triggerEvent->getObjects()[key]);
      }
    }
  }
  else if (triggerObjects.isValid()) {
    const auto& triggerPaths = iEvent.triggerNames(triggerResults);
    for (auto triggerObject : *triggerObjects) {
      triggerObject.unpackPathNames(triggerPaths);
      for (const auto& filter : triggerObject.filterLabels()) {
        triggerObjectsFilterMap[filter].emplace_back(triggerObject);
      }
    }
  }
  else throw(cms::Exception("TriggerObjectAnalyzer") << "Trigger object collection not found!");

  // clear trigger information
  for (auto& trg : triggerInfo_) {
    trg.id.clear();
    trg.pt.clear();
    trg.eta.clear();
    trg.phi.clear();
    trg.mass.clear();
  }

  // fill trigger information
  for (size_t i = 0; i < triggerNames_.size(); i++) {
    const auto& triggerName = triggerNames_[i];
    // find filter name
    std::string filterName = "";
    // case:: trigger filter
    if (triggerName.rfind("hlt", 0)==0) {
      filterName = triggerName;
    }
    // case: trigger path
    else if (triggerNamesInMenu_.find(triggerName)!=triggerNamesInMenu_.end()) {
      filterName = triggerFiltersInMenu_[triggerName];
      // require trigger path fired
      const auto& triggerNameInMenu = triggerNamesInMenu_[triggerName];
      const auto& triggerIndex = hltConfig_.triggerIndex(triggerNameInMenu);
      if (!triggerResults.accept(triggerIndex)) continue;
    }
    // check if filter has trigger objects
    const auto& triggerObjectsInFilter = triggerObjectsFilterMap.find(filterName);
    if (triggerObjectsInFilter == triggerObjectsFilterMap.end()) continue;
    // add trigger objects
    auto& trg = triggerInfo_[i];
    for (const auto& triggerObject : triggerObjectsInFilter->second) {
      trg.id.push_back(triggerObject.pdgId());
      trg.pt.push_back(triggerObject.pt());
      trg.eta.push_back(triggerObject.eta());
      trg.phi.push_back(triggerObject.phi());
      trg.mass.push_back(triggerObject.mass());
    }
  }

  // fill tree
  for (auto& trg : triggerInfo_) {
    trg.tree->Fill();
  }
}

// ------------ method called when starting to processes a run  ------------
void
TriggerObjectAnalyzer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup)
{
  // initialize HLT config provider
  bool hltMenuChanged(true);
  if (!hltConfig_.init(iRun, iSetup, processName_, hltMenuChanged)) throw(cms::Exception("TriggerObjectAnalyzer") << "HLT config provider failed initialization!");
  if (!hltMenuChanged) return;

  // extract trigger paths and filters
  std::set<std::string> filters;
  const auto& triggerPaths = hltConfig_.triggerNames();
  for (const auto& triggerPath : triggerPaths) {
    for (const auto& module : hltConfig_.moduleLabels(triggerPath)) {
      if (module=="hltBoolEnd" || hltConfig_.moduleEDMType(module)!="EDFilter") continue;
      filters.insert(module);
    }
  }

  // initialize trigger information
  if (triggerInfo_.empty()) {
    if (triggerNames_.empty()) {
      triggerNames_.reserve(triggerPaths.size());
      for (const auto& triggerPath : triggerPaths) {
	if (triggerPath.rfind("_v")==std::string::npos) continue;
	triggerNames_.emplace_back(triggerPath.substr(0, triggerPath.find("_v")+2));
      }
    }
    triggerInfo_.resize(triggerNames_.size());
    for (size_t i = 0; i < triggerNames_.size(); i++) {
      auto& trg = triggerInfo_[i];
      const TString& triggerName = triggerNames_[i].data();
      edm::Service<TFileService> fs;
      trg.tree = fs->make<TTree>(triggerName, "");
      trg.tree->Branch("TriggerObjID", &trg.id);
      trg.tree->Branch("pt", &trg.pt);
      trg.tree->Branch("eta", &trg.eta);
      trg.tree->Branch("phi", &trg.phi);
      trg.tree->Branch("mass", &trg.mass);
    }
  }

  // find triggers in menu
  filtersInMenu_.clear();
  triggerNamesInMenu_.clear();
  triggerFiltersInMenu_.clear();
  for (size_t i = 0; i < triggerNames_.size(); i++) {
    const auto& triggerName = triggerNames_[i];
    bool found(false);
    // check trigger filter
    if (triggerName.rfind("hlt",0)==0) {
      found = filters.find(triggerName)!=filters.end();
      if (found) filtersInMenu_.insert(triggerName);
    }
    // check trigger paths
    else if (triggerName.rfind("_v")!=std::string::npos) {
      for (const auto& triggerPath : triggerPaths) {
        found = TString(triggerPath).Contains(TRegexp(TString(triggerName)));
        if (!found) continue;
	// find last filter: start from last module in the path, iterate back until a filter is found
	std::string filterName;
	const auto& triggerIndex = hltConfig_.triggerIndex(triggerPath);
	const auto& modules = hltConfig_.moduleLabels(triggerIndex);
        for (size_t iMod = modules.size()-1; iMod > 0; iMod--) {
          const auto& module = modules[iMod];
          if (filters.find(module)!=filters.end()) { filterName = module; break; }
        }
        if (filterName=="") throw cms::Exception("TriggerObjectAnalyzer") << "No filter found for " << triggerName;
	// fill trigger information
	filtersInMenu_.insert(filterName);
	triggerNamesInMenu_[triggerName] = triggerPath;
	triggerFiltersInMenu_[triggerName] = filterName;
        break;
      }
    }
    else continue;
    if (!found) edm::LogWarning("TriggerObjectAnalyzer") << "Trigger " << triggerName << " not found in HLT menu. Skipping..." << std::endl;
  }
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
TriggerObjectAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::vector<std::string> >("triggerNames", {});
  desc.add<edm::InputTag>("triggerResults", edm::InputTag("TriggerResults::HLT"));
  desc.add<edm::InputTag>("triggerEvent", edm::InputTag("hltTriggerSummaryAOD::HLT"));
  desc.add<edm::InputTag>("triggerObjects", edm::InputTag("slimmedPatTrigger::PAT"));
  descriptions.add("hltobject", desc);
}

// declare this class as a framework plugin
DEFINE_FWK_MODULE(TriggerObjectAnalyzer);
