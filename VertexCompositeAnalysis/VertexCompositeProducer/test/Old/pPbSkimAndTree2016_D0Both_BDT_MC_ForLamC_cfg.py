import FWCore.ParameterSet.Config as cms
process = cms.Process("ANASKIM")

process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.MessageLogger.cerr.FwkReport.reportEvery = 200

# The line below always has to be included to make VarParsing work
from FWCore.ParameterSet.VarParsing import VarParsing

# In teh line below 'analysis' is an instance of VarParsing object
options = VarParsing ('analysis')

if not options.inputFiles:
    options.inputFiles = [
#'/store/himc/pPb816Summer16DR/NonPromptD0_D0pT-1p2_pPb-EmbEPOS_8p16_Pythia8/AODSIM/pPbEmb_80X_mcRun2_pA_v4-v1/90000/0007792C-D8A0-E711-9B47-FA163EEC8769.root'
'file:/eos/home-s/soohwan/store/PromptD0_D0pT-1p2_pPb-EmbEPOS_8p16_Pythia8/009F94B7-5D9B-E711-B27B-009C02AABEB8.root',
]

process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring(options.inputFiles)
)

# =============== Other Statements =====================
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))
process.GlobalTag.globaltag = '80X_mcRun2_pA_v4'

# =============== Import Sequences =====================
#Trigger Selection
### Comment out for the timing being assuming running on secondary dataset with trigger bit selected already
import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltHM = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
#process.hltHM.HLTPaths = ['HLT_PAFullTracks_Multiplicity280_v*']
process.hltHM.HLTPaths = ['HLT_PAFullTracks_Multiplicity280_v*']
process.hltHM.andOr = cms.bool(True)
process.hltHM.throw = cms.bool(False)

process.load('VertexCompositeAnalysis.VertexCompositeProducer.collisionEventSelection_cff')
process.colEvtSel = cms.Sequence(process.hfCoincFilter * process.primaryVertexFilterPA * process.NoScraping)

process.eventFilter_HM = cms.Sequence(
#    process.hltHM *
    process.colEvtSel
)

process.eventFilter_HM_step = cms.Path( process.eventFilter_HM )

#process.dEdx_step = cms.Path( process.eventFilter_HM * process.produceEnergyLoss )

########## D0 candidate rereco ###############################################################
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cff")
process.generalD0CandidatesNew = process.generalD0Candidates.clone()
#process.generalD0CandidatesNew.trkPtSumCut = cms.double(1.0)
#process.generalD0CandidatesNew.trkEtaDiffCut = cms.double(4.8)
#process.generalD0CandidatesNew.tkNhitsCut = cms.int32(11)
#process.generalD0CandidatesNew.tkPtErrCut = cms.double(0.2)
#process.generalD0CandidatesNew.tkPtCut = cms.double(0.4)
#process.generalD0CandidatesNew.alphaCut = cms.double(2.0)
#process.generalD0CandidatesNew.alpha2DCut = cms.double(2.0)
#process.generalD0CandidatesNew.dPtCut = cms.double(1.9)

process.generalD0CandidatesNewWrongSign = process.generalD0CandidatesNew.clone(isWrongSign = cms.bool(True))

process.d0rereco_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNew )
process.d0rereco_wrongsign_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNewWrongSign )

###############################################################################################

process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.d0selector_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.d0analyzer_tree_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.d0analyzer_ntp_cff")

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('d0ana_mc_tree.root')
)

process.d0ana_mc.useAnyMVA = cms.bool(True)
process.d0ana_mc.VertexCompositeCollection = cms.untracked.InputTag("d0selectorMC:D0")
process.d0ana_mc.MVACollection = cms.InputTag("d0selectorMC:MVAValuesNewD0")
process.d0ana_mc_wrongsign = process.d0ana_mc.clone()
process.d0ana_mc_wrongsign.VertexCompositeCollection = cms.untracked.InputTag("d0selectorMCWS:D0")
process.d0ana_mc_wrongsign.MVACollection = cms.InputTag("d0selectorMCWS:MVAValuesNewD0")

process.d0selectorMCBDTPreCut.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_v2.root')
process.d0selectorMC = process.d0selectorMCBDTPreCut.clone()
process.d0selectorMCWS = process.d0selectorMC.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generalD0CandidatesNewWrongSign:D0"),
  MVACollection = cms.InputTag("generalD0CandidatesNewWrongSign:MVAValues")
)

process.d0selectorMCNewReduced = process.d0selectorMC.clone()
process.d0selectorMCNewReduced.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_NoPtErrNHitDLAngle2D_v3.root')
process.d0selectorMCNewReduced.DCAValCollection = cms.InputTag("generalD0CandidatesNew:DCAValuesD0")
process.d0selectorMCNewReduced.DCAErrCollection = cms.InputTag("generalD0CandidatesNew:DCAErrorsD0")
process.d0selectorMCNewReducedWS = process.d0selectorMCWS.clone()
process.d0selectorMCNewReducedWS.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_NoPtErrNHitDLAngle2D_v3.root')
process.d0selectorMCNewReducedWS.DCAValCollection = cms.InputTag("generalD0CandidatesNewWrongSign:DCAValuesD0")
process.d0selectorMCNewReducedWS.DCAErrCollection = cms.InputTag("generalD0CandidatesNewWrongSign:DCAErrorsD0")

process.d0ana_mc_newreduced = process.d0ana_mc.clone()
process.d0ana_mc_newreduced.saveTree = True
process.d0ana_mc_newreduced.VertexCompositeCollection = cms.untracked.InputTag("d0selectorMCNewReduced:D0")
process.d0ana_mc_newreduced.MVACollection = cms.InputTag("d0selectorMCNewReduced:MVAValuesNewD0")
process.d0ana_mc_newreduced.DCAValCollection = cms.InputTag("d0selectorMCNewReduced:DCAValuesNewD0")
process.d0ana_mc_newreduced.DCAErrCollection = cms.InputTag("d0selectorMCNewReduced:DCAErrorsNewD0")

process.d0ana_mc_newreduced_wrongsign = process.d0ana_mc_wrongsign.clone()
process.d0ana_mc_newreduced_wrongsign.saveTree = True
process.d0ana_mc_newreduced_wrongsign.VertexCompositeCollection = cms.untracked.InputTag("d0selectorMCNewReducedWS:D0")
process.d0ana_mc_newreduced_wrongsign.MVACollection = cms.InputTag("d0selectorMCNewReducedWS:MVAValuesNewD0")
process.d0ana_mc_newreduced_wrongsign.DCAValCollection = cms.InputTag("d0selectorMCNewReducedWS:DCAValuesNewD0")
process.d0ana_mc_newreduced_wrongsign.DCAErrCollection = cms.InputTag("d0selectorMCNewReducedWS:DCAErrorsNewD0")

process.d0ana_seq2 = cms.Sequence(process.d0selectorMCNewReduced * process.d0ana_mc_newreduced)

process.d0ana_wrongsign_seq2 = cms.Sequence(process.d0selectorMCNewReducedWS * process.d0ana_mc_newreduced_wrongsign)

process.p1 = cms.Path(process.eventFilter_HM * process.d0ana_seq2)
process.p2 = cms.Path(process.eventFilter_HM * process.d0ana_wrongsign_seq2)

process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.d0rereco_step,
    process.d0rereco_wrongsign_step,
    process.p1,
    process.p2,
)
