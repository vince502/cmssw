import FWCore.ParameterSet.Config as cms

# Base configuration for PATCompositeNtupleProducer
patCompositeNtupleProducerDefault = cms.EDAnalyzer('PATCompositeNtupleProducer',
    # Candidate type: D0, D04P, DStar, DStar5P, BPlus, BZero, Bc
    candidateType = cms.string('D0'),
    
    # Input collections
    beamSpotSrc = cms.InputTag('offlineBeamSpot'),
    vertexSrc = cms.InputTag('offlinePrimaryVertices'),
    candidateSrc = cms.InputTag('generalD0CandidatesNew', 'D0'),
    
    # Options (auto-configured based on candidateType, but can be overridden)
    twoLayerDecay = cms.untracked.bool(False),
    doMuon = cms.untracked.bool(False),
    doElectron = cms.untracked.bool(False),
    nDaughters = cms.untracked.uint32(2),
    nGrandDaughters = cms.untracked.uint32(0),
    
    # Centrality (for HI)
    isCentrality = cms.untracked.bool(False),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
    
    # Output
    saveTree = cms.untracked.bool(True),
)

# D0 analyzer (2-prong: K + pi)
d0NtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'D0',
    candidateSrc = cms.InputTag('generalD0CandidatesNew', 'D0'),
)

# D0 4-prong analyzer (K + 3pi)
d04pNtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'D04P',
    candidateSrc = cms.InputTag('generalD04PCandidatesNew', 'D04P'),
    nDaughters = cms.untracked.uint32(4),
)

# DStar analyzer (D0 + slow pi)
dStarNtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'DStar',
    candidateSrc = cms.InputTag('generalDStarCandidatesNew', 'DStar'),
    twoLayerDecay = cms.untracked.bool(True),
    nGrandDaughters = cms.untracked.uint32(2),
)

# DStar 5-prong analyzer (D0(4P) + slow pi)
dStar5pNtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'DStar5P',
    candidateSrc = cms.InputTag('generalDStar5PCandidatesNew', 'DStar5P'),
    twoLayerDecay = cms.untracked.bool(True),
    nGrandDaughters = cms.untracked.uint32(4),
)

# B+ analyzer (J/psi + K)
bPlusNtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'BPlus',
    candidateSrc = cms.InputTag('generalBPlusCandidates', 'BPlus'),
    twoLayerDecay = cms.untracked.bool(True),
    doMuon = cms.untracked.bool(True),
    nGrandDaughters = cms.untracked.uint32(2),
)

# Bc analyzer (J/psi + pi)
bcNtupleProducer = patCompositeNtupleProducerDefault.clone(
    candidateType = 'Bc',
    candidateSrc = cms.InputTag('generalBcCandidates', 'Bc'),
    twoLayerDecay = cms.untracked.bool(True),
    doMuon = cms.untracked.bool(True),
    nGrandDaughters = cms.untracked.uint32(2),
)
