### D meson DATA: D0(2P), D0(4P), D*(3P), D*(5P), DDbar combination
# Usage: cmsRun testDmeson_DATA_cfg.py maxEvents=10000
#
# For OO 2025 data with:
#   - D0 -> K pi (2-prong)
#   - D0 -> K 3pi (4-prong)
#   - D* -> D0 pi (3-prong)
#   - D* -> D0(4P) pi (5-prong)
#
# All paths share the same eventFilter to ensure identical event selection

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Process
#----------------------------------------------------------------------------
process = cms.Process("DmesonDATA", eras.Run3_2024_ppRef)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "Dmeson_ppRef_DATA_val.root"
options.inputFiles = [
#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/506/00000/1dd00df3-7bfa-44cf-a7f8-fe6560f58731.root',
#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/528/00000/9f1edbff-c597-449f-aca5-346e2456052c.root',
#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/640/00000/31302378-f489-4659-a3d1-fb03bbcd70b0.root',
#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/711/00000/f9f2afeb-2c4d-450c-99d3-a42e7a787cce.root',
#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/721/00000/b4326b21-314f-4875-97bd-a982c7a92b1d.root',

#'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/474/00000/39ec3ca5-2225-4aca-8973-e91c1f3adb84.root',
'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/570/00000/8cfe8549-e981-4628-a139-ce410a6fdc37.root',
'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/670/00000/0790be20-7e89-44d4-aaa0-3053c5e088cc.root',
'/store/data/Run2024J/PPRefZeroBiasPlusForward7/MINIAOD/PromptReco-v1/000/387/721/00000/2d156a3c-5131-4933-97e5-2de2f7fe9da3.root'
]
options.maxEvents = 700000
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
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')

#----------------------------------------------------------------------------
# Event Selection Filters (shared by all paths)
#----------------------------------------------------------------------------
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)

#----------------------------------------------------------------------------
# Track unpacking for MiniAOD
#----------------------------------------------------------------------------
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")

# Track analyzer configuration
process.ppTracks.doTrack = cms.untracked.bool(True)
process.ppTracks.trackPtMin = cms.untracked.double(0.01)
process.ppTracks.trackEtaMax = cms.untracked.double(4.0)
process.ppTracks.applyTrackSelections = cms.untracked.bool(False)
process.ppTracks.vertexSrc = cms.InputTag("unpackedTracksAndVertices")
process.ppTracks.trackSrc = cms.InputTag("unpackedTracksAndVertices")
process.ppTracks.beamSpotSrc = cms.untracked.InputTag('offlineBeamSpot')
process.ppTracks.dedxEstimators = cms.VInputTag()

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
    tkPtCut = cms.double(0.3),
    tkEtaCut = cms.double(2.4),
    tkNhitsCut = cms.int32(5),
    tkChi2Cut = cms.double(5.0),
    tkPtErrCut = cms.double(0.1),
    
    # D0 cuts
    tkEtaDiffCut = cms.double (999),
    mPiKCutMin = cms.double(1.72),
    mPiKCutMax = cms.double(2.01),
    tkDCACut  = cms.double(1.0),
    d0MassCut = cms.double(0.14),
    dPtCut = cms.double(0.0),
    d0AbsYCut = cms.double(1.6),
    
    # Vertex cuts
    VtxChiProbCut = cms.double(0.01),
    dauTransImpactSigCut = cms.double(0.0),
    dauLongImpactSigCut = cms.double(0.0),
    vtxSignificance2DCut = cms.double(0.0),
    vtxSignificance3DCut = cms.double(0.0),
    alphaCut = cms.double (1.0),
    alpha2DCut = cms.double (1.0),
    
    isWrongSign = cms.bool(False),
    useAnyMVA = cms.bool(True),
    #onnxModelFileName = cms.string('BDT_ppRefPromptD0_PU_XGB_30Dec2025.onnx'),
    onnxModelFileName = cms.string('BDT_ppRefPromptD0_PU_XGB_05Jan2026.onnx'),
    label_MVA = cms.string('ppref_pr'),
    input_names  = cms.vstring('float_input'),
    output_names = cms.vstring('label', 'probabilities'),
    mvaCut = cms.double(0.96),
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
)

# D0 (2P) wrong sign tree
process.d0TreeWS = d0NtupleProducer.clone(
    candidateSrc = cms.InputTag('d0CandidatesWS', 'D0'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
)

# D0 (4P) tree
process.d04pTree = d04pNtupleProducer.clone(
    candidateSrc = cms.InputTag('d04pCandidates', 'D04P'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
)

# D* (3P) tree
process.dStarTree = dStarNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStarCandidates', 'DStar'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
)

# D* (3P) wrong sign tree
process.dStarTreeWS = dStarNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStarCandidatesWS', 'DStar'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
)

# D* (5P) tree
process.dStar5pTree = dStar5pNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStar5pCandidates', 'DStar5P'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(False),
)
#----------------------------------------------------------------------------
# CandViewCount event filter
#----------------------------------------------------------------------------
#from CommonTools.CandAlgos.
process.d0candCountFilter = cms.EDFilter("CandViewCountFilter", 
    src = cms.InputTag("d0Candidates", "D0"),
    minNumber = cms.uint32(1),
)

#----------------------------------------------------------------------------
# Paths with event filter
#----------------------------------------------------------------------------
# D0 (2P) path
process.d0Path = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.d0Candidates +
    process.d0Tree
)

# D0 (2P) wrong sign path
process.d0PathWS = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.d0CandidatesWS +
    process.d0TreeWS
)

# D0 (4P) path
process.d04pPath = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.d04pCandidates +
    process.d04pTree
)

# D* (3P) path - depends on D0
process.dStarPath = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    #process.trackSequencePP +
    process.d0Candidates +
    process.d0candCountFilter + 
    process.d0Tree +
    process.dStarCandidates +
    process.dStarTree
)

# D* (3P) wrong sign path
process.dStarPathWS = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.d0CandidatesWS +
    process.dStarCandidatesWS +
    process.dStarTreeWS
)

# D* (5P) path - depends on D0(4P)
process.dStar5pPath = cms.Path(
    process.eventFilter +
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

import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/Cert_Collisions2024_ppref_387474_387721_golden.json').getVLuminosityBlockRange()

#----------------------------------------------------------------------------
# Output trees:
#   - d0Tree/VertexCompositeNtuple     : D0 -> K pi (2-prong)
#   - d0TreeWS/VertexCompositeNtuple   : D0 wrong sign background
#   - d04pTree/VertexCompositeNtuple   : D0 -> K 3pi (4-prong)
#   - dStarTree/VertexCompositeNtuple  : D* -> D0 pi (3-prong)
#   - dStarTreeWS/VertexCompositeNtuple: D* wrong sign background
#   - dStar5pTree/VertexCompositeNtuple: D* -> D0(4P) pi (5-prong)
#
# All trees share same events (via eventFilter)
#----------------------------------------------------------------------------
