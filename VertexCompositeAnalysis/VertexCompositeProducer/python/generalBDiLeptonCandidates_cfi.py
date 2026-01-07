import FWCore.ParameterSet.Config as cms

# Unified B meson producer supporting both dimuon and dielectron channels
# Input can be from HiOnia2MuMuPAT (dimuon) or HiOnia2EEPAT (dielectron)
# The producer automatically detects the lepton channel from the dilepton structure

generalBDiLeptonCandidates = cms.EDProducer("BDiLeptonProducer",
                                         
    # InputTag that tells which collections to use
    # For dimuon: use onia2MuMuPatGlbGlb or similar from HiOnia2MuMuPAT
    # For dielectron: use onia2ElectronElectronPAT from HiOnia2EEPAT
    dileptonCollection = cms.InputTag('onia2MuMuPatGlbGlb'),
    trackRecoAlgorithm = cms.InputTag('generalTracks'),
    vertexRecoAlgorithm = cms.InputTag('offlinePrimaryVertices'),

    trackQualities = cms.vstring('highPurity'),
                                         
    # Track selection cuts
    tkChi2Cut = cms.double(7.0),        # track Chi2/ndf <
    tkNhitsCut = cms.int32(11),         # track Nhits >=
    tkPtErrCut = cms.double(0.1),       # track pT error/pT <
    tkPtCut = cms.double(0.7),          # track pT >
    tkEtaCut = cms.double(2.4),         # track abs(eta) <
    tkDCACut = cms.double(1.0),         # PCA distance between tracks <
    tkDCACutLow = cms.double(0.0),      # PCA distance between tracks >

    # Dilepton selection cuts (applies to both dimuon and dielectron)
    dileptonMassMin = cms.double(2.9),    # dilepton mass >
    dileptonMassMax = cms.double(3.3),    # dilepton mass < (J/psi window)
    dileptonPtCut = cms.double(6.5),      # dilepton pT >
    dileptonYCut = cms.double(2.4),       # dilepton abs(y) <

    # B meson selection cuts
    bPlusMassCut = cms.double(0.5),     # |M(B+) - M(B+)_PDG| <
    bZeroMassCut = cms.double(0.5),     # |M(B0) - M(B0)_PDG| <
    bPtCut = cms.double(7.0),           # B meson pT >
    bYCut = cms.double(2.4),            # B meson abs(y) <

    # Bc meson selection cuts
    bcMassCut = cms.double(0.6),        # |M(Bc) - M(Bc)_PDG| <
    bcPtCut = cms.double(8.0),          # Bc meson pT >
    bcYCut = cms.double(2.4),           # Bc meson abs(y) <

    # Vertex quality cuts
    vtxChi2Cut = cms.double(10.0),      # vertex Chi2/ndf <
    vtxProbCut = cms.double(0.01),      # vertex probability >
    rVtxCut = cms.double(0.0),          # 2D decay length >
    rVtxSigCut = cms.double(2.0),       # 2D decay length significance >
    lVtxCut = cms.double(0.0),          # 3D decay length >
    lVtxSigCut = cms.double(2.0),       # 3D decay length significance >
    collinCut2D = cms.double(-2.0),     # cos(pointAngle2D) >
    collinCut3D = cms.double(-2.0),     # cos(pointAngle3D) >
    alphaCut = cms.double(999.0),       # pointAngle3D <
    alpha2DCut = cms.double(999.0),     # pointAngle2D <

    # Impact parameter cuts
    dauTransImpactSigCut = cms.double(1.0),  # daughter transverse impact significance >
    dauLongImpactSigCut = cms.double(1.0),   # daughter longitudinal impact significance >

    # K*0 reconstruction cuts (for B0 mode)
    kstarMassMin = cms.double(0.792),   # K*0 mass >
    kstarMassMax = cms.double(0.992),   # K*0 mass <
    kstarPtCut = cms.double(1.0),       # K*0 pT >

    # Decay mode flags
    isWrongSign = cms.bool(False),
    doJPsi = cms.bool(True),
    doPsi2S = cms.bool(False),
    doUpsilon = cms.bool(False),
    doBPlus = cms.bool(True),
    doBZero = cms.bool(True),
    doBc = cms.bool(False),

    # MVA 
    useAnyMVA = cms.bool(False),
)

# Clone for dielectron channel - just change the input collection
generalBDiElectronCandidates = generalBDiLeptonCandidates.clone(
    dileptonCollection = cms.InputTag('onia2ElectronElectronPAT'),
)

# Clone for loose selection (wider mass window, looser cuts)
generalBDiLeptonCandidatesLoose = generalBDiLeptonCandidates.clone(
    dileptonMassMin = cms.double(2.5),
    dileptonMassMax = cms.double(4.0),
    dileptonPtCut = cms.double(0.0),
    bPlusMassCut = cms.double(1.0),
    bZeroMassCut = cms.double(1.0),
    bcMassCut = cms.double(1.0),
    bPtCut = cms.double(0.0),
    vtxProbCut = cms.double(0.001),
    rVtxSigCut = cms.double(0.0),
    lVtxSigCut = cms.double(0.0),
)
