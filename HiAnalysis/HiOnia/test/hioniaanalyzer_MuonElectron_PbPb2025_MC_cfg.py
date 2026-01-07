import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Combined Muon + Electron ONIA TREE: 2025 PbPb MC
#----------------------------------------------------------------------------

print("="*80)
print("COMBINED MUON + ELECTRON ONIA TREE ANALYZER")
print("Configuration: 2025 PbPb MC (Realistic)")
print("="*80)

HLTProcess     = "HLT"
isMC           = True
muonSelection  = "GlbOrTrk"
applyEventSel  = False
OnlySoftMuons  = False
applyCuts      = False
SumETvariables = True
SofterSgMuAcceptance = False
doTrimuons     = False
doDimuonTrk    = False
atLeastOneCand = False
OneMatchedHLTMu = -1
keepExtraColl  = False
miniAOD        = True
UsePropToMuonSt = True
pdgId = 443  # J/Psi
useMomFormat = "vector"
addEventPlane = False

# Print settings
print("[INFO] Settings:")
print(f"[INFO]   isMC                 = {isMC}")
print(f"[INFO]   muonSelection        = {muonSelection}")
print(f"[INFO]   applyEventSel        = {applyEventSel}")
print(f"[INFO]   miniAOD              = {miniAOD}")
print(f"[INFO]   useMomFormat         = {useMomFormat}")
print("="*80)

# Set up process
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2024)

# Setup options
options = VarParsing.VarParsing('analysis')
options.inputFiles = [
#    '/store/user/fdamas/PbPb2025/RunPrepMC/JpsiDielectron_pTHatMin4_HydjetEmbedded_Pythia8_TuneCP5_1510pre6/PAT_151X_mcRun3_2025_realistic_HI_v1/251002_094458/0000/step4_PAT_102.root',
'file:/eos/cms/store/group/phys_heavyions/soohwan/Run3/MC/RecoECALDriven/RECO_v1_191.root',
'file:/eos/cms/store/group/phys_heavyions/soohwan/Run3/MC/RecoECALDriven/RECO_v1_192.root',
'file:/eos/cms/store/group/phys_heavyions/soohwan/Run3/MC/RecoECALDriven/RECO_v1_193.root',
'file:/eos/cms/store/group/phys_heavyions/soohwan/Run3/MC/RecoECALDriven/RECO_v1_198.root',

]
options.outputFile = 'OniaTree_MuonElectron_PbPb2025_MC.root'
options.maxEvents = -1
#options.parseArguments()

#----------------------------------------------------------------------------
# Global Tag and Services
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = '151X_mcRun3_2025_realistic_HI_v1'  # 2025 PbPb MC
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

print(f"[INFO] Using Global Tag: {globalTag}")

# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

# HI rho producer for electron isolation
process.load('HeavyIonsAnalysis.JetAnalysis.hiFJRhoAnalyzer_cff')

#----------------------------------------------------------------------------
# MUON Analysis
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

from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=muonTriggerList,
                 HLTProName=HLTProcess,
                 muonSelection=muonSelection,
                 L1Stage=2,
                 isMC=isMC,
                 pdgID=pdgId,
                 outputFileName=options.outputFile,
                 doTrimu=doTrimuons,
                 OnlySingleMuons=False)

process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 2.4 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1. && abs(eta) < 2.4 && isTrackerMuon")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

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
process.hionia.genealogyInfo = cms.bool(True)

process.oniaTreeAna.replace(process.hionia, process.centralityBin * process.hionia)

#----------------------------------------------------------------------------
# ELECTRON Analysis
#----------------------------------------------------------------------------
electronTriggerList = [
    'HLT_HIEle10Gsf_v',
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

# Configure for J/psi -> ee selection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 1.5 && charge == 0"
)
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 0.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 0.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.genParticles = cms.InputTag("prunedGenParticles")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerList)

# Electron analyzer
from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons
process.hioniaElectrons = hioniaElectrons.clone(
    srcElectron = cms.InputTag('hiElectrons'),  # Will use corrected electrons with HI ID/ISO
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    primaryVertexTag = cms.InputTag('offlineSlimmedPrimaryVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    triggerResults = cms.InputTag('TriggerResults', '', 'HLT'),
    CentralitySrc = cms.InputTag('hiCentrality'),
    CentralityBinSrc = cms.InputTag('centralityBin', 'HFtowers'),
    genParticles = cms.InputTag('prunedGenParticles'),
    triggerPathNames = cms.vstring(*electronTriggerList),
    checkTriggerNames = cms.bool(False),
    storeGenInfo = cms.bool(True),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(True),
    useEvtPlane = cms.untracked.bool(False)
)
process.hioniaElectrons.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")

#----------------------------------------------------------------------------
# Paths and Schedule
#----------------------------------------------------------------------------
# Setup MiniAOD first
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)

# Load required modules for electron analysis
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
# Note: unpackedTracksAndVertices is already loaded and added by changeToMiniAOD()

# NOTE: Skipping HI electron ID/ISO for now (BDT model files not available)
# Just use standard slimmedElectrons
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

# Update electron analyzer to use standard electrons
process.hioniaElectrons.srcElectron = cms.InputTag('slimmedElectrons')

# Add electron analysis to the muon path
# Note: unpackedTracksAndVertices and centralityBin are already in oniaTreeAna
process.oniaTreeAna = cms.Path(
    process.oniaTreeAna *
    process.onia2ElectronElectronPatGlbGlb *
    process.hioniaElectrons
)

#----------------------------------------------------------------------------
# Input/Output Configuration
#----------------------------------------------------------------------------
import sys

fileToRun=str(sys.argv[1])
process.source = cms.Source("PoolSource",
    #fileNames = cms.untracked.vstring(options.inputFiles),
    fileNames = cms.untracked.vstring(str(fileToRun))
)

process.TFileService = cms.Service("TFileService",
    #fileName = cms.string(options.outputFile)
    #fileName = cms.string(f'file:/afs/cern.ch/work/s/soohwan/private/Analysis/OniaTree2025/JpsiToEETest/CMSSW_15_1_0_pre6/src/HiAnalysis/HiOnia/test/output/hionia_{sys.argv[2]}.root')
    fileName = cms.string(f'file:/eos/home-s/soohwan/Run2025/outputRecoV2Oct27/hionia_{sys.argv[2]}.root')
)
#process.source = cms.Source("PoolSource",
#    fileNames = cms.untracked.vstring(options.inputFiles),
#)

#process.TFileService = cms.Service("TFileService",
#    fileName = cms.string(options.outputFile)
#)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = 1

# Schedule the combined path
process.schedule = cms.Schedule(process.oniaTreeAna)

print("="*80)
print("Configuration complete!")
print("Output will contain TWO trees:")
print("  1. hionia/myTree           - Muon analysis (J/psi -> mu+mu-)")
print("  2. hioniaElectrons/eleTree - Electron analysis (J/psi -> e+e-)")
print(f"Output file: {options.outputFile}")
print("="*80)
