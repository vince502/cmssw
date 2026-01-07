"""
Configuration for BToJpsiHadronsNtuplizer
Unified ntuplizer for B meson candidates from BToJpsiHadronsProducer

Output tree structure (one entry per event, vectors for candidates):
  Event info:
    - run, lumi, event, nPV, pvX, pvY, pvZ, nCand
    - centrality, Ntrkoffline (if enabled)
  
  B candidate (vectors):
    - B_mass, B_pt, B_eta, B_phi, B_y, B_charge
    - B_decayId, B_nTracks, B_hasDau2
    - B_vtxX, B_vtxY, B_vtxZ, B_vtxChi2, B_vtxNdof, B_vtxProb
    - B_lxy, B_lxySig, B_l3D, B_l3DSig, B_cosAlpha
  
  J/psi info:
    - jpsiIdx (for matching with hionia tree via Reco_QQ_collIdx)
    - Jpsi_mass, Jpsi_pt, Jpsi_eta, Jpsi_phi, Jpsi_y
  
  Resonance mass:
    - resMass (phi, K*, or pipi mass depending on decay mode)
  
  Hadron 1:
    - trk1_pt, trk1_eta, trk1_phi, trk1_charge, trk1_massHypo, trk1_idx
  
  Hadron 2 (filled with -999 for 1-track modes):
    - trk2_pt, trk2_eta, trk2_phi, trk2_charge, trk2_massHypo, trk2_idx

Usage:
  # In your CMSSW config:
  from VertexCompositeAnalysis.VertexCompositeAnalyzer.bToJpsiHadronsNtuplizer_cfi import bToJpsiHadronsNtuplizer
  process.bTree = bToJpsiHadronsNtuplizer.clone()
"""

import FWCore.ParameterSet.Config as cms

bToJpsiHadronsNtuplizer = cms.EDAnalyzer("BToJpsiHadronsNtuplizer",
    # Input
    candidateSrc = cms.InputTag("bToJpsiHadrons", "BToJpsiHadrons"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    
    # Options
    saveTree = cms.untracked.bool(True),
    isCentrality = cms.untracked.bool(False),
    
    # Centrality (only used if isCentrality = True)
    centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
    centralitySrc = cms.InputTag("hiCentrality")
)

# For mu+mu- channel
bToJpsiHadronsMuMuNtuplizer = bToJpsiHadronsNtuplizer.clone(
    candidateSrc = cms.InputTag("bToJpsiHadronsMuMu", "BToJpsiHadrons")
)

# For e+e- channel
bToJpsiHadronsEENtuplizer = bToJpsiHadronsNtuplizer.clone(
    candidateSrc = cms.InputTag("bToJpsiHadronsEE", "BToJpsiHadrons")
)

# Heavy ion configuration
bToJpsiHadronsNtuplizerHI = bToJpsiHadronsNtuplizer.clone(
    isCentrality = cms.untracked.bool(True)
)
