import FWCore.ParameterSet.Config as cms


apolloGTTStubPacker = cms.EDProducer(
    "l1t::GTTStubPacker",
    counts=cms.InputTag("apolloGTTStubCounts"),
    tracks=cms.InputTag(
        "l1tTTTracksFromTrackletEmulation",
        "Level1TTTracks",
    ),
    regionLinks=cms.vuint32(0, 2, 4, 6, 8, 10, 12, 14, 16),
    metadataTrackSlot=cms.uint32(103),
    throwOnTrackSlotOccupied=cms.bool(True),
)
