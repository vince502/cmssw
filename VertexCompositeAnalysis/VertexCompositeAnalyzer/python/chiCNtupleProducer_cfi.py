import FWCore.ParameterSet.Config as cms

# Chi_c ntuple producer
# Reads chi_c candidates from OniaPhotonProducer and writes to TTree
# Includes MC generator-level chi_c -> J/psi + gamma information
# Can also store raw reco::Conversion collection
chiCNtupleProducer = cms.EDAnalyzer('ChiCNtupleProducer',
    candidateSrc = cms.InputTag("chiCandidatesMuMu"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    
    # Centrality (for HI)
    isCentrality = cms.untracked.bool(False),
    centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
    centralitySrc = cms.InputTag("hiCentrality"),
    
    # MC generator info
    isMC = cms.untracked.bool(False),
    genParticles = cms.InputTag("prunedGenParticles"),
    
    # Reco conversions (photon -> e+e-)
    storeConversions = cms.untracked.bool(False),
    conversions = cms.InputTag("reducedEgamma", "reducedConversions"),
    
    # Tree name
    treeName = cms.untracked.string("chiTree"),
)

# Chi_c from mu+mu-
chiCMuMuNtupleProducer = chiCNtupleProducer.clone(
    candidateSrc = cms.InputTag("chiCandidatesMuMu"),
)

# Chi_c from e+e-
chiCEENtupleProducer = chiCNtupleProducer.clone(
    candidateSrc = cms.InputTag("chiCandidatesEE"),
)
