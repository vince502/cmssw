import FWCore.ParameterSet.Config as cms

from L1Trigger.L1TTrackStub.gttStubCounts_cfi import apolloGTTStubCounts
from L1Trigger.L1TTrackStub.gttStubDecisionFilter_cfi import (
    gttStubDecisionFilter,
)
from L1Trigger.L1TTrackStub.gttStubPacker_cfi import apolloGTTStubPacker
from L1Trigger.L1TTrackStub.gttStubUnpacker_cfi import (
    serenityGTTStubUnpacker,
)
from L1Trigger.L1TTrackStub.gttStubValidation_cfi import (
    gttStubValidation as _gttStubValidation,
)
from L1Trigger.Phase2L1GT.l1GTAcceptFilter_cff import l1tGTAcceptFilter
from L1Trigger.Phase2L1GT.l1tGTAlgoBlockProducer_cff import (
    l1tGTAlgoBlockProducer,
)


# The Apollo-side packer reuses the standard 18-link GTT track codec and places
# one tagged 64-bit count word in reserved 96-bit track slot 103 per region.
apolloGTTStubSequence = cms.Sequence(
    apolloGTTStubCounts
    + apolloGTTStubPacker
)

# This producer is the Serenity/GTT receiver boundary. Downstream modules see
# only the decoded primitive, never the original TTStub or TTDTC collections.
serenityGTTStubSequence = cms.Sequence(serenityGTTStubUnpacker)

gttStubEmulation = cms.Sequence(
    apolloGTTStubSequence
    + serenityGTTStubSequence
)

# The decision runs as an ordinary trigger path so the standard Phase-2 GT
# algorithm-block producer can apply menu logic, masks, and prescales. The
# threshold is a demonstrator default and must be tuned for each menu.
gttStubLowOccupancyDecision = gttStubDecisionFilter.clone(
    maxStubCount=500,
)
gttStubLowOccupancyPath = cms.Path(gttStubLowOccupancyDecision)

gttStubValidation = _gttStubValidation.clone(
    maxStubCount=500,
)

gttStubAlgoBlocks = l1tGTAlgoBlockProducer.clone(
    algorithms=cms.VPSet(
        cms.PSet(
            name=cms.string("L1_GTT_LowStubOccupancy"),
            expression=cms.string("gttStubLowOccupancyPath"),
            triggerTypes=cms.vint32(1),
        )
    )
)
gttStubAlgoBlockPath = cms.Path(gttStubAlgoBlocks)

gttStubAccept = l1tGTAcceptFilter.clone(
    algoBlocksTag=cms.InputTag("gttStubAlgoBlocks"),
)
gttStubAcceptPath = cms.Path(gttStubAccept)

gttStubValidationEndPath = cms.EndPath(gttStubValidation)

gttStubEmulationEventContent = cms.PSet(
    outputCommands=cms.untracked.vstring(
        "keep *_apolloGTTStubCounts_*_*",
        "keep *_apolloGTTStubPacker_*_*",
        "keep *_serenityGTTStubUnpacker_*_*",
        "keep *_gttStubAlgoBlocks_*_*",
    )
)
