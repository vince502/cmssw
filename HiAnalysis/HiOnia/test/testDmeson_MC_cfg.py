### D meson MC: D0(2P), D0(4P), D*(3P), D*(5P), DDbar combination
# Usage: cmsRun testDmeson_MC_cfg.py maxEvents=100
#
# For OO MC sample with:
#   - D0 -> K pi (2-prong)
#   - D0 -> K 3pi (4-prong)
#   - D* -> D0 pi (3-prong)
#   - D* -> D0(4P) pi (5-prong)
#
# Looser cuts for MC study

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Process
#----------------------------------------------------------------------------
process = cms.Process("DmesonMC", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "Dmeson_MC.root"
options.inputFiles = [
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_860.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_363.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_428.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_595.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_674.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_30.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_967.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_200.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_287.root',
'/store/user/junseok/Genproduction/RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/DStarKpipi/crab_RECO_MC_DStarKpipi_Prompt_wokinematicCut_ForceD0toKpi_CMSSW_14_1_7_15Sep25_v1/250915_044457/0000/minbias_RECO_837.root',
    'file:minbias_RECO_385.root',
]
options.maxEvents = -1
options.parseArguments()

#----------------------------------------------------------------------------
# Load standard sequences
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_mcRun3_2025_forOO_realistic_v7', '')

#----------------------------------------------------------------------------
# Track unpacking for MiniAOD
#----------------------------------------------------------------------------
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")

#----------------------------------------------------------------------------
# D0 -> K pi (2-prong)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi import generalD0Candidates

process.d0Candidates = generalD0Candidates.clone(
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    
    # dEdx source (stored in MiniAOD from RECO)
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
    
    # Track cuts
    tkPtCut = cms.double(0.8),
    tkEtaCut = cms.double(2.4),
    tkNhitsCut = cms.int32(5),
    tkChi2Cut = cms.double(5.0),
    tkPtErrCut = cms.double(0.1),
    
    # D0 cuts
    tkEtaDiffCut = cms.double (1.00),
    mPiKCutMin = cms.double(1.72),
    mPiKCutMax = cms.double(2.01),
    tkDCACut  = cms.double(1.0),
    d0MassCut = cms.double(0.14),
    dPtCut = cms.double(1.0),
    d0AbsYCut = cms.double(1.6),
    
    # Vertex cuts
    VtxChiProbCut = cms.double(0.01),
    dauTransImpactSigCut = cms.double(0.0),
    dauLongImpactSigCut = cms.double(0.0),
    vtxSignificance2DCut = cms.double(0.0),
    vtxSignificance3DCut = cms.double(0.0),
    alphaCut = cms.double (0.4),
    alpha2DCut = cms.double (0.4),
    
    isWrongSign = cms.bool(False),
)

# Wrong sign for background
process.d0CandidatesWS = process.d0Candidates.clone(
    isWrongSign = cms.bool(True),
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
)

#----------------------------------------------------------------------------
# D0 -> K 3pi (4-prong)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.generalD04PCandidates_cfi import generalD04PCandidates

process.d04pCandidates = generalD04PCandidates.clone(
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    
    # dEdx source (stored in MiniAOD from RECO)
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
    
    # Track cuts
    tkPtCut = cms.double(1.0),
    tkEtaCut = cms.double(2.4),
    tkNhitsCut = cms.int32(0),
    tkChi2Cut = cms.double(5.0),
    tkPtErrCut = cms.double(0.1),
    
    # D0 cuts
    mPiKCutMin = cms.double(1.72),
    mPiKCutMax = cms.double(2.01),
    d0MassCut = cms.double(0.15),
    dPtCut = cms.double(0.0),
    
    # Vertex cuts
    VtxChiProbCut = cms.double(0.001),
    dauTransImpactSigCut = cms.double(0.0),
    dauLongImpactSigCut = cms.double(0.0),
    vtxSignificance2DCut = cms.double(0.0),
    alpha2DCut = cms.double (0.2),
    tkEtaDiffCut = cms.double (1.0),
    
    isWrongSign = cms.bool(False),
)

#----------------------------------------------------------------------------
# D* -> D0 pi (3-prong)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cfi import generalDStarCandidates

process.dStarCandidates = generalDStarCandidates.clone(
    d0Collection = cms.InputTag('d0Candidates', 'D0'),
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    
    # dEdx source (stored in MiniAOD from RECO)
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
    
    # Soft pion cuts
    tkPtCut = cms.double(0.20),
    tkEtaCut = cms.double(2.4),
    tkNhitsCut = cms.int32(0),
    
    # D* cuts
    dStarMassCut = cms.double(0.20),  # |m(D*) - m(D0) - 0.1454| < 0.20
    dPtCut = cms.double(1.0),
    
    # Vertex cuts
    VtxChiProbCut = cms.double(0.001),
    
    isWrongSign = cms.bool(False),
)

# Wrong sign for background
process.dStarCandidatesWS = process.dStarCandidates.clone(
    d0Collection = cms.InputTag('d0CandidatesWS', 'D0'),
    isWrongSign = cms.bool(True),
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
)

#----------------------------------------------------------------------------
# D* -> D0(4P) pi (5-prong)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.generalDStar5PCandidates_cfi import generalDStar5PCandidates

process.dStar5pCandidates = generalDStar5PCandidates.clone(
    d0Collection = cms.InputTag('d04pCandidates', 'D04P'),
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    
    # dEdx source (stored in MiniAOD from RECO)
    dedxSrc = cms.InputTag('dedxEstimator', 'dedxAllLikelihood'),
    
    # Soft pion cuts
    tkPtCut = cms.double(0.20),
    tkEtaCut = cms.double(2.4),
    tkNhitsCut = cms.int32(0),
    
    # D* cuts
    dStarMassCut = cms.double(0.20),
    dPtCut = cms.double(1.0),
    
    # Vertex cuts
    VtxChiProbCut = cms.double(0.001),
    
    isWrongSign = cms.bool(False),
)

#----------------------------------------------------------------------------
# Ntuplizers
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import (
    d0NtupleProducer, d04pNtupleProducer, dStarNtupleProducer, dStar5pNtupleProducer
)

# D0 (2P) tree
process.d0Tree = d0NtupleProducer.clone(
    candidateSrc = cms.InputTag('d0Candidates', 'D0'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),  # Enable gen matching for MC
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
    genMatchDRMax = cms.untracked.double(0.10),
    genTrackMatchDRMax = cms.untracked.double(0.03),
    genTrackMatchPtRatio = cms.untracked.double(0.5),
)

# D0 (2P) wrong sign tree
process.d0TreeWS = d0NtupleProducer.clone(
    candidateSrc = cms.InputTag('d0CandidatesWS', 'D0'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
    genMatchDRMax = cms.untracked.double(0.10),
    genTrackMatchDRMax = cms.untracked.double(0.03),
    genTrackMatchPtRatio = cms.untracked.double(0.5),
)

# D0 (4P) tree
process.d04pTree = d04pNtupleProducer.clone(
    candidateSrc = cms.InputTag('d04pCandidates', 'D04P'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
)

# D* (3P) tree
process.dStarTree = dStarNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStarCandidates', 'DStar'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
    genMatchDRMax = cms.untracked.double(0.10),
    genTrackMatchDRMax = cms.untracked.double(0.03),
    genTrackMatchPtRatio = cms.untracked.double(0.5),
)

# D* (3P) wrong sign tree
process.dStarTreeWS = dStarNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStarCandidatesWS', 'DStar'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
    genMatchDRMax = cms.untracked.double(0.10),
    genTrackMatchDRMax = cms.untracked.double(0.03),
    genTrackMatchPtRatio = cms.untracked.double(0.5),
)

# D* (5P) tree
process.dStar5pTree = dStar5pNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStar5pCandidates', 'DStar5P'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
)

#----------------------------------------------------------------------------
# Paths with event filter
#----------------------------------------------------------------------------
# D0 (2P) path
process.d0Path = cms.Path(
    process.unpackedTracksAndVertices +
    process.d0Candidates +
    process.d0Tree
)

# D0 (2P) wrong sign path
process.d0PathWS = cms.Path(
    process.unpackedTracksAndVertices +
    process.d0CandidatesWS +
    process.d0TreeWS
)

# D0 (4P) path
process.d04pPath = cms.Path(
    process.unpackedTracksAndVertices +
    process.d04pCandidates +
    process.d04pTree
)

# D* (3P) path - depends on D0
process.dStarPath = cms.Path(
    process.unpackedTracksAndVertices +
    process.d0Candidates +
    process.d0Tree +
    process.dStarCandidates +
    process.dStarTree
)

# D* (3P) wrong sign path
process.dStarPathWS = cms.Path(
    process.unpackedTracksAndVertices +
    process.d0CandidatesWS +
    process.dStarCandidatesWS +
    process.dStarTreeWS
)

# D* (5P) path - depends on D0(4P)
process.dStar5pPath = cms.Path(
    process.unpackedTracksAndVertices +
    process.d04pCandidates +
    process.dStar5pCandidates +
    process.dStar5pTree
)

#----------------------------------------------------------------------------
# Input/Output
#----------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    numberOfThreads = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(1),
)

#----------------------------------------------------------------------------
# Schedule
#----------------------------------------------------------------------------
process.schedule = cms.Schedule(
#    process.d0Path,
#    process.d0PathWS,
#    process.d04pPath,
    process.dStarPath,
#    process.dStarPathWS,
#    process.dStar5pPath,
)

#----------------------------------------------------------------------------
# Output trees:
#   - d0Tree/VertexCompositeNtuple     : D0 -> K pi (2-prong)
#   - d0TreeWS/VertexCompositeNtuple   : D0 wrong sign background
#   - d04pTree/VertexCompositeNtuple   : D0 -> K 3pi (4-prong)
#   - dStarTree/VertexCompositeNtuple  : D* -> D0 pi (3-prong)
#   - dStarTreeWS/VertexCompositeNtuple: D* wrong sign background
#   - dStar5pTree/VertexCompositeNtuple: D* -> D0(4P) pi (5-prong)
#
#----------------------------------------------------------------------------
