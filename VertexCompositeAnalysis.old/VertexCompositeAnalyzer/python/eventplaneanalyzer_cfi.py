import FWCore.ParameterSet.Config as cms

eventplane=cms.EDAnalyzer('PATEventPlaneTrack',
        doRecoNtuple=cms.untracked.bool(True),
        saveTree=cms.untracked.bool(True),
        saveHistogram=cms.untracked.bool(True),
        beamSpotSrc = cms.untracked.InputTag("offlineBeamSpot"),
        VertexCollection=cms.untracked.InputTag("offlinePrimaryVertices"),
        VertexCompositeCollection = cms.untracked.InputTag("generalD0CandidatesNew:D0"),
        TrackCollection = cms.untracked.InputTag("generalTracks"),

        isCentrality = cms.bool(False),
        centralityBinLabel = cms.InputTag("centralityBin","HFtowers"),
        centralitySrc = cms.InputTag("hiCentrality")
        
        )




