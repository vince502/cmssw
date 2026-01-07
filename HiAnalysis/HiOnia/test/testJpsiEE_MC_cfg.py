### Test J/psi -> e+e- MC with updated gen info
# Usage: cmsRun testJpsiEE_MC_cfg.py maxEvents=1000

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.maxEvents = 1000
options.outputFile = 'JpsiEE_MC.root'
options.parseArguments()

process = cms.Process('JpsiEEMC', eras.Run3_pp_on_PbPb_2025)

###############################################################################
# Input
###############################################################################
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/user/fdamas/PbPb2025/RunPrepMC/JpsiDielectron_pTHatMin4_HydjetEmbedded_Pythia8_TuneCP5_1510pre6/PAT_151X_mcRun3_2025_realistic_HI_v1/251002_094458/0000/step4_PAT_26.root',
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(4),
    numberOfStreams = cms.untracked.uint32(1),
    wantSummary = cms.untracked.bool(True),
)

###############################################################################
# Load geometry, GT, etc.
###############################################################################
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '151X_mcRun3_2025_realistic_HI_v1', '')

###############################################################################
# TransientTrackBuilder
###############################################################################
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

###############################################################################
# Event Filters
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)

###############################################################################
# Output
###############################################################################
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

###############################################################################
# Dielectron Producer
###############################################################################
process.load("HiSkim.HiOnia2EE.onia2EEPAT_cfi")
process.dielectrons = process.onia2ElectronElectronPAT.clone(
    dielectronSelection = cms.string("2.6 < mass && mass < 3.5"),  # J/psi mass window
    addCommonVertex = cms.bool(True),
    addElectronlessPrimaryVertex = cms.bool(False),
    resolvePileUpAmbiguity = cms.bool(False),
    primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotTag = cms.InputTag("offlineBeamSpot"),
    electrons = cms.InputTag("slimmedElectrons"),
    conversions = cms.InputTag("reducedEgamma", "reducedConversions"),  # Use reduced for MiniAOD
    higherPuritySelection = cms.string(""),  # No cuts for MC study
    lowerPuritySelection = cms.string(""),
    applyConversionVeto = cms.bool(False),  # Important: disable for J/psi -> ee
)

###############################################################################
# HiOniaElectronAnalyzer
###############################################################################
process.load("HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi")
process.hioniaElectrons.srcElectron = cms.InputTag("slimmedElectrons")
process.hioniaElectrons.srcDielectron = cms.InputTag("dielectrons")
process.hioniaElectrons.primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices")
process.hioniaElectrons.beamSpotTag = cms.InputTag("offlineBeamSpot")
process.hioniaElectrons.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.hioniaElectrons.triggerResults = cms.InputTag("TriggerResults", "", "HLT")
process.hioniaElectrons.triggerPathNames = cms.vstring()
process.hioniaElectrons.checkTriggerNames = cms.bool(False)
process.hioniaElectrons.storeGenInfo = cms.bool(True)  # Enable MC gen info
process.hioniaElectrons.isHI = cms.untracked.bool(True)
process.hioniaElectrons.isMC = cms.untracked.bool(True)
process.hioniaElectrons.useEvtPlane = cms.untracked.bool(False)
process.hioniaElectrons.fillTree = cms.bool(True)
process.hioniaElectrons.fillHistos = cms.bool(False)
process.hioniaElectrons.genParticles = cms.InputTag("prunedGenParticles")

###############################################################################
# Path
###############################################################################
process.analysisPath = cms.Path(
    process.eventFilter +
    process.dielectrons +
    process.hioniaElectrons
)

process.schedule = cms.Schedule(process.analysisPath)

###############################################################################
# Summary
###############################################################################
# Output tree (hioniaElectrons/eleTree):
#
# Generator info (MC only):
#   Gen_ele_size      - Number of gen electrons (status=1)
#   Gen_ele_pt/eta/phi/y/mass - Kinematics
#   Gen_ele_charge    - Charge
#   Gen_ele_pdgId     - PDG ID (+/-11)
#   Gen_ele_status    - Status code
#   Gen_ele_motherId  - Mother PDG ID
#   Gen_ele_grandmotherId - Grandmother PDG ID
#   Gen_ele_vx/vy/vz  - Vertex position
#   Gen_ele_whichRec  - Index of matched reco electron (-1 if not matched)
#
#   Gen_ee_size       - Number of gen dielectrons (J/psi, Upsilon -> e+e-)
#   Gen_ee_pt/eta/phi/y/mass - Kinematics
#   Gen_ee_elepl_idx  - Index of positive electron in Gen_ele arrays
#   Gen_ee_elemi_idx  - Index of negative electron in Gen_ele arrays
#   Gen_ee_momId      - Mother PDG ID (443 for J/psi)
#   Gen_ee_ctau       - Proper decay length (2D) in mm
#   Gen_ee_ctau3D     - Proper decay length (3D) in mm
#   Gen_ee_vx/vy/vz   - Decay vertex
#   Gen_ee_whichRec   - Index of matched reco dielectron (-1 if not matched)
###############################################################################
