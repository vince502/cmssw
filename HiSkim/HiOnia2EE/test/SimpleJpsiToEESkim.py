import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process('JPSIEESKIM', eras.Run3_pp_on_PbPb_2024)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        # Add your input files here
        #'/store/hidata/HIRun2024A/HIPhysicsRawPrime2/MINIAOD/PromptReco-v1/000/387/908/00000/93a76f4f-4e90-4357-9a7f-3c64a1be8e29.root'
        '/store/data/Run2024J/PPRefZeroBiasPlusForward0/MINIAOD/PromptReco-v1/000/387/696/00000/0037fb37-713f-4df8-9668-a2ce4665a93c.root'
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000)
)

# Load the onia2EE producer
from HiSkim.HiOnia2EE.onia2EEPAT_cff import setupOnia2EEForMiniAOD

process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# Configure for J/psi -> ee selection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 2.8 && mass < 3.4 && charge == 0"
)
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = True
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(
    'HLT_HIEle20Gsf_v',
    'HLT_HIDoubleEle10Gsf_v'
)
process.onia2ElectronElectronPatGlbGlb.triggerMatchDR = 0.2

from HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi import ggHiNtuplizer
process.ggHiNtuplizer = ggHiNtuplizer.clone(
    doPhotons = False,
    doMuons = False,
    doSuperClusters = False
)

process.hiTriggerObjects = cms.EDAnalyzer(
    "TriggerObjectAnalyzer",
    processName = cms.string("HLT"),
    triggerNames = cms.vstring('HLT_HIEle20Gsf_v', 'HLT_HIDoubleEle10Gsf_v'),
    triggerResults = cms.InputTag("TriggerResults","","HLT"),
    triggerObjects = cms.InputTag("slimmedPatTrigger")
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("triggerAndEGM.root")
)

# Filter: require at least one J/psi candidate
process.onia2ElectronElectronFilter = cms.EDFilter("CandViewCountFilter",
    src = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    minNumber = cms.uint32(1)
)

# Output definition
process.AODSIMoutput = cms.OutputModule("PoolOutputModule",
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(4),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('AODSIM'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(15728640),
    fileName = cms.untracked.string('JpsiToEE_skim.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_onia2ElectronElectronPatGlbGlb_*_*',
        'keep patElectrons_*_*_*',
        'keep *_offlinePrimaryVertices_*_*',
        'keep *_offlineBeamSpot_*_*',
        'keep *_generator_*_*',
        'keep *_genParticles_*_*',
        'keep *_TriggerResults_*_HLT'
    ),
    # SelectEvents = cms.untracked.PSet(
    #     SelectEvents = cms.vstring('skim_path')
    # )
)

# Path definition
process.skim_path = cms.Path(
    process.onia2ElectronElectronPatGlbGlb *
    process.onia2ElectronElectronFilter *
    process.ggHiNtuplizer *
    process.hiTriggerObjects
)

setupOnia2EEForMiniAOD(process)

process.AODSIMoutput_step = cms.EndPath(process.AODSIMoutput)

# Schedule definition
process.schedule = cms.Schedule(process.skim_path, process.AODSIMoutput_step)

# Message logger
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
