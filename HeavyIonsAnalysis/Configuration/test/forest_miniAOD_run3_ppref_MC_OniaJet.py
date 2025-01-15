### HiForest Configuration
# Collisions: pp
# Type: MC
# Input: miniAOD

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_2024_ppRef_cff import Run3_2024_ppRef
process = cms.Process('HiForest', Run3_2024_ppRef)
process.options = cms.untracked.PSet()

#####################################################################################
# HiForest labelling info
#####################################################################################

process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 141X, mc")

#####################################################################################
# Input source
#####################################################################################


print (sys.argv)
i = ''
if len(sys.argv) > 1:
    import json
    json_file = open(sys.argv[2])
    data = json.load(json_file)
    list_file = []
    for element in data:
        list_file.append(element['file'][0]['name'])
    i = int(sys.argv[1])
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( list_file[i*10:(i+1)*10] ))
else:
    process.source = cms.Source("PoolSource",
        duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
         fileNames = cms.untracked.vstring(
             'file:/afs/cern.ch/work/s/soohwan/private/Analysis/General2024Analysis/TEST_CMSSW_14_1_5/CMSSW_14_1_5/src/HeavyIonsAnalysis/Configuration/test/step3_JpsiShower_1.root'
         )
    )

# Number of events we want to process, -1 = all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(13000)
)

#####################################################################################
# Load Global Tag, Geometry, etc.
#####################################################################################

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

# TODO: Global tag complete guess from the list. Probably wrong. But does not crash
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_mcRun3_2024_realistic_ppRef5TeV_v5', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

# TODO: Old calibration here, might need to update
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("BTagTrackProbability3DRcd"),
             tag = cms.string("JPcalib_MC94X_2017pp_v2"),
             connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS")

         )
      ])

#####################################################################################
# Define tree output
#####################################################################################

process.TFileService = cms.Service("TFileService",
    fileName = cms.string((f'HiForestMiniAOD_MC_{i}.root')))

#####################################################################################
# Additional Reconstruction and Analysis: Main Body
#####################################################################################

#############################
# Jets
#############################
process.load("HeavyIonsAnalysis.JetAnalysis.ak4PFJetSequence_ppref_mc_cff")
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
#Turn on MC info
process.hiEvtAnalyzer.doMC = cms.bool(True) # general MC info
process.hiEvtAnalyzer.doHiMC = cms.bool(False) # HI specific MC info
process.hiEvtAnalyzer.doHFfilters = cms.bool(False) # Disable HF filters for ppRef

process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltobject_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')

# TODO: Many of these triggers are not available in the test file
from HeavyIonsAnalysis.EventAnalysis.hltobject_cfi import trigger_list_mc
process.hltobject.triggerNames = trigger_list_mc

# Gen particles
process.load('HeavyIonsAnalysis.EventAnalysis.HiGenAnalyzer_cfi')

#####################################################################################
# Onia Jets
triggerList = {
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
                 muonSelection="Trk", 
                 L1Stage=2, 
                 isMC=True, 
                 pdgID=100443, 
                 outputFileName= "", 
                 doTrimu=False,
                 OnlySingleMuons=False
)

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("(2.5 < mass && mass < 5.0) && charge == 0 && pt > 4.0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25 && daughter('muon1').isPFMuon() && daughter('muon2').isPFMuon()")
process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.01")
process.hionia.primaryVertexTag = "unpackedTracksAndVertices"
process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
# process.hionia.muonLessPV       = cms.bool(False)
process.hionia.SumETvariables   = cms.bool(False)
process.hionia.applyCuts        = cms.bool(False)
process.hionia.AtLeastOneCand   = cms.bool(False)
process.hionia.OneMatchedHLTMu  = cms.int32(-1)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)
process.hionia.mom4format       = cms.string("vector")
process.hionia.isHI = cms.untracked.bool(False)
process.hionia.genealogyInfo    = cms.bool(True)

from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
changeToMiniAOD(process)
process.unpackedMuons.addPropToMuonSt = cms.bool(True)
process.hionia.genParticles = cms.InputTag("prunedGenParticles");
################################
# jet reco sequence
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_mc_cff')

# HF jet reco sequence
process.load("RecoHI.HiJetAlgos.PFCandCompositeProducer_cfi")
#process.pfCandComposites.pfCandTag    = cms.InputTag('particleFlowNoHF')
process.pfCandComposites.pfCandTag    = cms.InputTag('packedPFCandidates')
process.pfCandComposites.replaceOniaMM = True

process.pfCandComposites.massOnia =  3.0969 # Jpsi of the psi(2S)
process.pfCandComposites.compositeTag = cms.InputTag("onia2MuMuPatGlbGlb")
process.pfCandComposites.jpsiTrigFilter = cms.string('hltL1fL1sDoubleMu0L1Filtered0PPRef')
process.onia2MuMuPatGlbGlb.addMuonlessPrimaryVertex = False

from HeavyIonsAnalysis.JetAnalysis.clusterJetsFromMiniAOD_cff import setupPprefJets

# process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
# process.unpackedMuons.muonSelectors = []

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
# process.ggHiNtuplizer.doGenParticles = cms.bool(True)
# process.ggHiNtuplizer.doMuons = cms.bool(False) # unpackedMuons collection not found from file
# process.ggHiNtuplizer.useValMapIso = cms.bool(False) # True here causes seg fault
# process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

####################################################################################

#########################
# Main analysis list
#########################

process.forest = cms.Path(
    process.HiForestInfo +
    process.hltanalysis *
    process.hiEvtAnalyzer *
    process.hltobject +
    process.l1object +
    process.HiGenParticleAna 
    # process.ggHiNtuplizer +
    # process.trackSequencePP
)

#####################################################################################

addR3Jets = True
addR4Jets = True
addR5Jets = True

if addR3Jets or addR4Jets :
    process.load("HeavyIonsAnalysis.JetAnalysis.extraJets_cff")
    from HeavyIonsAnalysis.JetAnalysis.clusterJetsFromMiniAOD_cff import setupPprefJets

    if addR3Jets :
        process.jetsR3 = cms.Sequence()
        setupPprefJets('ak3PF', process.jetsR3, process, isMC = 1, radius = 0.30, JECTag = 'AK3PF')
        process.ak3PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak3PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak3PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak3PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak3PFpatJets", jetName = 'ak3PF', genjetTag = "ak3GenJetsNoNu")
        process.ak3PFpatJets.jetIDMap = 'ak3JetID'
        process.forest += process.extraPpJetsMC * process.jetsR3 * process.ak3PFJetAnalyzer

    if addR4Jets :
        # Recluster using an alias "0" in order not to get mixed up with the default AK4 collections
        process.jetsR4 = cms.Sequence()
        setupPprefJets('ak04PF', process.jetsR4, process, isMC = 1, radius = 0.40, JECTag = 'AK4PF')
        process.ak04PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak04PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak04PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak4PFJetAnalyzer.jetTag = 'ak04PFpatJets'
        process.ak4PFJetAnalyzer.jetName = 'ak04PF'
        process.forest += process.extraPpJetsMC * process.jetsR4 * process.ak4PFJetAnalyzer
    if addR5Jets :
        process.jetsR5 = cms.Sequence()
        setupPprefJets('ak5PF', process.jetsR5, process, isMC = 1, radius = 0.50, JECTag = 'AK5PF')
        process.ak5PFpatJetCorrFactors.levels = ['L2Relative', 'L3Absolute']
        process.ak5PFpatJetCorrFactors.primaryVertices = "offlineSlimmedPrimaryVertices"
        process.ak5PFJets.src = 'pfCandComposites'
        process.load("HeavyIonsAnalysis.JetAnalysis.candidateBtaggingMiniAOD_cff")
        process.ak5PFJetAnalyzer = process.ak4PFJetAnalyzer.clone(jetTag = "ak5PFpatJets", jetName = 'ak5PF', genjetTag = "ak5GenJetsNoNu")
        process.ak5PFpatJets.jetIDMap = 'ak5JetID'
        process.forest += process.extraPpJetsMC * process.jetsR5 * process.ak5PFJetAnalyzer
        
else:
    process.forest+= process.ak4PFJetAnalyzer

process.load("PhysicsTools.JetMCAlgos.SelectPartons_cff")
process.allPartons = process.myPartons.clone(src = "prunedGenParticles")


process.load("GeneratorInterface.RivetInterface.mergedGenParticles_cfi")

process.load("RecoJets.Configuration.GenJetParticles_cff")
process.genParticlesForJets.src = 'mergedGenParticles'
process.genParticlesForJets.storeJMM = cms.untracked.bool(True)

process.load("RecoHI.HiJetAlgos.dynGroomedGenJets_cfi")
process.dynGroomedGenJets.chargedOnly = cms.bool(False)
process.dynGroomedGenJets.jetSrc = cms.InputTag("ak4GenJetsNoNu")
process.dynGroomedGenJets.constitSrc = cms.InputTag("genParticlesForJets")


process.load("RecoHI.HiJetAlgos.dynGroomedPatJets_cfi")
process.dynGroomedPatJets.chargedOnly = cms.bool(False)
process.dynGroomedPatJets.jetSrc = cms.InputTag("ak4PFXpatJets")
process.dynGroomedPatJets.constitSrc = cms.InputTag("pfCandComposites")



process.ak4PFXpatJets = cms.EDFilter("PatJetXSelector",
                                     src = cms.InputTag("ak04PFpatJets"),
                                     cut = cms.string("pt > 0.0 && abs(rapidity()) < 3.")
                                     )
process.jetsR4.remove(process.ak4PFJetAnalyzer)
process.ak4PFJetAnalyzer.jetTag = "ak4PFXpatJets"
# # process.ak4PFJetAnalyzer.doCandidateBtagging = False
process.ak4PFJetAnalyzer.doSubJets = False
process.ak4PFJetAnalyzer.doSubJetsNew = True
process.ak4PFJetAnalyzer.jetName = 'ak04PF'
process.ak4PFJetAnalyzer.doSubEvent = False 
process.ak4PFJetAnalyzer.groomedJets = cms.untracked.InputTag("dynGroomedPatJets")
process.ak4PFJetAnalyzer.groomedGenJets = cms.untracked.InputTag("dynGroomedGenJets")
process.ak4PFJetAnalyzer.genjetTag = "ak4GenJetsNoNu"
process.ak4PFJetAnalyzer.doGenSubJets = False

process.ak04PFJets.jetPtMin = 0.
process.ak4PFJetAnalyzer.jetPtMin = 0.

process.ak3PFXpatJets = cms.EDFilter("PatJetXSelector",
                                     src = cms.InputTag("ak3PFpatJets"),
                                     cut = cms.string("pt > 0.0 && abs(eta()) < 2.")
                                     )

process.jetsR3.remove(process.ak3PFJetAnalyzer)
process.ak3PFJetAnalyzer.jetTag = "ak3PFXpatJets"
# process.ak3PFJetAnalyzer.doCandidateBtagging = False
process.ak3PFJetAnalyzer.doSubJets = False
process.ak3PFJetAnalyzer.doSubJetsNew = True
process.ak3PFJetAnalyzer.jetName = 'ak03PF'
process.ak3PFJetAnalyzer.doSubEvent = False 
process.ak3PFJetAnalyzer.groomedJets = cms.untracked.InputTag("dynGroomedPatJets")
process.ak3PFJetAnalyzer.groomedGenJets = cms.untracked.InputTag("dynGroomedGenJets")
process.ak3PFJetAnalyzer.genjetTag = "ak3GenJetsNoNu"
process.ak3PFJetAnalyzer.doGenSubJets = False

process.ak3PFJets.jetPtMin = 0.
process.ak3PFJetAnalyzer.jetPtMin = 0.

process.jpsiJets = cms.Sequence(
    # process.particleFlowNoHF * 
    process.hiSignalGenParticles *
    process.pfCandComposites *
    process.allPartons *
    process.mergedGenParticles *
    process.genParticlesForJets *
    process.jetsR3 *
    process.jetsR4 *
    # process.jetsR5 *
    process.ak3PFXpatJets *
    process.ak4PFXpatJets *
    # process.dynGroomedGenJets *
    # process.dynGroomedPatJets *
    process.ak3PFJetAnalyzer *
    process.ak4PFJetAnalyzer 
    # process.ak5PFJetAnalyzer
)

import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltHI = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltHI.HLTPaths = ["HLT_PPRefL*DoubleMu*_v*","HLT_PPRefL*SingleMu*_v*"]
process.hltHI.throw = False
process.hltHI.andOr = True
# process.oniaTreeAna.replace(process.patMuonSequence, process.primaryVertexFilter * process.hltHI * process.patMuonSequence )
process.oniaTreeAna.replace(process.patMuonSequence, process.hltHI * process.patMuonSequence )

process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalOttCandidates_cff")
process.generalOttCandidatesNew = process.generalOttCandidates.clone()
process.generalOttCandidatesNew.dimuons = cms.InputTag('onia2MuMuPatGlbGlb')
process.generalOttCandidatesNew.vertexRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices')
process.generalOttCandidatesNew.trackRecoAlgorithm = cms.InputTag('unpackedTracksAndVertices')

#process.generalOttCandidatesNew.vertexRecoAlgorithm = cms.InputTag('offlineSlimmedPrimaryVertices')
process.generalOttCandidatesNew.usePixelTracks = cms.bool(False)
process.generalOttCandidatesNew.pixelTracks = cms.InputTag('unpackedPixelTracks')


process.generalOttCandidatesNew.batTrkPtSumCut = cms.double(0.0)
process.generalOttCandidatesNew.batTrkEtaDiffCut = cms.double(100.0)
process.generalOttCandidatesNew.batTkChi2Cut = cms.double(10000)
process.generalOttCandidatesNew.batTkNhitsCut = cms.int32(0)
process.generalOttCandidatesNew.batTkPtErrCut = cms.double(0.10)
process.generalOttCandidatesNew.batTkPtCut = cms.double(0.10)
process.generalOttCandidatesNew.alphaCut = cms.double(999.0)
process.generalOttCandidatesNew.alpha2DCut = cms.double(999.0)
process.generalOttCandidatesNew.bPtCut = cms.double(4)
process.generalOttCandidatesNew.bVtxChiProbCut = cms.double(0.005)
process.generalOttCandidatesNew.mPiKCutMin = cms.double(0.0)
process.generalOttCandidatesNew.mPiKCutMax = cms.double(40.0)
process.generalOttCandidatesNew.bMassCut = cms.double(7)
process.generalOttCandidatesNew.bQMassCut = cms.double(333)
process.generalOttCandidatesNew.bOniaWindow = cms.vdouble(5.2, 0.2, 0.4, 0.4)
process.generalOttCandidatesNew.trk1cosPhiCut = cms.double(0.0)
process.generalOttCandidatesNew.trk2cosPhiCut = cms.double(0.0)
process.generalOttCandidatesNew.trk12cosPhiCut = cms.double(0.0)
process.generalOttCandidatesNew.trk1dRCut = cms.double(99.0)
process.generalOttCandidatesNew.trk2dRCut = cms.double(99.0)
process.generalOttCandidatesNew.trk12dRCut = cms.double(99.0)
process.generalOttCandidatesNew.oniapTCut = cms.double(0.0)
process.generalOttCandidatesNew.trk1pTCut = cms.double(0.1)
process.generalOttCandidatesNew.trk2pTCut = cms.double(0.1)

#process.load("VertexCompositeAnalysis.VertexCompositeProducer.VertexToPatCompositeCandidateConverter_cfi")
#process.OttCandPatCandidate = process.VertexToPatCompositeCandidateConverter.clone()
#process.OttCandPatCandidate.src = cms.InputTag("generalOttCandidatesNew")

process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.ottanalyzer_tree_cff")
process.ottana_new = process.ottana_mc.clone()
process.ottana_new.VertexCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.TrackCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.doRecoNtuple = True
process.ottana_new.PID = 100443

process.ottana_new.threeProngDecay = True 
process.ottana_new.balancedTree = False 
process.ottana_new.PID_dau1 = 443
#process.ottana_new.PID_dau2 = 113
process.ottana_new.PID_dau2 = -211
process.ottana_new.PID_dau3 = 211

process.ottana_new.doJetConstituentCompare = cms.untracked.bool(True)
process.ottana_new.jetCompareOnlyNonMuons = cms.untracked.bool(True)
process.ottana_new.jetInclDimuon = cms.untracked.bool(True)
process.ottana_new.jetNames = cms.vstring("ak3PFXpatJets", "ak4PFXpatJets")


# process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb + process.jpsiJets + process.hionia)
process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb +  process.onia2MuMuPatGlbGlbFilter + process.generalOttCandidatesNew + process.jpsiJets + process.ottana_new + process.hionia)

# process.Onia2MuMuPAT.remove(process.patTriggerAll)
# process.forest.remove(process.patTriggerAll)


# process.schedule = cms.Schedule( process.forest, process.jpsiJetsPath)
process.schedule = cms.Schedule( process.jpsiJetsPath)


process.output = cms.OutputModule("PoolOutputModule",
   outputCommands = cms.untracked.vstring(["drop *", "keep *_onia2MuMu*_*_*","keep *_*_*_HiForest"]),
   #fileName = cms.untracked.string("/eos/cms/store/group/phys_heavyions/soohwan/Run3_2024/ppRef_2024/output2.root"),
   fileName = cms.untracked.string("outputMC2.root"),
)
process.output_path = cms.EndPath(process.output)
#process.schedule.append( process.output_path )

process.options.numberOfThreads = 1
process.options.numberOfStreams = 0
