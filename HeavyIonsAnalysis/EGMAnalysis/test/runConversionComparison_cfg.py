import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("ConvCompare")

# Command line options
options = VarParsing.VarParsing('analysis')
options.inputFiles = '/store/hidata/OORun2025/IonPhysics0/USER/IonDimuon-PromptReco-v1/000/394/169/00000/2d344473-ed2c-4981-a76b-ed74374e660d.root'  # Replace with your file
options.maxEvents = -1
options.outputFile = 'conversionComparison.root'
options.parseArguments()

# Standard configuration
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

# TFileService for output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# Load the analyzer
process.load("HeavyIonsAnalysis.EGMAnalysis.conversionComparison_cfi")

# Default configuration uses:
# - gsfTracksOpenConversions:gsfTracksOpenConversions
# - reducedEgamma:reducedConversions  
# - reducedEgamma:reducedSingleLegConversions
# - oniaPhotonCandidates:conversions

# You can override if needed:
# process.conversionComparisonAnalyzer.gsfOpenConversions = cms.InputTag("gsfTracksOpenConversions", "gsfTracksOpenConversions")
# process.conversionComparisonAnalyzer.reducedConversions = cms.InputTag("reducedEgamma", "reducedConversions")
# process.conversionComparisonAnalyzer.reducedSingleLegConversions = cms.InputTag("reducedEgamma", "reducedSingleLegConversions")
# process.conversionComparisonAnalyzer.oniaConversions = cms.InputTag("oniaPhotonCandidates", "conversions")

process.p = cms.Path(process.conversionComparisonAnalyzer)
