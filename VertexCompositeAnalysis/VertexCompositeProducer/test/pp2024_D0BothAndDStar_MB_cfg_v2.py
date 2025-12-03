import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.StandardSequences.Eras import eras

options = VarParsing('python')
options.register('number', '0',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "number"
)
options.register('inputfile', '',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Input files"
)

options.parseArguments()

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
    #fileNames = cms.untracked.vstring("file:/eos/cms/store/group/phys_heavyions/dileptons/Data2023/MINIAOD/HIPhysicsRawPrime0/Run375064/7ed5766f-6b1d-415e-8916-e62825a6347f.root"),
#    fileNames = cms.untracked.vstring("/store/user/junseok/Genproduction/RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_241026_v1/DStarKpipiPU/crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_241026_v1/241026_084606/0000/step4_1.root"),
   # fileNames= cms.untracked.vstring("root://cmsxrootd.fnal.gov//store/mc/HINPbPbSpring23MiniAOD/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/MINIAODSIM/132X_mcRun3_2023_realistic_HI_v9-v2/2560000/bf4f838b-571d-4570-805c-cd3cb84839c2.root"),
    #fileNames= cms.untracked.vstring("file:bf4f838b-571d-4570-805c-cd3cb84839c2.root"),  #DPt>1
    # fileNames= cms.untracked.vstring("file:minbias_RECO_106.root"), #DPt>0
    #fileNames=cms.untracked.vstring('/store/data/Run2024J/PPRefZeroBiasPlusForward0/MINIAOD/PromptReco-v1/000/387/696/00000/0037fb37-713f-4df8-9668-a2ce4665a93c.root',),
    #fileNames=cms.untracked.vstring('file:/eos/cms/store/group/phys_heavyions/wangj/RECO2025/miniaod_PhysicsIonPhysics0_393952/reco_run393952_ls0075_streamPhysicsIonPhysics0_StorageManager.root',),
    fileNames = cms.untracked.vstring(options.inputfile)

   # fileNames = cms.untracked.vstring(
        # '/store/user/junseok/Genproduction/RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_081924_v1/DStarKpipiPU/crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_081924_v1/240819_054039/0001/step4_1619.root',
#        '/store/user/junseok/Genproduction/RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_082724_v1/DStarKpipiPU/crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_082724_v1/240827_082226/0000/step4_105.root',
        #),
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#process.GlobalTag.globaltag = cms.string('132X_dataRun3_Prompt_v4')
process.GlobalTag.globaltag = cms.string('150X_dataRun3_Express_v2')

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


# =============== Import Sequences =====================
#Trigger Selection
### Comment out for the timing being assuming running on secondary dataset with trigger bit selected already
# Add trigger selection
import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltFilter = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltFilter.andOr = cms.bool(True)
process.hltFilter.throw = cms.bool(False)
process.hltFilter.HLTPaths = [
#        "HLT_OxyZeroBias_*"

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
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cff")
process.generalD0CandidatesNew = process.generalD0Candidates.clone()
process.generalD0CandidatesNewWrongSign = process.generalD0CandidatesNew.clone(isWrongSign = cms.bool(True))

process.generalD0CandidatesNew.tkNhitsCut = cms.int32(3)
#process.generalD0CandidatesNew.tkPtErrCut = cms.double(0.1)
process.generalD0CandidatesNew.tkPtCut = cms.double(0.4)
#process.generalD0CandidatesNew.alphaCut = cms.double(1.0)
#process.generalD0CandidatesNew.alpha2DCut = cms.double(1.0)
process.generalD0CandidatesNew.collinearityCut3D = cms.double(0.90)
#process.generalD0CandidatesNew.dPtCut = cms.double(0.0)
process.generalD0CandidatesNew.tkChi2Cut = cms.double(3)
process.generalD0CandidatesNew.VtxChiProbCut = cms.double(0.010)
process.generalD0CandidatesNew.vtxSignificance3DCut = cms.double(3)
process.generalD0CandidatesNew.mPiKCutMin = cms.double(1.74)
process.generalD0CandidatesNew.mPiKCutMax = cms.double(2.00)
process.generalD0CandidatesNew.d0AbsYCut = cms.double(1.5)
process.generalD0CandidatesNew.d0MassCut = cms.double(0.15)


process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cff")
process.generalDStarCandidatesNew = process.generalDStarCandidates.clone()

process.generalDStarCandidatesNew.tkNhitsCut = cms.int32(3)
#process.generalDStarCandidatesNew.tkPtErrCut = cms.double(0.1)
process.generalDStarCandidatesNew.tkPtCut = cms.double(0.4)
process.generalDStarCandidatesNew.tkChi2Cut = cms.double(3)
#process.generalDStarCandidatesNew.vtxSignificance3DCut = cms.double(0)
#process.generalDStarCandidatesNew.alphaCut = cms.double(1)
#process.generalDStarCandidatesNew.alpha2DCut = cms.double(1)
process.generalDStarCandidatesNew.dauLongImpactSigCut = cms.double(0.0)
process.generalDStarCandidatesNew.dauTransImpactSigCut = cms.double(0.0)
process.generalDStarCandidatesNew.dStarMassCut = cms.double(0.22)
#process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cff")
#process.generalD0CandidatesNew = process.generalD0Candidates.clone()
##process.generalD0CandidatesNew.trkPtSumCut = cms.double(1.6)
#process.generalD0CandidatesNew.tkChi2Cut = cms.double(5)
#process.generalD0CandidatesNew.tkNhitsCut = cms.int32(0)
#process.generalD0CandidatesNew.tkPtErrCut = cms.double(0.025)
#process.generalD0CandidatesNew.tkPtCut = cms.double(0.4)
#process.generalD0CandidatesNew.tkEtaCut = cms.double(1.6)
#process.generalD0CandidatesNew.tkPtSumCut = cms.double(0.0)
#process.generalD0CandidatesNew.trkEtaDiffCut = cms.double(1.0)
#process.generalD0CandidatesNew.dauTransImpactSigCut = cms.double(0.)
#process.generalD0CandidatesNew.dauLongImpactSigCut = cms.double(0.)
#process.generalD0CandidatesNew.tkDCACut = cms.double(0.03)
#process.generalD0CandidatesNew.vtxChi2Cut = cms.double(9999.0)
#process.generalD0CandidatesNew.VtxChiProbCut = cms.double(0.01)
#process.generalD0CandidatesNew.collinearityCut2D = cms.double(-2.0)
#process.generalD0CandidatesNew.collinearityCut3D = cms.double(-2.0)
#process.generalD0CandidatesNew.alphaCut = cms.double(0.4)
#process.generalD0CandidatesNew.alpha2DCut = cms.double(999.0)
#process.generalD0CandidatesNew.rVtxCut = cms.double(0.0)
#process.generalD0CandidatesNew.lVtxCut = cms.double(0.0)
#process.generalD0CandidatesNew.vtxSignificance2DCut = cms.double(1.0)
#process.generalD0CandidatesNew.vtxSignificance3DCut = cms.double(0.0)
#process.generalD0CandidatesNew.d0MassCut = cms.double(0.15)
#process.generalD0CandidatesNew.d0AbsYCut = cms.double(1.6)
#process.generalD0CandidatesNew.dPtCut = cms.double(0.0)
#
#
##process.generalD0CandidatesNew.tkPtErrCut = cms.double(0.1)
##process.generalD0CandidatesNew.tkPtCut = cms.double(1)
##process.generalD0CandidatesNew.alphaCut = cms.double(1.0)
##process.generalD0CandidatesNew.alpha2DCut = cms.double(1.0)
##process.generalD0CandidatesNew.collinearityCut3D = cms.double(0.99)
##process.generalD0CandidatesNew.dPtCut = cms.double(0.0)
##process.generalD0CandidatesNew.tkChi2Cut = cms.double(4)
##process.generalD0CandidatesNew.VtxChiProbCut = cms.double(0.010)
##process.generalD0CandidatesNew.vtxSignificance3DCut = cms.double(1)
#process.generalD0CandidatesNew.mPiKCutMin = cms.double(1.74)
#process.generalD0CandidatesNew.mPiKCutMax = cms.double(2.00)
##process.generalD0CandidatesNewWrongSign = process.generalD0CandidatesNew.clone(isWrongSign = cms.bool(True))
#
#process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cff")
#process.generalDStarCandidatesNew = process.generalDStarCandidates.clone()
#process.generalDStarCandidatesNew.tkChi2Cut = cms.double(999)
#process.generalDStarCandidatesNew.tkNhitsCut = cms.int32(0)
#process.generalDStarCandidatesNew.tkPtErrCut = cms.double(9999.0)
#process.generalDStarCandidatesNew.tkPtCut = cms.double(0.4)
#process.generalDStarCandidatesNew.tkEtaCut = cms.double(999.0)
#process.generalDStarCandidatesNew.tkPtSumCut = cms.double(0.0)
#process.generalDStarCandidatesNew.tkEtaDiffCut = cms.double(999.0)
#process.generalDStarCandidatesNew.dauTransImpactSigCut = cms.double(0.)
#process.generalDStarCandidatesNew.dauLongImpactSigCut = cms.double(0.)
#process.generalDStarCandidatesNew.tkDCACut = cms.double(9999.)
#process.generalDStarCandidatesNew.vtxChi2Cut = cms.double(9999.0)
#process.generalDStarCandidatesNew.VtxChiProbCut = cms.double(0.0001)
#process.generalDStarCandidatesNew.collinearityCut2D = cms.double(-2.0)
#process.generalDStarCandidatesNew.collinearityCut3D = cms.double(-2.0)
#process.generalDStarCandidatesNew.alphaCut = cms.double(999.0)
#process.generalDStarCandidatesNew.alpha2DCut = cms.double(999.0)
#process.generalDStarCandidatesNew.rVtxCut = cms.double(0.0)
#process.generalDStarCandidatesNew.lVtxCut = cms.double(0.0)
#process.generalDStarCandidatesNew.vtxSignificance2DCut = cms.double(0.0)
#process.generalDStarCandidatesNew.vtxSignificance3DCut = cms.double(0.0)
#process.generalDStarCandidatesNew.dStarMassCut = cms.double(0.22)
#process.generalDStarCandidatesNew.dPtCut = cms.double(0.0)
#process.generalDStarCandidatesNew.trkPtSumCut = cms.double(0.0)
#process.generalDStarCandidatesNew.trkEtaDiffCut = cms.double(0.0)
#process.generalDStarCandidatesNew.tkNhitsCut = cms.int32(0)
#process.generalDStarCandidatesNew.tkPtErrCut = cms.double(0.1)
#process.generalDStarCandidatesNew.tkPtCut = cms.double(0.4)
#process.generalDStarCandidatesNew.tkChi2Cut = cms.double(3)
#process.generalDStarCandidatesNew.vtxSignificance3DCut = cms.double(3)
#process.generalDStarCandidatesNew.alphaCut = cms.double(1)
#process.generalDStarCandidatesNew.alpha2DCut = cms.double(1)
#process.generalDStarCandidatesNew.dauLongImpactSigCut = cms.double(0.0)
#process.generalDStarCandidatesNew.dauTransImpactSigCut = cms.double(0.0)# it will be cut of by 3 in selector 
#process.generalD0CandidatesNew = process.generalD0Candidates.clone()
##process.generalD0CandidatesNew.trkPtSumCut = cms.double(1.6)
##process.generalD0CandidatesNew.trkEtaDiffCut = cms.double(2.0)
#process.generalD0CandidatesNew.tkNhitsCut = cms.int32(5)
##process.generalD0CandidatesNew.tkPtErrCut = cms.double(0.1)
#process.generalD0CandidatesNew.tkPtCut = cms.double(0.5)
##process.generalD0CandidatesNew.alphaCut = cms.double(1.0)
##process.generalD0CandidatesNew.alpha2DCut = cms.double(1.0)
#process.generalD0CandidatesNew.collinearityCut3D = cms.double(0.99)
##process.generalD0CandidatesNew.dPtCut = cms.double(0.0)
#process.generalD0CandidatesNew.tkChi2Cut = cms.double(2.5)
#process.generalD0CandidatesNew.VtxChiProbCut = cms.double(0.010)
#process.generalD0CandidatesNew.vtxSignificance3DCut = cms.double(5)
#process.generalD0CandidatesNew.mPiKCutMin = cms.double(1.74)
#process.generalD0CandidatesNew.mPiKCutMax = cms.double(2.00)
#process.generalD0CandidatesNewWrongSign = process.generalD0CandidatesNew.clone(isWrongSign = cms.bool(True))
#
#process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDStarCandidates_cff")
#process.generalDStarCandidatesNew = process.generalDStarCandidates.clone()
##process.generalDStarCandidatesNew.trkPtSumCut = cms.double(0.0)
##process.generalDStarCandidatesNew.trkEtaDiffCut = cms.double(0.0)
#process.generalDStarCandidatesNew.tkNhitsCut = cms.int32(3)
##process.generalDStarCandidatesNew.tkPtErrCut = cms.double(0.1)
#process.generalDStarCandidatesNew.tkPtCut = cms.double(0.4)
#process.generalDStarCandidatesNew.tkChi2Cut = cms.double(3)
##process.generalDStarCandidatesNew.vtxSignificance3DCut = cms.double(3)
##process.generalDStarCandidatesNew.alphaCut = cms.double(1)
##process.generalDStarCandidatesNew.alpha2DCut = cms.double(1)
#process.generalDStarCandidatesNew.dauLongImpactSigCut = cms.double(0.0)
#process.generalDStarCandidatesNew.dauTransImpactSigCut = cms.double(0.0)# it will be cut of by 3 in selector 


process.d0rereco_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNew)
#process.d0rereco_wrongsign_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNewWrongSign )



# produce D0 trees
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.d0selector_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.d0analyzer_tree_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dStaranalyzer_tree_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.eventinfotree_cff")
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.eventplaneanalyzer_cfi")

process.TFileService = cms.Service("TFileService",
    fileName =
    cms.string("output"+options.number+".root")
    )

process.d0ana.GenParticleCollection =  cms.untracked.InputTag("prunedGenParticles")
process.d0ana.useAnyMVA = cms.bool(False)
process.d0ana.multMin = cms.untracked.double(0)
process.d0ana.multMax = cms.untracked.double(100000)

#process.d0ana_mc.VertexCompositeCollection = cms.untracked.InputTag("d0selectorNewReduced:D0")
#process.d0ana_mc.MVACollection = cms.InputTag("d0selectorNewReduced:MVAValuesNewD0")
process.d0ana_newreduced = process.d0ana.clone()
process.d0ana_newreduced.CompositeCollection = cms.untracked.InputTag("generalD0CandidatesNew:D0")
process.d0ana_newreduced.VertexCompositeCollection = cms.untracked.InputTag("generalD0CandidatesNew:D0")

#process.dStarana.GenParticleCollection =  cms.untracked.InputTag("prunedGenParticles")
process.dStarana.useAnyMVA = cms.bool(False)
process.dStarana.CompositeCollection = cms.untracked.InputTag("generalDStarCandidatesNew:DStar")
process.dStarana.MVACollection = cms.InputTag("generalDStarCandidatesNew:MVAValuesNewDStar")
#process.dStarana_mc.doRecoNtuple= cms.untracked.bool(False)
process.generalDStarCandidatesNew.d0Collection = cms.InputTag("generalD0CandidatesNew:D0")


# process.d0ana_newreduced.MVACollection = cms.InputTag("generalD0CandidatesNew:MVAValuesNewD0")
#process.d0ana_newreduced.DCAValCollection = cms.InputTag("generalD0CandidatesNew:DCAValuesNewD0")
#process.d0ana_newreduced.DCAErrCollection = cms.InputTag("generalD0CandidatesNew:DCAErrorsNewD0")


#process.dStarAna_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNew*process.generalDStarCandidatesNew *process.dStarana)
process.dStarAna_step = cms.Path( process.eventFilter_HM * process.generalD0CandidatesNew* process.d0ana_newreduced*process.generalD0CandidatesNew*process.generalDStarCandidatesNew *process.dStarana)
#process.dStarAna_step = cms.Path( process.eventFilter_HM *  process.dStarana_mc)
#*process.eventplane)

# eventinfoana must be in EndPath, and process.eventinfoana.selectEvents must be the name of eventFilter_HM Path
process.eventinfoana.selectEvents = cms.untracked.string('eventFilter_HM_step')
process.eventinfoana.triggerPathNames = cms.untracked.vstring(
        "HLT_PPRefZeroBias_v*"
    )
process.eventinfoana.eventFilterNames = cms.untracked.vstring(
    'Flag_colEvtSel',
    'Flag_hfCoincFilter',
    'Flag_primaryVertexFilter', 
    )
process.eventinfoana.triggerFilterNames = cms.untracked.vstring()
process.eventinfoana.stageL1Trigger = cms.uint32(2)
process.pevt = cms.EndPath(process.eventinfoana)


# Add the Conversion tree

# Define the process schedule
process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.dStarAna_step,
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

