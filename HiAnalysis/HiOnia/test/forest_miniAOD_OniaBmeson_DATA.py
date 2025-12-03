### HiForest Configuration with Onia and B meson analysis
# Input: miniAOD
# Type: data
# Includes: Event, Jet, OniaMuMu, OniaEE, BMeson (from mumu and ee)

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_pp_on_PbPb_2024_cff import Run3_pp_on_PbPb_2024
process = cms.Process('HiForest', Run3_pp_on_PbPb_2024)

###############################################################################
# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 150X, data, OniaBmeson")

###############################################################################
# Configuration flags
isMC = False
HLTProName = 'HLT'

###############################################################################
# Input files
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/hidata/HIRun2024A/HIPhysicsRawPrime2/MINIAOD/PromptReco-v1/000/387/908/00000/93a76f4f-4e90-4357-9a7f-3c64a1be8e29.root'
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

###############################################################################
# Load Global Tag, geometry, etc.
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

# ZDC conditions for 2024 data
process.es_prefer = cms.ESPrefer('HcalTextCalibrations', 'es_ascii')
process.es_ascii = cms.ESSource('HcalTextCalibrations',
    input = cms.VPSet(
        cms.PSet(
            object = cms.string('Gains'),
            file = cms.FileInPath('HeavyIonsAnalysis/Configuration/data/ZDCConditions_1400V/DumpGainsForUpload_AllChannels.txt')
        ),
        cms.PSet(
            object = cms.string('TPChannelParameters'),
            file = cms.FileInPath('HeavyIonsAnalysis/Configuration/data/ZDCConditions_1400V/DumpTPChannelParameters_Run387473.txt')
        ),
    )
)

###############################################################################
# Centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

###############################################################################
# Output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestOniaBmeson.root")
)

###############################################################################
# Event Analysis
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')

###############################################################################
# Track unpacking (needed for B meson reconstruction)
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")

###############################################################################
# Muon Analysis
process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
process.load("HeavyIonsAnalysis.MuonAnalysis.muonAnalyzer_cfi")

###############################################################################
# Jet Analysis
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_data_cff')
process.load('HeavyIonsAnalysis.JetAnalysis.akPu4CaloJetSequence_pponPbPb_data_cff')
process.akPu4CaloJetAnalyzer.doHiJetID = True

###############################################################################
# ZDC Analysis
process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load('HeavyIonsAnalysis.ZDCAnalysis.ZDCAnalyzersPbPb_cff')

###############################################################################
# Onia MuMu Analysis
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer

muonTriggerList = {
    'DoubleMuonTrigger': cms.vstring(
        "HLT_HIL1DoubleMu0_v*",
        "HLT_HIL1DoubleMu10_v*",
        "HLT_HIL2DoubleMuOpen_v*",
        "HLT_HIL3DoubleMuOpen_v*",
    ),
    'SingleMuonTrigger': cms.vstring(
        "HLT_HIL1SingleMu0_v*",
        "HLT_HIL2SingleMu3_v*",
        "HLT_HIL3SingleMu3_v*",
        "HLT_HIL3SingleMu5_v*",
        "HLT_HIL3SingleMu7_v*",
    ),
}

electronTriggerList = {
    'SingleElectronTrigger': cms.vstring(
        "HLT_HIEle10Gsf_v*",
        "HLT_HIEle15Gsf_v*",
        "HLT_HIEle20Gsf_v*",
    ),
    'DoubleElectronTrigger': cms.vstring(
        "HLT_HIDoubleEle10Gsf_v*",
        "HLT_HIDoubleEle15Gsf_v*",
    ),
}

oniaTreeAnalyzer(process,
    muonTriggerList=muonTriggerList,
    HLTProName=HLTProName,
    muonSelection="All",
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

# Configure onia analyzers for HI
process.hionia.isHI = cms.untracked.bool(True)
process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
process.hionia.useEvtPlane = cms.untracked.bool(True)
process.hionia.EvtPlane = cms.InputTag("hiEvtPlaneFlat")

###############################################################################
# B Meson Production and Analysis (from dimuon)
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiLeptonCandidates_cfi")

# B meson producer from dimuon
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

# B meson producer from dielectron
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
# B Meson Ntuplizer
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.bDiMuMuNtuplizer_cfi")

# Ntuplizer for B from dimuon
process.bMesonMuMuNtuplizer = process.bDiMuMuNtuplizer.clone(
    bPlusCollection = cms.InputTag("bMesonMuMu:BPlus"),
    bZeroCollection = cms.InputTag("bMesonMuMu:BZero"),
    bcCollection = cms.InputTag("bMesonMuMu:Bc"),
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(True),
    isMC = cms.bool(isMC),
)

# Ntuplizer for B from dielectron
process.bMesonEENtuplizer = process.bDiMuMuNtuplizer.clone(
    bPlusCollection = cms.InputTag("bMesonEE:BPlus"),
    bZeroCollection = cms.InputTag("bMesonEE:BZero"),
    bcCollection = cms.InputTag("bMesonEE:Bc"),
    saveBPlus = cms.bool(True),
    saveBZero = cms.bool(True),
    saveBc = cms.bool(True),
    isMC = cms.bool(isMC),
)

###############################################################################
# Main Forest Sequence
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.hiEvtAnalyzer +
    process.hltanalysis +
    process.l1object +
    process.trackSequencePbPb +
    process.particleFlowAnalyser +
    process.unpackedMuons +
    process.muonAnalyzer +
    process.zdcSequencePbPb +
    process.akPu4CaloJetAnalyzer
)

# Onia analysis sequence (includes both mumu and ee)
process.oniaPath = cms.Path(process.oniaTreeAna)

# B meson from dimuon sequence
process.bMesonMuMuPath = cms.Path(
    process.bMesonMuMu +
    process.bMesonMuMuNtuplizer
)

# B meson from dielectron sequence
process.bMesonEEPath = cms.Path(
    process.bMesonEE +
    process.bMesonEENtuplizer
)

###############################################################################
# Event Selection Filters
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.pclusterCompatibilityFilter = cms.Path(process.clusterCompatibilityFilter)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)

process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')
process.pphfCoincFilter2Th4 = cms.Path(process.phfCoincFilter2Th4)
process.pphfCoincFilter4Th2 = cms.Path(process.phfCoincFilter4Th2)

process.pAna = cms.EndPath(process.skimanalysis)

###############################################################################
# Schedule
process.schedule = cms.Schedule(
    process.forest,
    process.oniaPath,
    process.bMesonMuMuPath,
    process.bMesonEEPath,
    process.pclusterCompatibilityFilter,
    process.pprimaryVertexFilter,
    process.pphfCoincFilter2Th4,
    process.pphfCoincFilter4Th2,
    process.pAna,
)
