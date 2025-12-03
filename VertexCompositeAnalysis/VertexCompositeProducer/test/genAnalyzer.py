import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM', eras.Run3_2023)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_Data_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
#'file:/eos/home-s/soohwan/Analysis/DmesonpPb/MCGen/CMSSW_8_0_36_patch1/src/Closure/HIN-pPb816Spring16GS-00122.root',
#'file:/eos/home-s/soohwan/Analysis/DmesonpPb/MCGen/CMSSW_8_0_36_patch1/src/Closure/HIN-pPb816Spring16GS-00122_Soft.root',
#'file:step4_100.root',
#'file:/afs/cern.ch/work/s/soohwan/private/Analysis/DmesonAna/MCGen/CMSSW_13_2_11/src/DStarKpipi_py_GEN_SIM.root',
'file:/afs/cern.ch/work/s/soohwan/private/Analysis/DmesonAna/MCGen/CMSSW_13_2_11/src/step3PAT_PAT.root',
)
)

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#process.GlobalTag.globaltag = cms.string('132X_dataRun3_Prompt_v4')
process.GlobalTag.globaltag = cms.string('132X_mcRun3_2023_realistic_HI_v9')

# =============== Other Statements =====================
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(2000))
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(50000))
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# =============== Import Sequences =====================

process.load('VertexCompositeAnalysis.VertexCompositeAnalyzer.simplegenana_cfi')
process.genAna.GenParticleCollection = cms.untracked.InputTag("prunedGenParticles")
# process.genAna.GenParticleCollection = cms.untracked.InputTag("genParticles")
process.analysis = cms.Path(process.genAna)
process.schedule = cms.Schedule(process.analysis)

#process.output = cms.OutputModule("PoolOutputModule",
#    outputCommands = cms.untracked.vstring("keep *_*_*_ANASKIM", "keep *_*_generalTracks_*"),
#    fileName = cms.untracked.string('output.root'),
#)
#
#process.outputPath = cms.EndPath(process.output)
#process.schedule.append(process.outputPath)
