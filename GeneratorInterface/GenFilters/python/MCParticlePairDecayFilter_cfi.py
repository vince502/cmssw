import FWCore.ParameterSet.Config as cms

_defaultDecayNode = cms.PSet(
    ParticleIDs = cms.vint32(),
    Status = cms.int32(0),
    MinPt = cms.double(0.0),
    MaxPt = cms.double(1.0e9),
    MinY = cms.double(-1.0e9),
    MaxY = cms.double(1.0e9),
    Daughters = cms.VPSet(),
    AllowAdditionalDaughters = cms.bool(True),
    AllowedAdditionalDaughterIDs = cms.vint32(),
)

mcParticlePairDecayFilter = cms.EDFilter(
    "MCParticlePairDecayFilter",
    src = cms.InputTag("generator", "unsmeared"),
    Leg1 = _defaultDecayNode,
    Leg2 = _defaultDecayNode,
    MinAccepted1 = cms.int32(1),
    MaxAccepted1 = cms.int32(-1),
    MinAccepted2 = cms.int32(1),
    MaxAccepted2 = cms.int32(-1),
    MinPairCount = cms.int32(1),
    MaxPairCount = cms.int32(-1),
)
