### HiForest Configuration
# Collisions: pp
# Type: Data
# Input: miniAOD

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_2024_ppRef_cff import Run3_2024_ppRef
process = cms.Process('HiForest', Run3_2024_ppRef)
process.options = cms.untracked.PSet()

process.options.numberOfThreads = 1
process.options.numberOfStreams = 0


#####################################################################################
# HiForest labelling info
#####################################################################################

process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 141X, data")

#####################################################################################
# Input source
#####################################################################################

process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
#        '/store/data/Run2024J/PPRefHardProbes4/MINIAOD/PromptReco-v1/000/387/570/00000/c855cc0a-2470-4978-acbb-e4618979cf0e.root'
#        'file:/afs/cern.ch/work/s/soohwan/private/Analysis/General2024Analysis/CMSSW_14_1_4_patch1/src/VertexCompositeAnalysis/VertexCompositeProducer/test/3be8f20e-6df1-4678-baed-b3c65b5ac756.root',

    # 'file:/eos/home-s/soohwan/store/DataRun3/2024/ppRef/060df347-423a-446b-9e63-ebc8730a3ba9.root',
'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/1817e6b2-c106-47bc-a105-3fe4c9142048.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/42d5456d-5863-4a72-b80e-64ef7851c206.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/71cb6085-452f-48a9-891b-659985118537.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/565dbe34-5881-4510-bdbe-d80d01d39eee.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/baa4e1cf-24a0-4a89-b87f-4a7e5acf3ef6.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/2c0b5aae-6401-4aa7-8f11-ffae24ff694b.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/288bb825-6f7c-4c44-a9e5-bcb3798a9d92.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/0467d4fc-0a7f-49c8-8422-8a56da46c0d2.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/6fcb971f-426f-4012-9665-6b7f07431e1b.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/7b158f94-1593-410c-8217-4568b380e4c2.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/4c137d1c-3919-4ab1-b346-737c767cfc46.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/55b42c85-ed26-432d-b919-2278df25547e.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/67329610-2d84-4590-a837-28e08ceae396.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/2cdfa8b4-29c8-452e-8414-15e6fcfb902e.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/1bb53bd3-4dbc-4c39-8309-afdf74fd96f4.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/53fb8e58-c917-4ffe-bc1f-f93792cd6130.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/803d1c0e-687c-4a60-9673-5b4b39adab53.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/076cbe8e-b2d7-4ad5-8b58-5391fb80e1e0.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/d66a4db9-f748-4fb5-84fd-37daf0a9a985.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/661d5b63-7be9-420a-8ea7-ff4998f9b550.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/a1360d53-cbe3-46e9-890f-8669738fca2d.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/e981f36f-face-4903-9fe2-8523c48e13e4.root',
#'/store/data/Run2024J/PPRefDoubleMuon0/MINIAOD/PromptReco-v1/000/387/696/00000/d2250f81-f39f-47dc-869f-f2ebc9580d42.root',
    )
)

# Number of events we want to process, -1 = all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

#####################################################################################
# Load Global Tag, Geometry, etc.
#####################################################################################

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

#GlobalTag used in Prompt RECO
#https://cms-conddb.cern.ch/cmsDbBrowser/list/Prod/gts/132X_dataRun3_Prompt_v3
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/Cert_Collisions2024_ppref_387474_387721_Muon.json').getVLuminosityBlockRange()


# FIXME: Old calibration here, might need to update
# Commenting out until understood
#process.GlobalTag.toGet.extend([
#    cms.PSet(record = cms.string("BTagTrackProbability3DRcd"),
#             tag = cms.string("JPcalib_MC94X_2017pp_v2"),
#             connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS")
#
#         )
#      ])

#####################################################################################
# Define tree output
#####################################################################################

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestMiniAOD_Upsilon.root"))

#####################################################################################
# Additional Reconstruction and Analysis: Main Body
#####################################################################################

#############################
# Jets
#############################
process.load("HeavyIonsAnalysis.JetAnalysis.ak4PFJetSequence_ppref_data_cff")

#####################################################################################

############################
# Event Analysis
############################
# use data version to avoid PbPb MC
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.hiEvtAnalyzer.Vertex = cms.InputTag("offlineSlimmedPrimaryVertices")
process.hiEvtAnalyzer.doCentrality = cms.bool(False)
process.hiEvtAnalyzer.doEvtPlane = cms.bool(False)
process.hiEvtAnalyzer.doEvtPlaneFlat = cms.bool(False)
#Turn off MC info
process.hiEvtAnalyzer.doMC = cms.bool(False) # general MC info
process.hiEvtAnalyzer.doHiMC = cms.bool(False) # HI specific MC info
process.hiEvtAnalyzer.doHFfilters = cms.bool(False) # Disable HF filters for ppRef

process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltobject_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')

process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')

process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')

# FIXME: Do we have an updated trigger list?
#from HeavyIonsAnalysis.EventAnalysis.hltobject_cfi import trigger_list_data_2023_skimmed
#process.hltobject.triggerNames = trigger_list_data_2023_skimmed

#####################################################################################
# Onia Jets

triggerList    = {
		# Double Muon Trigger List
		'DoubleMuonTrigger' : cms.vstring(
            "HLT_PPRefL1DoubleMu0_Open_v",
            "HLT_PPRefL1DoubleMu0_v",
            "HLT_PPRefL1DoubleMu0_SQ_v",
            "HLT_PPRefL1DoubleMu2_v",
            "HLT_PPRefL1DoubleMu2_SQ_v",
            "HLT_PPRefL2DoubleMu0_Open_v",
            "HLT_PPRefL2DoubleMu0_v",
            "HLT_PPRefL3DoubleMu0_Open_v",
            "HLT_PPRefL3DoubleMu0_v"
            ),
        # Single Muon Trigger List
        'SingleMuonTrigger' : cms.vstring(
            "HLT_PPRefL1SingleMu7_v",
            "HLT_PPRefL1SingleMu12_v",
            "HLT_PPRefL2SingleMu7_v",
            "HLT_PPRefL2SingleMu12_v",
            "HLT_PPRefL2SingleMu15_v",
            "HLT_PPRefL2SingleMu20_v",
            "HLT_PPRefL3SingleMu3_v",
            "HLT_PPRefL3SingleMu5_v",
            "HLT_PPRefL3SingleMu7_v",
            "HLT_PPRefL3SingleMu12_v",
            "HLT_PPRefL3SingleMu15_v",
            "HLT_PPRefL3SingleMu20_v",
			)
                }

from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList= triggerList,
                 HLTProName="HLT",
                #  muonSelection="GlbTrk", 
                 muonSelection="All", 
                 L1Stage=2, 
                 isMC=False, 
                 pdgID=443, 
                 outputFileName= "", 
                 doTrimu=False,
                 OnlySingleMuons=False
)

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("(6 < mass && mass < 14)  && pt > 0")
process.onia2MuMuPatGlbGlb.lowerPuritySelection       = cms.string("pt>2 && abs(eta) < 2.4")
process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.000")

process.hionia.primaryVertexTag = "unpackedTracksAndVertices"
process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
process.hionia.muonLessPV       = cms.bool(False)
process.hionia.SumETvariables   = cms.bool(False)
process.hionia.applyCuts        = cms.bool(False)
process.hionia.AtLeastOneCand   = cms.bool(False)
process.hionia.OneMatchedHLTMu  = cms.int32(-1)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)
process.hionia.mom4format       = cms.string("vector")
process.hionia.isHI = cms.untracked.bool(False)

from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
changeToMiniAOD(process)
process.unpackedMuons.addPropToMuonSt = cms.bool(True)
################################
# jet reco sequence
process.load('HeavyIonsAnalysis.JetAnalysis.ak4PFJetSequence_ppref_data_cff')

# HF jet reco sequence
process.load("RecoHI.HiJetAlgos.PFCandCompositeProducer_cfi")
#process.pfCandComposites.pfCandTag    = cms.InputTag('particleFlowNoHF')
process.pfCandComposites.pfCandTag    = cms.InputTag('packedPFCandidates')
process.pfCandComposites.replaceOniaMM = True
#process.pfCandComposites.massOnia = 3.096900 # J/psi
process.pfCandComposites.massOnia = 3.686097 # psi(2S)
process.pfCandComposites.compositeTag = cms.InputTag("onia2MuMuPatGlbGlb")
process.pfCandComposites.jpsiTrigFilter = cms.string('hltL1fL1sDoubleMu0L1Filtered0PPRef')
process.onia2MuMuPatGlbGlb.addMuonlessPrimaryVertex = False

from HeavyIonsAnalysis.JetAnalysis.clusterJetsFromMiniAOD_cff import setupPprefJets

# process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
#process.unpackedMuons.muonSelectors = []

#temporarily remove vertex refit
process.pfCandComposites.isHI = True


#########################
# Track Analyzer
#########################
# process.load('HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff')

#####################################################################################

#####################
# photons
######################
# process.load('HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi')
# process.ggHiNtuplizer.doGenParticles = cms.bool(False)
# process.ggHiNtuplizer.doMuons = cms.bool(False) # unpackedMuons collection not found from file
# process.ggHiNtuplizer.useValMapIso = cms.bool(False) # True here causes seg fault
# process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

####################################################################################

#########################
# ZDC RecHit Producer && Analyzer
#########################
# to prevent crash related to HcalSeverityLevelComputerRcd record
# process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
# process.load('HeavyIonsAnalysis.ZDCAnalysis.ZDCAnalyzersPP_cff')

###############################################################################

#########################
# Main analysis list
#########################

process.forest = cms.Path(
    process.HiForestInfo +
    process.hltanalysis *
    process.hiEvtAnalyzer *
#    process.hltobject +
    process.l1object +
    # process.ggHiNtuplizer +
    # process.zdcSequencePP +
    # process.trackSequencePP +
    process.particleFlowAnalyser
)


# Schedule definition
process.pAna = cms.EndPath(process.skimanalysis)

process.primaryVertexFilter = cms.EDFilter("VertexSelector",
    src = cms.InputTag("offlineSlimmedPrimaryVertices"),
    cut = cms.string("!isFake && abs(z) <= 25 && position.Rho <= 2"), #in miniADO trackSize()==0, however there is no influence.
    filter = cms.bool(True), # otherwise it won't filter the event
)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)


#####################################################################################

addR2Jets = False
addR3Jets = True
addR4Jets = True
addR5Jets = True
addR8Jets = False

if addR2Jets or addR3Jets or addR4Jets or addR8Jets:
    process.load("HeavyIonsAnalysis.JetAnalysis.extraJets_cff")
    from HeavyIonsAnalysis.JetAnalysis.clusterJetsFromMiniAOD_cff import setupPprefJets

    if addR2Jets :
        process.jetsR2 = cms.Sequence()
        setupPprefJets('ak2PF', process.jetsR2, process, isMC = 0, radius = 0.20, JECTag = 'AK2PF')
        process.ak2PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak2PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak2PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak2PFpatJets", jetName = 'ak2PF', genjetTag = "ak2GenJetsNoNu")
        process.forest += process.jetsR2 * process.ak2PFJetAnalyzer

    if addR3Jets :
        process.jetsR3 = cms.Sequence()
        setupPprefJets('ak3PF', process.jetsR3, process, isMC = 0, radius = 0.30, JECTag = 'AK3PF')
        process.ak3PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak3PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak3PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak3PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak3PFpatJets", jetName = 'ak3PF', genjetTag = "ak3GenJetsNoNu")
        process.forest += process.jetsR3 * process.ak3PFJetAnalyzer

    if addR4Jets :
        # Recluster using an alias "0" in order not to get mixed up with the default AK4 collections
        process.jetsR4 = cms.Sequence()
        setupPprefJets('ak04PF', process.jetsR4, process, isMC = 0, radius = 0.40, JECTag = 'AK4PF')
        process.ak04PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak04PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak04PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak4PFJetAnalyzer.jetTag = 'ak04PFpatJets'
        process.ak4PFJetAnalyzer.jetName = 'ak04PF'
        process.ak4PFJetAnalyzer.doSubEvent = False # Need to disable this, since there is some issue with the gen jet constituents. More debugging needed is want to use constituents.
        process.forest += process.jetsR4 * process.ak4PFJetAnalyzer
    if addR5Jets :
        # Recluster using an alias "0" in order not to get mixed up with the default AK4 collections
        process.jetsR5 = cms.Sequence()
        setupPprefJets('ak5PF', process.jetsR5, process, isMC = 0, radius = 0.50, JECTag = 'AK5PF')
        process.ak5PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak5PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak5PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak5PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak5PFpatJets", jetName = 'ak5PF', genjetTag = "ak5GenJetsNoNu")
        process.ak5PFJetAnalyzer.jetTag = 'ak5PFpatJets'
        process.ak5PFJetAnalyzer.jetName = 'ak5PF'
        process.ak5PFJetAnalyzer.doSubEvent = False # Need to disable this, since there is some issue with the gen jet constituents. More debugging needed is want to use constituents.
        process.forest += process.jetsR5 * process.ak5PFJetAnalyzer

    if addR8Jets :
        process.jetsR8 = cms.Sequence()
        setupPprefJets('ak8PF', process.jetsR8, process, isMC = 0, radius = 0.80, JECTag = 'AK8PF')
        process.ak8PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak8PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak8PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak8PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak8PFpatJets", jetName = 'ak8PF', genjetTag = "ak8GenJetsNoNu")
        process.forest += process.jetsR8 * process.ak8PFJetAnalyzer

else:
    process.forest+= process.ak4PFJetAnalyzer


# process.load("PhysicsTools.JetMCAlgos.SelectPartons_cff")
# process.allPartons = process.myPartons.clone(src = "prunedGenParticles")


# process.load("GeneratorInterface.RivetInterface.mergedGenParticles_cfi")

# process.load("RecoJets.Configuration.GenJetParticles_cff")
# process.genParticlesForJets.src = 'mergedGenParticles'
# process.genParticlesForJets.storeJMM = cms.untracked.bool(True)

process.load("RecoHI.HiJetAlgos.dynGroomedGenJets_cfi")
process.dynGroomedGenJets.chargedOnly = cms.bool(False)
process.dynGroomedGenJets.jetSrc = cms.InputTag("ak4GenJetsNoNu")
process.dynGroomedGenJets.constitSrc = cms.InputTag("genParticlesForJets")


process.load("RecoHI.HiJetAlgos.dynGroomedPatJets_cfi")
process.dynGroomedPatJets.chargedOnly = cms.bool(False)
process.dynGroomedPatJets.jetSrc = cms.InputTag("ak4PFXpatJets")
process.dynGroomedPatJets.constitSrc = cms.InputTag("pfCandComposites")



#process.ak4PFXpatJets = cms.EDFilter("PatJetXSelector",
#                                     src = cms.InputTag("ak04PFpatJets"),
#                                     cut = cms.string("pt > 0.0 && abs(rapidity()) < 3.")
#                                     )
#
#process.ak5PFXpatJets = cms.EDFilter("PatJetXSelector",
#                                     src = cms.InputTag("ak05PFpatJets"),
#                                     cut = cms.string("pt > 0.0 && abs(rapidity()) < 3.")
#                                     )
#
#process.jetsR4.remove(process.ak4PFJetAnalyzer)
#process.ak4PFJetAnalyzer.jetTag = "ak4PFXpatJets"
## process.ak4PFJetAnalyzer.doCandidateBtagging = False
#process.ak4PFJetAnalyzer.doSubJets = False
#process.ak4PFJetAnalyzer.doSubJetsNew = True
#process.ak4PFJetAnalyzer.jetName = 'ak04PF'
#process.ak4PFJetAnalyzer.doSubEvent = False 
#process.ak4PFJetAnalyzer.groomedJets = cms.untracked.InputTag("dynGroomedPatJets")
#process.ak4PFJetAnalyzer.groomedGenJets = cms.untracked.InputTag("dynGroomedGenJets")
#process.ak4PFJetAnalyzer.genjetTag = "ak4GenJetsNoNu"
#process.ak4PFJetAnalyzer.doGenSubJets = False


# Kine cut
process.ak04PFJets.jetPtMin = 0.
process.ak4PFJetAnalyzer.jetPtMin = 0.

process.ak5PFJets.jetPtMin = 0.
process.ak5PFJetAnalyzer.jetPtMin = 0.

process.jpsiJets = cms.Sequence(
    # process.particleFlowNoHF * 
    # process.hiSignalGenParticles *
    process.pfCandComposites *
    # process.allPartons *
    # process.mergedGenParticles *
    # process.genParticlesForJets *
    # process.jetsR3 *
    process.jetsR4 *
    # process.jetsR5 *
    # process.ak4PFXpatJets *
    # process.ak5PFXpatJets *
    # process.ak8PFXpatJets *
    # process.dynGroomedGenJets *
    # process.dynGroomedPatJets *
    # process.ak3PFJetAnalyzer *
    process.ak4PFJetAnalyzer 
    # process.ak5PFJetAnalyzer
)

import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltHI = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltHI.HLTPaths = ["HLT_PPRefL*DoubleMu0_v*","HLT_PPRefL*SingleMu*_v*"]
process.hltHI.throw = False
process.hltHI.andOr = True
process.oniaTreeAna.replace(process.patMuonSequence, process.primaryVertexFilter * process.hltHI * process.patMuonSequence )

process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalOttCandidates_cff")
process.generalOttCandidatesNew = process.generalOttCandidates.clone()
process.generalOttCandidatesNew.dimuons = cms.InputTag('onia2MuMuPatGlbGlb')
process.generalOttCandidatesNew.vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices')
process.generalOttCandidatesNew.trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices')

#process.generalOttCandidatesNew.vertexRecoAlgorithm = cms.InputTag('offlineSlimmedPrimaryVertices')
process.generalOttCandidatesNew.usePixelTracks = cms.bool(False)
process.generalOttCandidatesNew.pixelTracks = cms.InputTag('unpackedPixelTracks')

process.generalOttCandidatesNew.batTrkPtSumCut = cms.double(0.1)
process.generalOttCandidatesNew.batTrkEtaDiffCut = cms.double(2.4)
process.generalOttCandidatesNew.batTkChi2Cut = cms.double(3)
process.generalOttCandidatesNew.batTkNhitsCut = cms.int32(0)
process.generalOttCandidatesNew.batTkPtErrCut = cms.double(0.1)
process.generalOttCandidatesNew.batTkPtCut = cms.double(0.20)
process.generalOttCandidatesNew.alphaCut = cms.double(999.0)
process.generalOttCandidatesNew.alpha2DCut = cms.double(999.0)
process.generalOttCandidatesNew.bPtCut = cms.double(0.0)
process.generalOttCandidatesNew.bVtxChiProbCut = cms.double(0.010)
process.generalOttCandidatesNew.bQMassCut = cms.double(0.3)
process.generalOttCandidatesNew.bOniaMass = cms.vdouble([3.096])
process.generalOttCandidatesNew.bOniaWindow = cms.vdouble([999.0]) # This is wrong in cpp
process.generalOttCandidatesNew.bMassCut = cms.double(5)

process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.ottanalyzer_tree_cff")
process.ottana_new = process.ottana.clone()
process.ottana_new.VertexCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.TrackCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.doRecoNtuple = True
process.ottana_new.PID = 20443

process.ottana_new.threeProngDecay = False
process.ottana_new.PID_dau1 = 443
process.ottana_new.PID_dau2 = 113

# Name is Jpsi but it's for upsilon y'a know
# process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb + process.generalOttCandidatesNew + process.jpsiJets + process.ottana_new + process.hionia)
# No ott here for now
#process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb + process.generalOttCandidatesNew + process.jpsiJets + process.hionia)
process.jpsiJetsPath = cms.Path( process.hiEvtAnalyzer + process.patMuonSequence+process.onia2MuMuPatGlbGlb + process.hionia)
#process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb+ process.hionia + process.jpsiJets )

process.schedule = cms.Schedule( process.jpsiJetsPath)

process.output = cms.OutputModule("PoolOutputModule",
   outputCommands = cms.untracked.vstring(["drop *", "keep *_onia2MuMu*_*_*","keep *_*_*_HiForest"]),
   #fileName = cms.untracked.string("/eos/cms/store/group/phys_heavyions/soohwan/Run3_2024/ppRef_2024/output2.root"),
   fileName = cms.untracked.string("output2.root"),
)
#process.output_path = cms.EndPath(process.output)
#process.schedule.append( process.output_path )

process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck",ignoreTotal = cms.untracked.int32(1) )
