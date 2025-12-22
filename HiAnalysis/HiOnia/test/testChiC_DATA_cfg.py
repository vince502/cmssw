### Chi_c DATA: onia2MuMuPAT + hionia + chi_c producer + chi_c ntuplizer
# Usage: cmsRun testChiC_DATA_cfg.py maxEvents=10000
#
# For OO 2025 data with chi_c -> J/psi(mumu) + gamma

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
process = cms.Process("ChiCDATA", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "ChiC_DATA.root"
options.inputFiles = [
#    '/store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root',
'/store/hidata/OORun2025/IonPhysics59/USER/IonDimuon-PromptReco-v1/000/394/209/00000/ce9b426b-6202-4f73-abb5-93a9d172f393.root',
'/store/hidata/OORun2025/IonPhysics59/USER/IonDimuon-PromptReco-v1/000/394/209/00000/c0bdde60-5c36-45c9-a049-14abd8b36004.root',
]
options.maxEvents = -1
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
# Event Selection Filters
#----------------------------------------------------------------------------
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v3', '')

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
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.4")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

# HiOnia settings
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)

# Set up path
process.oniaTreeAna = cms.Path(process.oniaTreeAna)

# Apply MiniAOD customization
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)

#----------------------------------------------------------------------------
# Chi_c Analysis (J/psi + photon conversion)
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.oniaPhotonCandidates_cfi import chiCandidates

process.chiCandidatesMuMu = chiCandidates.clone(
    oniaSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    conversionSrc = cms.InputTag("oniaPhotonCandidates", "conversions"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    # Conversion cuts
    minConvPt = cms.double(0.1),
    minConvRho = cms.double(0.0),
    rejectPi0 = cms.bool(False),
    
    # J/psi cuts
    minOniaPt = cms.double(1.0),
    maxOniaAbsY = cms.double(2.4),
    requireTriggerMatch = cms.bool(False),
    
    # Matching cuts
    dzMax = cms.double(2.0),
    deltaMassRange = cms.vdouble(0.0, 2.0),
    
    # Combined candidate cuts
    minCandPt = cms.double(0.0),
    maxCandAbsY = cms.double(2.4),
    
    # Kinematic refit
    doKinematicRefit = cms.bool(True),
    constraintMass = cms.double(3.0969),
    parentType = cms.string("jpsi"),
)

#----------------------------------------------------------------------------
# Chi_c Ntuplizer
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeAnalyzer.chiCNtupleProducer_cfi import chiCNtupleProducer

process.chiTreeMuMu = chiCNtupleProducer.clone(
    candidateSrc = cms.InputTag("chiCandidatesMuMu"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    isCentrality = cms.untracked.bool(False),
    isMC = cms.untracked.bool(False),
    treeName = cms.untracked.string("chiTree"),
)

# Chi_c path
process.chiCMuMuPath = cms.Path(
    process.eventFilter +
    process.chiCandidatesMuMu +
    process.chiTreeMuMu
)


# Add filters to onia path
process.oniaTreeAna.replace(
    process.patMuonSequence,
    process.eventFilter * process.patMuonSequence
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
    process.chiCMuMuPath,
)

import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions25OO/Cert_Collisions2025OO_394153_394217_muon.json').getVLuminosityBlockRange()
#process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions25OO/Cert_Collisions2025OO_394153_394217_silver.json').getVLuminosityBlockRange()
