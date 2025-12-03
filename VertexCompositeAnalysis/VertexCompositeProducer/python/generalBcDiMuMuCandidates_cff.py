import FWCore.ParameterSet.Config as cms

# Import the base configuration
from VertexCompositeAnalysis.VertexCompositeProducer.generalBcDiMuMuCandidates_cfi import *

# ===== Complete Bc Reconstruction Sequence =====

# Sequence for Bc reconstruction with multiple configurations
bcDiMuMuCandidatesTask = cms.Task()

# Add the main Bc reconstruction
bcDiMuMuCandidatesTask.add(generalBcDiMuMuCandidates)

# ===== Analysis-Specific Sequences =====

# Sequence for trigger studies
bcDiMuMuTriggerTask = cms.Task(
    generalBcDiMuMuCandidatesTrigger
)

# Sequence for precision analysis
bcDiMuMuPrecisionTask = cms.Task(
    generalBcDiMuMuCandidatesPrecision  
)

# Sequence for systematic studies (multiple flip modes)
bcDiMuMuSystematicsTask = cms.Task()

# Create systematic variations with different flip modes
bcDiMuMuSystematics0 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 0)  # z-flip
bcDiMuMuSystematics1 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 1)  # x,z-flip  
bcDiMuMuSystematics2 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 2)  # y,z-flip
bcDiMuMuSystematics3 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 3)  # x,y,z-flip
bcDiMuMuSystematics4 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 4)  # z-flip only
bcDiMuMuSystematics5 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 5)  # no flip
bcDiMuMuSystematics6 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 6)  # x-flip
bcDiMuMuSystematics7 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 7)  # y-flip
bcDiMuMuSystematics8 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 8)  # x,y-flip

# Add systematics to task
bcDiMuMuSystematicsTask.add(
    bcDiMuMuSystematics0,
    bcDiMuMuSystematics1, 
    bcDiMuMuSystematics2,
    bcDiMuMuSystematics3,
    bcDiMuMuSystematics4,
    bcDiMuMuSystematics5,
    bcDiMuMuSystematics6,
    bcDiMuMuSystematics7,
    bcDiMuMuSystematics8
)

# ===== Collision System Specific Sequences =====

# Heavy ion sequence
bcDiMuMuPbPbTask = cms.Task(
    generalBcDiMuMuCandidatesPbPb
)

# Proton-proton sequence  
bcDiMuMuPPTask = cms.Task(
    generalBcDiMuMuCandidatesPP
)

# ===== Complete Analysis Sequences =====

# Full analysis sequence (standard + precision)
bcDiMuMuFullAnalysisTask = cms.Task(
    generalBcDiMuMuCandidates,
    generalBcDiMuMuCandidatesPrecision
)

# Complete sequence with systematics
bcDiMuMuCompleteTask = cms.Task(
    generalBcDiMuMuCandidates,
    generalBcDiMuMuCandidatesPrecision,
    bcDiMuMuSystematicsTask
)

# ===== Sequences (for backwards compatibility) =====

# Convert tasks to sequences for older CMSSW versions
bcDiMuMuCandidatesSequence = cms.Sequence(bcDiMuMuCandidatesTask)
bcDiMuMuTriggerSequence = cms.Sequence(bcDiMuMuTriggerTask)
bcDiMuMuPrecisionSequence = cms.Sequence(bcDiMuMuPrecisionTask)
bcDiMuMuSystematicsSequence = cms.Sequence(bcDiMuMuSystematicsTask)
bcDiMuMuPbPbSequence = cms.Sequence(bcDiMuMuPbPbTask)
bcDiMuMuPPSequence = cms.Sequence(bcDiMuMuPPTask)
bcDiMuMuFullAnalysisSequence = cms.Sequence(bcDiMuMuFullAnalysisTask)
bcDiMuMuCompleteSequence = cms.Sequence(bcDiMuMuCompleteTask)

# ===== Validation and Monitoring =====

# Validation sequence for testing
bcDiMuMuValidationTask = cms.Task()

# Add a loose configuration for validation
bcDiMuMuValidation = generalBcDiMuMuCandidates.clone(
    bcMassCut = 1.5,                    # Very loose for validation
    bcPtCut = 3.0,
    dimuonPtCut = 2.0,
    tkPtCut = 0.3,
    vtxProbCut = 0.001,
    rVtxSigCut = 1.0,
    pointingAngleCut = 0.5,
    saveDetailedInfo = True,            # Full info for validation
    saveTrimuonInfo = True,
)

bcDiMuMuValidationTask.add(bcDiMuMuValidation)
bcDiMuMuValidationSequence = cms.Sequence(bcDiMuMuValidationTask)

# ===== Output Module Helpers =====

# Define output collections for different scenarios
bcDiMuMuOutputCollections = [
    'keep *_generalBcDiMuMuCandidates_Bc_*',
]

bcDiMuMuOutputCollectionsPrecision = [
    'keep *_generalBcDiMuMuCandidates_Bc_*',
    'keep *_generalBcDiMuMuCandidatesPrecision_Bc_*',
    'keep *_generalBcDiMuMuCandidatesPrecision_Trimuon_*',
]

bcDiMuMuOutputCollectionsSystematics = [
    'keep *_generalBcDiMuMuCandidates_Bc_*',
    'keep *_bcDiMuMuSystematics*_Bc_*',
]

bcDiMuMuOutputCollectionsComplete = [
    'keep *_generalBcDiMuMuCandidates*_*_*',
    'keep *_bcDiMuMuSystematics*_*_*',
]

# ===== Common Customization Functions =====

def customizeBcForTrigger(process):
    """Customize Bc reconstruction for trigger studies"""
    if hasattr(process, 'generalBcDiMuMuCandidates'):
        process.generalBcDiMuMuCandidates.bcMassCut = 1.0
        process.generalBcDiMuMuCandidates.bcPtCut = 5.0
        process.generalBcDiMuMuCandidates.vtxProbCut = 0.005
        process.generalBcDiMuMuCandidates.saveDetailedInfo = False
    return process

def customizeBcForPrecision(process):
    """Customize Bc reconstruction for precision analysis"""
    if hasattr(process, 'generalBcDiMuMuCandidates'):
        process.generalBcDiMuMuCandidates.bcMassCut = 0.3
        process.generalBcDiMuMuCandidates.bcPtCut = 12.0
        process.generalBcDiMuMuCandidates.vtxProbCut = 0.05
        process.generalBcDiMuMuCandidates.saveDetailedInfo = True
        process.generalBcDiMuMuCandidates.saveTrimuonInfo = True
    return process

def customizeBcForSystematics(process):
    """Add systematic variations to Bc reconstruction"""
    # Add systematic variations if not already present
    if not hasattr(process, 'bcDiMuMuSystematics0'):
        process.bcDiMuMuSystematics0 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 0)
        process.bcDiMuMuSystematics1 = generalBcDiMuMuCandidatesSystematics.clone(jpsiFlipMode = 1)
        # Add more as needed...
    return process

def customizeBcForHeavyIon(process):
    """Customize Bc reconstruction for heavy ion collisions"""
    if hasattr(process, 'generalBcDiMuMuCandidates'):
        process.generalBcDiMuMuCandidates.vtxProbCut = 0.02
        process.generalBcDiMuMuCandidates.impactParameterCut = 2.5
        # Update input collections for heavy ions
        process.generalBcDiMuMuCandidates.vertexCollection = "hiSelectedVertex"
        process.generalBcDiMuMuCandidates.muonCollection = "hiMuons"
    return process

# ===== Era-Specific Configurations =====

# For Run 2 data/MC
def customizeBcForRun2(process):
    """Customize for Run 2 conditions"""
    # Run 2 typically has different collection names and slightly different cuts
    if hasattr(process, 'generalBcDiMuMuCandidates'):
        process.generalBcDiMuMuCandidates.vtxProbCut = 0.01
        process.generalBcDiMuMuCandidates.tkChi2Cut = 7.0
    return process

# For Run 3 data/MC  
def customizeBcForRun3(process):
    """Customize for Run 3 conditions"""
    # Run 3 may have improved tracking and different thresholds
    if hasattr(process, 'generalBcDiMuMuCandidates'):
        process.generalBcDiMuMuCandidates.vtxProbCut = 0.012
        process.generalBcDiMuMuCandidates.tkChi2Cut = 6.0
        process.generalBcDiMuMuCandidates.pointingAngleCut = 0.92
    return process