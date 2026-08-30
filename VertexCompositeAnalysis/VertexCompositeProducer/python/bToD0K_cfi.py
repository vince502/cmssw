import FWCore.ParameterSet.Config as cms

bToD0K = cms.EDProducer("BToD0KProducer",
    # Input collections
    d0Src = cms.InputTag("generalD0Candidates", "D0"),
    kaonSrc = cms.InputTag("packedPFCandidates"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    dedxSrc = cms.InputTag(""),

    # Bachelor kaon selection
    minKaonPt = cms.double(0.8),
    maxKaonEta = cms.double(2.4),
    minKaonNhits = cms.int32(3),
    requireHighPurity = cms.bool(True),

    # D0 preselection
    minD0Pt = cms.double(0.0),
    maxD0AbsY = cms.double(2.4),
    d0MassMin = cms.double(1.72),
    d0MassMax = cms.double(2.01),
    requireD0PdgId = cms.bool(True),
    isWrongSign = cms.bool(False),

    # B selection
    minBPt = cms.double(5.0),
    maxBAbsY = cms.double(2.4),
    bMassMin = cms.double(4.57934),
    bMassMax = cms.double(5.97934),
    minVtxProb = cms.double(0.001),

    # Options
    doVertexFit = cms.bool(True),
    maxDeltaR_overlap = cms.double(0.01),
)

bToD0KWrongSign = bToD0K.clone(
    isWrongSign = cms.bool(True)
)

bToD0KFromD0New = bToD0K.clone(
    d0Src = cms.InputTag("generalD0CandidatesNew", "D0")
)
