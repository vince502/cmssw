import FWCore.ParameterSet.Config as cms

genAna = cms.EDAnalyzer('GenParticleSimpleAnalyzer',
    GenParticleCollection = cms.untracked.InputTag("genParticles"),
    pdgIDs = cms.untracked.vint32([413, -413])
)