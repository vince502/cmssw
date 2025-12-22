### Validation config for TriggerAnalyzer2 and UnpackedTrackComparisonAnalyzer
import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('Validation', eras.Run3_2025)

process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v3', '')

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/hidata/HIRun2025A/HIPhysicsRawPrime12/MINIAOD/PromptReco-v1/000/399/766/00000/20708bc3-28e5-469a-a92d-51f39fcb6021.root',
    ),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("BmesonPbPb2025_validation.root")
)

# Original TriggerAnalyzer
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')

# Filtered TriggerAnalyzer2
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis2_cfi')
from HeavyIonsAnalysis.EventAnalysis.hltanalysis2_cfi import configureOniaJetTriggers
configureOniaJetTriggers(process.hltanalysis2)

# Track unpacker
process.load('HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff')

# UnpackedTrackComparisonAnalyzer
process.load('HeavyIonsAnalysis.TrackAnalysis.unpackedTrackComparison_cfi')

# Path
process.validation = cms.Path(
    process.hltanalysis +
    process.hltanalysis2 +
    process.unpackedTracksAndVertices +
    process.unpackedTrackComparison
)

process.schedule = cms.Schedule(process.validation)
