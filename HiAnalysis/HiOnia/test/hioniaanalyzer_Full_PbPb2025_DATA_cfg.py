import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#============================================================================
# FULL ONIA + EGM + JET ANALYZER
# Configuration: 2025 PbPb Prompt Reco DATA
#
# This config produces:
#   - hioniaCombined/OniaTree: Dimuon + Dielectron candidates (Onia style)
#   - ggHiNtuplizer/EventTree: Single electrons/photons (Forest style)  
#   - akCs4PFJetAnalyzer/t: Jets
#============================================================================


#----------------------------------------------------------------------------
# Configuration Flags
#----------------------------------------------------------------------------
HLTProcess     = "HLT"
isMC           = False
muonSelection  = "GlbOrTrk"
OnlySoftMuons  = False
applyCuts      = False
SumETvariables = True
SofterSgMuAcceptance = False
atLeastOneCand = False
OneMatchedHLTMu = -1
keepExtraColl  = False
miniAOD        = True
UsePropToMuonSt = True
useMomFormat   = "vector"
addEventPlane  = False

# Jet configuration
jetPtMin       = 15
jetAbsEtaMax   = 2.5
doHIJetID      = True
doWTARecluster = True
doBtagging     = False

#----------------------------------------------------------------------------
# Process Definition
#----------------------------------------------------------------------------
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2025)

# Setup options
options = VarParsing.VarParsing('analysis')
options.inputFiles = [
    '/store/hidata/HIRun2025A/HIPhysicsRawPrime1/MINIAOD/PbPbEW-PromptReco-v1/000/399/720/00000/8a860b70-5f6c-414b-b7c7-8d866d5160a3.root',
]
options.outputFile = 'OniaTree_Full_PbPb2025_DATA.root'
options.maxEvents = -1

#----------------------------------------------------------------------------
# Global Tag and Core Services
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = '151X_dataRun3_Prompt_v1'
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

# TransientTrack builder (needed for vertexing)
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')

#============================================================================
# PART 1: ONIA DIMUON + DIELECTRON ANALYSIS
#============================================================================

muonTriggerList = {
    'DoubleMuonTrigger': cms.vstring(
        "HLT_HIL1DoubleMu0_MaxDr3p5_Open_v",
        "HLT_HIL1DoubleMu0_v",
        "HLT_HIL1DoubleMu0_SQ_v",
        "HLT_HIL2DoubleMu0_Open_v",
        "HLT_HIL2DoubleMu0_M1p5to6_Open_v",
        "HLT_HIL2DoubleMu2p8_M1p5to6_Open_v",
        "HLT_HIL2DoubleMu0_M7to15_Open_v",
        "HLT_HIL2DoubleMu3_M7to15_Open_v",
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_HIL1SingleMu0_Open_v",
        "HLT_HIL1SingleMu0_v",
        "HLT_HIL2SingleMu3_Open_v",
        "HLT_HIL2SingleMu5_v",
        "HLT_HIL2SingleMu7_v",
        "HLT_HIL2SingleMu12_v",
    )
}

electronTriggerList = [
    'HLT_HIMinimumBiasHF1ANDZDC1nOR_v',
    'HLT_HIMinimumBiasHF1AND_v',
    'HLT_HIEle15Gsf_v',
    'HLT_HIEle20Gsf_v',
    'HLT_HIEle30Gsf_v',
    'HLT_HIEle40Gsf_v',
    'HLT_HIEle50Gsf_v',
    'HLT_HIDoubleEle10Gsf_v',
    'HLT_HIEle15Ele10Gsf_v',
    'HLT_HIEle15Ele10GsfMass50_v'
]

# Load muon PAT sequence (sets up unpackedMuons, patMuons, etc.)
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=muonTriggerList,
                 HLTProName=HLTProcess,
                 muonSelection=muonSelection,
                 L1Stage=2,
                 isMC=isMC,
                 pdgID=443,
                 outputFileName=options.outputFile,
                 doTrimu=False,
                 OnlySingleMuons=False)

# Dimuon selection
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 2.4 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 5. && abs(eta) < 2.4 && isTrackerMuon")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

# Load electron producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Dielectron selection for Z boson
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 60 && charge == 0")
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 10.0 && abs(eta) < 2.5")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 10.0 && abs(eta) < 2.5")
process.onia2ElectronElectronPatGlbGlb.onlyGoodElectrons = cms.bool(False)
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerList)

# Combined Onia analyzer
from HiAnalysis.HiOnia.hioniaCombinedAnalyzer_cfi import hioniaCombined
process.hioniaCombined = hioniaCombined.clone(
    primaryVertexTag = cms.InputTag('unpackedTracksAndVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    triggerResults = cms.InputTag('TriggerResults', '', HLTProcess),
    CentralitySrc = cms.InputTag('hiCentrality'),
    CentralityBinSrc = cms.InputTag('centralityBin', 'HFtowers'),
    genParticles = cms.InputTag('prunedGenParticles'),
    
    srcMuon = cms.InputTag('patMuonsWithTrigger'),
    srcMuonNoTrig = cms.InputTag('patMuonsWithoutTrigger'),
    srcDimuon = cms.InputTag('onia2MuMuPatGlbGlb'),
    
    srcElectron = cms.InputTag('slimmedElectrons'),
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(isMC),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    useEvtPlane = cms.untracked.bool(addEventPlane),
    checkTrigNames = cms.bool(False),
    mom4format = cms.string(useMomFormat),
    
    fillMuons = cms.bool(True),
    fillSingleMuons = cms.bool(True),
    muonSel = cms.string(muonSelection),
    dblMuonTriggerPathNames = muonTriggerList['DoubleMuonTrigger'],
    sglMuonTriggerPathNames = muonTriggerList['SingleMuonTrigger'],
    
    fillElectrons = cms.bool(True),
    storeGenInfo = cms.bool(False),
    electronTriggerPathNames = cms.vstring(*electronTriggerList),
)

#============================================================================
# PART 2: ggHiNtuplizer (Forest-style single electron/photon)
#============================================================================

process.load('HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi')
process.ggHiNtuplizer.doGenParticles = cms.bool(isMC)
process.ggHiNtuplizer.doMuons = cms.bool(False)  # We have muons in Onia tree
process.ggHiNtuplizer.doElectrons = cms.bool(True)
process.ggHiNtuplizer.doPhotons = cms.bool(True)
process.ggHiNtuplizer.doSuperClusters = cms.bool(False)
process.ggHiNtuplizer.muonSrc = cms.InputTag("unpackedMuons")

#============================================================================
# PART 3: JET ANALYSIS (Following HiForest approach)
#============================================================================

# Load HiForest info
process.load('HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi')
process.HiForestInfo.info = cms.vstring("HiForest+Onia, miniAOD, 151X, data")

# Load base jet analyzers
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_data_cff')
process.load('HeavyIonsAnalysis.JetAnalysis.akPu4CaloJetSequence_pponPbPb_data_cff')

# Configure CaloJet analyzer
process.akPu4CaloJetAnalyzer.jetTag = cms.InputTag("slimmedCaloJets")
process.akPu4CaloJetAnalyzer.jetPtMin = jetPtMin
process.akPu4CaloJetAnalyzer.doHiJetID = doHIJetID
process.akPu4CaloJetAnalyzer.hltTrgResults = cms.untracked.string('TriggerResults::' + HLTProcess)

#============================================================================
# MAIN FOREST PATH (like HiForest - single path with all analyzers)
#============================================================================
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.ggHiNtuplizer +
    process.akPu4CaloJetAnalyzer
)

#============================================================================
# PF JET SETUP (Following HiForest approach exactly)
#============================================================================
# Setup jet reconstruction chain using Forest function
from HeavyIonsAnalysis.JetAnalysis.setupJets_PbPb_cff import candidateBtaggingMiniAOD

# Configure jet labels - "4" means R=0.4
jetLabelsCS = ["4"]
matchJets = False

for jetLabel in jetLabelsCS:
    candidateBtaggingMiniAOD(process, 
                             isMC=False, 
                             jetPtMin=jetPtMin, 
                             jetCorrLevels=['L2Relative', 'L2L3Residual'], 
                             doBtagging=doBtagging, 
                             labelR=jetLabel)
    
    # Setup jet analyzer (cloned from base)
    setattr(process, "akCs" + jetLabel + "PFJetAnalyzer", process.akCs4PFJetAnalyzer.clone())
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").jetTag = "selectedUpdatedPatJetsAK" + jetLabel + "PFBtag"
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").jetName = 'akCs' + jetLabel + 'PF'
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").matchJets = matchJets
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").matchTag = 'patJetsAK' + jetLabel + 'PFUnsubJets'
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").doBtagging = doBtagging
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").doHiJetID = doHIJetID
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").doWTARecluster = doWTARecluster
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").jetPtMin = jetPtMin
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").jetAbsEtaMax = cms.untracked.double(jetAbsEtaMax)
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").rParam = 0.4 if jetLabel == "0" else float(jetLabel.replace("Flow", "")) * 0.1
    getattr(process, "akCs" + jetLabel + "PFJetAnalyzer").hltTrgResults = cms.untracked.string('TriggerResults::' + HLTProcess)
    # Add to forest path
    process.forest += getattr(process, "akCs" + jetLabel + "PFJetAnalyzer")


#============================================================================
# MiniAOD Setup
#============================================================================
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)
    
    process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
    process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
    process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

#============================================================================
# PATH DEFINITIONS
#============================================================================

# Onia path: Dimuon + Dielectron candidates (runs independently)
process.oniaPath = cms.Path(
    process.patMuonSequence *
    process.onia2MuMuPatGlbGlb *
    process.onia2ElectronElectronPatGlbGlb *
    process.hioniaCombined
)

# Forest path already defined above with:
# - HiForestInfo
# - centralityBin  
# - ggHiNtuplizer
# - akPu4CaloJetAnalyzer
# - akCs4PFJetAnalyzer (added via loop)

#============================================================================
# INPUT/OUTPUT
#============================================================================
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck")
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = 1

# Schedule: Onia path + Forest path (which includes EGM, jets, info)
process.schedule = cms.Schedule(
    process.oniaPath,
    process.forest
)

