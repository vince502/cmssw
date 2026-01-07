import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Combined Muon + Electron ONIA TREE: 2024 ppRef MC
#----------------------------------------------------------------------------

print("="*80)
print("COMBINED MUON + ELECTRON ONIA TREE ANALYZER")
print("Configuration: 2024 ppRef (5.36 TeV) MC")
print("="*80)

HLTProcess     = "HLT"
isMC           = True
muonSelection  = "All"
applyEventSel  = True
OnlySoftMuons  = False
applyCuts      = False
SumETvariables = False
SofterSgMuAcceptance = False
doTrimuons     = False
doDimuonTrk    = False
atLeastOneCand = False
OneMatchedHLTMu = -1
keepExtraColl  = False
miniAOD        = True
UsePropToMuonSt = True
pdgId = 443  # J/Psi
useMomFormat = "array"

# Print settings
print("[INFO] Settings:")
print(f"[INFO]   isMC                 = {isMC}")
print(f"[INFO]   muonSelection        = {muonSelection}")
print(f"[INFO]   applyEventSel        = {applyEventSel}")
print(f"[INFO]   miniAOD              = {miniAOD}")
print(f"[INFO]   useMomFormat         = {useMomFormat}")
print("="*80)

# Set up process
process = cms.Process("HIOnia", eras.Run3_2024_ppRef)

# Setup options
options = VarParsing.VarParsing('analysis')
options.outputFile = "OniaTree_MuonElectron_ppRef2024_MC.root"
options.inputFiles = [
    '/store/user/fdamas/RunPrep2024/Jpsi_pTHatMin4/jpsi_pileupAverage5_RECOMINIAOD/241015_115034/0000/miniAOD_101.root',
]
options.maxEvents = -1
options.parseArguments()

#----------------------------------------------------------------------------
# Global Tag and Services
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = '141X_mcRun3_2024_realistic_ppRef5TeV_v5'  # 2024 ppRef MC
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

print(f"[INFO] Using Global Tag: {globalTag}")

#----------------------------------------------------------------------------
# MUON Analysis
#----------------------------------------------------------------------------
muonTriggerList = {
    'DoubleMuonTrigger': cms.vstring(
        "HLT_PPRefL1DoubleMu0_Open_v",
        "HLT_PPRefL1DoubleMu0_v",
        "HLT_PPRefL1DoubleMu0_SQ_v",
        "HLT_PPRefL1DoubleMu2_v",
        "HLT_PPRefL1DoubleMu2_SQ_v",
        "HLT_PPRefL2DoubleMu0_Open_v",
        "HLT_PPRefL2DoubleMu0_v",
        "HLT_PPRefL3DoubleMu0_Open_v",
        "HLT_PPRefL3DoubleMu0_v"
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_PPRefL1SingleMu7_v",
        "HLT_PPRefL1SingleMu12_v",
        "HLT_PPRefL2SingleMu7_v",
        "HLT_PPRefL2SingleMu12_v",
        "HLT_PPRefL2SingleMu15_v",
        "HLT_PPRefL3SingleMu3_v",
        "HLT_PPRefL3SingleMu5_v",
        "HLT_PPRefL3SingleMu7_v",
        "HLT_PPRefL3SingleMu12_v",
        "HLT_PPRefL3SingleMu15_v",
        "HLT_PPRefL3SingleMu20_v",
    )
}

from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=muonTriggerList,
                 muonSelection=muonSelection,
                 L1Stage=2,
                 isMC=isMC,
                 pdgID=pdgId,
                 outputFileName=options.outputFile,
                 doTrimu=doTrimuons)

process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 2.0 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.4")
if applyCuts:
    process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")

process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)
process.hionia.minimumFlag = cms.bool(keepExtraColl)
process.hionia.useGeTracks = cms.untracked.bool(keepExtraColl)
process.hionia.fillRecoTracks = cms.bool(keepExtraColl)
process.hionia.SofterSgMuAcceptance = cms.bool(SofterSgMuAcceptance)
process.hionia.SumETvariables = cms.bool(SumETvariables)
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu = cms.int32(OneMatchedHLTMu)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)
process.hionia.genealogyInfo = cms.bool(True)

#----------------------------------------------------------------------------
# ELECTRON Analysis
#----------------------------------------------------------------------------
electronTriggerList = [
    'HLT_PPRefEle10Gsf_v',
    'HLT_PPRefEle15Gsf_v',
    'HLT_PPRefEle20Gsf_v',
    'HLT_PPRefDoubleEle10Gsf_v',
]

# Load electron producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Configure for J/psi -> ee selection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 2.0 && charge == 0"  # J/psi -> ee
)
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.genParticles = cms.InputTag("prunedGenParticles")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerList)

# Electron analyzer
from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons
process.hioniaElectrons = hioniaElectrons.clone(
    srcElectron = cms.InputTag('slimmedElectrons'),
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    primaryVertexTag = cms.InputTag('offlineSlimmedPrimaryVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    triggerResults = cms.InputTag('TriggerResults', '', 'HLT'),
    CentralitySrc = cms.InputTag(''),
    CentralityBinSrc = cms.InputTag(''),
    genParticles = cms.InputTag('prunedGenParticles'),
    triggerPathNames = cms.vstring(*electronTriggerList),
    checkTriggerNames = cms.bool(False),
    storeGenInfo = cms.bool(True),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    isHI = cms.untracked.bool(False),
    isMC = cms.untracked.bool(True),
    useEvtPlane = cms.untracked.bool(False)
)

#----------------------------------------------------------------------------
# Event Selection (Optional)
#----------------------------------------------------------------------------
if applyEventSel:
    process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
    
    import HLTrigger.HLTfilters.hltHighLevel_cfi
    process.hltHI = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
    process.hltHI.HLTPaths = ["HLT_PPRefL*SingleMu*_v*", "HLT_PPRefEle*_v*"]
    process.hltHI.throw = False
    process.hltHI.andOr = True
    
    process.oniaTreeAna.replace(
        process.patMuonSequence,
        process.primaryVertexFilter * 
        process.clusterCompatibilityFilter * 
        process.patMuonSequence
    )

#----------------------------------------------------------------------------
# Paths and Schedule
#----------------------------------------------------------------------------
# Convert muon sequence to path and setup miniAOD
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)

process.oniaTreeAna = cms.Path(process.oniaTreeAna)

if miniAOD and applyEventSel:
    process.oniaTreeAna.replace(process.hionia, process.beamScrapingFilter * process.hionia)

# Load required modules for electron analysis
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')

# Update electron source for onia producer (use standard slimmedElectrons for ppRef)
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

# Add electron analysis to muon path
process.oniaTreeAna = cms.Path(
    process.oniaTreeAna *
    process.onia2ElectronElectronPatGlbGlb *
    process.hioniaElectrons
)

#----------------------------------------------------------------------------
# Input/Output Configuration
#----------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = 2

# Schedule the combined path
process.schedule = cms.Schedule(process.oniaTreeAna)

print("="*80)
print("Configuration complete!")
print("Output will contain TWO trees:")
print("  1. hionia/myTree           - Muon analysis (J/psi -> mu+mu-)")
print("  2. hioniaElectrons/eleTree - Electron analysis (J/psi -> e+e-)")
print(f"Output file: {options.outputFile}")
print("NOTE: ppRef uses standard (non-HI) electron ID/ISO")
print("="*80)
