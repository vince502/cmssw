// TriggerAnalyzer2: HLT/L1 trigger analyzer with pattern-based filtering
// Stores only triggers matching specified patterns, reducing tree size significantly
//
// Usage in python config:
//   process.hltanalysis2 = cms.EDAnalyzer('TriggerAnalyzer2',
//       HLTProcessName = cms.string('HLT'),
//       hltresults = cms.InputTag('TriggerResults::HLT'),
//       l1results = cms.InputTag('gtStage2Digis'),
//       hltPSProvCfg = cms.PSet(stageL1Trigger = cms.uint32(2)),
//       hltPatterns = cms.vstring('HLT_*Mu*', 'HLT_*Jet*', 'HLT_*ZeroBias*'),
//       l1Patterns = cms.vstring('L1_*Mu*', 'L1_*Jet*', 'L1_ZeroBias*'),
//   )

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmAlgorithm.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/L1TGlobal/interface/GlobalExtBlk.h"
#include "FWCore/Common/interface/Provenance.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

#include "TTree.h"

#include <regex>
#include <fnmatch.h>

class TriggerAnalyzer2 : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  TriggerAnalyzer2(edm::ParameterSet const& conf);
  ~TriggerAnalyzer2() override;

  void analyze(const edm::Event& e, const edm::EventSetup& iSetup) override;
  void endJob() override;
  void beginRun(const edm::Run& run, const edm::EventSetup& es) override;
  void endRun(const edm::Run& run, const edm::EventSetup& es) override;

private:
  bool matchesAnyPattern(const std::string& name, const std::vector<std::string>& patterns);

  TTree* t_;

  unsigned long long fEvent;
  int fLumiBlock;
  int fRun;
  int fBx;
  int fOrbit;

  int HltEvtCnt;
  int L1EvtCnt;
  
  static constexpr int kMaxHLTFlag = 1000;
  static constexpr int kMaxL1Flag = 1000;
  
  int* hltflag;
  int* l1flag;
  int* hltPrescaleNumerator;
  int* hltPrescaleDenominator;
  int* l1Prescl;

  std::string processName_;

  // Pattern-based filtering
  std::vector<std::string> hltPatterns_;
  std::vector<std::string> l1Patterns_;

  std::map<std::string, int> pathtoindex;
  
  // Track which triggers passed the filter
  std::vector<std::string> selectedHltPaths_;
  std::vector<std::string> selectedL1Paths_;

  edm::EDGetTokenT<edm::TriggerResults> hltresultsToken_;
  edm::EDGetTokenT<GlobalAlgBlkBxCollection> l1resultsToken_;
  edm::ESGetToken<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd> l1GtMenuToken_;

  std::unique_ptr<HLTPrescaleProvider> hltPrescaleProvider_;

  // Prescale caching per lumisection
  int cachedLumiBlock_ = -1;
  std::map<std::string, std::pair<int, int>> cachedHltPrescales_;  // trigname -> (numerator, denominator)
  std::map<std::string, int> cachedL1Prescales_;  // trigname -> prescale
};

TriggerAnalyzer2::TriggerAnalyzer2(edm::ParameterSet const& conf)
    : fEvent(0),
      fLumiBlock(-1),
      fRun(-1),
      fBx(-1),
      fOrbit(-1),
      HltEvtCnt(0),
      L1EvtCnt(0),
      hltflag(new int[kMaxHLTFlag]),
      l1flag(new int[kMaxL1Flag]),
      hltPrescaleNumerator(new int[kMaxHLTFlag]),
      hltPrescaleDenominator(new int[kMaxHLTFlag]),
      l1Prescl(new int[kMaxL1Flag]),
      processName_(conf.getParameter<std::string>("HLTProcessName")),
      hltPatterns_(conf.getParameter<std::vector<std::string>>("hltPatterns")),
      l1Patterns_(conf.getParameter<std::vector<std::string>>("l1Patterns")),
      hltresultsToken_(consumes<edm::TriggerResults>(conf.getParameter<edm::InputTag>("hltresults"))),
      l1resultsToken_(consumes<GlobalAlgBlkBxCollection>(conf.getParameter<edm::InputTag>("l1results"))),
      l1GtMenuToken_(esConsumes<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd>()),
      hltPrescaleProvider_(
          new HLTPrescaleProvider(conf.getParameter<edm::ParameterSet>("hltPSProvCfg"), consumesCollector(), *this)) {
  
  edm::Service<TFileService> fs;
  t_ = fs->make<TTree>("HltTree", "");

  t_->Branch("Event", &fEvent, "Event/l");
  t_->Branch("LumiBlock", &fLumiBlock, "LumiBlock/I");
  t_->Branch("Run", &fRun, "Run/I");
  t_->Branch("Bx", &fBx, "Bx/I");
  t_->Branch("Orbit", &fOrbit, "Orbit/I");
  
  // Log the patterns
  edm::LogInfo("TriggerAnalyzer2") << "HLT patterns to keep: " << hltPatterns_.size();
  for (const auto& p : hltPatterns_) {
    edm::LogInfo("TriggerAnalyzer2") << "  " << p;
  }
  edm::LogInfo("TriggerAnalyzer2") << "L1 patterns to keep: " << l1Patterns_.size();
  for (const auto& p : l1Patterns_) {
    edm::LogInfo("TriggerAnalyzer2") << "  " << p;
  }
}

TriggerAnalyzer2::~TriggerAnalyzer2() {
  delete[] hltflag;
  delete[] l1flag;
  delete[] hltPrescaleNumerator;
  delete[] hltPrescaleDenominator;
  delete[] l1Prescl;
}

bool TriggerAnalyzer2::matchesAnyPattern(const std::string& name, const std::vector<std::string>& patterns) {
  // If no patterns specified, accept all
  if (patterns.empty()) {
    return true;
  }
  
  bool matched = false;
  for (const auto& pattern : patterns) {
    // Exclusion pattern starts with '!'
    if (!pattern.empty() && pattern[0] == '!') {
      std::string excludePattern = pattern.substr(1);
      if (fnmatch(excludePattern.c_str(), name.c_str(), 0) == 0) {
        return false;  // Excluded
      }
    } else {
      // Use fnmatch for glob-style matching (*, ?)
      if (fnmatch(pattern.c_str(), name.c_str(), 0) == 0) {
        matched = true;
      }
    }
  }
  return matched;
}

void TriggerAnalyzer2::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) {
  edm::Handle<edm::TriggerResults> hltresults;
  edm::Handle<GlobalAlgBlkBxCollection> l1results;

  iEvent.getByToken(hltresultsToken_, hltresults);
  iEvent.getByToken(l1resultsToken_, l1results);

  fEvent = iEvent.id().event();
  fLumiBlock = iEvent.luminosityBlock();
  fRun = iEvent.id().run();
  fBx = iEvent.bunchCrossing();
  fOrbit = iEvent.orbitNumber();

  if (hltresults.isValid()) {
    // Reset accept status
    for (int i = 0; i < kMaxHLTFlag; ++i) {
      hltflag[i] = -1;
      hltPrescaleNumerator[i] = -1;
      hltPrescaleDenominator[i] = -1;
    }

    int ntrigs = hltresults->size();
    edm::TriggerNames const& triggerNames = iEvent.triggerNames(*hltresults);

    // First event: Book branches only for triggers matching patterns
    if (HltEvtCnt == 0) {
      int itrig_filtered = 0;
      
      for (int itrig = 0; itrig != ntrigs; ++itrig) {
        const std::string& trigname = triggerNames.triggerName(itrig);
        
        // Only add if matches pattern
        if (matchesAnyPattern(trigname, hltPatterns_)) {
          TString hltname = trigname;
          t_->Branch(hltname, hltflag + itrig_filtered, hltname + "/I");
          t_->Branch(hltname + "_PrescaleNumerator", hltPrescaleNumerator + itrig_filtered, hltname + "_PrescaleNumerator/I");
          t_->Branch(hltname + "_PrescaleDenominator", hltPrescaleDenominator + itrig_filtered, hltname + "_PrescaleDenominator/I");
          pathtoindex[trigname] = itrig_filtered;
          selectedHltPaths_.push_back(trigname);
          ++itrig_filtered;
        }
      }
      
      edm::LogInfo("TriggerAnalyzer2") << "Selected " << itrig_filtered << " HLT paths out of " << ntrigs;
      HltEvtCnt++;
    }

    // Update prescale cache if lumisection changed
    bool lumiChanged = (fLumiBlock != cachedLumiBlock_);
    if (lumiChanged) {
      cachedLumiBlock_ = fLumiBlock;
      cachedHltPrescales_.clear();
      
      // Cache all selected HLT prescales for this lumisection
      for (const auto& trigname : selectedHltPaths_) {
        FractionalPrescale ps = hltPrescaleProvider_->prescaleValue<FractionalPrescale>(iEvent, iSetup, trigname);
        cachedHltPrescales_[trigname] = std::make_pair(ps.numerator(), ps.denominator());
      }
    }

    // Fill values for selected triggers (use cached prescales)
    for (int itrig = 0; itrig != ntrigs; ++itrig) {
      const std::string& trigname = triggerNames.triggerName(itrig);
      
      auto it = pathtoindex.find(trigname);
      if (it != pathtoindex.end()) {
        int index = it->second;
        hltflag[index] = hltresults->accept(itrig);
        
        auto psIt = cachedHltPrescales_.find(trigname);
        if (psIt != cachedHltPrescales_.end()) {
          hltPrescaleNumerator[index] = psIt->second.first;
          hltPrescaleDenominator[index] = psIt->second.second;
        }
      }
    }
  }

  // L1 triggers
  auto& l1GtUtils = const_cast<l1t::L1TGlobalUtil&>(hltPrescaleProvider_->l1tGlobalUtil());
  l1GtUtils.retrieveL1(iEvent, iSetup);

  auto const& menu = iSetup.getHandle(l1GtMenuToken_);

  if (l1results.isValid() && l1results->size() != 0) {
    // Reset
    for (int i = 0; i < kMaxL1Flag; ++i) {
      l1flag[i] = -1;
      l1Prescl[i] = -1;
    }

    // First event: Book branches only for L1 triggers matching patterns
    if (L1EvtCnt == 0) {
      int il1_filtered = 0;
      
      for (auto const& keyval : menu->getAlgorithmMap()) {
        std::string const& trigname = keyval.second.getName();
        
        // Only add if matches pattern
        if (matchesAnyPattern(trigname, l1Patterns_)) {
          TString l1name = trigname;
          // Use offset to avoid collision with HLT indices
          int offset = selectedHltPaths_.size();
          t_->Branch(l1name, l1flag + il1_filtered, l1name + "/I");
          t_->Branch(l1name + "_Prescl", l1Prescl + il1_filtered, l1name + "_Prescl/I");
          pathtoindex[trigname] = offset + il1_filtered;
          selectedL1Paths_.push_back(trigname);
          ++il1_filtered;
        }
      }
      
      edm::LogInfo("TriggerAnalyzer2") << "Selected " << il1_filtered << " L1 paths out of " << menu->getAlgorithmMap().size();
      L1EvtCnt++;
    }

    // Update L1 prescale cache if lumisection changed
    bool lumiChanged = cachedL1Prescales_.empty() || (fLumiBlock != cachedLumiBlock_);
    if (lumiChanged) {
      cachedL1Prescales_.clear();
      for (auto const& keyval : menu->getAlgorithmMap()) {
        auto const& l1name = keyval.second.getName();
        int l1index = keyval.second.getIndex();
        if (pathtoindex.find(l1name) != pathtoindex.end()) {
          double prescale = -1;
          l1GtUtils.getPrescaleByBit(l1index, prescale);
          cachedL1Prescales_[l1name] = prescale;
        }
      }
    }

    GlobalAlgBlk const& result = l1results->at(0, 0);

    // Fill values for selected L1 triggers (use cached prescales)
    int offset = selectedHltPaths_.size();
    for (auto const& keyval : menu->getAlgorithmMap()) {
      auto const& l1name = keyval.second.getName();
      int l1index = keyval.second.getIndex();

      auto it = pathtoindex.find(l1name);
      if (it != pathtoindex.end()) {
        int index = it->second - offset;  // Remove offset for l1flag array
        
        l1flag[index] = result.getAlgoDecisionFinal(l1index);
        
        auto psIt = cachedL1Prescales_.find(l1name);
        if (psIt != cachedL1Prescales_.end()) {
          l1Prescl[index] = psIt->second;
        }
      }
    }
  }

  t_->Fill();
}

void TriggerAnalyzer2::beginRun(const edm::Run& run, const edm::EventSetup& es) {
  bool changed(true);
  if (hltPrescaleProvider_->init(run, es, processName_, changed)) {
    if (changed) {
      edm::LogWarning("TriggerAnalyzer2") << "HLT config change";
    }
  } else {
    edm::LogInfo("TriggerAnalyzer2") << "HLT initialisation failed for process " << processName_;
  }
}

void TriggerAnalyzer2::endRun(const edm::Run& run, const edm::EventSetup& es) {}

void TriggerAnalyzer2::endJob() {
  edm::LogInfo("TriggerAnalyzer2") << "Summary: Stored " << selectedHltPaths_.size() 
                                   << " HLT and " << selectedL1Paths_.size() << " L1 trigger paths";
}

DEFINE_FWK_MODULE(TriggerAnalyzer2);
