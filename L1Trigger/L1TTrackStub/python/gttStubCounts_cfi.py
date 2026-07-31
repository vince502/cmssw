import FWCore.ParameterSet.Config as cms


apolloGTTStubCounts = cms.EDProducer(
    "l1t::GTTStubCountProducer",
    rawStubs=cms.InputTag("TTStubsFromPhase2TrackerDigis", "StubAccepted"),
    acceptedStubs=cms.InputTag("ProducerDTC", "StubAccepted"),
    lostStubs=cms.InputTag("ProducerDTC", "StubLost"),
)
