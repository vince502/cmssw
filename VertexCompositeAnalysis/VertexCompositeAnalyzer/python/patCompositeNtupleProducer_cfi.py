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
    
    # Gen matching (MC only)
    genealogyInfo = cms.untracked.bool(False),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
    # Gen matching criteria (optional, uses optimized defaults if not specified)
    # genMatchDRMax: Maximum ΔR for candidate matching (default: 0.1)
    #   - D mesons: 0.05-0.1 recommended
    #   - B mesons: 0.1-0.2 recommended (wider due to larger decay length)
    # genMatchMassWindow: Maximum |Δm| in GeV (default: -1 = auto by type)
    #   - D0: 0.15 GeV (~3σ mass resolution)
    #   - D*: 0.20 GeV
    #   - B mesons: 0.30 GeV
    # genTrackMatchDRMax: Maximum ΔR for track matching (default: 0.03)
    #   - Typical: 0.03-0.05 for good track resolution
    # genTrackMatchPtRatio: Maximum pT ratio difference (default: 0.5)
    #   - Typical: 0.3-0.5 (30-50% pT difference allowed)
    genMatchDRMax = cms.untracked.double(0.1),
    genMatchMassWindow = cms.untracked.double(-1.0),  # -1 = auto by type
    genTrackMatchDRMax = cms.untracked.double(0.03),
    genTrackMatchPtRatio = cms.untracked.double(0.5),
    
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
