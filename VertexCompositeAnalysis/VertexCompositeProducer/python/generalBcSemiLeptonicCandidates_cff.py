import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeProducer.generalBcSemiLeptonicCandidates_cfi import (
    generalBcSemiLeptonicCandidates,
    bcToJpsiMuMuEAndEEMuCandidates,
)

bcSemiLeptonicSequence = cms.Sequence(generalBcSemiLeptonicCandidates)
bcSemiLeptonicCrossSequence = cms.Sequence(bcToJpsiMuMuEAndEEMuCandidates)
