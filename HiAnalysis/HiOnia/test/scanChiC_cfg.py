import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process('SCAN', eras.Run3_2025_OXY)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/s/soohwan/private/Analysis/MC/OxygenMC/CMSSW_15_0_11/src/RAW2DIGI_L1Reco_RECO_PAT.root'),
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_mcRun3_2025_forOO_realistic_v7', '')

# Simple analyzer to print gen particles
process.genDump = cms.EDAnalyzer("ParticleListDrawer",
    maxEventsToPrint = cms.untracked.int32(5),
    printOnlyHardInteraction = cms.untracked.bool(False),
    printVertex = cms.untracked.bool(True),
    src = cms.InputTag("prunedGenParticles")
)

process.p = cms.Path(process.genDump)
