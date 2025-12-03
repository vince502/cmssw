import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeAnalyzer.dStaranalyzer_tree_cfi import *

dStarana_wrongsign = dStarana.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generalDStarCandidatesNewWrongSign:D0")
  #MVACollection = cms.InputTag("generalDStarCandidatesNewWrongSign:MVAValues")
)

#dStarana_wrongsign_mc = dStarana_mc.clone(
#  VertexCompositeCollection = cms.untracked.InputTag("generalD0CandidatesNewWrongSign:D0"),
#  MVACollection = cms.InputTag("generalD0CandidatesNewWrongSign:MVAValues")
#)

dStarana_tof = dStarana.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)

#dStarana_tof_wrongsign = dStarana_wrongsign.clone(
#  doGenMatchingTOF = cms.untracked.bool(True)
#)

dStarana_tof_mc = dStarana_mc.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)

#dStarana_tof_wrongsign_mc = dStarana_wrongsign_mc.clone(
#  doGenMatchingTOF = cms.untracked.bool(True)
#)

