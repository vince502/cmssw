### Chi_c + X(3872) MC: onia2MuMuPAT + hionia + chi_c + X(3872) producers
# Usage: cmsRun testChiX_MC_cfg.py maxEvents=100
#
# For OO MC sample with:
#   - chi_c -> J/psi(mumu) + gamma (photon conversion: e+e- from general tracks)
#   - X(3872) -> J/psi(mumu) + pi+ pi-
#
# All paths share the same event selection

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Settings
#----------------------------------------------------------------------------
HLTProcess     = "HLT"
isMC           = True
muonSelection  = "GlbOrTrk"
OnlySoftMuons  = False
applyCuts      = False
atLeastOneCand = False
miniAOD        = True
pdgId          = 443  # J/Psi
useMomFormat   = "vector"

#----------------------------------------------------------------------------
# Process
#----------------------------------------------------------------------------
process = cms.Process("ChiXMC", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "ChiX_MC.root"
options.inputFiles = [
    'file:/afs/cern.ch/work/s/soohwan/private/Analysis/MC/OxygenMC/CMSSW_15_0_11/src/RAW2DIGI_L1Reco_RECO_PAT.root',
]
options.maxEvents = -1
options.parseArguments()

# Trigger list (empty for MC)
triggerList = {
    'DoubleMuonTrigger': cms.vstring(),
    'SingleMuonTrigger': cms.vstring(),
}

#----------------------------------------------------------------------------
# Load standard sequences
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_mcRun3_2025_forOO_realistic_v7', '')

#----------------------------------------------------------------------------
# TransientTrack Builder (needed for X vertex fitting)
#----------------------------------------------------------------------------
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

#----------------------------------------------------------------------------
# Track unpacking for MiniAOD (general tracks from packedPFCandidates)
#----------------------------------------------------------------------------
process.load("HeavyIonsAnalysis.TrackAnalysis.unpackedTracksAndVertices_cfi")

#----------------------------------------------------------------------------
# Onia Tree Analyzer
#----------------------------------------------------------------------------
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
    muonTriggerList=triggerList,
    muonSelection=muonSelection,
    L1Stage=2,
    isMC=isMC,
    pdgID=pdgId,
    outputFileName=options.outputFile,
    doTrimu=False,
)

# Dimuon selection for J/psi
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
    "mass > 2.5 && mass < 4.0 && charge==0"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.4")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.001")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

# HiOnia settings
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)
process.hionia.genealogyInfo = cms.bool(True)  # Enable detailed gen info

# Set up onia path
process.oniaTreeAna = cms.Path(process.oniaTreeAna)

# Apply MiniAOD customization
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)

#----------------------------------------------------------------------------
# X(3872) + Chi_c Analysis using OniaPiPiProducer
# - OniaPiPi: X(3872) / psi(2S) -> J/psi + pi+ pi-
# - OniaEE: chi_c -> J/psi + gamma -> J/psi + e+ e- (conversion from general tracks)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.oniaPiPiCandidates_cfi import (
    oniaPiPiCandidates, oniaPiPiCandidatesWS,
    oniaConversionCandidates, oniaConversionCandidatesWS
)

# X(3872) / psi(2S) -> J/psi + pi+ pi- (using general tracks)
process.xCandidates = oniaPiPiCandidates.clone(
    oniaSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    trackSrc = cms.InputTag("unpackedTracksAndVertices"),
    vertexSrc = cms.InputTag("unpackedTracksAndVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    doPiPi = cms.bool(True),
    doConversion = cms.bool(False),
    
    # J/psi cuts (looser for MC)
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts for pions
    tkPtCut = cms.double(0.2),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(10.0),
    tkNhitsCut = cms.int32(3),
    tkPtErrCut = cms.double(0.2),
    tkDCACut = cms.double(2.0),
    
    # Di-pion cuts (wider for MC study)
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.28),   # > 2*m_pi
    ditrackMassMax = cms.double(1.5),    # Wider for MC
    
    # X candidate cuts (wider for MC)
    candMassMin = cms.double(3.5),
    candMassMax = cms.double(4.5),
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts (looser for MC)
    vtxChi2Cut = cms.double(0.001),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.0),
    alpha2DCut = cms.double(1.0),
    
    isWrongSign = cms.bool(False),
)

# Wrong-sign for background
process.xCandidatesWS = process.xCandidates.clone(
    isWrongSign = cms.bool(True),
)

# chi_c -> J/psi + gamma -> J/psi + e+ e- (conversion from general tracks)
process.chiCCandidates = oniaConversionCandidates.clone(
    oniaSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    trackSrc = cms.InputTag("unpackedTracksAndVertices"),
    vertexSrc = cms.InputTag("unpackedTracksAndVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    doPiPi = cms.bool(False),
    doConversion = cms.bool(True),
    
    # J/psi cuts (looser for MC)
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts for electrons (from conversion)
    tkPtCut = cms.double(0.1),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(10.0),
    tkNhitsCut = cms.int32(3),
    tkPtErrCut = cms.double(0.3),
    tkDCACut = cms.double(1.0),
    
    # Di-electron (photon) cuts - mass should be near zero
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.0),
    ditrackMassMax = cms.double(0.15),   # Slightly wider for MC
    
    # chi_c candidate cuts (wider for MC study)
    candMassMin = cms.double(3.2),
    candMassMax = cms.double(3.8),
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts (looser for MC)
    vtxChi2Cut = cms.double(0.0001),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.5),
    alpha2DCut = cms.double(1.5),
    
    # Conversion radius cuts (wider for MC)
    convRadiusMin = cms.double(0.5),
    convRadiusMax = cms.double(80.0),
    
    isWrongSign = cms.bool(False),
)

# Wrong-sign for background
process.chiCCandidatesWS = process.chiCCandidates.clone(
    isWrongSign = cms.bool(True),
)

#----------------------------------------------------------------------------
# Ntuplizers using PATCompositeNtupleProducer
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import patCompositeNtupleProducerDefault

# X(3872) ntuplizer
process.xTree = patCompositeNtupleProducerDefault.clone(
    candidateType = cms.string('X3872'),
    candidateSrc = cms.InputTag('xCandidates', 'OniaPiPi'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    beamSpotSrc = cms.InputTag('offlineBeamSpot'),
    twoLayerDecay = cms.untracked.bool(True),
    doMuon = cms.untracked.bool(True),
    nDaughters = cms.untracked.uint32(3),      # onia, pi+, pi-
    nGrandDaughters = cms.untracked.uint32(2), # mu+, mu-
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),  # Enable gen matching for MC
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
)

# X(3872) wrong-sign ntuplizer
process.xTreeWS = process.xTree.clone(
    candidateSrc = cms.InputTag('xCandidatesWS', 'OniaPiPi'),
)

# chi_c ntuplizer (conversion mode)
process.chiCTree = patCompositeNtupleProducerDefault.clone(
    candidateType = cms.string('ChiC'),
    candidateSrc = cms.InputTag('chiCCandidates', 'OniaEE'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    beamSpotSrc = cms.InputTag('offlineBeamSpot'),
    twoLayerDecay = cms.untracked.bool(True),
    doMuon = cms.untracked.bool(True),
    doElectron = cms.untracked.bool(True),
    nDaughters = cms.untracked.uint32(2),      # onia, photon(e+e-)
    nGrandDaughters = cms.untracked.uint32(2), # mu+, mu- (from onia)
    isCentrality = cms.untracked.bool(False),
    genealogyInfo = cms.untracked.bool(True),  # Enable gen matching for MC
    genParticles = cms.untracked.InputTag('prunedGenParticles'),
)

# chi_c wrong-sign ntuplizer
process.chiCTreeWS = process.chiCTree.clone(
    candidateSrc = cms.InputTag('chiCCandidatesWS', 'OniaEE'),
)

#----------------------------------------------------------------------------
# Paths
#----------------------------------------------------------------------------
# X(3872) path
process.xPath = cms.Path(
    process.unpackedTracksAndVertices +
    process.xCandidates +
    process.xTree
)

# X(3872) wrong-sign path
process.xPathWS = cms.Path(
    process.unpackedTracksAndVertices +
    process.xCandidatesWS +
    process.xTreeWS
)

# chi_c (conversion) path
process.chiCPath = cms.Path(
    process.unpackedTracksAndVertices +
    process.chiCCandidates +
    process.chiCTree
)

# chi_c wrong-sign path
process.chiCPathWS = cms.Path(
    process.unpackedTracksAndVertices +
    process.chiCCandidatesWS +
    process.chiCTreeWS
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
    process.oniaTreeAna,
    process.xPath,
    process.xPathWS,
    process.chiCPath,
    process.chiCPathWS,
)

#----------------------------------------------------------------------------
# Output trees:
#   - hionia/myTree      : J/psi -> mu+mu- candidates
#   - xTree/X3872        : X(3872) -> J/psi + pi+pi- candidates (right-sign)
#   - xTreeWS/X3872      : X(3872) -> J/psi + pi+pi- candidates (wrong-sign)
#   - chiCTree/ChiC      : chi_c -> J/psi + e+e- candidates (right-sign)
#   - chiCTreeWS/ChiC    : chi_c -> J/psi + e+e- candidates (wrong-sign)
#
# All trees share same events
# Use oniaIdx to match X/chi_c candidates with J/psi in hionia tree
#----------------------------------------------------------------------------
