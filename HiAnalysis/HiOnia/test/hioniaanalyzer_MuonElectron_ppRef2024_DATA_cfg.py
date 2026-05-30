import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Combined Muon + Electron Z TREE: 2026 PbPb Prompt Reco DATA
#----------------------------------------------------------------------------

print("="*80)
print("COMBINED MUON + ELECTRON Z TREE ANALYZER")
print("Configuration: 2026 PbPb Prompt Reco DATA")
print("="*80)

HLTProcess     = "HLT"
isMC           = False
muonSelection  = "Glb"
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
pdgId = 23  # Z
useMomFormat = "vector"

# Print settings
print("[INFO] Settings:")
print(f"[INFO]   isMC                 = {isMC}")
print(f"[INFO]   muonSelection        = {muonSelection}")
print(f"[INFO]   applyEventSel        = {applyEventSel}")
print(f"[INFO]   miniAOD              = {miniAOD}")
print(f"[INFO]   useMomFormat         = {useMomFormat}")
print("="*80)

# Set up process
process = cms.Process("HIOnia", eras.Run3_pp_on_PbPb_2026)

# Setup options
options = VarParsing.VarParsing('analysis')
options.outputFile = "OniaTree_MuonElectron_PbPb2026_DATA.root"
options.inputFiles = []
options.register(
    'datasetFile',
    'datasets.txt',
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    'Text file containing one input MINIAOD file per line'
)
options.maxEvents = -1
options.parseArguments()

if options.datasetFile:
    with open(options.datasetFile) as dataset_file:
        dataset_files = [
            line.strip()
            for line in dataset_file
            if line.strip() and not line.lstrip().startswith('#')
        ]
    if not dataset_files:
        raise RuntimeError(f"No input files found in datasetFile={options.datasetFile}")
    options.inputFiles = dataset_files
    print(f"[INFO] Loaded {len(options.inputFiles)} input files from {options.datasetFile}")

#----------------------------------------------------------------------------
# Global Tag and Services
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
globalTag = 'auto:run3_data_prompt'
process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, '')

print(f"[INFO] Using Global Tag: {globalTag}")

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
                 muonSelection=muonSelection,
                 L1Stage=2,
                 isMC=isMC,
                 pdgID=pdgId,
                 outputFileName=options.outputFile,
                 doTrimu=doTrimuons)

# Z -> mu+mu- preselection
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
    "mass > 60 && mass < 120 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 15.0 && abs(eta) < 2.4 && isTrackerMuon")
if applyCuts:
    process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.001")

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
process.hionia.dblTriggerPathNames = cms.vstring()
process.hionia.sglTriggerPathNames = cms.vstring()
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(True)
process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")

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
    'HLT_HIEle15Ele10GsfMass50_v',
]

# Load electron producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Z -> e+e- preselection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 60 && mass < 120 && charge == 0"
)
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 15.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 15.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerList)

# Electron analyzer
from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons
process.hioniaElectrons = hioniaElectrons.clone(
    srcElectron = cms.InputTag('unpackedElectrons'),
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
    storeGenInfo = cms.bool(False),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(False),
    useEvtPlane = cms.untracked.bool(False)
)

#----------------------------------------------------------------------------
# Event Selection (Optional)
#----------------------------------------------------------------------------
if applyEventSel:
    process.load('HeavyIonsAnalysis.Configuration.collisionEventSelection_cff')
    process.primaryVertexFilter = cms.EDFilter(
        "VertexSelector",
        src=cms.InputTag("offlineSlimmedPrimaryVertices"),
        cut=cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.Rho <= 2"),
        filter=cms.bool(True),
    )
    
    import HLTrigger.HLTfilters.hltHighLevel_cfi
    process.hltHI = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
    process.hltHI.HLTPaths = [
        "HLT_HIL*SingleMu*_v*",
        "HLT_HIL*DoubleMu*_v*",
        "HLT_HIEle*_v*",
        "HLT_HIDoubleEle*_v*",
    ]
    process.hltHI.throw = False
    process.hltHI.andOr = True
    
    process.oniaTreeAna.replace(
        process.patMuonSequence,
        process.primaryVertexFilter *
        process.hltHI *
        process.patMuonSequence
    )

#----------------------------------------------------------------------------
# Paths and Schedule
#----------------------------------------------------------------------------
# Convert muon sequence to path and setup miniAOD
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)
    if hasattr(process, "unpackedMuons"):
        process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)
    # Canonical MiniAOD vertex source for muon-side modules
    process.onia2MuMuPatGlbGlb.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
    process.patMuonsWithoutTrigger.pvSrc = cms.InputTag("unpackedTracksAndVertices")
    process.hionia.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")

process.oniaTreeAna = cms.Path(process.oniaTreeAna)

if miniAOD and applyEventSel:
    if hasattr(process, "beamScrapingFilter"):
        process.oniaTreeAna.replace(process.hionia, process.beamScrapingFilter * process.hionia)
    else:
        print("[WARN] beamScrapingFilter missing; skipping beam-scraping filter insertion")

# Load required modules for electron analysis. HIPhysicsRawPrime MINIAOD does not
# carry slimmedLowPtElectrons, so use slimmedElectrons directly.
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
process.hioniaElectrons.srcElectron = cms.InputTag('slimmedElectrons')
process.hioniaElectrons.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

# Add electron analysis modules to existing path
process.oniaTreeAna *= process.onia2ElectronElectronPatGlbGlb
process.oniaTreeAna *= process.hioniaElectrons

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
print("  1. hionia/myTree           - Muon analysis (Z -> mu+mu-)")
print("  2. hioniaElectrons/eleTree - Electron analysis (Z -> e+e-)")
print(f"Output file: {options.outputFile}")
print(f"Input files: {len(options.inputFiles)}")
print("NOTE: using heavy-ion HLT filter patterns")
print("="*80)
