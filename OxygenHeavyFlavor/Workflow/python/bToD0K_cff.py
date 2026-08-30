import FWCore.ParameterSet.Config as cms


def addBToD0K(process, options, isMC, applyEventSelection):
    from VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi import (
        generalD0Candidates,
    )
    from VertexCompositeAnalysis.VertexCompositeProducer.bToD0K_cfi import bToD0K
    from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import (
        d0NtupleProducer,
        bToD0KNtupleProducer,
    )

    process.bToD0KD0Candidates = generalD0Candidates.clone(
        trackRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        vertexRecoAlgorithm=cms.InputTag("unpackedTracksAndVertices"),
        dedxSrc=cms.InputTag("dedxEstimator", "dedxAllLikelihood") if isMC else cms.InputTag(""),
        tkPtCut=cms.double(options.d0DaughterPtCut),
        tkEtaCut=cms.double(2.4),
        tkEtaDiffCut=cms.double(options.d0EtaGapCut),
        VtxChiProbCut=cms.double(0.01),
        d0MassCut=cms.double(0.15),
        dPtCut=cms.double(options.d0PtCut),
        useAnyMVA=cms.bool(True),
        mvaType=cms.string("TMVA"),
        tmvaWeightsFile=cms.string("TMVAClassification_BDT.weights.xml"),
        mvaCut=cms.double(options.d0MvaCut),
    )
    process.bToD0K = bToD0K.clone(
        d0Src=cms.InputTag("bToD0KD0Candidates", "D0"),
        kaonSrc=cms.InputTag("packedPFCandidates"),
        pvSrc=cms.InputTag("offlineSlimmedPrimaryVertices"),
        beamSpotSrc=cms.InputTag("offlineBeamSpot"),
        dedxSrc=cms.InputTag("dedxEstimator", "dedxAllLikelihood") if isMC else cms.InputTag(""),
        minKaonPt=cms.double(options.bachelorKaonPtCut),
    )

    commonTreeOptions = dict(
        isCentrality=cms.untracked.bool(False),
        genealogyInfo=cms.untracked.bool(isMC),
        genParticles=cms.untracked.InputTag("prunedGenParticles"),
    )
    process.bToD0KD0Tree = d0NtupleProducer.clone(
        candidateSrc=cms.InputTag("bToD0KD0Candidates", "D0"),
        vertexSrc=cms.InputTag("unpackedTracksAndVertices"),
        genMatchDRMax=cms.untracked.double(0.08),
        genMatchMassWindow=cms.untracked.double(0.15),
        **commonTreeOptions,
    )
    process.bToD0KTree = bToD0KNtupleProducer.clone(
        candidateSrc=cms.InputTag("bToD0K", "BToD0K"),
        vertexSrc=cms.InputTag("offlineSlimmedPrimaryVertices"),
        genMatchDRMax=cms.untracked.double(0.15),
        genMatchMassWindow=cms.untracked.double(0.30),
        **commonTreeOptions,
    )

    process.bToD0KD0CandidateFilterSequence = cms.Sequence()
    if options.minD0Candidates > 0:
        process.bToD0KD0CandidateFilter = cms.EDFilter(
            "CandViewCountFilter",
            src=cms.InputTag("bToD0KD0Candidates", "D0"),
            minNumber=cms.uint32(options.minD0Candidates),
        )
        process.bToD0KD0CandidateFilterSequence = cms.Sequence(
            process.bToD0KD0CandidateFilter
        )

    process.bToD0KCandidateFilterSequence = cms.Sequence()
    if options.minBToD0KCandidates > 0:
        process.bToD0KCandidateFilter = cms.EDFilter(
            "CandViewCountFilter",
            src=cms.InputTag("bToD0K", "BToD0K"),
            minNumber=cms.uint32(options.minBToD0KCandidates),
        )
        process.bToD0KCandidateFilterSequence = cms.Sequence(
            process.bToD0KCandidateFilter
        )

    eventFilter = process.oxygenEventFilter if applyEventSelection else cms.Sequence()
    process.bToD0KPath = cms.Path(
        eventFilter
        + process.unpackedTracksAndVertices
        + process.bToD0KD0Candidates
        + process.bToD0KD0CandidateFilterSequence
        + process.bToD0K
        + process.bToD0KCandidateFilterSequence
        + process.bToD0KD0Tree
        + process.bToD0KTree
    )
    return process.bToD0KPath
