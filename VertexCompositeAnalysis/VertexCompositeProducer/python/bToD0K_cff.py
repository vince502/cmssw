import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeProducer.bToD0K_cfi import bToD0K
from VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi import generalD0Candidates

bToD0KSequence = cms.Sequence(bToD0K)
bToD0KRecoSequence = cms.Sequence(generalD0Candidates + bToD0K)
