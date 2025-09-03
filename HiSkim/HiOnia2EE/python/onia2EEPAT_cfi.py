import FWCore.ParameterSet.Config as cms

onia2ElectronElectronPAT = cms.EDProducer('HiOnia2EEPAT',
    electrons = cms.InputTag("patElectrons"),
    beamSpotTag = cms.InputTag("offlineBeamSpot"),
    primaryVertexTag = cms.InputTag("offlinePrimaryVertices"),
    srcTracks = cms.InputTag("generalTracks"),
    genParticles = cms.InputTag("genParticles"),
    conversions = cms.InputTag("conversions"),
    
    # daughter electron selection
    higherPuritySelection = cms.string("pt > 3.5 && abs(eta) < 2.4"),
    lowerPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4"),
    
    # dielectron selection
    dielectronSelection = cms.string("mass > 2.5 && mass < 4.0"),
    
    # trielectron selection (optional)
    doTriElectrons = cms.bool(False),
    trielectronSelection = cms.string(""),
    
    # dielectron+track selection (optional)
    DiElectronTrk = cms.bool(False),
    DiEleTrkSelection = cms.string(""),
    
    # late selections (optional)
    LateDiElectronSel = cms.string(""),
    LateDiEleTrkSel = cms.string(""),
    LateTriElectronSel = cms.string(""),
    
    # options
    addCommonVertex = cms.bool(True),
    addElectronlessPrimaryVertex = cms.bool(False),
    resolvePileUpAmbiguity = cms.bool(True),
    onlyGoodElectrons = cms.bool(False),
    onlySingleElectrons = cms.bool(False),
    flipJpsiDirection = cms.int32(0),
    
    # track-related parameters for dielectron+track (if enabled)
    particleType = cms.int32(211),  # pion
    trackMass = cms.double(0.13957018)  # pion mass
)