import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeAnalyzer.dplus3panalyzer_ntp_cfi import *

dplus3pana_wrongsign = dplus3pana.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generalDPlus3PCandidatesNewWrongSign:DPlus3P"),
  MVACollection = cms.InputTag("generalDPlus3PCandidatesNewWrongSign:MVAValues")
)

dplus3pana_wrongsign_mc = dplus3pana_mc.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generalDPlus3PCandidatesNewWrongSign:DPlus3P"),
  MVACollection = cms.InputTag("generalDPlus3PCandidatesNewWrongSign:MVAValues")
)

dplus3pana_tof = dplus3pana.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)

dplus3pana_tof_wrongsign = dplus3pana_wrongsign.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)

dplus3pana_tof_mc = dplus3pana_mc.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)

dplus3pana_tof_wrongsign_mc = dplus3pana_wrongsign_mc.clone(
  doGenMatchingTOF = cms.untracked.bool(True)
)
