import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM', eras.Run3_2023)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_Data_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 5
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring("file:/eos/cms/store/group/phys_heavyions/dileptons/Data2023/MINIAOD/HIPhysicsRawPrime0/Run375064/7ed5766f-6b1d-415e-8916-e62825a6347f.root"),
)

# =============== Other Statements =====================
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(2000))
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))
# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('132X_dataRun3_Prompt_v4')

# Set ZDC information
#process.es_pool = cms.ESSource("PoolDBESSource",
#    timetype = cms.string('runnumber'),
#    toGet = cms.VPSet(cms.PSet(record = cms.string("HcalElectronicsMapRcd"), tag = cms.string("HcalElectronicsMap_2021_v2.0_data"))),
#    connect = cms.string('frontier://FrontierProd/CMS_CONDITIONS'),
#    authenticationMethod = cms.untracked.uint32(1)
#)
#process.es_prefer = cms.ESPrefer('HcalTextCalibrations', 'es_ascii')
#process.es_ascii = cms.ESSource('HcalTextCalibrations',
#    input = cms.VPSet(cms.PSet(object = cms.string('ElectronicsMap'), file = cms.FileInPath("emap_2023_newZDC_v3.txt")))
#)

# Add PbPb centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("HeavyIonRcd"),
        tag = cms.string("CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374810"),
        connect = cms.string("sqlite_file:CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374810.db"),
        label = cms.untracked.string("HFtowers")
        )
    ]
)
process.cent_seq = cms.Sequence(process.centralityBin)
process.cent_seq = cms.Sequence(process.centralityBin)

# =============== Import Sequences =====================
#Trigger Selection
### Comment out for the timing being assuming running on secondary dataset with trigger bit selected already
# Add trigger selection
import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltFilter = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltFilter.andOr = cms.bool(True)
process.hltFilter.throw = cms.bool(False)
process.hltFilter.HLTPaths = [
    "HLT_HIMinimumBiasHF1AND_v*", #24
    "HLT_HIMinimumBiasHF1ANDZDC2nOR_v*", #25
    "HLT_HIMinimumBiasHF1ANDZDC1nOR_v*", #26
]

# Add PbPb collision event selection
process.load('VertexCompositeAnalysis.VertexCompositeProducer.collisionEventSelection_cff')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.hfCoincFilter_cff')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.hffilter_cfi')
process.colEvtSel = cms.Sequence()

from VertexCompositeAnalysis.VertexCompositeProducer.PATAlgos_cff import changeToMiniAOD

# Define the event selection sequence
process.eventFilter_HM = cms.Sequence(
    process.hltFilter
)
process.eventFilter_HM_step = cms.Path( process.eventFilter_HM )

########## DPlus3P candidate rereco ###############################################################
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDPlus3PCandidates_cff")
process.generalDPlus3PCandidatesNew = process.generalDPlus3PCandidates.clone()
#process.generalDPlus3PCandidatesNew.trkPtSumCut = cms.double(0.0)
#process.generalDPlus3PCandidatesNew.trkEtaDiffCut = cms.double(4.0)
process.generalDPlus3PCandidatesNew.tkChi2Cut = cms.double(7) #trk Chi2 <
process.generalDPlus3PCandidatesNew.tkEtaCut = cms.double(2.4) #trk abs(eta) <
process.generalDPlus3PCandidatesNew.tkNhitsCut = cms.int32(11)
process.generalDPlus3PCandidatesNew.tkPtErrCut = cms.double(0.1)
process.generalDPlus3PCandidatesNew.tkPtCut = cms.double(1.0)
process.generalDPlus3PCandidatesNew.alphaCut = cms.double(1.4)
process.generalDPlus3PCandidatesNew.alpha2DCut = cms.double(1.4)
process.generalDPlus3PCandidatesNew.dPtCut = cms.double(0.0)

process.generalDPlus3PCandidatesNewWrongSign = process.generalDPlus3PCandidatesNew.clone(isWrongSign = cms.bool(True))
process.dplus3prereco_step = cms.Path( process.eventFilter_HM * process.generalDPlus3PCandidatesNew)
process.dplus3prereco_wrongsign_step = cms.Path( process.eventFilter_HM * process.generalDPlus3PCandidatesNewWrongSign )

# produce DPlus3P trees
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dplus3pselector_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dplus3panalyzer_ntp_cff")
#process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dplus3panalyzer_tree_cff")
#process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dStarselector_cff")
##process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dStaranalyzer_tree_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.eventinfotree_cff")

process.TFileService = cms.Service("TFileService",
    fileName =
    cms.string('dplus3pana_tree.root')
    )

# set up selectors
process.dplus3pselector = process.dplus3pselectorCut.clone()
process.dplus3pselector.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_v2.root')
process.dplus3pselector.multMin = cms.untracked.double(0)
process.dplus3pselector.multMax = cms.untracked.double(100000)
process.dplus3pselectorWS = process.dplus3pselector.clone(
    VertexCompositeCollection = cms.untracked.InputTag("generalDPlus3PCandidatesNewWrongSign:DPlus3P"),
    MVACollection = cms.InputTag("generalDPlus3PCandidatesNewWrongSign:MVAValues")
)

process.dplus3pana.useAnyMVA = cms.bool(False)
process.dplus3pana.multMin = cms.untracked.double(0)
process.dplus3pana.multMax = cms.untracked.double(100000)
process.dplus3pana.VertexCompositeCollection = cms.untracked.InputTag("dplus3pselector:DPlus3P")
process.dplus3pana.MVACollection = cms.InputTag("dplus3pselector:MVAValuesNewDPlus3P")
process.dplus3pana_wrongsign.useAnyMVA = cms.bool(False)
process.dplus3pana_wrongsign.multMin = cms.untracked.double(0)
process.dplus3pana_wrongsign.multMax = cms.untracked.double(100000)
process.dplus3pana_wrongsign.VertexCompositeCollection = cms.untracked.InputTag("dplus3pselectorWS:DPlus3P")
process.dplus3pana_wrongsign.MVACollection = cms.InputTag("dplus3pselectorWS:MVAValuesNewDPlus3P")


process.dplus3pselectorNewReduced = process.dplus3pselector.clone()
process.dplus3pselectorNewReduced.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_NoPtErrNHitDLAngle2D_v3.root')
process.dplus3pselectorNewReduced.DCAValCollection = cms.InputTag("generalDPlus3PCandidatesNew:DCAValuesDPlus3P")
process.dplus3pselectorNewReduced.DCAErrCollection = cms.InputTag("generalDPlus3PCandidatesNew:DCAErrorsDPlus3P")

process.dplus3pselectorWSNewReduced = process.dplus3pselectorWS.clone()
process.dplus3pselectorWSNewReduced.GBRForestFileName = cms.string('GBRForestfile_BDT_PromptD0InpPb_default_HLT185_WS_Pt1p5MassPeak_NoPtErrNHitDLAngle2D_v3.root')
process.dplus3pselectorWSNewReduced.DCAValCollection = cms.InputTag("generalDPlus3PCandidatesNewWrongSign:DCAValuesDPlus3P")
process.dplus3pselectorWSNewReduced.DCAErrCollection = cms.InputTag("generalDPlus3PCandidatesNewWrongSign:DCAErrorsDPlus3P")

process.dplus3pana_newreduced = process.dplus3pana.clone()
process.dplus3pana_newreduced.VertexCompositeCollection = cms.untracked.InputTag("dplus3pselectorNewReduced:DPlus3P")
process.dplus3pana_newreduced.MVACollection = cms.InputTag("dplus3pselectorNewReduced:MVAValuesNewDPlus3P")
process.dplus3pana_newreduced.DCAValCollection = cms.InputTag("dplus3pselectorNewReduced:DCAValuesNewDPlus3P")
process.dplus3pana_newreduced.DCAErrCollection = cms.InputTag("dplus3pselectorNewReduced:DCAErrorsNewDPlus3P")

process.dplus3pana_wrongsign_newreduced = process.dplus3pana_wrongsign.clone()
process.dplus3pana_wrongsign_newreduced.VertexCompositeCollection = cms.untracked.InputTag("dplus3pselectorWSNewReduced:DPlus3P")
process.dplus3pana_wrongsign_newreduced.MVACollection = cms.InputTag("dplus3pselectorWSNewReduced:MVAValuesNewDPlus3P")
process.dplus3pana_wrongsign_newreduced.DCAValCollection = cms.InputTag("dplus3pselectorWSNewReduced:DCAValuesNewDPlus3P")
process.dplus3pana_wrongsign_newreduced.DCAErrCollection = cms.InputTag("dplus3pselectorWSNewReduced:DCAErrorsNewDPlus3P")


process.dplus3pana_seq2 = cms.Sequence(process.eventFilter_HM * process.dplus3pselectorNewReduced * process.dplus3pana_newreduced)
#process.dplus3pana_seq2 = cms.Sequence(process.eventFilter_HM * process.dplus3pselectorNewReduced )
process.dplus3pana_wrongsign_seq2 = cms.Sequence(process.eventFilter_HM * process.dplus3pselectorWSNewReduced * process.dplus3pana_wrongsign_newreduced)

# eventinfoana must be in EndPath, and process.eventinfoana.selectEvents must be the name of eventFilter_HM Path
process.eventinfoana.selectEvents = cms.untracked.string('eventFilter_HM_step')
process.eventinfoana.triggerPathNames = cms.untracked.vstring(
    "HLT_HIMinimumBiasHF1AND_v*", #24
    "HLT_HIMinimumBiasHF1ANDZDC2nOR_v*", #25
    "HLT_HIMinimumBiasHF1ANDZDC1nOR_v*", #26
    )
process.eventinfoana.triggerFilterNames = cms.untracked.vstring()
process.eventinfoana.stageL1Trigger = cms.uint32(2)
process.pevt = cms.EndPath(process.eventinfoana)

process.p = cms.Path(process.dplus3pana_seq2)
process.pws = cms.Path(process.dplus3pana_wrongsign_seq2)

# Add the Conversion tree
# Define the process schedule
process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.dplus3prereco_step,
    process.p,
    process.pevt,
)

# Add the event selection filters
process.Flag_colEvtSel = cms.Path(process.eventFilter_HM * process.colEvtSel)
#process.Flag_hfCoincFilter = cms.Path(process.eventFilter_HM * process.hfCoincFilter2Th4)
process.Flag_primaryVertexFilter = cms.Path(process.eventFilter_HM * process.primaryVertexFilter * process.clusterCompatibilityFilter)
# follow the exactly same config of process.eventinfoana.eventFilterNames
#eventFilterPaths = [ process.Flag_colEvtSel , process.Flag_hfCoincFilter , process.Flag_primaryVertexFilter ]
eventFilterPaths = [ process.Flag_colEvtSel  , process.Flag_primaryVertexFilter ]
for P in eventFilterPaths:
    process.schedule.insert(0, P)

changeToMiniAOD(process)
process.options.numberOfThreads = 1
process.options.numberOfStreams = 1

# Define the output
process.output = cms.OutputModule("PoolOutputModule",
    outputCommands = cms.untracked.vstring("keep *"),
    fileName = cms.untracked.string("output.root"),
)
process.output_path = cms.EndPath(process.output)

#process.schedule.append( process.output_path )
