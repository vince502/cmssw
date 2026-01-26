import FWCore.ParameterSet.Config as cms

process = cms.Process("GEN")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

# Input source - adjust this to your GEN-SIM file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:your_gen_sim_file.root'
    )
)

# TFileService for output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("genTrackTree.root")
)

# GenTrackNtuplizer
process.load("HeavyIonsAnalysis.TrackAnalysis.GenTrackNtuplizer_cfi")
process.genTrackNtuplizer.genParticleSrc = cms.InputTag("genParticles")
process.genTrackNtuplizer.onlyStable = cms.bool(True)      # Only stable particles (status == 1)
process.genTrackNtuplizer.onlyCharged = cms.bool(True)      # Only charged particles

# Path
process.p = cms.Path(process.genTrackNtuplizer)

# Schedule
process.schedule = cms.Schedule(process.p)
