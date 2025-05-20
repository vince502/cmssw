import FWCore.ParameterSet.Config as cms

vtxProp = cms.EDAnalyzer('quickVertexAnalysis',
    hasReco = cms.untracked.bool(True),
    useGen = cms.untracked.bool(False),
    g4Label = cms.untracked.InputTag("g4SimHits"),
    hepMCLabel = cms.untracked.InputTag("generatorSmeared"),
    genEvtInfo = cms.untracked.InputTag("generator"),
    recoVtxLabel = cms.untracked.InputTag("offlineSlimmedPrimaryVertices"),
    pileupInfo = cms.InputTag("slimmedAddPileupInfo"),
)