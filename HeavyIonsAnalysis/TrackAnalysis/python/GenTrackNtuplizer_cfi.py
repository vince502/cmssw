import FWCore.ParameterSet.Config as cms

genTrackNtuplizer = cms.EDAnalyzer('GenTrackNtuplizer',
    # Use either genParticleSrc (for GenParticleCollection) or hepmcSrc (for HepMCProduct)
    # genParticleSrc = cms.InputTag("genParticles"),
    hepmcSrc = cms.InputTag("generatorSmeared", "", "SIM"),
    onlyStable = cms.bool(True),      # Only save stable particles (status == 1)
    onlyCharged = cms.bool(True),      # Only save charged particles
)
