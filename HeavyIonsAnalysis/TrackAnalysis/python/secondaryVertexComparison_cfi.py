import FWCore.ParameterSet.Config as cms

secondaryVertexComparison = cms.EDAnalyzer('SecondaryVertexComparisonAnalyzer',
    # Collection 1: MiniAOD slimmed SVs (pre-made)
    svSrc1 = cms.InputTag("slimmedSecondaryVertices"),
    
    # Collection 2: Custom re-made SVs
    svSrc2 = cms.InputTag("inclusiveCandidateSecondaryVertices"),
    
    # Primary vertex for dxy calculation
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    
    # Labels for histogram titles
    label1 = cms.string("slimmedSV"),
    label2 = cms.string("customSV"),
    
    # Matching parameters
    maxDeltaR = cms.double(0.3),  # Maximum deltaR for SV matching
    maxDeltaZ = cms.double(1.0),  # Maximum deltaZ for SV matching [cm]
)
