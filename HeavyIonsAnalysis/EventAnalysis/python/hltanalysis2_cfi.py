import FWCore.ParameterSet.Config as cms

# TriggerAnalyzer2: Pattern-filtered HLT/L1 analyzer
# Only stores triggers matching the specified glob patterns
#
# Example patterns:
#   'HLT_*Mu*'      - all muon triggers
#   'HLT_*Jet*'     - all jet triggers  
#   'HLT_*ZeroBias*'- zero bias triggers
#   'L1_*Mu*'       - L1 muon triggers

hltanalysis2 = cms.EDAnalyzer(
    'TriggerAnalyzer2',
    HLTProcessName = cms.string('HLT'),
    hltresults = cms.InputTag('TriggerResults::HLT'),
    l1results = cms.InputTag('gtStage2Digis'),
    hltPSProvCfg = cms.PSet(stageL1Trigger = cms.uint32(2)),
    # Default: empty patterns = store ALL triggers (same as original)
    hltPatterns = cms.vstring([]),
    l1Patterns = cms.vstring([]),
    # Set to False to skip prescale queries (much faster)
    storePrescales = cms.bool(True),
)

# Preset for Onia + Jet analysis (no UPC, no Photon)
# Use '!' prefix for exclusion patterns
def configureOniaJetTriggers(analyzer):
    """Configure analyzer to keep only Muon/Jet/MB/ZB triggers (no UPC)"""
    analyzer.hltPatterns = cms.vstring([
        'HLT_HIMinimumBias*',
        'HLT_HIL1DoubleMu*',
        'HLT_HIL2DoubleMu*',
        'HLT_HIL3DoubleMu*',
        'HLT_HIL1SingleMu*',
        'HLT_HIL2SingleMu*',
        'HLT_HIL3SingleMu*',
        'HLT_HIPuAK4CaloJet*',
        'HLT_HICsAK4PFJet*',
        'HLT_HIZeroBias_v*',
        'HLT_HIZeroBias_HighRate*',
        '!*UPC*',  # Exclude UPC triggers
    ])
    analyzer.l1Patterns = cms.vstring([
        'L1_MinimumBiasHF*',
        'L1_DoubleMu*',
        'L1_SingleMu0*',
        'L1_SingleMu3*',
        'L1_SingleMu5*',
        'L1_SingleMu7*',
        'L1_SingleMu12*',
        'L1_SingleMu22*',
        'L1_SingleJet*',
        'L1_DoubleJet*',
        'L1_ZeroBias*',
        '!*UPC*',  # Exclude UPC triggers
    ])
    return analyzer

# Preset for muon-only analysis
def configureMuonTriggers(analyzer):
    """Configure analyzer to keep only muon triggers"""
    analyzer.hltPatterns = cms.vstring([
        'HLT_*Mu*',
        'HLT_*mu*',
        'HLT_HIMinimumBias*',
        'HLT_*ZeroBias*',
    ])
    analyzer.l1Patterns = cms.vstring([
        'L1_*Mu*',
        'L1_DoubleMu*',
        'L1_SingleMu*',
        'L1_MinimumBias*',
        'L1_ZeroBias*',
    ])
    return analyzer

# Create preconfigured instances
hltanalysis2_OniaJet = hltanalysis2.clone()
configureOniaJetTriggers(hltanalysis2_OniaJet)

hltanalysis2_Muon = hltanalysis2.clone()
configureMuonTriggers(hltanalysis2_Muon)
