### HiForest Configuration with Full Onia, B meson, and D meson analysis
# Input: miniAOD
# Type: data (PbPb 2025)
# Includes: Event, Jet, OniaMuMu, OniaEE, BMeson (from mumu and ee), D0, DStar

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('HiForest', eras.Run3_pp_on_PbPb_2025)

###############################################################################
# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 151X, PbPb 2025 data, OniaBmesonDmeson")

###############################################################################
# Configuration flags
isMC = False
HLTProName = 'HLT'

###############################################################################
# Input files (PbPb 2025)
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/hidata/HIRun2025A/HIPhysicsRawPrime12/MINIAOD/PromptReco-v1/000/399/766/00000/20708bc3-28e5-469a-a92d-51f39fcb6021.root',
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(3000)
)

###############################################################################
# Multi-threading settings
process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(0),  # 0 = same as numberOfThreads
    wantSummary = cms.untracked.bool(True),
)

###############################################################################
# Load Global Tag, geometry, etc.
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '151X_dataRun3_Prompt_v1', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

###############################################################################
# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

###############################################################################
# Lighter unpackedTracksAndVertices - skip lostTracks (trade: lose +42% high-pT tracks)
# Set to True for faster processing, False for maximum track recovery
useLightUnpacker = False  # <-- Toggle this

if useLightUnpacker:
    from PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi import unpackedTracksAndVertices
    process.unpackedTracksAndVertices = unpackedTracksAndVertices.clone(
        packedCandidates = cms.VInputTag("packedPFCandidates"),  # Only PF, no lostTracks
        packedCandidateNormChi2Map = cms.VInputTag("packedPFCandidateTrackChi2"),
    )

###############################################################################
# Output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestPbPb2025_OniaBmesonDmeson.root")
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
process.hiEvtAnalyzer.doHFfilters = cms.bool(True)

process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')

# TriggerAnalyzer2 with pattern filtering (for validation, run both)
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis2_cfi')
from HeavyIonsAnalysis.EventAnalysis.hltanalysis2_cfi import configureOniaJetTriggers
configureOniaJetTriggers(process.hltanalysis2)
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
process.akPu4CaloJetAnalyzer.jetPtMin = 60.0
process.akCs4PFJetAnalyzer.jetPtMin = 60.0

###############################################################################
# Jet B-tagging Configuration
###############################################################################
# Jet settings
matchJets = False
jetPtMin = 60
jetAbsEtaMax = 2.5
doHIJetID = True
doWTARecluster = True
doBtagging = True   # Enable UPT scores

# Jet collections to process
# "4" = R=0.4 CS jets, "4Flow" = R=0.4 flow-subtracted CS jets
jetLabelsCS = ["4"]
jetLabelsFlowCS = []
allJetLabels = jetLabelsCS

# Define empty forest path first (required by candidateBtaggingMiniAOD)
process.forest = cms.Path()

# Add candidate b-tagging
from HeavyIonsAnalysis.JetAnalysis.setupJets_PbPb_cff import candidateBtaggingMiniAOD

for jetLabel in allJetLabels:
    candidateBtaggingMiniAOD(process, isMC=False, jetPtMin=jetPtMin, 
                             jetCorrLevels=['L2Relative', 'L2L3Residual'], 
                             doBtagging=doBtagging, labelR=jetLabel)
    
    # Configure jet analyzer
    setattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag", process.akCs4PFJetAnalyzer.clone())
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").jetTag = "selectedUpdatedPatJetsAK"+jetLabel+"PFBtag"
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").jetName = 'akCs'+jetLabel+'PF'
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").matchJets = matchJets
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").matchTag = 'patJetsAK'+jetLabel+'PFUnsubJets'
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").doBtagging = doBtagging
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").doHiJetID = doHIJetID
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").doWTARecluster = doWTARecluster
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").jetPtMin = jetPtMin
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").jetAbsEtaMax = cms.untracked.double(jetAbsEtaMax)
    getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").rParam = 0.4
    
    if doBtagging:
        # Standard pp Run3 UPT (default)
        getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").pfJetProbabilityBJetTag = cms.untracked.string(
            "pfJetProbabilityBJetTagsAK"+jetLabel+"PFBtag")
        getattr(process, "akCs"+jetLabel+"PFJetAnalyzer_btag").pfUnifiedParticleTransformerAK4JetTags = cms.untracked.string(
            "pfUnifiedParticleTransformerAK4JetTagsAK"+jetLabel+"PFBtag")
        
        # Use HI PbPb 2023 UPT model
        getattr(process, "pfUnifiedParticleTransformerAK4JetTagsAK"+jetLabel+"PFBtag").model_path = \
            'RecoBTag/Combined/data/UParTAK4/HIN/V00/UParTAK4_PbPb_2023.onnx'
        getattr(process, "pfUnifiedParticleTransformerAK4TagInfosAK"+jetLabel+"PFBtag").sort_cand_by_pt = True
        getattr(process, "pfUnifiedParticleTransformerAK4TagInfosAK"+jetLabel+"PFBtag").fix_lt_sorting = True

###############################################################################
# ZDC Analysis
process.load('HeavyIonsAnalysis.ZDCAnalysis.ZDCAnalyzersPbPb_cff')

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
        "HLT_HIL1DoubleMu0_MaxDr3p5_Open_v",
        "HLT_HIL1DoubleMu0_v",
        "HLT_HIL1DoubleMu0_SQ_v",
        "HLT_HIL2DoubleMu0_Open_v",
        "HLT_HIL2DoubleMu0_M1p5to6_Open_v",
        "HLT_HIL2DoubleMu2p8_M1p5to6_Open_v",
        "HLT_HIL2DoubleMu0_M7to15_Open_v",
        "HLT_HIL2DoubleMu3_M7to15_Open_v",
        "HLT_HIMinimumBiasHF1AND_v",
        "HLT_HIMinimumBiasHF1ANDZDC1nOR_v",
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_HIL1SingleMu0_Open_v",
        "HLT_HIL1SingleMu0_v",
        "HLT_HIL2SingleMu3_Open_v",
        "HLT_HIL2SingleMu5_v",
        "HLT_HIL2SingleMu7_v",
        "HLT_HIL2SingleMu12_v",
        "HLT_HIMinimumBiasHF1AND_v",
        "HLT_HIMinimumBiasHF1ANDZDC1nOR_v",
    ),
}

electronTriggerList = {
    'SingleElectronTrigger': cms.vstring(
        "HLT_HIEle10Gsf_v",
        "HLT_HIEle15Gsf_v",
        "HLT_HIEle20Gsf_v",
    ),
    'DoubleElectronTrigger': cms.vstring(
        "HLT_HIDoubleEle10Gsf_v",
        "HLT_HIEle15Ele10Gsf_v",
    ),
}

# Electron trigger list for dielectron producer
electronTriggerPathNames = [
    'HLT_HIMinimumBiasHF1ANDZDC1nOR_v',
    'HLT_HIMinimumBiasHF1AND_v',
    'HLT_HIEle10Gsf_v',
    'HLT_HIEle15Gsf_v',
    'HLT_HIEle20Gsf_v',
    'HLT_HIDoubleEle10Gsf_v',
    'HLT_HIEle15Ele10Gsf_v',
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
    "mass > 2.5 && mass < 150 && charge==0 && "
    "abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25"
)
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.5 && abs(eta) < 2.4 && isTrackerMuon")
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string("userFloat(\"vProb\")>0.01")
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(OnlySoftMuons)

#----------------------------------------------------------------------------
# Dielectron Selection (J/psi to Upsilon: 2.5 - 15 GeV)
#----------------------------------------------------------------------------
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 2.5 && mass < 150 && charge == 0")
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.5 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = cms.bool(False)
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerPathNames)

# Configure onia analyzers for PbPb (isHI=True for full HI treatment)
process.hionia.isHI = cms.untracked.bool(True)

# Apply MiniAOD customization
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
process.hionia.useEvtPlane = cms.untracked.bool(False)
process.hionia.checkTrigNames = cms.bool(False)
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
process.forest += (
    process.HiForestInfo +
    process.centralityBin +
    process.hiEvtAnalyzer +
    process.hltanalysis +  # Original for comparison
    process.hltanalysis2 +  # Filtered trigger analyzer with prescale caching
#    process.l1object +
#    process.trackSequencePbPb +
    process.particleFlowAnalyser +
    process.unpackedTracksAndVertices +
    process.unpackedMuons +
    process.vertexAnalyzer +
#    process.muonAnalyzer +
#    process.ggHiNtuplizer +
    process.akCs4PFJetAnalyzer +
    process.akPu4CaloJetAnalyzer +
    process.akCs4PFJetAnalyzer_btag  # Jet analyzer with UPT b-tagging (jetPtMin=60)
#    process.zdcSequencePbPb
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
    process.bPlusMuMuNtuplizer
#    process.bPlusMuMuNtuplizer +
#    process.bcMuMuNtuplizer
)

###############################################################################
# B meson from dielectron sequence
###############################################################################
process.bMesonEEPath = cms.Path(
    process.bMesonEE +
    process.bPlusEENtuplizer 
#    process.bPlusEENtuplizer +
#    process.bcEENtuplizer
)

###############################################################################
# Event Selection Filters
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

# HF filters for PbPb
process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')

# Common event filter sequence for early termination
# Events failing these filters will skip heavy processing (Onia, B meson, etc.)
process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter +
    process.phfCoincFilter2Th4  # HF coincidence filter for PbPb
)

# Filter paths for skim tree (records which events pass)
process.pclusterCompatibilityFilter = cms.Path(process.clusterCompatibilityFilter)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)
process.pphfCoincFilter2Th4 = cms.Path(process.phfCoincFilter2Th4)
process.pphfCoincFilter4Th2 = cms.Path(process.phfCoincFilter4Th2)

process.pAna = cms.EndPath(process.skimanalysis)

###############################################################################
# Schedule - with early termination on bad events
###############################################################################
# Prepend event filter to heavy processing paths
process.forest.insert(0, process.eventFilter)
process.oniaPath.insert(0, process.eventFilter)
process.bMesonMuMuPath.insert(0, process.eventFilter)
process.bMesonEEPath.insert(0, process.eventFilter)
# process.dMesonPath.insert(0, process.eventFilter)  # Uncomment if using D mesons

process.schedule = cms.Schedule(
    process.forest,
    process.oniaPath,
#    process.dMesonPath,
    process.bMesonMuMuPath,
    process.bMesonEEPath,
    process.pclusterCompatibilityFilter,
    process.pprimaryVertexFilter,
    process.pphfCoincFilter2Th4,
    process.pphfCoincFilter4Th2,
    process.pAna,
)
