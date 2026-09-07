import FWCore.ParameterSet.Config as cms

eecAnalyzer = cms.EDAnalyzer('EECAnalyzer',
    jpsiSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    pfCandidateSrc = cms.InputTag("packedPFCandidates"),
    vertexSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    
    # J/psi mass window
    jpsiMassMin = cms.double(2.9),  # Signal window
    jpsiMassMax = cms.double(3.3),
    sidebandLowMin = cms.double(2.6),  # Lower sideband
    sidebandLowMax = cms.double(2.8),
    sidebandHighMin = cms.double(3.4),  # Upper sideband
    sidebandHighMax = cms.double(3.6),
    
    # PF candidate selection
    pfPtMin = cms.double(0.5),  # GeV
    pfAbsEtaMax = cms.double(2.4),
    pfDzMax = cms.double(0.2),  # cm
    pfDxyMax = cms.double(0.05),  # cm
    # Note: PU mitigation via fromPV() check for charged particles
    # Neutrals are included but are pileup-sensitive - consider removing for initial analysis
    
    # Prompt/nonprompt separation
    ctauCut = cms.double(0.03),  # cm, prompt if ctau < cut
    separatePromptNonprompt = cms.bool(True),
    
    # EEC binning
    nCosChiBins = cms.int32(50),
    cosChiMin = cms.double(-1.0),
    cosChiMax = cms.double(1.0),
    
    # J/psi pT and rapidity bins
    jpsiPtBins = cms.vdouble(6.0, 10.0, 15.0, 30.0, 50.0),
    jpsiRapBins = cms.vdouble(-2.4, -1.2, 0.0, 1.2, 2.4),
    
    # Options
    removeJpsiMuons = cms.bool(True),  # Remove the two muons from J/psi
)
