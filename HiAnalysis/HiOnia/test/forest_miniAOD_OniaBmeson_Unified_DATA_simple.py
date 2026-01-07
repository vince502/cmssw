### HiForest Configuration with Unified B Meson Producer
# Input: miniAOD
# Type: data (OO 2025)
# Includes: Event, Jet, OniaMuMu, OniaEE, Unified BMeson (all decay modes in one collection)
#
# Key difference from forest_miniAOD_OniaBmeson_DATA.py:
#   - Uses BToJpsiHadronsProducer for unified B meson reconstruction
#   - Single output tree with decayId to distinguish modes
#   - jpsiIdx for matching with hionia tree (via Reco_QQ_collIdx)
#
# Electron Configuration Options (see "Electron Configuration" section below):
#   - electronSource: "slimmedElectrons" (default), "slimmedLowPtElectrons", or "merged"
#   - electronIDType: "mva" (default), "cutbased", "hardcoded", "hiMVA", "none"
#   - electronIDWP: "wp90" (default), "wp80", "loose", "medium", "tight", etc.
#   - applyConversionVeto: True (default) - set to False to increase J/psi->ee yield
#
# Tips for J/psi -> e+e- analysis:
#   - To increase yield: set applyConversionVeto = False
#   - To use looser ID: set electronIDWP = "wp95" or electronIDType = "none"
#   - For soft J/psi: try electronSource = "slimmedLowPtElectrons"

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('HiForest', eras.Run3_2025_OXY)

###############################################################################
# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 150X, OO 2025 data, UnifiedBMeson")

###############################################################################
# Configuration flags
isMC = False
HLTProName = 'HLT'

###############################################################################
# Electron Configuration
# Choose electron source and ID settings
###############################################################################
# Electron source options:
#   "slimmedElectrons"         - Standard GSF electrons (default)
#   "slimmedLowPtElectrons"    - Low-pT electrons (better for soft J/psi)
#   "merged"                   - Merge both collections (requires mergedElectrons module)
electronSource = "slimmedElectrons"

# Electron ID settings
# electronIDType: "hardcoded", "cutbased", "mva", "hiMVA", "none"
# electronIDWP: working point
#   - For cutbased: "veto", "loose", "medium", "tight"
#   - For mva: "wp90", "wp80" (90% or 80% signal efficiency)
#   - For hiMVA: "95", "90", "85", "80"
electronIDType = "mva"
electronIDWP = "wp90"

# Conversion veto: reject electrons from photon conversions
# Set to False for J/psi -> ee analysis to increase yield
applyConversionVeto = True

# Low-pT electron specific settings (used when electronSource includes low-pT)
# Note: slimmedLowPtElectrons use different ID (BDT-based)
lowPtElectronIDThreshold = 1.0  # BDT score threshold (higher = purer, lower = more efficient)

###############################################################################
# Input files
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root',
'/store/hidata/OORun2025/IonPhysics59/MINIAOD/PromptReco-v1/000/394/217/00000/930f484d-5c72-4f4a-aa0b-edd7fe42de79.root',
'/store/hidata/OORun2025/IonPhysics59/MINIAOD/PromptReco-v1/000/394/217/00000/60396782-15ae-4615-b6b1-58856f329796.root',
'/store/hidata/OORun2025/IonPhysics59/MINIAOD/PromptReco-v1/000/394/217/00000/5f2bdad8-4af7-41d9-9788-f9d210b2dba2.root',
'/store/hidata/OORun2025/IonPhysics59/MINIAOD/PromptReco-v1/000/394/217/00000/b8e59554-5c1c-475e-8390-39abfdbdb78c.root',
'/store/hidata/OORun2025/IonPhysics59/MINIAOD/PromptReco-v1/000/394/217/00000/fee7aee8-6d3e-4614-8f0c-d1ddf93f801e.root',
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100000)
)

###############################################################################
# Multi-threading settings
process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(6),
    numberOfStreams = cms.untracked.uint32(1),
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
    fileName = cms.string("HiForestOO_UnifiedBMeson.root")
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
process.hiEvtAnalyzer.doHFfilters = cms.bool(False)

process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')

###############################################################################
# Track unpacking (needed for B meson reconstruction)
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
# Jet B-tagging with UParT (using slimmedSecondaryVertices to save time)
###############################################################################
doBtagging = True
jetPtMin = 60
jetAbsEtaMax = 2.5

if doBtagging:
    # Use slimmedSecondaryVertices from MiniAOD instead of remaking them
    svSource = cms.InputTag("slimmedSecondaryVertices")
    
    # B-tagging discriminators to compute
    # Note: pfDeepFlavourTagInfos requires pfDeepCSVTagInfos
    bTagInfos = [
        'pfImpactParameterTagInfos',
        'pfInclusiveSecondaryVertexFinderTagInfos',
        'pfDeepCSVTagInfos',
        'pfDeepFlavourTagInfos',
        'pfUnifiedParticleTransformerAK4TagInfos'
    ]
    
    bTagDiscriminators = [
        'pfUnifiedParticleTransformerAK4JetTags:probb',
        'pfUnifiedParticleTransformerAK4JetTags:probbb',
        'pfUnifiedParticleTransformerAK4JetTags:probc',
        'pfUnifiedParticleTransformerAK4JetTags:probg',
        'pfUnifiedParticleTransformerAK4JetTags:problepb',
        'pfUnifiedParticleTransformerAK4JetTags:probu',
        'pfUnifiedParticleTransformerAK4JetTags:probd',
        'pfUnifiedParticleTransformerAK4JetTags:probs',
        'pfUnifiedParticleTransformerAK4JetTags:probtaup1h0p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaup1h1p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaup1h2p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaup3h0p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaup3h1p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaum1h0p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaum1h1p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaum1h2p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaum3h0p',
        'pfUnifiedParticleTransformerAK4JetTags:probtaum3h1p',
        'pfUnifiedParticleTransformerAK4JetTags:probele',
        'pfUnifiedParticleTransformerAK4JetTags:probmu',
        'pfUnifiedParticleTransformerAK4JetTags:ptcorr',
        'pfUnifiedParticleTransformerAK4JetTags:ptnu',
    ]
    
    # Update jet collection with b-tagging
    from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
    updateJetCollection(
        process,
        labelName = "AK4PFBtag",
        jetSource = cms.InputTag("slimmedJets"),
        jetCorrections = ('AK4PF', ['L2Relative'], 'None'),  # L2L3Residual may not be available
        pfCandidates = cms.InputTag('packedPFCandidates'),
        pvSource = cms.InputTag("offlineSlimmedPrimaryVertices"),
        svSource = svSource,
        muSource = cms.InputTag('slimmedMuons'),
        elSource = cms.InputTag('slimmedElectrons'),
        btagInfos = bTagInfos,
        btagDiscriminators = bTagDiscriminators,
        explicitJTA = False
    )
    
    # Configure UParT model (Run3 default or HI PbPb 2023)
    process.pfUnifiedParticleTransformerAK4JetTagsAK4PFBtag.model_path = \
        'RecoBTag/Combined/data/UParTAK4/HIN/V00/UParTAK4_PbPb_2023.onnx'
    process.pfUnifiedParticleTransformerAK4TagInfosAK4PFBtag.sort_cand_by_pt = True
    process.pfUnifiedParticleTransformerAK4TagInfosAK4PFBtag.fix_lt_sorting = True
    
    # Remove PUPPI (not needed for HI)
    if hasattr(process, 'packedpuppi'):
        process.patAlgosToolsTask.remove(process.packedpuppi)
    if hasattr(process, 'packedpuppiNoLep'):
        process.patAlgosToolsTask.remove(process.packedpuppiNoLep)
    
    process.pfInclusiveSecondaryVertexFinderTagInfosAK4PFBtag.weights = ""
    for taginfo in ["pfDeepFlavourTagInfosAK4PFBtag", "pfUnifiedParticleTransformerAK4TagInfosAK4PFBtag"]:
        if hasattr(process, taginfo):
            getattr(process, taginfo).fallback_puppi_weight = True
            getattr(process, taginfo).fallback_vertex_association = True
            getattr(process, taginfo).puppi_value_map = ""
    
    # Add jet probability tagger
    from RecoBTag.ImpactParameter.pfJetProbabilityBJetTags_cfi import pfJetProbabilityBJetTags
    process.pfJetProbabilityBJetTagsAK4PFBtag = pfJetProbabilityBJetTags.clone(
        tagInfos = ["pfImpactParameterTagInfosAK4PFBtag"]
    )
    process.patAlgosToolsTask.add(process.pfJetProbabilityBJetTagsAK4PFBtag)
    
    # Create jet analyzer with b-tagging
    process.akCs4PFJetAnalyzer_btag = process.akCs4PFJetAnalyzer.clone(
        jetTag = cms.InputTag("selectedUpdatedPatJetsAK4PFBtag"),
        jetName = cms.untracked.string('akCs4PF'),
        doBtagging = cms.untracked.bool(True),
        doHiJetID = cms.untracked.bool(True),
        jetPtMin = cms.double(jetPtMin),
        jetAbsEtaMax = cms.untracked.double(jetAbsEtaMax),
        pfJetProbabilityBJetTag = cms.untracked.string("pfJetProbabilityBJetTagsAK4PFBtag"),
        pfUnifiedParticleTransformerAK4JetTags = cms.untracked.string("pfUnifiedParticleTransformerAK4JetTagsAK4PFBtag"),
    )

###############################################################################
# TransientTrack Builder (needed for vertex fitting)
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

###############################################################################
# Onia MuMu + EE Analysis
###############################################################################
from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer

muonSelection = "GlbOrTrk"
OnlySoftMuons = False
useMomFormat = "vector"

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
        "HLT_MinimumBiasHF_OR_BptxAND_v",
        "HLT_MinimumBiasHF_AND_BptxAND_v",
    ),
}

electronTriggerList = {
    'SingleElectronTrigger': cms.vstring(),
    'DoubleElectronTrigger': cms.vstring(),
}

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
# Dimuon Selection
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
# Dielectron Selection
#----------------------------------------------------------------------------
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 2.5 && mass < 150 && charge == 0")
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string("pt > 1.5 && abs(eta) < 2.4")
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag("reducedEgamma", "reducedConversions")
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = cms.bool(False)
process.onia2ElectronElectronPatGlbGlb.triggerPaths = cms.vstring(*electronTriggerPathNames)

# Configure for OO
process.hionia.isHI = cms.untracked.bool(False)

# Apply MiniAOD customization
from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
changeToMiniAOD(process)

#----------------------------------------------------------------------------
# MiniAOD Electron Configuration
#----------------------------------------------------------------------------
# Apply electron settings from configuration section above
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.electronIDType = cms.string(electronIDType)
process.onia2ElectronElectronPatGlbGlb.electronIDWP = cms.string(electronIDWP)
process.onia2ElectronElectronPatGlbGlb.applyConversionVeto = cms.bool(applyConversionVeto)

# Configure electron source
if electronSource == "merged":
    # Merge slimmedElectrons + slimmedLowPtElectrons
    # Note: This creates a merged collection with overlap removal
    from PhysicsTools.PatAlgos.slimming.slimmedElectrons_cfi import slimmedElectrons
    process.mergedElectrons = cms.EDProducer("PATElectronMerger",
        src = cms.VInputTag(
            cms.InputTag("slimmedElectrons"),
            cms.InputTag("slimmedLowPtElectrons")
        ),
        # Simple merger - no overlap removal (electrons from different seeds)
    )
    process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('mergedElectrons')
    # For merged, use looser ID since low-pT electrons have different ID
    if electronIDType == "mva":
        # Low-pT electrons don't have standard MVA ID, fall back to none for them
        process.onia2ElectronElectronPatGlbGlb.electronIDType = cms.string("none")
elif electronSource == "slimmedLowPtElectrons":
    process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedLowPtElectrons')
    # Low-pT electrons have BDT ID, use "none" to disable standard ID
    # Selection is done via higherPuritySelection string cut on BDT score
    process.onia2ElectronElectronPatGlbGlb.electronIDType = cms.string("none")
    # Add BDT score cut to purity selection
    process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string(
        f"pt > 1.0 && abs(eta) < 2.4 && electronID('mvaEleID-Fall17-noIso-V2-wpLoose') > {lowPtElectronIDThreshold}"
    )
    process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string(
        f"pt > 0.5 && abs(eta) < 2.4"
    )
else:
    # Default: slimmedElectrons
    process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')

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
# Use slimmedElectrons directly for single electrons (not the filtered collection from dielectron producer)
process.hioniaElectrons.srcElectron = cms.InputTag("slimmedElectrons")

###############################################################################
# Unified B Meson Production (NEW!)
###############################################################################
# Uses BToJpsiHadronsProducer - single collection for all decay modes
# 
# Decay modes (decayId):
#   0 = B+  -> J/psi K+    (1 hadron)
#   1 = B+  -> J/psi pi+   (1 hadron)
#   2 = B0  -> J/psi K pi  (2 hadrons, K*)
#   3 = Bs  -> J/psi K K   (2 hadrons, phi)
#   4 = Bc+ -> J/psi pi+   (1 hadron)
#   5 = X   -> J/psi pi pi (2 hadrons)
#
# Output has jpsiIdx for matching with hionia tree (via Reco_QQ_collIdx)
###############################################################################

from VertexCompositeAnalysis.VertexCompositeProducer.bToJpsiHadrons_cfi import bToJpsiHadrons

# B mesons from J/psi -> mu+mu-
process.bToJpsiHadronsMuMu = bToJpsiHadrons.clone(
    jpsiSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    trackSrc = cms.InputTag("packedPFCandidates"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    # J/psi mass window (early cut to save combinatorics)
    jpsiMassMin = cms.double(2.7),
    jpsiMassMax = cms.double(3.4),
    minJpsiPt = cms.double(3.0),
    
    # Track cuts
    minTrackPt = cms.double(0.8),
    maxTrackEta = cms.double(2.4),
    minTrackNhits = cms.int32(3),
    requireHighPurity = cms.bool(True),
    
    # B candidate cuts
    minBPt = cms.double(5.0),
    maxBAbsY = cms.double(2.4),
    minVtxProb = cms.double(0.01),
    
    # Resonance mass windows
    phiMassWindow = cms.double(0.015),     # phi -> KK
    kstarMassWindow = cms.double(0.100),   # K* -> Kpi
    
    # B mass windows
    bPlusMassWindow = cms.double(0.5),
    b0MassWindow = cms.double(0.5),
    bsMassWindow = cms.double(0.5),
    bcMassWindow = cms.double(0.5),
    
    # Enable decay modes
    doBplusToJpsiK = cms.bool(True),    # B+ -> J/psi K+
    doBplusToJpsiPi = cms.bool(False),  # B+ -> J/psi pi+ (disabled to reduce combinatorics)
    doB0ToJpsiKPi = cms.bool(True),     # B0 -> J/psi K*
    doBsToJpsiKK = cms.bool(True),      # Bs -> J/psi phi
    doBcToJpsiPi = cms.bool(False),     # Bc+ -> J/psi pi+ (disabled - rare)
    doXToJpsiPiPi = cms.bool(False),    # X -> J/psi pipi (disabled)
    
    # Vertex fit
    doVertexFit = cms.bool(True),
)

# B mesons from J/psi -> e+e-
process.bToJpsiHadronsEE = bToJpsiHadrons.clone(
    jpsiSrc = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    trackSrc = cms.InputTag("packedPFCandidates"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotSrc = cms.InputTag("offlineBeamSpot"),
    
    # Wider J/psi mass window for electrons (bremsstrahlung)
    jpsiMassMin = cms.double(2.5),
    jpsiMassMax = cms.double(3.5),
    minJpsiPt = cms.double(3.0),
    
    # Track cuts
    minTrackPt = cms.double(0.8),
    maxTrackEta = cms.double(2.4),
    minTrackNhits = cms.int32(3),
    requireHighPurity = cms.bool(True),
    
    # B candidate cuts
    minBPt = cms.double(5.0),
    maxBAbsY = cms.double(2.4),
    minVtxProb = cms.double(0.01),
    
    # Resonance mass windows
    phiMassWindow = cms.double(0.015),
    kstarMassWindow = cms.double(0.100),
    
    # B mass windows
    bPlusMassWindow = cms.double(0.5),
    b0MassWindow = cms.double(0.5),
    bsMassWindow = cms.double(0.5),
    bcMassWindow = cms.double(0.5),
    
    # Enable decay modes
    doBplusToJpsiK = cms.bool(True),
    doBplusToJpsiPi = cms.bool(False),
    doB0ToJpsiKPi = cms.bool(True),
    doBsToJpsiKK = cms.bool(True),
    doBcToJpsiPi = cms.bool(False),
    doXToJpsiPiPi = cms.bool(False),
    
    doVertexFit = cms.bool(True),
)

###############################################################################
# Unified B Meson Ntuplizers
###############################################################################
from VertexCompositeAnalysis.VertexCompositeAnalyzer.bToJpsiHadronsNtuplizer_cfi import bToJpsiHadronsNtuplizer

# Ntuplizer for mu+mu- channel
process.bTreeMuMu = bToJpsiHadronsNtuplizer.clone(
    candidateSrc = cms.InputTag("bToJpsiHadronsMuMu", "BToJpsiHadrons"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
    centralitySrc = cms.InputTag("hiCentrality"),
)

# Ntuplizer for e+e- channel
process.bTreeEE = bToJpsiHadronsNtuplizer.clone(
    candidateSrc = cms.InputTag("bToJpsiHadronsEE", "BToJpsiHadrons"),
    pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
    isCentrality = cms.untracked.bool(True),
    centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
    centralitySrc = cms.InputTag("hiCentrality"),
)

###############################################################################
# Chi_c Analysis (J/psi + photon conversion)
###############################################################################
# Uses OniaPhotonProducer to combine J/psi candidates with converted photons
# from oniaPhotonCandidates:conversions (already in MiniAOD)
#
# Configuration options
doChiCAnalysis = True  # Enable/disable chi_c analysis
doChiCKinematicRefit = False  # Kinematic refit with J/psi mass constraint

if doChiCAnalysis:
    from VertexCompositeAnalysis.VertexCompositeProducer.oniaPhotonCandidates_cfi import chiCandidates
    
    # Chi_c producer from J/psi (mu+mu-) + gamma
    process.chiCandidatesMuMu = chiCandidates.clone(
        oniaSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
        conversionSrc = cms.InputTag("oniaPhotonCandidates", "conversions"),
        pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
        beamSpotSrc = cms.InputTag("offlineBeamSpot"),
        
        # Conversion cuts
        minConvPt = cms.double(0.5),
        minConvRho = cms.double(0.0),  # Already cut at 1.5 cm in oniaPhotonCandidates
        rejectPi0 = cms.bool(False),   # Don't reject based on pi0 flag
        
        # J/psi cuts
        minOniaPt = cms.double(3.0),
        maxOniaAbsY = cms.double(2.4),
        requireTriggerMatch = cms.bool(False),
        
        # Matching cuts
        dzMax = cms.double(1.0),       # |dz| between conversion and J/psi vertex
        deltaMassRange = cms.vdouble(0.0, 2.0),  # deltaM = M(chi) - M(J/psi)
        
        # Combined candidate cuts
        minCandPt = cms.double(0.0),
        maxCandAbsY = cms.double(2.4),
        
        # Kinematic refit
        doKinematicRefit = cms.bool(doChiCKinematicRefit),
        constraintMass = cms.double(3.0969),  # J/psi PDG mass
        
        parentType = cms.string("jpsi"),
    )
    
    # Chi_c producer from J/psi (e+e-) + gamma
    process.chiCandidatesEE = chiCandidates.clone(
        oniaSrc = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
        conversionSrc = cms.InputTag("oniaPhotonCandidates", "conversions"),
        pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
        beamSpotSrc = cms.InputTag("offlineBeamSpot"),
        
        # Conversion cuts
        minConvPt = cms.double(0.5),
        minConvRho = cms.double(0.0),
        rejectPi0 = cms.bool(False),
        
        # J/psi cuts (wider window for electrons due to bremsstrahlung)
        minOniaPt = cms.double(3.0),
        maxOniaAbsY = cms.double(2.4),
        requireTriggerMatch = cms.bool(False),
        
        # Matching cuts
        dzMax = cms.double(1.0),
        deltaMassRange = cms.vdouble(0.0, 2.0),
        
        # Combined candidate cuts
        minCandPt = cms.double(0.0),
        maxCandAbsY = cms.double(2.4),
        
        # Kinematic refit
        doKinematicRefit = cms.bool(doChiCKinematicRefit),
        constraintMass = cms.double(3.0969),
        
        parentType = cms.string("jpsi"),
    )

###############################################################################
# Chi_c Ntuplizer
###############################################################################
# Simple TTree producer for chi_c candidates
if doChiCAnalysis:
    from VertexCompositeAnalysis.VertexCompositeAnalyzer.chiCNtupleProducer_cfi import chiCNtupleProducer
    
    process.chiTreeMuMu = chiCNtupleProducer.clone(
        candidateSrc = cms.InputTag("chiCandidatesMuMu"),
        pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
        isCentrality = cms.untracked.bool(True),
        centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
        centralitySrc = cms.InputTag("hiCentrality"),
        treeName = cms.untracked.string("chiTree"),
    )
    
    process.chiTreeEE = chiCNtupleProducer.clone(
        candidateSrc = cms.InputTag("chiCandidatesEE"),
        pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"),
        isCentrality = cms.untracked.bool(True),
        centralityBinLabel = cms.InputTag("centralityBin", "HFtowers"),
        centralitySrc = cms.InputTag("hiCentrality"),
        treeName = cms.untracked.string("chiTree"),
    )

###############################################################################
# Vertex Analyzer
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.vertexAnalyzer_cfi')
process.vertexAnalyzer.vertexSrc = cms.InputTag('unpackedTracksAndVertices')
process.vertexAnalyzer.beamSpotSrc = cms.InputTag('offlineBeamSpot')

###############################################################################
# Main Forest Path
###############################################################################
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.hiEvtAnalyzer +
    process.hltanalysis +
    process.particleFlowAnalyser +
    process.unpackedTracksAndVertices +
    process.unpackedMuons +
#    process.vertexAnalyzer +
    process.akCs4PFJetAnalyzer +
    process.akPu4CaloJetAnalyzer
)

# Add b-tagging path if enabled
if doBtagging:
    process.forest.associate(process.patAlgosToolsTask)
    process.btagPath = cms.Path(process.akCs4PFJetAnalyzer_btag)

###############################################################################
# Onia analysis path
###############################################################################
# Add merged electrons producer if needed
if electronSource == "merged":
    process.oniaPath = cms.Path(process.mergedElectrons + process.oniaTreeAna)
else:
    process.oniaPath = cms.Path(process.oniaTreeAna)

###############################################################################
# Unified B meson paths
###############################################################################
process.bMesonMuMuPath = cms.Path(
    process.bToJpsiHadronsMuMu +
    process.bTreeMuMu
)

# Add merged electrons producer if needed for B meson EE path
if electronSource == "merged":
    process.bMesonEEPath = cms.Path(
        process.mergedElectrons +
        process.bToJpsiHadronsEE +
        process.bTreeEE
    )
else:
    process.bMesonEEPath = cms.Path(
        process.bToJpsiHadronsEE +
        process.bTreeEE
    )

###############################################################################
# Chi_c analysis paths
###############################################################################
if doChiCAnalysis:
    process.chiCMuMuPath = cms.Path(
        process.chiCandidatesMuMu +
        process.chiTreeMuMu
    )
    
    if electronSource == "merged":
        process.chiCEEPath = cms.Path(
            process.mergedElectrons +
            process.chiCandidatesEE +
            process.chiTreeEE
        )
    else:
        process.chiCEEPath = cms.Path(
            process.chiCandidatesEE +
            process.chiTreeEE
        )

###############################################################################
# Event Selection Filters
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)

process.pclusterCompatibilityFilter = cms.Path(process.clusterCompatibilityFilter)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)

process.pAna = cms.EndPath(process.skimanalysis)

###############################################################################
# Schedule with early termination
###############################################################################
process.forest.insert(0, process.eventFilter)
process.oniaPath.insert(0, process.eventFilter)
process.bMesonMuMuPath.insert(0, process.eventFilter)
process.bMesonEEPath.insert(0, process.eventFilter)

# Build schedule
scheduleList = [
    process.forest,
    process.oniaPath,
    process.bMesonMuMuPath,
    process.bMesonEEPath,
]

# Add chi_c paths if enabled
if doChiCAnalysis:
    process.chiCMuMuPath.insert(0, process.eventFilter)
    process.chiCEEPath.insert(0, process.eventFilter)
    scheduleList.append(process.chiCMuMuPath)
    scheduleList.append(process.chiCEEPath)

# Add b-tagging path if enabled
if doBtagging:
    process.btagPath.insert(0, process.eventFilter)
    scheduleList.append(process.btagPath)

scheduleList.extend([
    process.pclusterCompatibilityFilter,
    process.pprimaryVertexFilter,
    process.pAna,
])

process.schedule = cms.Schedule(*scheduleList)

###############################################################################
# Summary
###############################################################################
# Output trees:
#   - hiEvtAnalyzer/HiTree       : Event info
#   - hltanalysis/HltTree        : Trigger info  
#   - hionia/myTree              : J/psi -> mu+mu- (with Reco_QQ_collIdx for B matching)
#   - hioniaElectrons/eleTree    : J/psi -> e+e-
#   - akCs4PFJetAnalyzer/t       : Jets (basic, no b-tagging)
#   - akCs4PFJetAnalyzer_btag/t  : Jets with UParT b-tagging (if doBtagging=True)
#   - bTreeMuMu/bTree            : Unified B mesons from mu+mu- (with jpsiIdx)
#   - bTreeEE/bTree              : Unified B mesons from e+e- (with jpsiIdx)
#   - chiTreeMuMu/chiTree        : Chi_c from mu+mu- + gamma (if doChiCAnalysis=True)
#   - chiTreeEE/chiTree          : Chi_c from e+e- + gamma (if doChiCAnalysis=True)
#
# Jet b-tagging (akCs4PFJetAnalyzer_btag/t, if doBtagging=True):
#   Uses UParT (Unified Particle Transformer) with slimmedSecondaryVertices
#   Discriminators: discr_unifiedParticleTransformer (b+bb+lepb combined)
#   Individual probs: probb, probbb, problepb, probc, probg, probu, probd, probs
#   Tau probs: probtaup*, probtaum*
#   Lepton: probele, probmu
#   Corrections: ptcorr, ptnu
#
# B tree structure (one entry per event, vectors for candidates):
#   - B_mass, B_pt, B_eta, B_phi, B_y, B_charge
#   - B_decayId (0=B+K, 1=B+pi, 2=B0Kpi, 3=BsKK, 4=Bcpi, 5=Xpipi)
#   - B_nTracks (1 or 2), B_hasDau2 (0 or 1)
#   - B_vtxProb, B_lxy, B_lxySig, B_cosAlpha
#   - jpsiIdx -> matches Reco_QQ_collIdx in hionia tree
#   - Jpsi_mass, Jpsi_pt, Jpsi_eta, Jpsi_phi
#   - trk1_pt, trk1_eta, trk1_phi, trk1_charge, trk1_massHypo
#   - trk2_pt, trk2_eta, trk2_phi, trk2_charge, trk2_massHypo (=-999 for 1-track modes)
#   - resMass (phi, K*, or pipi mass for 2-track modes)
#
# Chi_c tree structure (if doChiCAnalysis=True):
#   Event: runNb, eventNb, lsNb, centrality, Ntrkoffline
#   Chi_c: chi_mass, chi_pt, chi_eta, chi_phi, chi_y
#   Indices: oniaIdx (->Reco_QQ_collIdx), convIdx
#   Matching: dz, deltaM, correctedMass
#   J/psi: onia_mass, onia_pt, onia_eta, onia_phi, onia_y, onia_vProb
#   Photon: conv_pt, conv_eta, conv_phi, conv_mass
#   Conv vertex: convVtx_x, convVtx_y, convVtx_z, convVtx_rho, conv_flags
#   Electron 1: ele1_pt, ele1_eta, ele1_phi, ele1_charge, ele1_chi2, ele1_nHits, ele1_dxy, ele1_dz
#   Electron 2: ele2_pt, ele2_eta, ele2_phi, ele2_charge, ele2_chi2, ele2_nHits, ele2_dxy, ele2_dz
#   Refit (optional): refit_mass, refit_vProb, refit_ctauPV, refit_ctauErrPV, refit_cosAlpha
###############################################################################
process.options.numberOfThreads= 2
process.options.numberOfStreams= 1
