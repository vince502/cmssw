import FWCore.ParameterSet.Config as cms

# Analyzer to compare packedPFCandidates with unpackedTracksAndVertices
# Validates track unpacking by comparing kinematics
# Does NOT include lostTracks in the comparison

unpackedTrackComparison = cms.EDAnalyzer('UnpackedTrackComparisonAnalyzer',
    packedPFCandidates = cms.InputTag('packedPFCandidates'),
    unpackedTracks = cms.InputTag('unpackedTracksAndVertices'),
    unpackedVertices = cms.InputTag('unpackedTracksAndVertices'),
    slimmedVertices = cms.InputTag('offlineSlimmedPrimaryVertices'),
)
