import FWCore.ParameterSet.Config as cms

# Bc meson reconstruction from J/ψ + muon + neutrino (semi-leptonic decay)
# Physics process: Bc+ → J/ψ(→ μ+μ-) + μ+ + ν_μ
# Uses functional approach with neutrino momentum inference

generalBcDiMuMuCandidates = cms.EDProducer("BcDiMuMuProducer",

    # ===== Input Collections =====
    
    # Primary vertex collection
    vertexCollection = cms.InputTag("unpackedTracksAndVertices"),
    
    # Beam spot for vertex constraint
    beamSpotCollection = cms.InputTag("offlineBeamSpot"),
    
    # J/ψ candidates (dimuon collection)
    dimuonCollection = cms.InputTag("onia2MuMuPatTrkTrk:JpsiTrkTrk"),
    
    # Additional muon collection for the third muon
    muonCollection = cms.InputTag("unpackedMuons"),

    # ===== Physics Selection Criteria =====
    
    # Bc meson cuts
    bcMassCut = cms.double(0.6),        # |M(Bc) - M(Bc)_PDG| < 0.6 GeV/c²
    bcPtCut = cms.double(8.0),          # Bc pT > 8.0 GeV/c
    bcYCut = cms.double(2.4),           # |y(Bc)| < 2.4
    
    # J/ψ (dimuon) selection
    dimuonMassMin = cms.double(2.9),    # J/ψ mass window: 2.9 < M(μμ) < 3.3 GeV/c²
    dimuonMassMax = cms.double(3.3),
    dimuonPtCut = cms.double(6.5),      # J/ψ pT > 6.5 GeV/c
    
    # Additional muon (third muon) cuts
    tkPtCut = cms.double(0.7),          # Track pT > 0.7 GeV/c
    tkChi2Cut = cms.double(7.0),        # Track χ²/ndf < 7.0
    
    # Vertex quality cuts (tighter for Bc due to short lifetime)
    vtxProbCut = cms.double(0.01),      # Vertex probability > 1%
    
    # Decay length significance cuts
    rVtxSigCut = cms.double(2.0),       # 2D decay length significance > 2σ
    rVtx3DSigCut = cms.double(2.0),     # 3D decay length significance > 2σ
    
    # Pointing angle cut (cosine of 3D pointing angle)
    pointingAngleCut = cms.double(0.9), # cos(pointing angle) > 0.9
    
    # Impact parameter significance cut
    impactParameterCut = cms.double(3.0), # IP significance < 3σ

    # ===== Systematic Studies and Analysis Control =====
    
    # J/ψ flipping for systematic uncertainty studies
    doJpsiFlip = cms.bool(False),       # Enable prong rotation studies
    jpsiFlipMode = cms.int32(0),        # Flip mode: 0-8 (see BcDiMuMuFunctional.h)
    
    # Vertex fitting options
    useBeamSpotConstraint = cms.bool(True),  # Use beam spot constraint in vertex fit
    
    # Analysis detail level
    saveDetailedInfo = cms.bool(True),       # Save detailed analysis variables
    saveTrimuonInfo = cms.bool(False),       # Save intermediate trimuon candidates

    # ===== Output Configuration =====
    
    # Collection labels for output
    bcLabel = cms.string("Bc"),              # Bc candidate collection
    trimuonLabel = cms.string("Trimuon"),    # Trimuon collection (if saved)

    # ===== Advanced Physics Parameters =====
    
    # These parameters fine-tune the physics algorithms
    # Based on OniaTreeSubmodule implementation and Bc physics
    
    # Mass constraints for kinematic fitting (if used)
    # jpsiMassConstraint = cms.double(3.096916),  # J/ψ PDG mass
    # muonMassConstraint = cms.double(0.1056583745), # Muon PDG mass
    
    # Neutrino momentum inference parameters
    # (Currently built into the algorithm, could be made configurable)
    
    # Selection optimization for different analysis scenarios
    # These can be adjusted based on specific analysis needs:
    
    # For trigger studies (looser cuts)
    # bcMassCut = 1.0, bcPtCut = 5.0, vtxProbCut = 0.005
    
    # For precision measurements (tighter cuts)  
    # bcMassCut = 0.3, bcPtCut = 12.0, vtxProbCut = 0.05
    
    # For systematic studies (medium cuts with variations)
    # Use jpsiFlipMode = 0-8 for different rotation patterns
)

# ===== Specialized Configurations =====

# Configuration for trigger-level reconstruction (loose cuts)
generalBcDiMuMuCandidatesTrigger = generalBcDiMuMuCandidates.clone(
    bcMassCut = 1.0,                    # Wider mass window for trigger
    bcPtCut = 5.0,                      # Lower pT threshold
    dimuonPtCut = 4.0,                  # Lower J/ψ pT
    tkPtCut = 0.5,                      # Lower track pT
    vtxProbCut = 0.005,                 # Looser vertex quality
    rVtxSigCut = 1.5,                   # Looser decay length
    pointingAngleCut = 0.8,             # Looser pointing angle
    saveDetailedInfo = False,           # Save space at trigger level
)

# Configuration for precision analysis (tight cuts)
generalBcDiMuMuCandidatesPrecision = generalBcDiMuMuCandidates.clone(
    bcMassCut = 0.3,                    # Narrow mass window
    bcPtCut = 12.0,                     # Higher pT threshold
    dimuonPtCut = 8.0,                  # Higher J/ψ pT
    tkPtCut = 1.0,                      # Higher track pT
    vtxProbCut = 0.05,                  # Tighter vertex quality
    rVtxSigCut = 3.0,                   # Tighter decay length
    rVtx3DSigCut = 3.0,
    pointingAngleCut = 0.95,            # Tighter pointing angle
    tkChi2Cut = 5.0,                    # Better track quality
    saveDetailedInfo = True,            # Full information
    saveTrimuonInfo = True,             # Include trimuon info
)

# Configuration for systematic studies
generalBcDiMuMuCandidatesSystematics = generalBcDiMuMuCandidates.clone(
    doJpsiFlip = True,                  # Enable systematic variations
    jpsiFlipMode = 0,                   # Default: z-flip only
    saveDetailedInfo = True,            # Need full info for systematics
)

# Systematic variation configurations
# These can be used in sequence to study systematic uncertainties
bcSystematicConfigs = []
for flipMode in range(9):  # 0-8 flip modes
    config = generalBcDiMuMuCandidatesSystematics.clone(
        jpsiFlipMode = flipMode
    )
    bcSystematicConfigs.append(config)

# ===== Heavy Ion Specific Configurations =====

# Configuration optimized for PbPb collisions
generalBcDiMuMuCandidatesPbPb = generalBcDiMuMuCandidates.clone(
    # Adjust for higher multiplicity environment
    vtxProbCut = 0.02,                  # Slightly tighter vertex quality
    impactParameterCut = 2.5,           # Tighter IP cut for high multiplicity
    
    # Use different collections appropriate for heavy ions
    vertexCollection = "hiSelectedVertex",
    muonCollection = "hiMuons",
    
    # Potentially different input for heavy ion dimuons
    # dimuonCollection = "onia2MuMuPatTrkTrkHI:JpsiTrkTrk",
)

# Configuration optimized for pp collisions  
generalBcDiMuMuCandidatesPP = generalBcDiMuMuCandidates.clone(
    # Standard settings should work well for pp
    # May allow slightly looser cuts due to cleaner environment
    vtxProbCut = 0.008,
    impactParameterCut = 3.5,
)