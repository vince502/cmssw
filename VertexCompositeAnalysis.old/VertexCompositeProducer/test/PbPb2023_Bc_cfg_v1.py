import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("BcANA")

# Setup VarParsing
options = VarParsing.VarParsing ('standard')
options.register('inputFiles',
                 '/store/user/davidlw/PbPb2023_BcJpsiPi_CRAB/PbPb2023_BcJpsiPi_374810_HIPM_MC_skim/231208_221043/0000/*.root',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Input files")
options.register('outputFile',
                 'Bc_PbPb2023.root',
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

# Configure dimuon reconstruction specifically for Bc studies
process.onia2MuMuPAT.muons = cms.InputTag("patMuonsWithTrigger")
process.onia2MuMuPAT.lowerPuritySelection = cms.string("isPFMuon && (isGlobalMuon || isTrackerMuon)")
process.onia2MuMuPAT.higherPuritySelection = cms.string("")
process.onia2MuMuPAT.dimuonSelection = cms.string("mass > 2.9 && mass < 3.3 && charge = 0")
process.onia2MuMuPAT.addCommonVertex = cms.bool(True)
process.onia2MuMuPAT.resolvePileUpAmbiguity = cms.bool(True)
process.onia2MuMuPAT.genParticles = cms.InputTag("genParticles")

# Load Bc-specific configuration
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiMuMuCandidates_cff")

# Configure Bc-only reconstruction
process.BcCandidates = process.generalBcCandidates.clone(
    dimuonCollection = cms.InputTag("onia2MuMuPAT"),
    trackRecoAlgorithm = cms.InputTag("generalTracks"),
    vertexRecoAlgorithm = cms.InputTag("offlinePrimaryVertices"),
    
    # Optimized cuts for Bc reconstruction
    tkPtCut = cms.double(0.8),          # Pion track pT
    dimuonPtCut = cms.double(6.0),      # J/psi pT
    bcPtCut = cms.double(7.0),          # Bc pT
    bcMassCut = cms.double(0.6),        # Bc mass window
    bcYCut = cms.double(2.4),           # Bc rapidity
    
    # Vertex quality for shorter Bc lifetime
    vtxProbCut = cms.double(0.01),      # Vertex probability
    rVtxSigCut = cms.double(1.5),       # 2D decay length significance
    lVtxSigCut = cms.double(1.5),       # 3D decay length significance
    
    # Impact parameter cuts for pion track
    dauTransImpactSigCut = cms.double(1.0),
    dauLongImpactSigCut = cms.double(1.0)
)

# Output module
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# Output content for Bc analysis
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('Bc_output.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_BcCandidates_Bc_*',
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
process.bc_step = cms.Path(
    process.onia2MuMuPAT *
    process.BcCandidates
)

process.output_step = cms.EndPath(process.out)

# Schedule
process.schedule = cms.Schedule(
    process.bc_step,
    process.output_step
)

# Event content and scheduling
from FWCore.ParameterSet.Utilities import convertToUnscheduledExecution
process = convertToUnscheduledExecution(process)

# Add early deletion of temporary data products to reduce memory consumption
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)