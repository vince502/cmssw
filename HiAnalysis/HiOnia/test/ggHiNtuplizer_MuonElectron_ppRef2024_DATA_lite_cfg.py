import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

process = cms.Process("GGLITE", eras.Run3_2024_ppRef)

options = VarParsing.VarParsing("analysis")
options.outputFile = "ggHiNtuplizer_MuonElectron_ppRef2024_DATA_lite.root"
options.maxEvents = 1000
options.inputFiles = [
    "/store/data/Run2024J/PPRefZeroBiasPlusForward0/MINIAOD/PromptReco-v1/000/387/696/00000/0037fb37-713f-4df8-9668-a2ce4665a93c.root"
]
options.register(
    "nThreads",
    5,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.int,
    "Number of CMSSW threads",
)
options.parseArguments()

process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "141X_dataRun3_Prompt_v3", "")

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(options.inputFiles),
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = cms.untracked.uint32(max(1, int(options.nThreads)))
process.options.numberOfStreams = cms.untracked.uint32(0)
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

from HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi import ggHiNtuplizer
process.ggHiNtuplizer = ggHiNtuplizer.clone(
    doGenParticles=cms.bool(False),
    doSuperClusters=cms.bool(False),
    doElectrons=cms.bool(True),
    doPhotons=cms.bool(False),
    doMuons=cms.bool(True),
    muonPtMin=cms.double(0.0),
    doEffectiveAreas=cms.bool(False),
    useValMapIso=cms.bool(False),
    doPhoERegression=cms.bool(False),
    doRecHitsEB=cms.bool(False),
    doRecHitsEE=cms.bool(False),
    doPfIso=cms.bool(False),
    electronSrc=cms.InputTag("slimmedElectrons"),
    muonSrc=cms.InputTag("slimmedMuons"),
    vertexSrc=cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc=cms.InputTag("offlineBeamSpot"),
    conversionsSrc=cms.InputTag("reducedEgamma", "reducedConversions"),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string(options.outputFile),
)

process.p = cms.Path(process.ggHiNtuplizer)
process.schedule = cms.Schedule(process.p)

print("============================================================")
print("ggHiNtuplizer lite cfg ready")
print("  outputFile =", options.outputFile)
print("  maxEvents  =", options.maxEvents)
print("  nThreads   =", options.nThreads)
print("  muonSrc    = slimmedMuons")
print("  electronSrc= slimmedElectrons")
print("============================================================")
