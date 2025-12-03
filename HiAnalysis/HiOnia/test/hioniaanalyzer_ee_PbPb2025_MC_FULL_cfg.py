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

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# Global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '151X_mcRun3_2025_realistic_HI_v1', '')

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/user/fdamas/PbPb2025/RunPrepMC/JpsiDielectron_pTHatMin4_HydjetEmbedded_Pythia8_TuneCP5_1510pre6/PAT_151X_mcRun3_2025_realistic_HI_v1/251002_094458/0000/step4_PAT_102.root',
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)  # Process 100 events for testing
)

# Load centrality for HI running
process.load('RecoHI.HiCentralityAlgos.CentralityBin_cfi')
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

# Load HI rho producer for isolation
process.load('HeavyIonsAnalysis.JetAnalysis.hiFJRhoAnalyzer_cff')

# Load the onia2EE producer and configure it with corrections
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

from HiSkim.HiOnia2EE.onia2EEPAT_cff import electron_trigger_paths

# Configure for J/psi -> ee selection
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 1.5 && mass < 16 && charge == 0"
)
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = False
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electron_trigger_paths)
process.onia2ElectronElectronPatGlbGlb.triggerMatchDR = 0.2

# Load the electron analyzer
from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons

process.hionia = hioniaElectrons.clone(
    srcElectron = cms.InputTag('hiElectrons'),  # Use corrected electrons with HI ID/ISO
    srcDielectron = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    primaryVertexTag = cms.InputTag('offlineSlimmedPrimaryVertices'),
    beamSpotTag = cms.InputTag('offlineBeamSpot'),
    conversions = cms.InputTag('reducedEgamma', 'reducedConversions'),
    triggerResults = cms.InputTag('TriggerResults', '', 'HLT'),
    CentralitySrc = cms.InputTag('hiCentrality'),
    CentralityBinSrc = cms.InputTag('centralityBin', 'HFtowers'),
    genParticles = cms.InputTag('prunedGenParticles'),
    triggerPathNames = cms.vstring(*electron_trigger_paths),
    checkTriggerNames = cms.bool(True),
    storeGenInfo = cms.bool(True),
    fillTree = cms.bool(True),
    fillHistos = cms.bool(False),
    isHI = cms.untracked.bool(True),
    isMC = cms.untracked.bool(True),
    useEvtPlane = cms.untracked.bool(False)
)

# TFile Service for output
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("OniaTree_ee_MC.root")
)

# Filter: require at least one J/psi candidate
process.onia2ElectronElectronFilter = cms.EDFilter("CandViewCountFilter",
    src = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    minNumber = cms.uint32(1)
)

# Path definition
process.skim_path = cms.Path(
    process.centralityBin *
    process.hiFJRhoProducerFinerBins *
    process.onia2ElectronElectronPatGlbGlb *
    process.onia2ElectronElectronFilter *
    process.hionia
)

# Setup for MiniAOD with full corrections chain
# This will:
# 1. Apply energy scale corrections (CorrectedElectronProducer)
# 2. Apply HI-specific MVA ID and isolation (HIElectronInfoProducer)
# 3. Configure the onia2EE producer to use corrected electrons
# 4. Insert all necessary sequences into the path
from HiSkim.HiOnia2EE.onia2EEPAT_cff import setupOnia2EEForMiniAOD
setupOnia2EEForMiniAOD(
    process, 
    producer='onia2ElectronElectronPatGlbGlb',
    runEnergyCorrections=True,
    runHIElectronID=True,
    isMC=True
)

# Schedule definition
process.schedule = cms.Schedule(process.skim_path)

print("="*80)
print("Configuration Summary:")
print("  - Running on MC with full electron corrections")
print("  - Energy scale corrections: ENABLED")
print("  - HI MVA ID and Isolation: ENABLED")
print("  - Output file: OniaTree_ee_MC.root")
print("="*80)
