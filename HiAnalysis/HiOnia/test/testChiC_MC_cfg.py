### Chi_c MC test: onia2MuMuPAT + hionia + chi_c producer + chi_c ntuplizer
# Usage: cmsRun testChiC_MC_cfg.py maxEvents=100
#
# Based on hioniaanalyzer_ppRef2024_PromptReco_cfg.py structure
# Modified for OO MC sample with chi_c -> J/psi(mumu) + gamma

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
process = cms.Process("ChiCMC", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "ChiC_MC.root"
options.inputFiles = [
    'file:/afs/cern.ch/work/s/soohwan/private/Analysis/MC/OxygenMC/CMSSW_15_0_11/src/RAW2DIGI_L1Reco_RECO_PAT_CHIC.root',
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
    
    # Loose cuts for MC study
    minConvPt = cms.double(0.0),
    minConvRho = cms.double(0.0),
    rejectPi0 = cms.bool(False),
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    requireTriggerMatch = cms.bool(False),
    dzMax = cms.double(2.0),
    deltaMassRange = cms.vdouble(0.0, 2.0),
    minCandPt = cms.double(0.0),
    maxCandAbsY = cms.double(2.4),
    doKinematicRefit = cms.bool(False),
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
    isMC = cms.untracked.bool(True),
    genParticles = cms.InputTag("prunedGenParticles"),
    treeName = cms.untracked.string("chiTree"),
)

# Chi_c path
process.chiCMuMuPath = cms.Path(
    process.chiCandidatesMuMu +
    process.chiTreeMuMu
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
