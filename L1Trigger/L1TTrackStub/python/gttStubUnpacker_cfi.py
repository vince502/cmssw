import FWCore.ParameterSet.Config as cms


serenityGTTStubUnpacker = cms.EDProducer(
    "l1t::GTTStubUnpacker",
    src=cms.InputTag("apolloGTTStubPacker"),
    regionLinks=cms.vuint32(0, 2, 4, 6, 8, 10, 12, 14, 16),
    metadataTrackSlot=cms.uint32(103),
    throwOnMalformed=cms.bool(True),
)
