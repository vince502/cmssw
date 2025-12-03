import FWCore.ParameterSet.Config as cms

bDiMuMuFlatProducer = cms.EDAnalyzer("BDiMuMuFlatProducer",
    # Input collections
    bPlusCollection = cms.InputTag("generalBDiMuMuCandidates:BPlus"),
    bZeroCollection = cms.InputTag("generalBDiMuMuCandidates:BZero"),
    bcCollection = cms.InputTag("generalBDiMuMuCandidates:Bc"),
    
    # Event-level collections
    vertexCollection = cms.InputTag("unpackedTracksAndVertices"),
    beamSpotCollection = cms.InputTag("offlineBeamSpot"),
    
    # Heavy ion specific (optional)
    centralityCollection = cms.InputTag("hiCentrality"),
    centralityBinCollection = cms.InputTag("centralityBin:HFtowers"),
    
    # Generator information (MC only)
    genParticleCollection = cms.InputTag("genParticles"),
    
    # Configuration options
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(False),  # Enable when Bc reconstruction is available
    isMC = cms.bool(False)
)