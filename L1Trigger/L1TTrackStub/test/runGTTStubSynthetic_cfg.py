import FWCore.ParameterSet.Config as cms


process = cms.Process("GTTSTUBTEST")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("L1Trigger.L1TTrackStub.gttStubEmulation_cff")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(5))
process.source = cms.Source(
    "EmptySource",
    firstRun=cms.untracked.uint32(1),
    firstLuminosityBlock=cms.untracked.uint32(1),
    firstEvent=cms.untracked.uint64(1),
    numberEventsInLuminosityBlock=cms.untracked.uint32(5),
)

process.options = cms.untracked.PSet(
    numberOfThreads=cms.untracked.uint32(2),
    numberOfStreams=cms.untracked.uint32(2),
    wantSummary=cms.untracked.bool(True),
)

process.gttStubSyntheticStreams = cms.EDProducer(
    "l1t::test::GTTStubSyntheticStreams"
)
process.serenityGTTStubUnpacker.src = cms.InputTag("gttStubSyntheticStreams")
process.serenityGTTStubUnpacker.throwOnMalformed = False

process.gttStubSyntheticValidation = cms.EDAnalyzer(
    "l1t::test::GTTStubSyntheticValidation",
    primitive=cms.InputTag("serenityGTTStubUnpacker"),
    algoBlocks=cms.InputTag("gttStubAlgoBlocks"),
    algorithmName=cms.string("L1_GTT_LowStubOccupancy"),
)

process.gttStubSyntheticPacketPath = cms.Path(
    process.gttStubSyntheticStreams + process.serenityGTTStubUnpacker
)
process.gttStubSyntheticValidationEndPath = cms.EndPath(
    process.gttStubSyntheticValidation
)

process.schedule = cms.Schedule(
    process.gttStubSyntheticPacketPath,
    process.gttStubLowOccupancyPath,
    process.gttStubAlgoBlockPath,
    process.gttStubSyntheticValidationEndPath,
)
