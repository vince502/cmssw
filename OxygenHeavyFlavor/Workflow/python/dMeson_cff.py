import FWCore.ParameterSet.Config as cms


def addDMeson(process, isMC, applyEventSelection):
    from VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi import (
        generalD0Candidates,
    )
    from VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cfi import (
        generalDStarCandidates,
    )
    from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import (
        d0NtupleProducer,
        dStarNtupleProducer,
    )

    if isMC:
        d0Settings = dict(
            tkPtCut=cms.double(0.8),
            tkEtaDiffCut=cms.double(1.0),
            mPiKCutMin=cms.double(1.72),
            mPiKCutMax=cms.double(2.01),
            d0MassCut=cms.double(0.14),
            dPtCut=cms.double(1.0),
            useAnyMVA=cms.bool(False),
        )
    else:
        d0Settings = dict(
            tkPtCut=cms.double(0.3),
            tkEtaDiffCut=cms.double(1.6),
            mPiKCutMin=cms.double(1.32),
            mPiKCutMax=cms.double(2.41),
            d0MassCut=cms.double(0.2),
            dPtCut=cms.double(0.0),
            useAnyMVA=cms.bool(True),
            onnxModelFileName=cms.string("BDT_XGB_19Dec2025.onnx"),
            input_names=cms.vstring("float_input"),
            output_names=cms.vstring("label", "probabilities"),
            mvaCut=cms.double(0.0),
        )

    process.inclusiveD0Candidates = generalD0Candidates.clone(
        trackRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        vertexRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        dedxSrc=cms.InputTag("dedxEstimator", "dedxAllLikelihood"),
        tkEtaCut=cms.double(2.4),
        tkNhitsCut=cms.int32(5),
        tkChi2Cut=cms.double(5.0 if isMC else 3.0),
        tkPtErrCut=cms.double(0.1),
        tkDCACut=cms.double(1.0),
        d0AbsYCut=cms.double(1.6),
        VtxChiProbCut=cms.double(0.01),
        dauTransImpactSigCut=cms.double(0.0),
        dauLongImpactSigCut=cms.double(0.0),
        vtxSignificance2DCut=cms.double(0.0),
        vtxSignificance3DCut=cms.double(0.0),
        alphaCut=cms.double(0.4),
        alpha2DCut=cms.double(0.4),
        isWrongSign=cms.bool(False),
        **d0Settings,
    )
    process.inclusiveDStarCandidates = generalDStarCandidates.clone(
        d0Collection=cms.InputTag("inclusiveD0Candidates", "D0"),
        trackRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        vertexRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        dedxSrc=cms.InputTag("dedxEstimator", "dedxAllLikelihood"),
        tkPtCut=cms.double(0.10),
        tkEtaCut=cms.double(2.4),
        tkNhitsCut=cms.int32(0),
        dStarMassCut=cms.double(0.30),
        dPtCut=cms.double(0.0),
        VtxChiProbCut=cms.double(0.001),
        isWrongSign=cms.bool(False),
    )

    treeOptions = dict(
        vertexSrc=cms.InputTag("unpackedTracksAndVertices"),
        isCentrality=cms.untracked.bool(False),
        genealogyInfo=cms.untracked.bool(isMC),
        genParticles=cms.untracked.InputTag("prunedGenParticles"),
    )
    process.inclusiveD0Tree = d0NtupleProducer.clone(
        candidateSrc=cms.InputTag("inclusiveD0Candidates", "D0"),
        genMatchDRMax=cms.untracked.double(0.10),
        genTrackMatchDRMax=cms.untracked.double(0.03),
        genTrackMatchPtRatio=cms.untracked.double(0.5),
        **treeOptions,
    )
    process.inclusiveDStarTree = dStarNtupleProducer.clone(
        candidateSrc=cms.InputTag("inclusiveDStarCandidates", "DStar"),
        genMatchDRMax=cms.untracked.double(0.10),
        genTrackMatchDRMax=cms.untracked.double(0.03),
        genTrackMatchPtRatio=cms.untracked.double(0.5),
        **treeOptions,
    )

    eventFilter = process.oxygenEventFilter if applyEventSelection else cms.Sequence()
    process.dMesonPath = cms.Path(
        eventFilter
        + process.unpackedTracksAndVertices
        + process.inclusiveD0Candidates
        + process.inclusiveD0Tree
        + process.inclusiveDStarCandidates
        + process.inclusiveDStarTree
    )
    return process.dMesonPath
