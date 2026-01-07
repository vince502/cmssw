### Chi_c + X(3872) DATA: onia2MuMuPAT + hionia + chi_c + X(3872) producers
# Usage: cmsRun testChiX_DATA_cfg.py maxEvents=10000
#
# For OO 2025 data with:
#   - chi_c -> J/psi(mumu) + gamma (photon conversion: e+e- from general tracks)
#   - X(3872) -> J/psi(mumu) + pi+ pi-
#
# All paths share the same eventFilter to ensure identical event selection

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Settings
#----------------------------------------------------------------------------
HLTProcess     = "HLT"
isMC           = False
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
process = cms.Process("ChiXDATA", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "ChiX_DATA.root"
options.inputFiles = [
#'/store/hidata/OORun2025/IonPhysics59/USER/IonDimuon-PromptReco-v1/000/394/209/00000/ce9b426b-6202-4f73-abb5-93a9d172f393.root',
#'/store/hidata/OORun2025/IonPhysics59/USER/IonDimuon-PromptReco-v1/000/394/209/00000/c0bdde60-5c36-45c9-a049-14abd8b36004.root',
'file:ce9b426b-6202-4f73-abb5-93a9d172f393.root',
]
options.maxEvents = 1000
options.parseArguments()

# Trigger list for OO 2025
triggerList = {
    'DoubleMuonTrigger': cms.vstring(
        "HLT_OxyL1DoubleMuOpen_v",
        "HLT_OxyL1DoubleMu0_v",
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_OxyL1SingleMuOpen_v",
        "HLT_OxyL1SingleMu0_v",
        "HLT_OxyL1SingleMu3_v",
        "HLT_OxyL1SingleMu5_v",
        "HLT_OxyL1SingleMu7_v",
        "HLT_MinimumBiasHF_OR_BptxAND_v",
        "HLT_MinimumBiasHF_AND_BptxAND_v",
    ),
}

#----------------------------------------------------------------------------
# Load standard sequences
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#----------------------------------------------------------------------------
# Event Selection Filters (shared by all paths)
#----------------------------------------------------------------------------
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v3', '')

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
    HLTProName=HLTProcess,
    muonSelection=muonSelection,
    L1Stage=2,
    isMC=isMC,
    pdgID=pdgId,
    outputFileName=options.outputFile,
    doTrimu=False,
)

# Dimuon selection for J/psi
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
    "mass > 2.5 && mass < 4.0 && charge==0 && "
    "abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

# HiOnia settings
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)

# Set up onia path with event filter
process.oniaTreeAna = cms.Path(process.oniaTreeAna)

# Apply MiniAOD customization
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)

# Add event filter to onia path
process.oniaTreeAna.replace(
    process.patMuonSequence,
    process.eventFilter * process.patMuonSequence
)

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
    dedxSrc = cms.InputTag(""),  # dEdx not available in MiniAOD without rerunning reco
    
    doPiPi = cms.bool(True),
    doConversion = cms.bool(False),
    
    # J/psi cuts
    minOniaPt = cms.double(3.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts for pions
    tkPtCut = cms.double(0.3),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(5.0),
    tkNhitsCut = cms.int32(5),
    tkPtErrCut = cms.double(0.1),
    tkDCACut = cms.double(1.0),
    
    # Di-pion cuts (rho region)
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.28),   # > 2*m_pi
    ditrackMassMax = cms.double(0.9),    # rho(770) region
    
    # X candidate cuts
    candMassMin = cms.double(3.6),       # Below psi(2S)
    candMassMax = cms.double(4.0),       # Above X(3872)
    candPtCut = cms.double(5.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts
    vtxChi2Cut = cms.double(0.01),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(0.5),
    alpha2DCut = cms.double(0.5),
    
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
    dedxSrc = cms.InputTag(""),  # dEdx not available in MiniAOD without rerunning reco
    
    doPiPi = cms.bool(False),
    doConversion = cms.bool(True),
    
    # J/psi cuts
    minOniaPt = cms.double(3.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts for electrons (from conversion)
    tkPtCut = cms.double(0.1),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(7.0),
    tkNhitsCut = cms.int32(0),
    tkPtErrCut = cms.double(0.1),
    tkDCACut = cms.double(0.1),
    
    # Di-electron (photon) cuts - mass should be near zero
    # Note: ditrackMass is raw e+e- mass BEFORE mass constraint
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.0),
    ditrackMassMax = cms.double(0.5),  # Looser for initial testing
    
    # chi_c candidate cuts
    candMassMin = cms.double(3.3),
    candMassMax = cms.double(3.7),
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts (looser for conversion)
    vtxChi2Cut = cms.double(0.001),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.0),
    alpha2DCut = cms.double(1.0),
    
    # Conversion radius cuts
    convRadiusMin = cms.double(1.0),     # Avoid beampipe
    convRadiusMax = cms.double(50.0),    # Within tracker
    
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
)

# chi_c wrong-sign ntuplizer
process.chiCTreeWS = process.chiCTree.clone(
    candidateSrc = cms.InputTag('chiCCandidatesWS', 'OniaEE'),
)

#----------------------------------------------------------------------------
# Paths with event filter
#----------------------------------------------------------------------------
# X(3872) path
process.xPath = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.xCandidates +
    process.xTree
)

# X(3872) wrong-sign path
process.xPathWS = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.xCandidatesWS +
    process.xTreeWS
)

# chi_c (conversion) path
process.chiCPath = cms.Path(
    process.eventFilter +
    process.unpackedTracksAndVertices +
    process.chiCCandidates +
    process.chiCTree
)

# chi_c wrong-sign path
process.chiCPathWS = cms.Path(
    process.eventFilter +
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
# Schedule - all paths use same eventFilter for consistent event selection
#----------------------------------------------------------------------------
process.schedule = cms.Schedule(
    process.oniaTreeAna,
    process.xPath,
    process.xPathWS,
    process.chiCPath,
    process.chiCPathWS,
)

import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions25OO/Cert_Collisions2025OO_394153_394217_muon.json').getVLuminosityBlockRange()

#----------------------------------------------------------------------------
# Output trees:
#   - hionia/myTree      : J/psi -> mu+mu- candidates
#   - xTree/X3872        : X(3872) -> J/psi + pi+pi- candidates (right-sign)
#   - xTreeWS/X3872      : X(3872) -> J/psi + pi+pi- candidates (wrong-sign)
#   - chiCTree/ChiC      : chi_c -> J/psi + e+e- candidates (right-sign)
#   - chiCTreeWS/ChiC    : chi_c -> J/psi + e+e- candidates (wrong-sign)
#
# All trees share same events (via eventFilter)
# Use oniaIdx to match X/chi_c candidates with J/psi in hionia tree
#----------------------------------------------------------------------------
