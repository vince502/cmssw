"""
Configuration for BToJpsiHadronsProducer
Unified B meson producer for multiple decay modes

Decay modes (decayId):
  0 = B+  -> J/psi K+    (1 hadron, K mass hypothesis)
  1 = B+  -> J/psi pi+   (1 hadron, pi mass hypothesis)  
  2 = B0  -> J/psi K pi  (2 hadrons, K* -> K pi)
  3 = Bs  -> J/psi K K   (2 hadrons, phi -> K K)
  4 = Bc+ -> J/psi pi+   (1 hadron, pi mass hypothesis, Bc mass window)
  5 = X   -> J/psi pi pi (2 hadrons, generic X resonance)

Output branch structure:
  - B_mass, B_pt, B_eta, B_phi, B_y
  - B_vtxX, B_vtxY, B_vtxZ, B_vtxChi2, B_vtxNdof, B_vtxProb
  - B_lxy, B_lxySig, B_l3D, B_l3DSig, B_cosAlpha
  - B_decayId, B_nTracks, B_hasDau2
  - jpsiIdx (for matching with hionia tree)
  - trk1_pt, trk1_eta, trk1_phi, trk1_charge, trk1_massHypo
  - trk2_pt, trk2_eta, trk2_phi, trk2_charge, trk2_massHypo (filled with -999 for 1-track modes)
"""

import FWCore.ParameterSet.Config as cms

bToJpsiHadrons = cms.EDProducer("BToJpsiHadronsProducer",
    # Input collections
    jpsiSrc = cms.InputTag("onia2MuMuPAT"),           # J/psi candidates (mu+mu- or e+e-)
    trackSrc = cms.InputTag("packedPFCandidates"),   # Charged tracks
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    # Track selection cuts
    minTrackPt = cms.double(0.8),         # Minimum track pT [GeV]
    maxTrackEta = cms.double(2.4),        # Maximum |eta|
    minTrackDxySig = cms.double(0.0),     # Minimum dxy significance
    minTrackDzSig = cms.double(0.0),      # Minimum dz significance
    minTrackNhits = cms.int32(3),         # Minimum number of hits
    requireHighPurity = cms.bool(True),   # Require high purity tracks
    
    # J/psi selection cuts (mass window applied early to save combinatorics time)
    minJpsiPt = cms.double(3.0),          # Minimum J/psi pT [GeV]
    maxJpsiAbsY = cms.double(2.4),        # Maximum |y|
    jpsiMassMin = cms.double(2.7),        # J/psi mass window min [GeV]
    jpsiMassMax = cms.double(3.4),        # J/psi mass window max [GeV]
    
    # B candidate selection cuts
    minBPt = cms.double(5.0),             # Minimum B pT [GeV]
    maxBAbsY = cms.double(2.4),           # Maximum |y|
    minVtxProb = cms.double(0.001),       # Minimum vertex probability
    
    # Resonance mass windows [GeV]
    phiMassWindow = cms.double(0.015),    # |m(KK) - m(phi)| < 15 MeV
    kstarMassWindow = cms.double(0.100),  # |m(Kpi) - m(K*)| < 100 MeV
    xPiPiMassMin = cms.double(0.0),       # Minimum m(pipi) for X resonance (Q2 cut)
    xPiPiMassMax = cms.double(1.5),       # Maximum m(pipi) for X resonance
    
    # B mass windows around PDG values [GeV]
    bPlusMassWindow = cms.double(0.5),    # for B+ -> J/psi K and B+ -> J/psi pi
    b0MassWindow = cms.double(0.5),       # for B0 -> J/psi K*
    bsMassWindow = cms.double(0.5),       # for Bs -> J/psi phi
    bcMassWindow = cms.double(0.5),       # for Bc -> J/psi pi
    xMassMin = cms.double(3.5),           # for X -> J/psi pipi
    xMassMax = cms.double(10.0),          # for X -> J/psi pipi
    
    # Enable/disable decay modes
    doBplusToJpsiK = cms.bool(True),      # Mode 0: B+ -> J/psi K+
    doBplusToJpsiPi = cms.bool(False),    # Mode 1: B+ -> J/psi pi+
    doB0ToJpsiKPi = cms.bool(True),       # Mode 2: B0 -> J/psi K pi (K*)
    doBsToJpsiKK = cms.bool(True),        # Mode 3: Bs -> J/psi K K (phi)
    doBcToJpsiPi = cms.bool(False),       # Mode 4: Bc+ -> J/psi pi+
    doXToJpsiPiPi = cms.bool(False),      # Mode 5: X -> J/psi pi pi
    
    # Other options
    doVertexFit = cms.bool(True),         # Perform vertex fit
    maxDeltaR_overlap = cms.double(0.01)  # dR threshold for track overlap removal
)

# Configuration for J/psi -> e+e- channel
bToJpsiHadronsEE = bToJpsiHadrons.clone(
    jpsiSrc = cms.InputTag("onia2EEPAT")
)

# Minimal configuration for B+ only
bPlusToJpsiK = bToJpsiHadrons.clone(
    doBplusToJpsiK = cms.bool(True),
    doBplusToJpsiPi = cms.bool(False),
    doB0ToJpsiKPi = cms.bool(False),
    doBsToJpsiKK = cms.bool(False),
    doBcToJpsiPi = cms.bool(False),
    doXToJpsiPiPi = cms.bool(False)
)

# Heavy Ion configuration (tighter cuts)
bToJpsiHadronsHI = bToJpsiHadrons.clone(
    minTrackPt = cms.double(1.0),
    minJpsiPt = cms.double(5.0),
    minBPt = cms.double(8.0),
    minVtxProb = cms.double(0.01)
)


def configureBToJpsiHadrons(process, isMC=False, isHI=False, doMuMu=True, doEE=True):
    """
    Configure B meson reconstruction in a CMSSW process
    
    Args:
        process: cms.Process object
        isMC: bool, True for MC samples
        isHI: bool, True for heavy ion collisions
        doMuMu: bool, enable J/psi -> mu+mu- channel
        doEE: bool, enable J/psi -> e+e- channel
    
    Returns:
        Sequence of B meson producers
    """
    
    producers = []
    
    if isHI:
        baseConfig = bToJpsiHadronsHI
    else:
        baseConfig = bToJpsiHadrons
    
    if doMuMu:
        process.bToJpsiHadronsMuMu = baseConfig.clone(
            jpsiSrc = cms.InputTag("onia2MuMuPAT")
        )
        producers.append(process.bToJpsiHadronsMuMu)
    
    if doEE:
        process.bToJpsiHadronsEE = baseConfig.clone(
            jpsiSrc = cms.InputTag("onia2EEPAT")
        )
        producers.append(process.bToJpsiHadronsEE)
    
    if len(producers) == 0:
        return cms.Sequence()
    elif len(producers) == 1:
        return cms.Sequence(producers[0])
    else:
        return cms.Sequence(producers[0] + producers[1])
