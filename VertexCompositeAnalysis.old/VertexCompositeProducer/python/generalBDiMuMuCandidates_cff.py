import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeProducer.generalBDiMuMuCandidates_cfi import *

# Standard B meson reconstruction sequence
bDiMuMuSequence = cms.Sequence(generalBDiMuMuCandidates)

# B+ only reconstruction
generalBPlusCandidates = generalBDiMuMuCandidates.clone(
    doBPlus = cms.bool(True),
    doBZero = cms.bool(False),
    doBc = cms.bool(False)
)

# B0 only reconstruction  
generalBZeroCandidates = generalBDiMuMuCandidates.clone(
    doBPlus = cms.bool(False),
    doBZero = cms.bool(True),
    doBc = cms.bool(False)
)

# Bc only reconstruction
generalBcCandidates = generalBDiMuMuCandidates.clone(
    doBPlus = cms.bool(False),
    doBZero = cms.bool(False),
    doBc = cms.bool(True),
    bcPtCut = cms.double(8.0),          # Higher pT for Bc due to larger mass
    vtxProbCut = cms.double(0.02),      # Tighter vertex quality for Bc
    rVtxSigCut = cms.double(1.5),       # Looser for shorter Bc lifetime
    lVtxSigCut = cms.double(1.5)
)

# Tight B meson selection for high purity
generalBDiMuMuCandidatesTight = generalBDiMuMuCandidates.clone(
    tkPtCut = cms.double(1.0),          # Higher track pT
    dimuonPtCut = cms.double(8.0),      # Higher dimuon pT  
    bPtCut = cms.double(10.0),          # Higher B meson pT
    vtxProbCut = cms.double(0.05),      # Stricter vertex probability
    rVtxSigCut = cms.double(3.0),       # Stricter decay length significance
    lVtxSigCut = cms.double(3.0),
    dauTransImpactSigCut = cms.double(2.0),  # Stricter impact parameter cuts
    dauLongImpactSigCut = cms.double(2.0)
)

# Loose B meson selection for efficiency studies
generalBDiMuMuCandidatesLoose = generalBDiMuMuCandidates.clone(
    tkPtCut = cms.double(0.5),          # Lower track pT
    dimuonPtCut = cms.double(5.0),      # Lower dimuon pT
    bPtCut = cms.double(5.0),           # Lower B meson pT
    vtxProbCut = cms.double(0.001),     # Looser vertex probability
    rVtxSigCut = cms.double(1.0),       # Looser decay length significance
    lVtxSigCut = cms.double(1.0),
    dauTransImpactSigCut = cms.double(0.5),  # Looser impact parameter cuts
    dauLongImpactSigCut = cms.double(0.5)
)

# Configuration for psi(2S) studies
generalBDiMuMuCandidatesPsi2S = generalBDiMuMuCandidates.clone(
    doJPsi = cms.bool(False),
    doPsi2S = cms.bool(True),
    dimuonMassMin = cms.double(3.6),    # psi(2S) mass window
    dimuonMassMax = cms.double(3.8),
    dimuonPtCut = cms.double(5.0)       # Lower pT for psi(2S)
)

# Configuration for Upsilon studies  
generalBDiMuMuCandidatesUpsilon = generalBDiMuMuCandidates.clone(
    doJPsi = cms.bool(False),
    doUpsilon = cms.bool(True),
    dimuonMassMin = cms.double(9.0),    # Upsilon mass window
    dimuonMassMax = cms.double(10.0),
    dimuonPtCut = cms.double(4.0),      # Lower pT for Upsilon
    bPlusMassCut = cms.double(1.0),     # Wider B mass window for Upsilon
    bZeroMassCut = cms.double(1.0)
)