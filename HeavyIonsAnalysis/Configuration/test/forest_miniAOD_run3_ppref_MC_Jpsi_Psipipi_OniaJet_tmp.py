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

'''
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
'''
process.source = cms.Source("PoolSource",
                            duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
                            fileNames = cms.untracked.vstring(
                                #'file:/afs/cern.ch/work/s/soohwan/private/Analysis/General2024Analysis/TEST_CMSSW_14_1_5/CMSSW_14_1_5/src/HeavyIonsAnalysis/Configuration/test/step3_JpsiShower_1.root'
                                # '/store/group/phys_heavyions/soohwan/Analysis/oniajet_run3/ppRef/step3_JpsiShower_1.root'
                                #'/store/user/soohwan/Run3_2024/MC/PAT_MC_PythiaJPsi_CMSW_14_1_6_16Dec2024_v2_FixHLT_t2/Jpsi_PythiaCP5_Noemb_ppRef5p36_13Dec_v1/PAT_MC_PythiaJPsi_CMSW_14_1_6_16Dec2024_v2_FixHLT_t2/241216_023840/0000/Jpsi_RECO_1.root'
                                '/store/user/bputra/JPsi_OniaShower_PythiaOnly_08Nov_v1/PPRef_JPsiToMuMuOniaShower_pTHat10_CMSSW_141X_mcRun3_2024_realistic_ppRef5TeV_v7_20250216_RECOPAT/250216_122723/0000/PPRef_JPsiToMuMuOniaShower_pTHat10_RECO_1.root'
                            )
                        )

# Number of events we want to process, -1 = all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(20000)
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
                                   fileName = cms.string((f'HiForestMiniAOD_MC.root')))
        #fileName = cms.string((f'HiForestMiniAOD_MC_{i}.root')))


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
                 muonSelection="GlbOrTrk", 
                 L1Stage=2, 
                 isMC=True, 
                 pdgID=443, 
                 outputFileName= "", 
                 doTrimu=False,
                 OnlySingleMuons=False
)

# was in config
# process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("(2.6 < mass && mass < 4.0) && charge == 0 && pt > 0")

# new in config
process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string(
    "(2.6 < mass && mass < 4.2) && \
    charge == 0 && pt > 6.0 && \
    abs(rapidity ) < 2.4 && \
    daughter('muon1').muonID('TMOneStationTight') && \
    daughter('muon2').muonID('TMOneStationTight') && \
    abs(daughter('muon1').innerTrack.hitPattern().trackerLayersWithMeasurement())  > 5 && \
    abs(daughter('muon2').innerTrack.hitPattern().trackerLayersWithMeasurement())  > 5 && \
    abs(daughter('muon1').innerTrack.hitPattern().pixelLayersWithMeasurement())  > 0 && \
    abs(daughter('muon2').innerTrack.hitPattern().pixelLayersWithMeasurement())  > 0 && \
    abs(daughter('muon1').innerTrack.dz) < 20.0 && \
    abs(daughter('muon2').innerTrack.dz) < 20.0 && \
    abs(daughter('muon1').innerTrack.dxy) < 0.3 && \
    abs(daughter('muon2').innerTrack.dxy) < 0.3 && \
    daughter('muon1').isPFMuon() && \
    daughter('muon2').isPFMuon() \
    "
)
'''
'''

process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.01")

process.selectedPatMuons = cms.EDFilter("PATCompositeCandidateSelector",
    src = cms.InputTag("onia2MuMuPatGlbGlb"),
    # cut = cms.string(" daughter('muon1').isSoftMuon(userData('PVwithmuons')) && daughter('muon2').isSoftMuon(userData('PVwithmuons'))"
    cut = cms.string(""
    )
)

# process.hionia.srcDimuon = cms.InputTag("selectedPatMuons")

process.hionia.primaryVertexTag = "unpackedTracksAndVertices"
process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
# process.hionia.muonLessPV       = cms.bool(False)
process.hionia.SumETvariables   = cms.bool(True)
process.hionia.applyCuts        = cms.bool(False)
process.hionia.AtLeastOneCand   = cms.bool(False)
process.hionia.OneMatchedHLTMu  = cms.int32(-1)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)
process.hionia.mom4format       = cms.string("vector")
process.hionia.isHI = cms.untracked.bool(False)
process.hionia.genealogyInfo    = cms.bool(True)

from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
changeToMiniAOD(process)
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
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
process.pfCandComposites.checkOnia = False # already cut string selector applied

process.pfCandComposites.assignClosestOniaMass = True # need to assign value
process.pfCandComposites.cutMassOnia = 3.4 

process.pfCandComposites.massOnia = 3.096916 # J/Psi  
process.pfCandComposites.massOnia2 = 3.686097 # psi(2S) #Used only when assignClosestOniaMass is on
process.pfCandComposites.compositeTag = cms.InputTag("onia2MuMuPatGlbGlb")
# process.pfCandComposites.compositeTag = cms.InputTag("selectedPatMuons")
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
from GeneratorInterface.RivetInterface.mergedGenParticles_cfi import mergedGenParticles
process.mergedGenParticles = mergedGenParticles.clone()

from RecoJets.Configuration.GenJetParticles_cff import genParticlesForJets
process.packedGenParticlesForJetsNoNu = genParticlesForJets.clone(src = 'mergedGenParticles')
process.packedGenParticlesForJetsNoNu.ignoreParticleIDs += [12,14,16]  # no neutrinos
process.packedGenParticlesForJetsNoNu.storeJMM = cms.untracked.bool(True)

#########################
# Main analysis list
#########################

process.forest = cms.Path(
    process.HiForestInfo +
    process.hltanalysis *
    process.hiEvtAnalyzer *
    process.hltobject +
    process.l1object +
    process.HiGenParticleAna  +
    process.mergedGenParticles +
    process.packedGenParticlesForJetsNoNu 
    # process.ggHiNtuplizer +
    # process.trackSequencePP
)

#####################################################################################
#####################################################################################                                                                  
                                                                                  
# Select the types of jets filled                                                                                                                      
matchJets = True             # Enables q/g and heavy flavor jet identification in MC                                                                   
jetPtMin = 0
jetAbsEtaMax = 2.5

# Choose which additional information is added to jet trees
doHIJetID = True             # Fill jet ID and composition information branches
doWTARecluster = False        # Add jet phi and eta for WTA axis                                                                                       
doBtagging  =  False         # Note that setting to True increases computing time a lot
                                                                                                      
                                          
# 0 means use original mini-AOD jets, otherwise use R value, e.g., 3,4,8                                                                             
jetLabel = "4"

# add candidate tagging, copy/paste to add other jet radii                                                                                             
from HeavyIonsAnalysis.JetAnalysis.setupJets_ppRef_cff import candidateBtaggingMiniAOD
candidateBtaggingMiniAOD(process, isMC = True, jetPtMin = jetPtMin, jetCorrLevels = ['L2Relative', 'L3Absolute'], doBtagging = doBtagging, labelR = jetLabel)

#filter out onia jets
# process.load("RecoJets.JetProducers.pfXpatJets_cfi")
# setattr(process,"ak"+jetLabel+"PFXpatJets",process.pfXpatJets.clone(src="selectedUpdatedPatJetsAK"+jetLabel+"PFCHSDeepFlavour"))
# TMP CHECK
process.ak4PFXpatJets = cms.EDFilter("PatJetXSelector",
                                     src = cms.InputTag("selectedUpdatedPatJetsAK4PFCHSDeepFlavour"),
                                     cut = cms.string("pt > 0.0 && abs(rapidity()) < 3.")
                                     )
process.forest += getattr(process,"ak"+jetLabel+"PFXpatJets")
#hack for low pt jets.  Should be moved to setup ppref
getattr(process,"patJetGenJetMatchAK"+jetLabel+"PFCHS").maxDeltaR = 0.8


#getattr(process,"ak"+jetLabel+"GenJetsWithNu").src = "packedGenParticlesForJetsNoNu"
# setup jet analyzer                                                                                                                                    
setattr(process,"ak"+jetLabel+"PFJetAnalyzer",process.ak4PFJetAnalyzer.clone())
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").jetTag = 'ak4PFXpatJets'
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").jetName = 'ak'+jetLabel+'PF'
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").matchJets = matchJets
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").matchTag = 'patJetsAK'+jetLabel+'PFUnsubJets'
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").doHiJetID = doHIJetID
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").doWTARecluster = doWTARecluster
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").jetPtMin = jetPtMin
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").jetAbsEtaMax = cms.untracked.double(jetAbsEtaMax)
getattr(process,"ak"+jetLabel+"PFJetAnalyzer").rParam = int(jetLabel)*0.1
#getattr(process,"ak"+jetLabel+"PFJetAnalyzer").jetFlavourInfos = "ak"+jetLabel+"PFFlavourInfos"
if jetLabel!="0": getattr(process,"ak"+jetLabel+"PFJetAnalyzer").genjetTag = "ak"+jetLabel+"GenJetsWithNu"
if doBtagging:
    getattr(process,"ak"+jetLabel+"PFJetAnalyzer").pfJetProbabilityBJetTag = cms.untracked.string("pfJetProbabilityBJetTagsDeepFlavour")
    getattr(process,"ak"+jetLabel+"PFJetAnalyzer").pfUnifiedParticleTransformerAK4JetTags = cms.untracked.string("pfUnifiedParticleTransformerAK4JetTagsDeepFlavour")
process.forest += getattr(process,"ak"+jetLabel+"PFJetAnalyzer")


process.jpsiJetsPath = cms.Path( process.patMuonSequence+process.onia2MuMuPatGlbGlb + process.selectedPatMuons + process.hionia + process.pfCandComposites)

# process.Onia2MuMuPAT.remove(process.patTriggerAll)
# process.forest.remove(process.patTriggerAll)


#process.schedule = cms.Schedule( process.forest, process.jpsiJetsPath)
# process.schedule = cms.Schedule( process.jpsiJetsPath)
process.schedule = cms.Schedule( process.jpsiJetsPath, process.forest)

process.options.numberOfThreads = 1
process.options.numberOfConcurrentLuminosityBlocks = 1
process.options.numberOfStreams = 0


### Onia trk trk analysis part

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

process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.ottanalyzer_tree_cff")
process.ottana_new = process.ottana_mc.clone()
process.ottana_new.VertexCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.TrackCollection = cms.untracked.InputTag('unpackedTracksAndVertices')
process.ottana_new.doGenNtuple = False
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
process.ottana_new.jetNames = cms.vstring("ak4PFXpatJets")

process.ottana_new.doJetConstituentCompare = cms.untracked.bool(False)

process.ottstep = cms.Path(process.generalOttCandidatesNew * process.ottana_new )
process.schedule.append(process.ottstep)

# # edm output for debugging purposes
process.output = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string('HiForestEDM.root'),
    outputCommands = cms.untracked.vstring(
        # 'keep *',
        'drop *',
        'keep *_selectedPatMuons_*_*',
        'keep *_ak4PFXpatJets_*_*',
        'keep *_selectedUpdatedPatJetsAK4PFCHSDeepFlavour_*_*',
        'keep *_ak4GenJetsRecluster_*_*',
        'keep *_patJetsAK4PFUnsubJets_*_*',
        'keep *_ak4GenJetsWithNu_*_*',
        )
    )

process.output_path = cms.EndPath(process.output)
# process.schedule.append(process.output_path)
