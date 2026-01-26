import FWCore.ParameterSet.Config as cms

simpleTrackNtuplizer = cms.EDAnalyzer('SimpleTrackNtuplizer',
    trackSrc = cms.InputTag("generalTracks"),
)
