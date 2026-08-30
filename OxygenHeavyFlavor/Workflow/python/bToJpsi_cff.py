import FWCore.ParameterSet.Config as cms


def configureBToJpsi(process, isMC):
    """Build B candidates from the shared framework HiOnia collection."""
    from VertexCompositeAnalysis.VertexCompositeProducer.generalBDiLeptonCandidates_cfi import (
        generalBDiLeptonCandidates,
    )

    process.bToJpsiCandidates = generalBDiLeptonCandidates.clone(
        dileptonCollection=cms.InputTag("onia2MuMuPatGlbGlb"),
        trackRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        vertexRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        tkPtCut=cms.double(0.3),
        dileptonMassMin=cms.double(2.9),
        dileptonMassMax=cms.double(3.3),
        dileptonPtCut=cms.double(0.0),
        bPtCut=cms.double(0.0),
        bcPtCut=cms.double(0.0),
        doBPlus=cms.bool(True),
        doBZero=cms.bool(True),
        doBc=cms.bool(True),
    )

    from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import (
        bPlusNtupleProducer,
        bZeroNtupleProducer,
        bcNtupleProducer,
    )

    commonTreeOptions = dict(
        vertexSrc=cms.InputTag("unpackedTracksAndVertices"),
        genealogyInfo=cms.untracked.bool(isMC),
        genParticles=cms.untracked.InputTag("prunedGenParticles"),
    )
    process.bPlusToJpsiTree = bPlusNtupleProducer.clone(
        candidateSrc=cms.InputTag("bToJpsiCandidates", "BPlus"),
        **commonTreeOptions,
    )
    process.bZeroToJpsiTree = bZeroNtupleProducer.clone(
        candidateSrc=cms.InputTag("bToJpsiCandidates", "BZero"),
        **commonTreeOptions,
    )
    process.bcToJpsiTree = bcNtupleProducer.clone(
        candidateSrc=cms.InputTag("bToJpsiCandidates", "Bc"),
        **commonTreeOptions,
    )

    process.bToJpsiSequence = cms.Sequence(
        process.bToJpsiCandidates
        + process.bPlusToJpsiTree
        + process.bZeroToJpsiTree
        + process.bcToJpsiTree
    )
    return process.bToJpsiSequence
