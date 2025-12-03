from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
#config.General.requestName = "Ntuplizer_test_AOD_Z_Run2018HI_Data"
config.General.requestName = "D0tarAna_MC_Step0_PromptD0Pt0_CMSSW_13_2_13_MVA_25Feb2025_v2"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_("JobType")
config.JobType.allowUndistributedCMSSW = True
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "PbPb2023_D0BothAndDStar_MB_cfg_mc_v1_Step0.py"
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 4000         # request high memory machines.
config.JobType.inputFiles=['CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374810.db']
config.JobType.maxJobRuntimeMin = 2750    # request longer runtime, ~48 hours.

config.section_("Data")
#config.Data.inputDataset = '/HIDoubleMuon/HIRun2018A-04Apr2019-v1/AOD'
config.Data.inputDataset = '/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/HINPbPbSpring23MiniAOD-132X_mcRun3_2023_realistic_HI_v9-v2/MINIAODSIM'
#config.Data.inputDataset = '/DStarKpipiPU/junseok-crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_082724_v1-e7a893e470c0a14923ed410f031778e3/USER'
#config.Data.ignoreLocality = True
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 4
config.Data.splitting = 'FileBased'
#config.Data.outLFNDirBase = '/store/group/phys_heavyions/junseok/DStarAna/Data/%s' % (config.General.requestName)
config.Data.outLFNDirBase = '/store/user/soohwan/Run3_2023/MC/SkimMVA/%s' % (config.General.requestName)
config.Data.publication = True
config.Data.totalUnits = 40
#config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/HI/PromptReco/Cert_326381-327564_HI_PromptReco_Collisions18_JSON_HF_and_MuonPhys.txt'

config.section_('Site')
#config.Site.storageSite = 'T2_CH_CERN'
config.Site.storageSite = 'T3_KR_KNU'
#config.Site.storageSite = 'T2_KR_KISTI'
#config.Site.whitelist = [ 'T2_US_*', 'T2_IT_*', 'T2_KR_*' ]
config.Site.ignoreGlobalBlacklist=True
