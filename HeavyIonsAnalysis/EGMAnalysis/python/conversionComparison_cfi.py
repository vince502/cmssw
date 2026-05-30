import FWCore.ParameterSet.Config as cms

conversionComparisonAnalyzer = cms.EDAnalyzer('ConversionComparisonAnalyzer',
    gsfOpenConversions = cms.InputTag("gsfTracksOpenConversions", "gsfTracksOpenConversions"),
    reducedConversions = cms.InputTag("reducedEgamma", "reducedConversions"),
    reducedSingleLegConversions = cms.InputTag("reducedEgamma", "reducedSingleLegConversions"),
    oniaConversions = cms.InputTag("oniaPhotonCandidates", "conversions"),
)
