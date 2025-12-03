import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM', eras.Run3_2023)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_Data_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))
process.FastTimerService = cms.Service("FastTimerService",
                                       printEventSummary = cms.untracked.bool(True),
                                       printRunSummary = cms.untracked.bool(True),
                                       printJobSummary = cms.untracked.bool(True),
                                       enableDQM = cms.untracked.bool(False)
)

# Define the input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring("file:/eos/cms/store/group/phys_heavyions/dileptons/Data2023/MINIAOD/HIPhysicsRawPrime0/Run375064/7ed5766f-6b1d-415e-8916-e62825a6347f.root"),
    #fileNames = cms.untracked.vstring("file:step4.root"),
    #fileNames = cms.untracked.vstring("/store/user/junseok/Genproduction/RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_081924_v1/DStarKpipiPU/crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_081924_v1/240819_054039/0001/step4_1619.root"),
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('132X_dataRun3_Prompt_v4')
#process.GlobalTag.globaltag = cms.string('132X_mcRun3_2023_realistic_HI_v9')

## Set ZDC information
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

# Define the event selection sequence
process.eventFilter_HM = cms.Sequence(
    process.hltFilter
)
process.eventFilter_HM_step = cms.Path( process.eventFilter_HM )

from VertexCompositeAnalysis.VertexCompositeProducer.PATAlgos_cff import changeToMiniAOD

# Define the analysis steps

########## D0 candidate rereco ###############################################################
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalD04PCandidates_cff")
process.generalD04PCandidatesNew = process.generalD04PCandidates.clone()
#process.generalD04PCandidatesNew.trkPtSumCut = cms.double(1.6)
process.generalD04PCandidatesNew.tkEtaDiffCut = cms.double(1.0)
process.generalD04PCandidatesNew.tkNhitsCut = cms.int32(10)
process.generalD04PCandidatesNew.tkPtErrCut = cms.double(0.1)
process.generalD04PCandidatesNew.tkPtCut = cms.double(2.0)
process.generalD04PCandidatesNew.tkEtaCut = cms.double(2.4)
#process.generalD04PCandidatesNew.alphaCut = cms.double(1.0)
#process.generalD04PCandidatesNew.alpha2DCut = cms.double(1.0)
process.generalD04PCandidatesNew.collinearityCut3D = cms.double(0.99)
#process.generalD04PCandidatesNew.dPtCut = cms.double(0.0)
process.generalD04PCandidatesNew.tkChi2Cut = cms.double(2.0)
process.generalD04PCandidatesNew.VtxChiProbCut = cms.double(0.010)
process.generalD04PCandidatesNew.vtxSignificance3DCut = cms.double(5)
process.generalD04PCandidatesNew.mPiKCutMin = cms.double(1.74)
process.generalD04PCandidatesNew.mPiKCutMax = cms.double(2.00)
#process.generalD04PCandidatesNewWrongSign = process.generalD04PCandidatesNew.clone(isWrongSign = cms.bool(True))

process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDStar5PCandidates_cff")
process.generalDStar5PCandidatesNew = process.generalDStar5PCandidates.clone()
process.generalDStar5PCandidatesNew.d0Collection = cms.InputTag("generalD04PCandidatesNew:D04P")
#process.generalDStar5PCandidatesNew.trkPtSumCut = cms.double(0.0)
#process.generalDStar5PCandidatesNew.trkEtaDiffCut = cms.double(0.0)
process.generalDStar5PCandidatesNew.tkNhitsCut = cms.int32(0)
#process.generalDStar5PCandidatesNew.tkPtErrCut = cms.double(0.1)
process.generalDStar5PCandidatesNew.tkPtCut = cms.double(0.4)
process.generalDStar5PCandidatesNew.tkChi2Cut = cms.double(2.5)
#process.generalDStar5PCandidatesNew.vtxSignificance3DCut = cms.double(3)
#process.generalDStar5PCandidatesNew.alphaCut = cms.double(1)
#process.generalDStar5PCandidatesNew.alpha2DCut = cms.double(1)
process.generalDStar5PCandidatesNew.dauLongImpactSigCut = cms.double(0.0)
process.generalDStar5PCandidatesNew.dauTransImpactSigCut = cms.double(0.0)# it will be cut of by 3 in selector 
#process.generalDStar5PCandidatesNew.dPtCut = cms.double(0.0)
# process.generalDStar5PCandidatesNew.useAnyMVA=cms.bool(True)
# process.generalDStar5PCandidatesNew.GBRForestFileName=cms.string('GBRForestfile_XGBDT_PromptDstarInPbPb_default_MB_OnlyMC.root')

process.d0rereco_step = cms.Path( process.eventFilter_HM * process.generalD04PCandidatesNew * process.generalDStar5PCandidatesNew)
#process.d0rereco_wrongsign_step = cms.Path( process.eventFilter_HM * process.generalD04PCandidatesNewWrongSign )

# eventinfoana must be in EndPath, and process.eventinfoana.selectEvents must be the name of eventFilter_HM Path
#process.eventinfoana.selectEvents = cms.untracked.string('eventFilter_HM_step')
#process.eventinfoana.triggerPathNames = cms.untracked.vstring(
#    "HLT_HIMinimumBiasHF1AND_v*", #24
#    "HLT_HIMinimumBiasHF1ANDZDC2nOR_v", #25
#    "HLT_HIMinimumBiasHF1ANDZDC1nOR_v", #26
#    )
#process.eventinfoana.eventFilterNames = cms.untracked.vstring(
#    'Flag_colEvtSel',
#    'Flag_hfCoincFilter',
#    'Flag_primaryVertexFilter', 
#    )
#process.eventinfoana.triggerFilterNames = cms.untracked.vstring()
#process.eventinfoana.stageL1Trigger = cms.uint32(2)
#process.pevt = cms.EndPath(process.eventinfoana)

#process.p = cms.Path(process.d0ana_seq2)
#process.c = cms.Path(process.cent_seq)
# process.pws = cms.Path(process.d0ana_wrongsign_seq2)

# Add the Conversion tree

# Define the process schedule
process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.d0rereco_step,
#   process.pevt,
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

process.output = cms.OutputModule("PoolOutputModule",
    outputCommands = cms.untracked.vstring("keep *_generalDStar5PCandidatesNew_*_ANASKIM"),
    fileName = cms.untracked.string('output.root'),
)

process.outputPath = cms.EndPath(process.output)
process.schedule.append(process.outputPath)
