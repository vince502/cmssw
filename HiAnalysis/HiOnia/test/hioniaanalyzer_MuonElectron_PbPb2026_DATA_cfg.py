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
options.inputFiles = [
'/store/hidata/HIRun2026A/HIPhysicsRawPrime0/MINIAOD/PbPbEW-PromptReco-v1/000/404/323/00000/da7e792b-9810-4d56-bb2a-34329c47c2ae.root',
'/store/hidata/HIRun2026A/HIPhysicsRawPrime0/MINIAOD/PbPbEW-PromptReco-v1/000/404/350/00000/31a777ae-e7b6-4859-9619-b2246f7a001e.root',
'/store/hidata/HIRun2026A/HIPhysicsRawPrime0/MINIAOD/PbPbEW-PromptReco-v1/000/404/354/00000/bad36c00-6a52-430b-84de-bac1e4377606.root',
'/store/hidata/HIRun2026A/HIPhysicsRawPrime0/MINIAOD/PbPbEW-PromptReco-v1/000/404/353/00000/f9a6cfe5-d03b-4545-89f5-9563f2336039.root',
]
options.maxEvents = -1
options.register(
    'disableRootCache',
    False,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.bool,
    'Set PoolSource cacheSize=0 for retries of TTreeCache::FillBuffer input-read failures'
)
options.register(
    'applyEventSel',
    applyEventSel,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.bool,
    'Apply the configured primary-vertex and HLT event filters'
)
options.parseArguments()
applyEventSel = options.applyEventSel

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
    "mass > 0 && mass < 120 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25"
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
process.hionia.dblTriggerPathNames = muonTriggerList['DoubleMuonTrigger']
process.hionia.sglTriggerPathNames = muonTriggerList['SingleMuonTrigger']
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(True)
process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")

# Decorate trigger-matched PAT muons with the heavy-ion MVA isolation used by
# the Run3 PbPb forest muon producer, then make HiOnia consume that collection.
process.load('HeavyIonsAnalysis.JetAnalysis.hiFJRhoAnalyzer_cff')
process.hiOniaRhoSequence = cms.Sequence(process.kt4PFJetsForRho * process.hiFJRhoProducerFinerBins)
process.load('HiAnalysis.HiOnia.hiOniaMuons_cfi')
process.hiOniaMuons.muons = cms.InputTag("patMuonsWithTrigger")
process.hiOniaMuons.pfCandidates = cms.InputTag("packedPFCandidates")
process.hiOniaMuons.centrality = cms.InputTag("centralityBin", "HFtowers")
process.hiOniaMuons.etaMap = cms.InputTag("hiFJRhoProducerFinerBins", "mapEtaEdges")
process.hiOniaMuons.rhoMap = cms.InputTag("hiFJRhoProducerFinerBins", "mapToRho")
process.onia2MuMuPatGlbGlb.muons = cms.InputTag("hiOniaMuons")
process.hionia.srcMuon = cms.InputTag("hiOniaMuons")

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

# Official Run3 cut-based electron VID IDs for offline comparison.
from PhysicsTools.SelectorUtils.tools.vid_id_tools import (
    DataFormat,
    setupAllVIDIdsInModule,
    setupVIDElectronSelection,
    switchOnVIDElectronIdProducer,
)
switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
electronIDModules = [
    'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Winter22_122X_V1_cff',
]
for electronIDModule in electronIDModules:
    setupAllVIDIdsInModule(process, electronIDModule, setupVIDElectronSelection)

# Z -> e+e- preselection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 0 && mass < 120 && charge == 0"
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
    srcElectron = cms.InputTag('slimmedElectrons'),
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
    useVIDElectronID = cms.bool(True),
    eleVIDVetoIdMap = cms.InputTag('egmGsfElectronIDs:cutBasedElectronID-Winter22-122X-V1-veto'),
    eleVIDLooseIdMap = cms.InputTag('egmGsfElectronIDs:cutBasedElectronID-Winter22-122X-V1-loose'),
    eleVIDMediumIdMap = cms.InputTag('egmGsfElectronIDs:cutBasedElectronID-Winter22-122X-V1-medium'),
    eleVIDTightIdMap = cms.InputTag('egmGsfElectronIDs:cutBasedElectronID-Winter22-122X-V1-tight'),
    computeHIMVAId = cms.bool(True),
    hiMVAIdModel = cms.FileInPath('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/eleid_BDT.ubj'),
    hiMVARho = cms.InputTag(''),
    hiMVARhoEtaMap = cms.InputTag('hiFJRhoProducerFinerBins', 'mapEtaEdges'),
    hiMVARhoMap = cms.InputTag('hiFJRhoProducerFinerBins', 'mapToRho'),
    storeGenInfo = cms.bool(False),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(False),
    useEvtPlane = cms.untracked.bool(False)
)

# Keep muon, electron, and event trees synchronized at the event level.
process.requireDileptonCandidate = cms.EDFilter(
    "DileptonEventFilter",
    src = cms.VInputTag(
        cms.InputTag("hiOniaMuons"),
        cms.InputTag("slimmedElectrons"),
    ),
    minNumber = cms.uint32(1),
)

# Event-level tree from HeavyIonsAnalysis/EventAnalysis, filled only after the
# shared dilepton and HLT filters below.
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.hiEvtAnalyzer.CentralitySrc = cms.InputTag("hiCentrality")
process.hiEvtAnalyzer.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
process.hiEvtAnalyzer.Vertex = cms.InputTag("offlineSlimmedPrimaryVertices")
process.hiEvtAnalyzer.pfCandidateSrc = cms.InputTag("packedPFCandidates")
process.hiEvtAnalyzer.doCentrality = cms.bool(True)
process.hiEvtAnalyzer.doEvtPlane = cms.bool(False)
process.hiEvtAnalyzer.doEvtPlaneFlat = cms.bool(False)
process.hiEvtAnalyzer.doMC = cms.bool(False)
process.hiEvtAnalyzer.doHiMC = cms.bool(False)
process.hiEvtAnalyzer.doHFfilters = cms.bool(False)
process.hiEvtAnalyzer.addClusterInfo = cms.bool(False)

#----------------------------------------------------------------------------
# Event Selection (Optional)
#----------------------------------------------------------------------------
if applyEventSel:
    process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
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
    process.hiEvtAnalyzer.Vertex = cms.InputTag("unpackedTracksAndVertices")

# Load required modules for electron analysis. HIPhysicsRawPrime MINIAOD does not
# carry slimmedLowPtElectrons, so use slimmedElectrons directly.
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
process.hioniaElectrons.srcElectron = cms.InputTag('slimmedElectrons')
process.hioniaElectrons.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

# Build one accepted-event path so all output trees have the same entries:
# event prefilters -> HLT -> lepton reco -> at least one muon or electron
# -> event tree + dimuon tree + dielectron tree.
preDileptonFilters = cms.Sequence()
postDileptonFilters = cms.Sequence(process.requireDileptonCandidate)
if applyEventSel:
    preDileptonFilters = cms.Sequence(process.primaryVertexFilter)
    if miniAOD and hasattr(process, "beamScrapingFilter"):
        preDileptonFilters *= process.beamScrapingFilter
    elif miniAOD:
        print("[WARN] beamScrapingFilter missing; skipping beam-scraping filter insertion")
    preDileptonFilters *= process.hltHI

process.dileptonRecoSequence = cms.Sequence(
    process.patMuonSequence *
    process.hiOniaRhoSequence *
    process.hiOniaMuons *
    process.onia2MuMuPatGlbGlb *
    process.egmGsfElectronIDSequence *
    process.onia2ElectronElectronPatGlbGlb
)

process.acceptedEventAnalysis = cms.Sequence(
    process.hiEvtAnalyzer *
    process.hionia *
    process.hioniaElectrons
)

process.oniaTreeAna = cms.Path(
    preDileptonFilters *
    process.dileptonRecoSequence *
    postDileptonFilters *
    process.acceptedEventAnalysis
)

#----------------------------------------------------------------------------
# Input/Output Configuration
#----------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)
if options.disableRootCache:
    process.source.cacheSize = cms.untracked.uint32(0)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = 1

# Schedule the combined path
process.schedule = cms.Schedule(process.oniaTreeAna)

print("="*80)
print("Configuration complete!")
print("Output will contain THREE synchronized trees after the shared HLT+dilepton filter:")
print("  1. hiEvtAnalyzer/HiTree    - Event-level heavy-ion info")
print("  2. hionia/myTree           - Muon analysis (Z -> mu+mu-)")
print("  3. hioniaElectrons/eleTree - Electron analysis (Z -> e+e-)")
print(f"Output file: {options.outputFile}")
print(f"Input files: {len(options.inputFiles)}")
print("NOTE: dimuon or dielectron candidates can be zero in one channel if the other channel accepted the event")
print("NOTE: using heavy-ion HLT filter patterns")
print("="*80)
