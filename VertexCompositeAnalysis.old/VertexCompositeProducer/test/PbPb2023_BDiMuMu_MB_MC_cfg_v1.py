import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("BDiMuMuMCANA")

# Setup VarParsing
options = VarParsing.VarParsing ('standard')
options.register('inputFiles',
                 '/store/user/davidlw/Hydjet_5p02TeV_TuneCP5_2023/crab_PbPb2023_Hydjet_TuneCP5_5p02TeV_374810_HIPM_MC_skim/231208_221043/0000/*.root',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Input files")
options.register('outputFile',
                 'BDiMuMu_PbPb2023_MC.root',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Output file")
options.register('maxEvents',
                 1000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Maximum number of events")
options.parseArguments()

# Load necessary conditions
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# Global tag for PbPb 2023 MC
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '132X_mcRun3_2023_realistic_HI_v9', '')

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

# Configure dimuon reconstruction for MC
process.onia2MuMuPAT.muons = cms.InputTag("patMuonsWithTrigger")
process.onia2MuMuPAT.lowerPuritySelection = cms.string("isPFMuon && (isGlobalMuon || isTrackerMuon)")
process.onia2MuMuPAT.higherPuritySelection = cms.string("")
process.onia2MuMuPAT.dimuonSelection = cms.string("mass > 2.9 && mass < 3.3 && charge = 0")
process.onia2MuMuPAT.addCommonVertex = cms.bool(True)
process.onia2MuMuPAT.resolvePileUpAmbiguity = cms.bool(True)
process.onia2MuMuPAT.genParticles = cms.InputTag("genParticles")

# Load B meson reconstruction
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiMuMuCandidates_cff")

# Configure B meson reconstruction for MC
process.generalBDiMuMuCandidates.dimuonCollection = cms.InputTag("onia2MuMuPAT")
process.generalBDiMuMuCandidates.trackRecoAlgorithm = cms.InputTag("generalTracks")
process.generalBDiMuMuCandidates.vertexRecoAlgorithm = cms.InputTag("offlinePrimaryVertices")

# Use looser cuts for MC efficiency studies
process.generalBDiMuMuCandidates.tkPtCut = cms.double(0.7)         # Track pT
process.generalBDiMuMuCandidates.dimuonPtCut = cms.double(5.0)     # J/psi pT
process.generalBDiMuMuCandidates.bPtCut = cms.double(6.0)          # B meson pT
process.generalBDiMuMuCandidates.vtxProbCut = cms.double(0.005)    # Looser vertex quality
process.generalBDiMuMuCandidates.rVtxSigCut = cms.double(1.5)      # Decay length significance
process.generalBDiMuMuCandidates.lVtxSigCut = cms.double(1.5)

# Enable B+, B0, and Bc reconstruction
process.generalBDiMuMuCandidates.doBPlus = cms.bool(True)
process.generalBDiMuMuCandidates.doBZero = cms.bool(True)
process.generalBDiMuMuCandidates.doBc = cms.bool(True)
process.generalBDiMuMuCandidates.doJPsi = cms.bool(True)

# Adjust Bc cuts for MC studies
process.generalBDiMuMuCandidates.bcPtCut = cms.double(6.0)      # Looser Bc pT for efficiency
process.generalBDiMuMuCandidates.bcMassCut = cms.double(0.8)    # Wider Bc mass window for MC

# Output module
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# Output content including MC truth information
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('BDiMuMu_MC_output.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_generalBDiMuMuCandidates_*_*',
        'keep *_onia2MuMuPAT_*_*',
        'keep *_offlinePrimaryVertices_*_*',
        'keep *_generalTracks_*_*',
        'keep recoBeamSpot_offlineBeamSpot_*_*',
        'keep recoVertexs_offlinePrimaryVertices_*_*',
        'keep *_centralityBin_*_*',
        'keep *_hiCentrality_*_*',
        'keep *_genParticles_*_*',
        'keep *_generator_*_*'
    )
)

# Define the path
process.bDiMuMu_step = cms.Path(
    process.onia2MuMuPAT *
    process.generalBDiMuMuCandidates
)

process.output_step = cms.EndPath(process.out)

# Schedule
process.schedule = cms.Schedule(
    process.bDiMuMu_step,
    process.output_step
)

# Event content and scheduling
from FWCore.ParameterSet.Utilities import convertToUnscheduledExecution
process = convertToUnscheduledExecution(process)

# Add early deletion of temporary data products to reduce memory consumption
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)