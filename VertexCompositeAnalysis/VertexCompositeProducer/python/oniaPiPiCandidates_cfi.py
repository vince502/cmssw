"""
Configuration for OniaPiPiProducer
Producer for Onia + Pi+ Pi- and Onia + e+ e- (conversion) candidates

Reconstructs:
  - X(3872) -> J/psi + pi+ pi-  (mass ~ 3.872 GeV)
  - psi(2S) -> J/psi + pi+ pi-  (mass ~ 3.686 GeV)
  - chi_c -> J/psi + gamma -> J/psi + e+ e- (conversion from general tracks)

Input:
  - oniaSrc: J/psi candidates (pat::CompositeCandidate)
  - trackSrc: General tracks for pions/electrons
  - vertexSrc: Primary vertices

Output:
  - OniaPiPi: J/psi + pi+ pi- candidates (pat::CompositeCandidateCollection)
  - OniaEE: J/psi + e+ e- candidates (pat::CompositeCandidateCollection)
"""

import FWCore.ParameterSet.Config as cms

# Base configuration for X(3872) / psi(2S) -> J/psi + pi+ pi-
oniaPiPiCandidates = cms.EDProducer("OniaPiPiProducer",
    # Input collections
    oniaSrc = cms.InputTag("onia2MuMuPAT"),
    trackSrc = cms.InputTag("generalTracks"),
    vertexSrc = cms.InputTag("offlinePrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    dedxSrc = cms.InputTag(""),  # Empty = disabled; for MiniAOD: "dedxHarmonic2"
    
    # Mode selection
    doPiPi = cms.bool(True),        # X(3872), psi(2S) -> J/psi pi+ pi-
    doConversion = cms.bool(False), # chi_c -> J/psi gamma -> J/psi e+ e-
    
    # Onia cuts
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts
    tkPtCut = cms.double(0.3),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(5.0),
    tkNhitsCut = cms.int32(5),
    tkPtErrCut = cms.double(0.1),
    tkDCACut = cms.double(1.0),
    
    # Di-track cuts (pi+ pi-)
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.2),   # rho(770) -> pi+ pi-: mass > 0.2 GeV
    ditrackMassMax = cms.double(1.0),   # Upper limit for pi+ pi-
    
    # Combined candidate cuts for X(3872) / psi(2S)
    candMassMin = cms.double(3.5),      # Below psi(2S)
    candMassMax = cms.double(4.0),      # Above X(3872)
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts
    vtxChi2Cut = cms.double(0.001),     # Probability cut
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.0),
    alpha2DCut = cms.double(1.0),
    
    # Conversion mode specific (not used in pipi mode)
    convRadiusMin = cms.double(0.0),
    convRadiusMax = cms.double(100.0),
    
    # Wrong-sign option
    isWrongSign = cms.bool(False),
)

# Wrong-sign for background estimation
oniaPiPiCandidatesWS = oniaPiPiCandidates.clone(
    isWrongSign = cms.bool(True),
)

# chi_c -> J/psi + gamma -> J/psi + e+ e- (conversion from general tracks)
oniaConversionCandidates = cms.EDProducer("OniaPiPiProducer",
    # Input collections
    oniaSrc = cms.InputTag("onia2MuMuPAT"),
    trackSrc = cms.InputTag("generalTracks"),
    vertexSrc = cms.InputTag("offlinePrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    dedxSrc = cms.InputTag(""),  # Empty = disabled; for MiniAOD: "dedxHarmonic2"
    
    # Mode selection
    doPiPi = cms.bool(False),
    doConversion = cms.bool(True),   # chi_c -> J/psi gamma -> J/psi e+ e-
    
    # Onia cuts
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts (electrons from conversion)
    tkPtCut = cms.double(0.2),       # Lower pT for soft electrons
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(10.0),    # Looser chi2 for electrons
    tkNhitsCut = cms.int32(3),       # Fewer hits for conversions
    tkPtErrCut = cms.double(0.2),
    tkDCACut = cms.double(0.5),      # Tighter DCA for conversion vertex
    
    # Di-track cuts (e+ e- = photon)
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.0),   # Photon mass ~ 0
    ditrackMassMax = cms.double(0.1),   # Very low mass for conversion
    
    # Combined candidate cuts for chi_c
    candMassMin = cms.double(3.3),      # chi_c0 ~ 3.41 GeV
    candMassMax = cms.double(3.7),      # chi_c2 ~ 3.56 GeV
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts
    vtxChi2Cut = cms.double(0.001),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.0),
    alpha2DCut = cms.double(1.0),
    
    # Conversion mode specific
    convRadiusMin = cms.double(1.0),    # Min conversion radius (avoid beampipe)
    convRadiusMax = cms.double(50.0),   # Max conversion radius (in tracker)
    
    # Wrong-sign option
    isWrongSign = cms.bool(False),
)

# Wrong-sign for background estimation
oniaConversionCandidatesWS = oniaConversionCandidates.clone(
    isWrongSign = cms.bool(True),
)

# Combined producer for both modes
oniaPiPiAndConversionCandidates = cms.EDProducer("OniaPiPiProducer",
    # Input collections
    oniaSrc = cms.InputTag("onia2MuMuPAT"),
    trackSrc = cms.InputTag("generalTracks"),
    vertexSrc = cms.InputTag("offlinePrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    dedxSrc = cms.InputTag(""),  # Empty = disabled; for MiniAOD: "dedxHarmonic2"
    
    # Mode selection - both modes enabled
    doPiPi = cms.bool(True),
    doConversion = cms.bool(True),
    
    # Onia cuts
    minOniaPt = cms.double(0.0),
    maxOniaAbsY = cms.double(2.4),
    
    # Track cuts (common for both modes)
    tkPtCut = cms.double(0.2),
    tkEtaCut = cms.double(2.4),
    tkChi2Cut = cms.double(5.0),
    tkNhitsCut = cms.int32(3),
    tkPtErrCut = cms.double(0.2),
    tkDCACut = cms.double(1.0),
    
    # Di-track cuts (wider range for both pi+pi- and e+e-)
    ditrackPtCut = cms.double(0.0),
    ditrackMassMin = cms.double(0.0),
    ditrackMassMax = cms.double(1.0),
    
    # Combined candidate cuts (covers both chi_c and X(3872))
    candMassMin = cms.double(3.3),
    candMassMax = cms.double(4.0),
    candPtCut = cms.double(0.0),
    candAbsYCut = cms.double(2.4),
    
    # Vertex cuts
    vtxChi2Cut = cms.double(0.001),
    vtxSignif3DCut = cms.double(0.0),
    alphaCut = cms.double(1.0),
    alpha2DCut = cms.double(1.0),
    
    # Conversion mode specific
    convRadiusMin = cms.double(1.0),
    convRadiusMax = cms.double(50.0),
    
    # Wrong-sign option
    isWrongSign = cms.bool(False),
)

# For MiniAOD with unpacked tracks
oniaPiPiCandidatesMiniAOD = oniaPiPiCandidates.clone(
    trackSrc = cms.InputTag("unpackedTracksAndVertices"),
    vertexSrc = cms.InputTag("unpackedTracksAndVertices"),
    dedxSrc = cms.InputTag("dedxHarmonic2"),  # Enable dEdx for MiniAOD
)

oniaConversionCandidatesMiniAOD = oniaConversionCandidates.clone(
    trackSrc = cms.InputTag("unpackedTracksAndVertices"),
    vertexSrc = cms.InputTag("unpackedTracksAndVertices"),
    dedxSrc = cms.InputTag("dedxHarmonic2"),  # Enable dEdx for MiniAOD
)
