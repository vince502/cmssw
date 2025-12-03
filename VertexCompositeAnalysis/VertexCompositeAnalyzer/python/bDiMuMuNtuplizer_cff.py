import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeAnalyzer.bDiMuMuNtuplizer_cfi import *

# Standard B meson ntuplizer sequence
bDiMuMuNtuplizerSequence = cms.Sequence(bDiMuMuNtuplizer)

# B+ only ntuplizer
bPlusNtuplizer = bDiMuMuNtuplizer.clone(
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(False),
    saveBc = cms.bool(False)
)

# B0 only ntuplizer  
bZeroNtuplizer = bDiMuMuNtuplizer.clone(
    saveBPlus = cms.bool(False),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(False)
)

# Bc only ntuplizer
bcNtuplizer = bDiMuMuNtuplizer.clone(
    saveBPlus = cms.bool(False),
    saveBZero = cms.bool(False),
    saveBc = cms.bool(True)
)

# MC ntuplizer with generator matching
bDiMuMuNtuplizerMC = bDiMuMuNtuplizer.clone(
    doGenMatching = cms.bool(True),
    isMC = cms.bool(True),
    saveBc = cms.bool(True)  # Enable Bc for MC studies
)

# Minimal ntuplizer for quick studies (reduced info)
bDiMuMuNtuplizerMinimal = bDiMuMuNtuplizer.clone(
    saveFullInfo = cms.bool(False),
    doGenMatching = cms.bool(False)
)

# All B mesons ntuplizer including Bc
bDiMuMuNtuplizerAll = bDiMuMuNtuplizer.clone(
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(True)
)