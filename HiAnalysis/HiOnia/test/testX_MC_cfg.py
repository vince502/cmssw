### X(3872) MC test: onia2MuMuPAT + hionia + X -> J/psi + pi+pi- producer + ntuplizer
# Usage: cmsRun testX_MC_cfg.py maxEvents=100
#
# For OO MC sample with X(3872) -> J/psi(mumu) + pi+ pi-
# Based on testChiC_MC_cfg.py structure

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

#----------------------------------------------------------------------------
# Settings
#----------------------------------------------------------------------------
HLTProcess     = "HLT"
isMC           = True
muonSelection  = "GlbOrTrk"
OnlySoftMuons  = False
applyCuts      = False
atLeastOneCand = False
miniAOD        = True
pdgId          = 443  # J/Psi
useMomFormat   = "vector"

#----------------------------------------------------------------------------
# Process
#----------------------------------------------------------------------------
process = cms.Process("XMC", eras.Run3_2025_OXY)

# Command line options
options = VarParsing.VarParsing('analysis')
options.outputFile = "X3872_MC.root"
options.inputFiles = [
    'file:/afs/cern.ch/work/s/soohwan/private/Analysis/MC/OxygenMC/CMSSW_15_0_11/src/RAW2DIGI_L1Reco_RECO_PAT.root',
]
options.maxEvents = -1
options.parseArguments()

# Trigger list (empty for MC)
triggerList = {
    'DoubleMuonTrigger': cms.vstring(),
    'SingleMuonTrigger': cms.vstring(),
}

#----------------------------------------------------------------------------
# Load standard sequences
#----------------------------------------------------------------------------
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_mcRun3_2025_forOO_realistic_v7', '')

#----------------------------------------------------------------------------
# TransientTrack Builder (needed for vertex fitting)
#----------------------------------------------------------------------------
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

#----------------------------------------------------------------------------
# Onia Tree Analyzer
#----------------------------------------------------------------------------
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
    muonTriggerList=triggerList,
    muonSelection=muonSelection,
    L1Stage=2,
    isMC=isMC,
    pdgID=pdgId,
    outputFileName=options.outputFile,
    doTrimu=False,
)

# Dimuon selection for J/psi
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
    "mass > 2.5 && mass < 4.0 && charge==0"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.4")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.001")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

# HiOnia settings
process.hionia.applyCuts = cms.bool(applyCuts)
process.hionia.AtLeastOneCand = cms.bool(atLeastOneCand)
process.hionia.checkTrigNames = cms.bool(False)
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)
process.hionia.genealogyInfo = cms.bool(True)  # Enable detailed gen info

# Set up path
process.oniaTreeAna = cms.Path(process.oniaTreeAna)

# Apply MiniAOD customization
if miniAOD:
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
    changeToMiniAOD(process)

#----------------------------------------------------------------------------
# X(3872) Analysis: J/psi + pi+ pi-
#----------------------------------------------------------------------------
# Uses BToJpsiHadronsProducer with doXToJpsiPiPi = True
# decayId = 5 for X -> J/psi pi pi
#
# X(3872) properties:
#   - Mass: 3871.65 MeV
#   - Decay: X -> J/psi pi+pi- (dominant visible mode via rho0)
#   - pipi mass peaks near rho(770) mass
#
# Looser cuts for MC study

from VertexCompositeAnalysis.VertexCompositeProducer.bToJpsiHadrons_cfi import bToJpsiHadrons

process.xToJpsiPiPiMuMu = bToJpsiHadrons.clone(
    jpsiSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    trackSrc = cms.InputTag("packedPFCandidates"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    # J/psi mass window (looser for MC)
    jpsiMassMin = cms.double(2.7),
    jpsiMassMax = cms.double(3.5),
    minJpsiPt = cms.double(0.0),
    
    # Track cuts (soft pions from X decay)
    minTrackPt = cms.double(0.1),  # 100 MeV
    maxTrackEta = cms.double(2.4),
    minTrackNhits = cms.int32(3),
    requireHighPurity = cms.bool(True),
    
    # X candidate cuts (loose for MC)
    minBPt = cms.double(0.0),
    maxBAbsY = cms.double(2.4),
    minVtxProb = cms.double(0.001),
    
    # Q2 = pipi mass window (wider for MC study)
    # X(3872) -> J/psi rho -> J/psi pi+pi-, Q2 peaks at rho mass (~770 MeV)
    xPiPiMassMin = cms.double(0.28),  # M(pipi) > 2*m_pi (~280 MeV)
    xPiPiMassMax = cms.double(1.5),   # M(pipi) < 1.5 GeV
    
    # X mass window (wider for MC study)
    xMassMin = cms.double(3.5),   # M(J/psi pipi) > 3.5 GeV
    xMassMax = cms.double(4.5),   # M(J/psi pipi) < 4.5 GeV
    
    # Disable all B modes, enable only X mode
    doBplusToJpsiK = cms.bool(False),
    doBplusToJpsiPi = cms.bool(False),
    doB0ToJpsiKPi = cms.bool(False),
    doBsToJpsiKK = cms.bool(False),
    doBcToJpsiPi = cms.bool(False),
    doXToJpsiPiPi = cms.bool(True),
    
    doVertexFit = cms.bool(True),
)

#----------------------------------------------------------------------------
# X(3872) Ntuplizer
#----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeAnalyzer.bToJpsiHadronsNtuplizer_cfi import bToJpsiHadronsNtuplizer

process.xTreeMuMu = bToJpsiHadronsNtuplizer.clone(
    candidateSrc = cms.InputTag("xToJpsiPiPiMuMu", "BToJpsiHadrons"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    isCentrality = cms.untracked.bool(False),
)

# X path
process.xMuMuPath = cms.Path(
    process.xToJpsiPiPiMuMu +
    process.xTreeMuMu
)

#----------------------------------------------------------------------------
# Input/Output
#----------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    numberOfThreads = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(1),
)

#----------------------------------------------------------------------------
# Schedule
#----------------------------------------------------------------------------
process.schedule = cms.Schedule(
    process.oniaTreeAna,
    process.xMuMuPath,
)

#----------------------------------------------------------------------------
# Output tree structure (xTreeMuMu/bTree):
#   Event: run, lumi, event, nPV, pvX, pvY, pvZ, nCand
#   X candidate (vectors):
#     - B_mass, B_pt, B_eta, B_phi, B_y, B_charge
#     - B_decayId (=5 for X->J/psi pipi)
#     - B_nTracks (=2), B_hasDau2 (=1)
#     - B_vtxX, B_vtxY, B_vtxZ, B_vtxChi2, B_vtxNdof, B_vtxProb
#     - B_lxy, B_lxySig, B_l3D, B_l3DSig, B_cosAlpha
#   J/psi:
#     - jpsiIdx (->Reco_QQ_collIdx in hionia tree)
#     - Jpsi_mass, Jpsi_pt, Jpsi_eta, Jpsi_phi, Jpsi_y
#   Dipion:
#     - resMass (= M(pi+pi-), should peak near rho mass ~770 MeV)
#   Pion 1:
#     - trk1_pt, trk1_eta, trk1_phi, trk1_charge, trk1_massHypo (=0.1396)
#   Pion 2:
#     - trk2_pt, trk2_eta, trk2_phi, trk2_charge, trk2_massHypo (=0.1396)
#----------------------------------------------------------------------------
