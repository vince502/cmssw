import FWCore.ParameterSet.Config as cms

# Default muon trigger paths (can be overridden)
muon_double_trigger_paths = [
    'HLT_HIL1DoubleMu0_MaxDr3p5_Open_v',
    'HLT_HIL1DoubleMu0_v',
    'HLT_HIL1DoubleMu0_SQ_v',
    'HLT_HIL2DoubleMu0_Open_v',
    'HLT_HIL2DoubleMu0_M1p5to6_Open_v',
    'HLT_HIL2DoubleMu2p8_M1p5to6_Open_v',
    'HLT_HIL2DoubleMu0_M7to15_Open_v',
    'HLT_HIL2DoubleMu3_M7to15_Open_v',
]

muon_single_trigger_paths = [
    'HLT_HIL1SingleMu0_Open_v',
    'HLT_HIL1SingleMu0_v',
    'HLT_HIL2SingleMu3_Open_v',
    'HLT_HIL2SingleMu5_v',
    'HLT_HIL2SingleMu7_v',
    'HLT_HIL2SingleMu12_v',
]

# Default electron trigger paths (can be overridden)
electron_trigger_paths = [
    'HLT_HIEle10Gsf_v',
    'HLT_HIEle15Gsf_v',
    'HLT_HIEle20Gsf_v',
    'HLT_HIEle30Gsf_v',
    'HLT_HIEle40Gsf_v',
    'HLT_HIEle50Gsf_v',
    'HLT_HIDoubleEle10Gsf_v',
    'HLT_HIEle15Ele10Gsf_v',
    'HLT_HIEle15Ele10GsfMass50_v',
]

hioniaCombined = cms.EDAnalyzer(
    'HiOniaCombinedAnalyzer',
    
    # =========================================
    # Common input collections
    # =========================================
    primaryVertexTag = cms.InputTag('offlinePrimaryVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    triggerResults = cms.InputTag('TriggerResults', '', 'HLT'),
    CentralitySrc = cms.InputTag(''),
    CentralityBinSrc = cms.InputTag(''),
    EvtPlane = cms.InputTag(''),
    genParticles = cms.InputTag('genParticles'),
    
    # =========================================
    # Muon input collections
    # =========================================
    srcMuon = cms.InputTag('patMuonsWithTrigger'),
    srcMuonNoTrig = cms.InputTag('patMuonsWithoutTrigger'),
    srcDimuon = cms.InputTag('onia2MuMuPatGlbGlb'),
    
    # =========================================
    # Electron input collections
    # =========================================
    srcElectron = cms.InputTag('slimmedElectrons'),
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    
    # =========================================
    # Common configuration
    # =========================================
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(False),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    useEvtPlane = cms.untracked.bool(False),
    checkTrigNames = cms.bool(False),
    mom4format = cms.string('vector'),  # 'vector' for flat tree, 'array' for TClonesArray
    
    # =========================================
    # Muon configuration
    # =========================================
    fillMuons = cms.bool(True),
    fillSingleMuons = cms.bool(True),
    muonSel = cms.string('GlbOrTrk'),  # Options: GlbGlb, GlbOrTrk, Trk, All
    dblMuonTriggerPathNames = cms.vstring(*muon_double_trigger_paths),
    sglMuonTriggerPathNames = cms.vstring(*muon_single_trigger_paths),
    
    # =========================================
    # Electron configuration
    # =========================================
    fillElectrons = cms.bool(True),
    storeGenInfo = cms.bool(False),
    electronTriggerPathNames = cms.vstring(*electron_trigger_paths),
    
    # =========================================
    # L1 Trigger stage (required by HLTPrescaleProvider)
    # =========================================
    stageL1Trigger = cms.uint32(2),  # Stage 2 for Run 3
)


# Helper function to configure combined analyzer for different scenarios
def configureHiOniaCombined(process, 
                            isMC=False,
                            isHI=True,
                            fillMuons=True,
                            fillElectrons=True,
                            useMiniAOD=True):
    """
    Configure the combined muon+electron analyzer.
    
    Args:
        process: The CMSSW process
        isMC: Whether this is MC
        isHI: Whether this is heavy-ion data
        fillMuons: Whether to fill muon branches
        fillElectrons: Whether to fill electron branches
        useMiniAOD: Whether using MiniAOD format
    """
    
    # Clone the default configuration
    process.hioniaCombined = hioniaCombined.clone(
        isMC = cms.untracked.bool(isMC),
        isHI = cms.untracked.bool(isHI),
        fillMuons = cms.bool(fillMuons),
        fillElectrons = cms.bool(fillElectrons),
        storeGenInfo = cms.bool(isMC),
    )
    
    if useMiniAOD:
        # MiniAOD input tags
        process.hioniaCombined.primaryVertexTag = cms.InputTag('offlineSlimmedPrimaryVertices')
        process.hioniaCombined.genParticles = cms.InputTag('prunedGenParticles')
        process.hioniaCombined.srcElectron = cms.InputTag('slimmedElectrons')
        process.hioniaCombined.conversions = cms.InputTag('reducedEgamma', 'reducedConversions')
    
    if isHI:
        process.hioniaCombined.CentralitySrc = cms.InputTag('hiCentrality')
        process.hioniaCombined.CentralityBinSrc = cms.InputTag('centralityBin', 'HFtowers')
    
    return process
