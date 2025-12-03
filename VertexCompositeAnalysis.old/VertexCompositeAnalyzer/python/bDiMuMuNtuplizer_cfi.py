import FWCore.ParameterSet.Config as cms

bDiMuMuNtuplizer = cms.EDAnalyzer("BDiMuMuNtuplizer",
    # Input collections
    bPlusCollection = cms.InputTag("generalBDiMuMuCandidates:BPlus"),
    bZeroCollection = cms.InputTag("generalBDiMuMuCandidates:BZero"),
    bcCollection = cms.InputTag("generalBDiMuMuCandidates:Bc"),
    
    # MVA collections (optional)
    bPlusMVACollection = cms.InputTag("generalBDiMuMuCandidates:MVAValuesBPlus"),
    bZeroMVACollection = cms.InputTag("generalBDiMuMuCandidates:MVAValuesBZero"),
    bcMVACollection = cms.InputTag("generalBDiMuMuCandidates:MVAValuesBc"),
    
    # Event-level collections
    vertexCollection = cms.InputTag("unpackedTracksAndVertices"),
    beamSpotCollection = cms.InputTag("offlineBeamSpot"),
    triggerResults = cms.InputTag("TriggerResults::HLT"),
    
    # Heavy ion specific (optional)
    centralityCollection = cms.InputTag("hiCentrality"),
    centralityBinCollection = cms.InputTag("centralityBin:HFtowers"),
    
    # Generator information (MC only)
    genParticleCollection = cms.InputTag("genParticles"),
    
    # Configuration options
    doGenMatching = cms.bool(False),
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(False),  # Enable when Bc reconstruction is available
    saveFullInfo = cms.bool(True),
    isMC = cms.bool(False),
    genMatchingDeltaR = cms.double(0.3)
)
