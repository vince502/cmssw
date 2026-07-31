import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

from Configuration.Eras.Era_Phase2C22I13M9_cff import Phase2C22I13M9


options = VarParsing("analysis")
options.register(
    "maxStubCount",
    500,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "Inclusive upper bound of the low-stub-occupancy trigger",
)
options.register(
    "histogramFile",
    "gttStubValidation.root",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "ROOT file for strict validation histograms",
)
options.setDefault("inputFiles", ["file:step2.root"])
options.setDefault("outputFile", "gttStubCounts.root")
options.parseArguments()

process = cms.Process("GTTSTUBCOUNT", Phase2C22I13M9)
process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryExtendedRun4D121Reco_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("L1Trigger.TrackTrigger.TrackTrigger_cff")
process.load("L1Trigger.TrackerDTC.DTC_cff")
process.load("L1Trigger.L1TTrackStub.gttStubEmulation_cff")

from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic_T35", "")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(options.inputFiles),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string(options.histogramFile),
)

process.gttStubLowOccupancyDecision.maxStubCount = options.maxStubCount
process.gttStubValidation.maxStubCount = options.maxStubCount

# Rebuild the DTC payload from retained TTStubs. Typical production event
# content does not keep TTDTC, and this also guarantees consistent accepted/lost
# products for the counter.
process.emulation = cms.Path(
    process.ProducerDTC
    + process.gttStubEmulation
)
process.output = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string(options.outputFile),
    outputCommands=cms.untracked.vstring(
        "drop *",
        *process.gttStubEmulationEventContent.outputCommands,
    ),
)
process.outputStep = cms.EndPath(process.output)
process.schedule = cms.Schedule(
    process.emulation,
    process.gttStubLowOccupancyPath,
    process.gttStubAlgoBlockPath,
    process.gttStubAcceptPath,
    process.gttStubValidationEndPath,
    process.outputStep,
)
