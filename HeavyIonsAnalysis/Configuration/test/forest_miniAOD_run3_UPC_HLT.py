### HiForest Configuration
# Input: miniAOD
# Type: data

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_2026_cff import Run3_2026
process = cms.Process('HiForest', Run3_2026)

###############################################################################

# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 160X, data")

# import subprocess, os
# version = subprocess.check_output(
#     ['git', '-C', os.path.expandvars('$CMSSW_BASE/src'), 'describe', '--tags'])
# if version == '':
#     version = 'no git info'
# process.HiForestInfo.HiForestVersion = cms.string(version)

###############################################################################

# input files
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
#'file:/afs/cern.ch/work/s/soohwan/private/HLT/Run2026/PixelTrackingUPC/CATune/CMSSW_16_0_0_pre4/src/workstation/output_custom.root',
#'file:/afs/cern.ch/work/s/soohwan/private/HLT/Run2026/PixelTrackingUPC/CATune/CMSSW_16_0_0_pre4/src/workstation/output_custom2.root',
'file:/afs/cern.ch/work/s/soohwan/private/HLT/Run2026/PixelTrackingUPC/CATune/CMSSW_16_0_0_pre4/src/workstation/output_custom2_mc.root',
    ), 
)

# number of events to process, set to -1 to process all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
    )

process.options = cms.untracked.PSet()

###############################################################################

# load Global Tag, geometry, etc.
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')


from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, '151X_dataRun3_HLT_v1', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

###############################################################################

# root output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestMiniAOD.root"))

# # edm output for debugging purposes
# process.output = cms.OutputModule(
#     "PoolOutputModule",
#     fileName = cms.untracked.string('HiForestEDM.root'),
#     outputCommands = cms.untracked.vstring(
#         'keep *',
#         )
#     )

# process.output_path = cms.EndPath(process.output)

###############################################################################

# simple track ntuplizer
process.load("HeavyIonsAnalysis.TrackAnalysis.SimpleTrackNtuplizer_cfi")
process.simpleTrackNtuplizer.trackSrc = cms.InputTag("hltPixelTracksPPOnAAlowPt")

process.upcTrackNtuplizer = process.simpleTrackNtuplizer.clone()
process.upcTrackNtuplizer.trackSrc = cms.InputTag("hltPixelTracksLowPtForUPCPPOnAA")

process.upcTrackNtuplizer2 = process.simpleTrackNtuplizer.clone()
process.upcTrackNtuplizer2.trackSrc = cms.InputTag("hltPixelTracksLowPtForUPCPPOnAA2")

# gen track ntuplizer (for GEN-SIM MC)
process.load("HeavyIonsAnalysis.TrackAnalysis.GenTrackNtuplizer_cfi")
# Using HepMCProduct from generatorSmeared
process.genTrackNtuplizer.hepmcSrc = cms.InputTag("generatorSmeared", "", "SIM")
process.genTrackNtuplizer.onlyStable = cms.bool(True)      # Only stable particles (status == 1)
process.genTrackNtuplizer.onlyCharged = cms.bool(True)      # Only charged particles

###############################################################################
# main forest sequence
process.forest = cms.Path(
    process.simpleTrackNtuplizer +
    process.upcTrackNtuplizer +
    process.upcTrackNtuplizer2 +
    process.genTrackNtuplizer +  # Gen tracks (only runs if genParticles exist)
    process.HiForestInfo 

    )

process.options.TryToContinue = cms.untracked.vstring('ProductNotFound')

