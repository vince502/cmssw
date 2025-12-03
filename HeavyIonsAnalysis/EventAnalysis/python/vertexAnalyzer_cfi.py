import FWCore.ParameterSet.Config as cms

vertexAnalyzer = cms.EDAnalyzer('VertexAnalyzer',
    vertexSrc = cms.InputTag('offlineSlimmedPrimaryVertices'),
    beamSpotSrc = cms.InputTag('offlineBeamSpot'),
)

# For unpacked vertices (from TrackAndVertexUnpacker)
vertexAnalyzerUnpacked = cms.EDAnalyzer('VertexAnalyzer',
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    beamSpotSrc = cms.InputTag('offlineBeamSpot'),
)
