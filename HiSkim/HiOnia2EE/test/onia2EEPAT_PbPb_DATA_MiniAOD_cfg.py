import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

# Use Run3 PbPb era for compatibility with CMSSW_14_1_X
process = cms.Process('ONIA2EE', eras.Run3_pp_on_PbPb_2024)

# Import standard sequences
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Global tag for Run3 data
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')

# Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        # Example miniAOD file - replace with actual file
        '/store/hidata/HIRun2024A/HIPhysicsRawPrime2/MINIAOD/PromptReco-v1/000/387/908/00000/93a76f4f-4e90-4357-9a7f-3c64a1be8e29.root'
    ),
    duplicateCheckMode = cms.untracked.string('noDuplicateCheck')
)

# Number of events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

# Output
process.output = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('onia2ee_output.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_onia2ElectronElectronPatGlbGlb_*_*',
        'keep *_offlinePrimaryVertices_*_*',
        'keep *_offlineBeamSpot_*_*'
    )
)

# Load the onia2EE producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Define the path and endpath
process.onia_ee_path = cms.Path(process.onia2ElectronElectronPatGlbGlb)
process.out = cms.EndPath(process.output)

# Message logger settings
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.MessageLogger.cerr.threshold = 'INFO'

# Options
process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True),
    wantSummary = cms.untracked.bool(True)
)
