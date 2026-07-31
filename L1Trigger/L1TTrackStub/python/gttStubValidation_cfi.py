import FWCore.ParameterSet.Config as cms


gttStubValidation = cms.EDAnalyzer(
    "l1t::GTTStubValidation",
    counts=cms.InputTag("apolloGTTStubCounts"),
    frames=cms.InputTag("apolloGTTStubPacker"),
    primitive=cms.InputTag("serenityGTTStubUnpacker"),
    tracks=cms.InputTag(
        "l1tTTTracksFromTrackletEmulation",
        "Level1TTTracks",
    ),
    decodedTracks=cms.InputTag(
        "serenityGTTStubUnpacker",
        "Level1TTTracks",
    ),
    algoBlocks=cms.InputTag("gttStubAlgoBlocks"),
    algorithmName=cms.string("L1_GTT_LowStubOccupancy"),
    regionLinks=cms.vuint32(0, 2, 4, 6, 8, 10, 12, 14, 16),
    metadataTrackSlot=cms.uint32(103),
    minStubCount=cms.uint32(0),
    maxStubCount=cms.uint32(589815),
    rejectOverflow=cms.bool(True),
    rejectTrackOverflow=cms.bool(True),
    strict=cms.bool(True),
)
