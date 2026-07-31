import FWCore.ParameterSet.Config as cms


gttStubDecisionFilter = cms.EDFilter(
    "l1t::GTTStubDecisionFilter",
    src=cms.InputTag("serenityGTTStubUnpacker"),
    minStubCount=cms.uint32(0),
    maxStubCount=cms.uint32(589815),
    rejectOverflow=cms.bool(True),
    rejectTrackOverflow=cms.bool(True),
)
