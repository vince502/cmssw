import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process('JPSIEESKIM', eras.Run3_pp_on_PbPb_2025)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# Global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v1', '')

import sys

fileToRun=str(sys.argv[1])
# Input source
process.source = cms.Source("PoolSource",
#    fileNames = cms.untracked.vstring(
#        # Add your input files here
#'file:/eos/cms/store/group/phys_heavyions_ops/soohwan/Run3_2024_ReReco2025/Data/HLTPhysics/PAT_ECALDriven_HLTPhysicsMB2024B_18Oct25_v1/HLTPhysicsRun2024B_ReReco2025/PAT_ECALDriven_HLTPhysicsMB2024B_18Oct25_v1/251018_102737/0000/RECO_v1_24.root',
#'file:/eos/cms/store/group/phys_heavyions_ops/soohwan/Run3_2024_ReReco2025/Data/HLTPhysics/PAT_ECALDriven_HLTPhysicsMB2024B_18Oct25_v1/HLTPhysicsRun2024B_ReReco2025/PAT_ECALDriven_HLTPhysicsMB2024B_18Oct25_v1/251018_102737/0000/RECO_v1_23.root',
#
#    )
    fileNames = cms.untracked.vstring(str(fileToRun)),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

# Load the onia2EE producer
from HiSkim.HiOnia2EE.onia2EEPAT_cff import setupOnia2EEForMiniAOD

process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

from HiSkim.HiOnia2EE.onia2EEPAT_cff import electron_trigger_paths

# Configure for J/psi -> ee selection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 1.5 && mass < 16 && charge == 0"
    # "mass > 0.0 && mass < 10000"
)
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag("slimmedElectrons")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electron_trigger_paths)
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
    fileName = cms.string(f'file:/afs/cern.ch/work/s/soohwan/private/Analysis/OniaTree2025/JpsiToEETest/CMSSW_15_1_0_pre6/src/HiAnalysis/HiOnia/test/output/triggerAndEGM_{sys.argv[2]}.root')
)

# Filter: require at least one J/psi candidate
process.onia2ElectronElectronFilter = cms.EDFilter("CandViewCountFilter",
    src = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    minNumber = cms.uint32(1)
)


from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons

process.hionia = hioniaElectrons.clone()
process.hionia.isHI = False
process.hionia.triggerPathNames = cms.vstring(*electron_trigger_paths)

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
    process.hionia
    # process.ggHiNtuplizer *
    # process.hiTriggerObjects
)

setupOnia2EEForMiniAOD(process)

process.AODSIMoutput_step = cms.EndPath(process.AODSIMoutput)

# Schedule definition
process.schedule = cms.Schedule(process.skim_path)

# Message logger
process.MessageLogger.cerr.FwkReport.reportEvery = 100


process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")

process.options.numberOfThreads = 1
process.options.numberOfStreams = 0
