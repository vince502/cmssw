import FWCore.ParameterSet.Config as cms

genParticleSimpleAnalyzer = cms.EDAnalyzer('GenParticleSimpleAnalyzer',
    GenParticleCollection = cms.untracked.InputTag('genParticles'),
    # PDG IDs to print (empty = print all)
    # Common particles:
    #   D0: 421, D+: 411, Ds: 431
    #   D*+: 413, D*0: 423
    #   B+: 521, B0: 511, Bs: 531, Bc: 541
    #   J/psi: 443, Upsilon: 553
    pdgIDs = cms.untracked.vint32(421, 413, 521, 443),
)
