import FWCore.ParameterSet.Config as cms

from HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff import (
    primaryVertexFilter,
    clusterCompatibilityFilter,
)
# Import only the modules, not the standalone diagnostic paths in hffilter_cfi.
from HeavyIonsAnalysis.EventAnalysis.hffilter_cfi import (
    phfCoincFilter2Th4,
    phfCoincFilter3Th5,
)
from HLTrigger.HLTfilters.hltHighLevel_cfi import hltHighLevel

# User-specified minimum-bias trigger selection. No silent acceptance when the
# requested trigger pattern is missing from a run's menu.
hltfilter = hltHighLevel.clone(
    TriggerResultsTag=cms.InputTag("TriggerResults", "", "HLT"),
    HLTPaths=cms.vstring("HLT_HIMinimumBias*"),
    andOr=cms.bool(True),
    throw=cms.bool(True),
)

# These are sequences, never independent paths: prepend the appropriate one
# to the reconstruction/tree path so every saved event passes the full AND.
eventFilters2023 = cms.Sequence(
    hltfilter * primaryVertexFilter * clusterCompatibilityFilter * phfCoincFilter2Th4
)
eventFilters2024 = cms.Sequence(
    hltfilter * primaryVertexFilter * clusterCompatibilityFilter * phfCoincFilter3Th5
)
eventFilters2025 = cms.Sequence(hltfilter * primaryVertexFilter * phfCoincFilter3Th5)
eventFilters2026 = cms.Sequence(hltfilter * primaryVertexFilter * phfCoincFilter3Th5)
