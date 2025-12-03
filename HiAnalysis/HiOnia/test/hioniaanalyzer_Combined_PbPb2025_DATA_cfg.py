import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# COMBINED MUON + ELECTRON ONIA TREE: Single Tree Output
# + LEGACY HIONIA for VALIDATION
# Configuration: 2025 PbPb Prompt Reco DATA
#----------------------------------------------------------------------------

print("="*80)
print("COMBINED MUON + ELECTRON ONIA TREE ANALYZER (SINGLE TREE)")
print("+ LEGACY HIONIA ANALYZER FOR VALIDATION")
print("Configuration: 2025 PbPb Prompt Reco DATA")
print("="*80)

HLTProcess     = "HLT"
isMC           = False
muonSelection  = "GlbOrTrk"
applyEventSel  = False
OnlySoftMuons  = False
applyCuts      = False
SumETvariables = True
SofterSgMuAcceptance = False
atLeastOneCand = False
OneMatchedHLTMu = -1
keepExtraColl  = False
miniAOD        = True
UsePropToMuonSt = True
useMomFormat = "vector"
addEventPlane = False

# Set up process
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2025)

# Setup options
options = VarParsing.VarParsing('analysis')
options.inputFiles = [
    '/store/hidata/HIRun2025A/HIPhysicsRawPrime1/MINIAOD/PbPbEW-PromptReco-v1/000/399/720/00000/8a860b70-5f6c-414b-b7c7-8d866d5160a3.root',
]
options.outputFile = 'OniaTree_Combined_PbPb2025_DATA.root'
options.maxEvents = -1

#----------------------------------------------------------------------------
# Global Tag and Services
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = '151X_dataRun3_Prompt_v1'
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

print(f"[INFO] Using Global Tag: {globalTag}")

# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

#----------------------------------------------------------------------------
# MUON PAT Sequence Setup
#----------------------------------------------------------------------------
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

# Build muon PAT sequence (from existing infrastructure)
# This also sets up the LEGACY hionia analyzer
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

process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 2.4 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 5. && abs(eta) < 2.4 && isTrackerMuon")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

#----------------------------------------------------------------------------
# LEGACY HIONIA Analyzer Configuration (for validation)
#----------------------------------------------------------------------------
process.hionia.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
process.hionia.minimumFlag = cms.bool(keepExtraColl)
process.hionia.useGeTracks = cms.untracked.bool(keepExtraColl)
process.hionia.fillRecoTracks = cms.bool(keepExtraColl)
process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
process.hionia.SofterSgMuAcceptance = cms.bool(SofterSgMuAcceptance)
process.hionia.SumETvariables = cms.bool(SumETvariables)
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu = cms.int32(OneMatchedHLTMu)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.useEvtPlane = cms.untracked.bool(addEventPlane)

#----------------------------------------------------------------------------
# ELECTRON PAT Sequence Setup
#----------------------------------------------------------------------------
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

# Load electron producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Dielectron selection for Z boson (mass ~91 GeV)
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 60 && charge == 0")
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 10.0 && abs(eta) < 2.5")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 10.0 && abs(eta) < 2.5")
process.onia2ElectronElectronPatGlbGlb.onlyGoodElectrons = cms.bool(False)  # Store all, apply ID in analysis
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerList)

#----------------------------------------------------------------------------
# COMBINED ANALYZER Setup
#----------------------------------------------------------------------------
from HiAnalysis.HiOnia.hioniaCombinedAnalyzer_cfi import hioniaCombined

process.hioniaCombined = hioniaCombined.clone(
    # Common settings
    primaryVertexTag = cms.InputTag('unpackedTracksAndVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    triggerResults = cms.InputTag('TriggerResults', '', HLTProcess),
    CentralitySrc = cms.InputTag('hiCentrality'),
    CentralityBinSrc = cms.InputTag('centralityBin', 'HFtowers'),
    genParticles = cms.InputTag('prunedGenParticles'),
    
    # Muon input
    srcMuon = cms.InputTag('patMuonsWithTrigger'),
    srcMuonNoTrig = cms.InputTag('patMuonsWithoutTrigger'),
    srcDimuon = cms.InputTag('onia2MuMuPatGlbGlb'),
    
    # Electron input
    srcElectron = cms.InputTag('slimmedElectrons'),
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    
    # Configuration
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(isMC),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    useEvtPlane = cms.untracked.bool(addEventPlane),
    checkTrigNames = cms.bool(False),
    mom4format = cms.string(useMomFormat),
    
    # Muon config
    fillMuons = cms.bool(True),
    fillSingleMuons = cms.bool(True),
    muonSel = cms.string(muonSelection),
    dblMuonTriggerPathNames = muonTriggerList['DoubleMuonTrigger'],
    sglMuonTriggerPathNames = muonTriggerList['SingleMuonTrigger'],
    
    # Electron config
    fillElectrons = cms.bool(True),
    storeGenInfo = cms.bool(False),
    electronTriggerPathNames = cms.vstring(*electronTriggerList),
)

#----------------------------------------------------------------------------
# Setup for MiniAOD
#----------------------------------------------------------------------------
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)
    
    # Update electron tags for miniAOD
    process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
    process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
    process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

# Load required modules
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')

#----------------------------------------------------------------------------
# Path Definition
#----------------------------------------------------------------------------
# Build combined path with BOTH legacy hionia and new hioniaCombined
# This allows direct comparison
# NOTE: patMuonSequence does NOT include onia2MuMuPatGlbGlb, we must add it explicitly!
process.combinedOniaPath = cms.Path(
    process.centralityBin *
    process.patMuonSequence *
    process.onia2MuMuPatGlbGlb *      # DIMUON PRODUCER - required!
    process.onia2ElectronElectronPatGlbGlb *
#    process.hionia *          # LEGACY analyzer for validation
    process.hioniaCombined    # NEW combined analyzer
)

#----------------------------------------------------------------------------
# Input/Output Configuration
#----------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = 1

# Schedule the combined path
process.schedule = cms.Schedule(process.combinedOniaPath)

print("="*80)
print("Configuration complete!")
print("Output will contain TWO trees for validation:")
print("  1. hionia/myTree       - LEGACY muon analyzer (reference)")
print("  2. hioniaCombined/OniaTree - NEW combined analyzer")
print("")
print("Compare Reco_QQ_size between the two trees to validate!")
print("Branches in combined tree:")
print("  - Reco_mu_*, Reco_QQ_* (muon analysis)")
print("  - Reco_ele_*, Reco_ee_* (electron analysis)")
print(f"Output file: {options.outputFile}")
print("="*80)
