### HiForest Configuration with Full Onia, B meson, and D meson analysis
# Input: miniAOD
# Type: data (OO 2025)
# Includes: Event, Jet, OniaMuMu, OniaEE, BMeson (from mumu and ee), D0, DStar

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('HiForest', eras.Run3_2025_OXY)

###############################################################################
# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 150X, OO 2025 data, OniaBmesonDmeson")

###############################################################################
# Configuration flags
isMC = False
HLTProName = 'HLT'

###############################################################################
# Input files (OO 2025)
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root',
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000)
)

###############################################################################
# Load Global Tag, geometry, etc.
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v3', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

# Centrality table for OO (using PbPb table as placeholder)
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("HeavyIonRcd"),
        tag = cms.string("CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_Run3v1302x04_Nominal_Offline"),
        connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
        label = cms.untracked.string("HFtowers")
    ),
])

###############################################################################
# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

###############################################################################
# Output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestOO_OniaBmesonDmeson1k.root")
)

###############################################################################
# Event Analysis
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.hiEvtAnalyzer.Vertex = cms.InputTag("offlineSlimmedPrimaryVertices")
process.hiEvtAnalyzer.doCentrality = cms.bool(True)
process.hiEvtAnalyzer.doEvtPlane = cms.bool(False)
process.hiEvtAnalyzer.doEvtPlaneFlat = cms.bool(False)
process.hiEvtAnalyzer.doMC = cms.bool(False)
process.hiEvtAnalyzer.doHiMC = cms.bool(False)
process.hiEvtAnalyzer.doHFfilters = cms.bool(False)  # Disable HF filters for OO

process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')

###############################################################################
# Track unpacking (needed for B/D meson reconstruction)
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")

###############################################################################
# Muon Analysis
process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
process.load("HeavyIonsAnalysis.MuonAnalysis.muonAnalyzer_cfi")

###############################################################################
# EGM Analysis (Electrons/Photons)
process.load("HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi")
process.ggHiNtuplizer.doElectrons = cms.bool(True)
process.ggHiNtuplizer.doPhotons = cms.bool(True)
process.ggHiNtuplizer.doMuons = cms.bool(False)
process.ggHiNtuplizer.doGenParticles = cms.bool(isMC)
process.ggHiNtuplizer.useValMapIso = cms.bool(False)

###############################################################################
# Jet Analysis
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_data_cff')
process.load('HeavyIonsAnalysis.JetAnalysis.akPu4CaloJetSequence_pponPbPb_data_cff')
process.akPu4CaloJetAnalyzer.doHiJetID = True

###############################################################################
# ZDC Analysis (disabled for OO)
#process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
#process.load('HeavyIonsAnalysis.ZDCAnalysis.ZDCAnalyzersPbPb_cff')

###############################################################################
# TransientTrack Builder (needed for vertex fitting)
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

###############################################################################
# Onia MuMu + EE Analysis
###############################################################################
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer

# Muon selection settings
muonSelection = "GlbOrTrk"  # Global or Tracker muons
OnlySoftMuons = False
useMomFormat = "vector"  # Use std::vector<float> for pt, eta, phi, M

muonTriggerList = {
    'DoubleMuonTrigger': cms.vstring(
        "HLT_OxyL1DoubleMuOpen_v",
        "HLT_OxyL1DoubleMu0_v",
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_OxyL1SingleMuOpen_v",
        "HLT_OxyL1SingleMu0_v",
        "HLT_OxyL1SingleMu3_v",
        "HLT_OxyL1SingleMu5_v",
        "HLT_OxyL1SingleMu7_v",
        "HLT_OxySingleMuCosmic_NotMBHF2AND_v",
        "HLT_OxySingleMuOpen_NotMBHF2AND_v",
        "HLT_MinimumBiasHF_OR_BptxAND_v",
        "HLT_MinimumBiasHF_AND_BptxAND_v",
    ),
}

electronTriggerList = {
    'SingleElectronTrigger': cms.vstring(
        # OO doesn't have electron triggers yet - placeholder
    ),
    'DoubleElectronTrigger': cms.vstring(
        # OO doesn't have electron triggers yet - placeholder
    ),
}

# Electron trigger list for dielectron producer
electronTriggerPathNames = [
    'HLT_MinimumBiasHF_OR_BptxAND_v',
    'HLT_MinimumBiasHF_AND_BptxAND_v',
]

oniaTreeAnalyzer(process,
    muonTriggerList=muonTriggerList,
    HLTProName=HLTProName,
    muonSelection=muonSelection,
    L1Stage=2,
    isMC=isMC,
    pdgID=443,
    outputFileName="OniaTree.root",
    muonlessPV=False,
    doTrimu=False,
    doDimuTrk=False,
    flipJpsiDir=0,
    OnlySingleMuons=False,
    doElectrons=True,
    electronTriggerList=electronTriggerList,
)

#----------------------------------------------------------------------------
# Dimuon Selection (J/psi to Upsilon: 2.5 - 15 GeV)
#----------------------------------------------------------------------------
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
    "mass > 2.5 && mass < 15 && charge==0 && "
    "abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.5 && abs(eta) < 2.4 && isTrackerMuon")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

#----------------------------------------------------------------------------
# Dielectron Selection (J/psi to Upsilon: 2.5 - 15 GeV)
#----------------------------------------------------------------------------
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 2.5 && mass < 15 && charge == 0")
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.5 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = cms.bool(False)
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerPathNames)

# Configure onia analyzers for OO (MUST set isHI=False BEFORE changeToMiniAOD)
process.hionia.isHI = cms.untracked.bool(False)  # OO is not full HI - this disables packedCandidateMuonID requirement

# Apply MiniAOD customization (must be AFTER oniaTreeAnalyzer call, but AFTER isHI is set)
from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
changeToMiniAOD(process)

#----------------------------------------------------------------------------
# MiniAOD Electron Configuration (Mode 1: slimmedElectrons + Run3 MVA ID)
#----------------------------------------------------------------------------
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.electronIDType = cms.string("mva")
process.onia2ElectronElectronPatGlbGlb.electronIDWP = cms.string("wp90")

#----------------------------------------------------------------------------
# HiOnia Muon Analyzer Settings
#----------------------------------------------------------------------------
process.hionia.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
process.hionia.useEvtPlane = cms.untracked.bool(False)  # Event plane not available in OO
process.hionia.checkTrigNames = cms.bool(False)  # Avoid trigger name warnings
process.hionia.mom4format = cms.string(useMomFormat)
process.hionia.SumETvariables = cms.bool(True)
process.hionia.applyCuts = cms.bool(False)
process.hionia.storeSameSign = cms.bool(True)
process.hionia.AtLeastOneCand = cms.bool(False)
process.hionia.fillTree = cms.bool(True)
process.hionia.fillHistos = cms.bool(False)
process.hionia.fillSingleMuons = cms.bool(True)
process.hionia.muonSel = cms.string(muonSelection)

#----------------------------------------------------------------------------
# HiOnia Electron Analyzer Settings
#----------------------------------------------------------------------------
process.hioniaElectrons.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
process.hioniaElectrons.CentralitySrc = cms.InputTag("hiCentrality")
process.hioniaElectrons.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
process.hioniaElectrons.useEvtPlane = cms.untracked.bool(False)
process.hioniaElectrons.checkTrigNames = cms.bool(False)
process.hioniaElectrons.mom4format = cms.string(useMomFormat)
process.hioniaElectrons.fillTree = cms.bool(True)
process.hioniaElectrons.fillHistos = cms.bool(False)
process.hioniaElectrons.fillSingleElectrons = cms.bool(True)
process.hioniaElectrons.conversions = cms.InputTag("reducedEgamma", "reducedConversions")

###############################################################################
# D Meson Production
###############################################################################
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi")
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cfi")

# D0 producer (D0 -> K pi)
process.d0Candidates = process.generalD0Candidates.clone(
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    tkPtCut = cms.double(0.5),
    tkEtaCut = cms.double(2.4),
    VtxChiProbCut = cms.double(0.01),
    d0MassCut = cms.double(0.15),
)

# DStar producer (D* -> D0 pi)
# Slow pion pT cut is 0.1 GeV (separate from D0 daughter track cuts)
process.dStarCandidates = process.generalDStarCandidates.clone(
    d0Collection = cms.InputTag('d0Candidates', 'D0'),
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    tkPtCut = cms.double(0.1),  # Slow pion pT > 0.1 GeV
    VtxChiProbCut = cms.double(0.01),
    dStarMassCut = cms.double(0.20),
)

###############################################################################
# D Meson Ntuplizers (using PATCompositeNtupleProducer)
###############################################################################
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi")

# D0 ntuplizer
process.d0Ntuplizer = process.d0NtupleProducer.clone(
    candidateSrc = cms.InputTag('d0Candidates', 'D0'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

# DStar ntuplizer
process.dStarNtuplizer = process.dStarNtupleProducer.clone(
    candidateSrc = cms.InputTag('dStarCandidates', 'DStar'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

###############################################################################
# B Meson Production
###############################################################################
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiLeptonCandidates_cfi")

# B meson producer from dimuon (J/psi -> mu mu)
process.bMesonMuMu = process.generalBDiLeptonCandidates.clone(
    dileptonCollection = cms.InputTag('onia2MuMuPatGlbGlb'),
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    dileptonMassMin = cms.double(2.9),
    dileptonMassMax = cms.double(3.3),
    dileptonPtCut = cms.double(0.0),
    bPtCut = cms.double(0.0),
    tkPtCut = cms.double(0.5),
    vtxProbCut = cms.double(0.01),
    rVtxSigCut = cms.double(0.0),
    lVtxSigCut = cms.double(0.0),
    bPlusMassCut = cms.double(1.0),
    bZeroMassCut = cms.double(1.0),
    doBPlus = cms.bool(True),
    doBZero = cms.bool(True),
    doBc = cms.bool(True),
)

# B meson producer from dielectron (J/psi -> e e)
process.bMesonEE = process.generalBDiLeptonCandidates.clone(
    dileptonCollection = cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices'),
    dileptonMassMin = cms.double(2.5),
    dileptonMassMax = cms.double(3.5),
    dileptonPtCut = cms.double(0.0),
    bPtCut = cms.double(0.0),
    tkPtCut = cms.double(0.5),
    vtxProbCut = cms.double(0.01),
    rVtxSigCut = cms.double(0.0),
    lVtxSigCut = cms.double(0.0),
    bPlusMassCut = cms.double(1.0),
    bZeroMassCut = cms.double(1.0),
    doBPlus = cms.bool(True),
    doBZero = cms.bool(True),
    doBc = cms.bool(True),
)

###############################################################################
# B Meson Ntuplizers (using PATCompositeNtupleProducer)
###############################################################################

# B+ from dimuon
process.bPlusMuMuNtuplizer = process.bPlusNtupleProducer.clone(
    candidateSrc = cms.InputTag('bMesonMuMu', 'BPlus'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

# Bc from dimuon
process.bcMuMuNtuplizer = process.bcNtupleProducer.clone(
    candidateSrc = cms.InputTag('bMesonMuMu', 'Bc'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

# B+ from dielectron
process.bPlusEENtuplizer = process.bPlusNtupleProducer.clone(
    candidateSrc = cms.InputTag('bMesonEE', 'BPlus'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    doMuon = cms.untracked.bool(False),
    doElectron = cms.untracked.bool(True),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

# Bc from dielectron
process.bcEENtuplizer = process.bcNtupleProducer.clone(
    candidateSrc = cms.InputTag('bMesonEE', 'Bc'),
    vertexSrc = cms.InputTag('unpackedTracksAndVertices'),
    doMuon = cms.untracked.bool(False),
    doElectron = cms.untracked.bool(True),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag('centralityBin', 'HFtowers'),
    centralitySrc = cms.InputTag('hiCentrality'),
)

###############################################################################
# Vertex Analyzer (detailed PV info)
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.vertexAnalyzer_cfi')
process.vertexAnalyzer.vertexSrc = cms.InputTag('unpackedTracksAndVertices')
process.vertexAnalyzer.beamSpotSrc = cms.InputTag('offlineBeamSpot')

###############################################################################
# Main Forest Sequence (Event, Track, Muon, Electron, Jet)
###############################################################################
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.hiEvtAnalyzer +
    process.hltanalysis +
    process.l1object +
#    process.trackSequencePbPb +
    process.particleFlowAnalyser +
    process.unpackedTracksAndVertices +
    process.unpackedMuons +
    process.vertexAnalyzer +
#    process.muonAnalyzer +
#    process.ggHiNtuplizer +
    process.akCs4PFJetAnalyzer +
    process.akPu4CaloJetAnalyzer
)

###############################################################################
# Onia analysis sequence (includes both mumu and ee)
###############################################################################
process.oniaPath = cms.Path(process.oniaTreeAna)

###############################################################################
# D meson sequences
###############################################################################
process.dMesonPath = cms.Path(
    process.d0Candidates +
    process.d0Ntuplizer +
    process.dStarCandidates +
    process.dStarNtuplizer
)

###############################################################################
# B meson from dimuon sequence
###############################################################################
process.bMesonMuMuPath = cms.Path(
    process.bMesonMuMu +
    process.bPlusMuMuNtuplizer +
    process.bcMuMuNtuplizer
)

###############################################################################
# B meson from dielectron sequence
###############################################################################
process.bMesonEEPath = cms.Path(
    process.bMesonEE +
    process.bPlusEENtuplizer +
    process.bcEENtuplizer
)

###############################################################################
# Event Selection Filters
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.pclusterCompatibilityFilter = cms.Path(process.clusterCompatibilityFilter)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)

# HF filters disabled for OO (hiHFfilters not available)
#process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')
#process.pphfCoincFilter2Th4 = cms.Path(process.phfCoincFilter2Th4)
#process.pphfCoincFilter4Th2 = cms.Path(process.phfCoincFilter4Th2)

process.pAna = cms.EndPath(process.skimanalysis)

###############################################################################
# Schedule
###############################################################################
process.schedule = cms.Schedule(
    process.forest,
    process.oniaPath,
    process.dMesonPath,
    process.bMesonMuMuPath,
    process.bMesonEEPath,
    process.pclusterCompatibilityFilter,
    process.pprimaryVertexFilter,
    process.pAna,
)
