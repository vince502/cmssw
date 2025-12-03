import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("BDiMuMuNTUPLE")

# Setup VarParsing
options = VarParsing.VarParsing ('standard')
options.register('inputFiles',
                 '/store/user/davidlw/PbPb2023_BDiMuMu_CRAB/PbPb2023_BDiMuMu_374810_HIPM_MC_skim/231208_221043/0000/*.root',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Input files")
options.register('outputFile',
                 'BDiMuMu_Ntuple_PbPb2023.root',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Output file")
options.register('maxEvents',
                 1000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Maximum number of events")
options.register('isMC',
                 False,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Is Monte Carlo")
options.parseArguments()

# Load necessary conditions
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# Global tag
from Configuration.AlCa.GlobalTag import GlobalTag
if options.isMC:
    process.GlobalTag = GlobalTag(process.GlobalTag, '132X_mcRun3_2023_realistic_HI_v9', '')
else:
    process.GlobalTag = GlobalTag(process.GlobalTag, '132X_dataRun3_HI_v8', '')

# Message logger configuration
process.MessageLogger.cerr.FwkReport.reportEvery = 100

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# Load HiOnia2MuMu configuration for dimuon reconstruction
process.load("HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff")

# Configure dimuon reconstruction
process.onia2MuMuPAT.muons = cms.InputTag("patMuonsWithTrigger")
process.onia2MuMuPAT.lowerPuritySelection = cms.string("isPFMuon && (isGlobalMuon || isTrackerMuon)")
process.onia2MuMuPAT.higherPuritySelection = cms.string("")
process.onia2MuMuPAT.dimuonSelection = cms.string("mass > 2.9 && mass < 3.3 && charge = 0")
process.onia2MuMuPAT.addCommonVertex = cms.bool(True)
process.onia2MuMuPAT.resolvePileUpAmbiguity = cms.bool(True)

if options.isMC:
    process.onia2MuMuPAT.genParticles = cms.InputTag("genParticles")

# Load B meson reconstruction
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiMuMuCandidates_cff")

# Configure B meson reconstruction
process.generalBDiMuMuCandidates.dimuonCollection = cms.InputTag("onia2MuMuPAT")
process.generalBDiMuMuCandidates.trackRecoAlgorithm = cms.InputTag("generalTracks")
process.generalBDiMuMuCandidates.vertexRecoAlgorithm = cms.InputTag("offlinePrimaryVertices")

# Enable all B meson types
process.generalBDiMuMuCandidates.doBPlus = cms.bool(True)
process.generalBDiMuMuCandidates.doBZero = cms.bool(True)
process.generalBDiMuMuCandidates.doBc = cms.bool(True)
process.generalBDiMuMuCandidates.doJPsi = cms.bool(True)

# Adjust cuts for better efficiency
process.generalBDiMuMuCandidates.tkPtCut = cms.double(0.7)
process.generalBDiMuMuCandidates.dimuonPtCut = cms.double(6.0)
process.generalBDiMuMuCandidates.bPtCut = cms.double(7.0)
process.generalBDiMuMuCandidates.bcPtCut = cms.double(8.0)
process.generalBDiMuMuCandidates.vtxProbCut = cms.double(0.01)

# Load B meson ntuplizer
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.bDiMuMuNtuplizer_cff")

# Configure ntuplizer
if options.isMC:
    process.bDiMuMuNtuplizer = process.bDiMuMuNtuplizerMC.clone()
else:
    process.bDiMuMuNtuplizer = process.bDiMuMuNtuplizerAll.clone()

# Update input collections to match producer
process.bDiMuMuNtuplizer.bPlusCollection = cms.InputTag("generalBDiMuMuCandidates:BPlus")
process.bDiMuMuNtuplizer.bZeroCollection = cms.InputTag("generalBDiMuMuCandidates:BZero")
process.bDiMuMuNtuplizer.bcCollection = cms.InputTag("generalBDiMuMuCandidates:Bc")

# Centrality for heavy ion collisions
if not options.isMC:
    process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
    process.centralityBin.Centrality = cms.InputTag("hiCentrality")
    process.centralityBin.centralityVariable = cms.string("HFtowers")

# TFileService for output tree
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# Define the reconstruction path
process.reco_step = cms.Path(
    process.onia2MuMuPAT *
    process.generalBDiMuMuCandidates
)

# Define the analysis path
if options.isMC:
    process.ana_step = cms.Path(process.bDiMuMuNtuplizer)
else:
    process.ana_step = cms.Path(
        process.centralityBin *
        process.bDiMuMuNtuplizer
    )

# Schedule
process.schedule = cms.Schedule(
    process.reco_step,
    process.ana_step
)

# Event content and scheduling
from FWCore.ParameterSet.Utilities import convertToUnscheduledExecution
process = convertToUnscheduledExecution(process)

# Add early deletion of temporary data products to reduce memory consumption
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)

print("="*50)
print("BDiMuMu Ntuplizer Configuration")
print("="*50)
print(f"Input: {options.inputFiles}")
print(f"Output: {options.outputFile}")
print(f"Max events: {options.maxEvents}")
print(f"Is MC: {options.isMC}")
print(f"B meson types: B+, B0, Bc")
print("="*50)