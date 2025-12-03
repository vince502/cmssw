from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
#config.General.requestName = "Ntuplizer_test_AOD_Z_Run2018HI_Data"
config.General.requestName = "DStarAna_Data_DStarKpipi_CMSSW_15_0_9_01Jul25_v1"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.allowUndistributedCMSSW = True
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "pO2024_D0BothAndDStar_MB_cfg_v2.py"
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 4000         # request high memory machines.
#config.JobType.inputFiles=['CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374810.db']
#config.JobType.maxJobRuntimeMin = 2750    # request longer runtime, ~48 hours.

config.section_("Data")
#config.Data.inputDataset = '/HIDoubleMuon/HIRun2018A-04Apr2019-v1/AOD'
#config.Data.inputDataset = '/DStarKpipi/junseok-crab_RECO_MC_DStarKpipi_wokinematicCut_CMSSW_14_1_7_250328_v1-3d41c4bd01f1795c62fe47a112c1c01b/USER'
#config.Data.inputDataset = '/DStarKpipi/junseok-crab_RECO_MC_DStarKpipi_wokinematicCut_CMSSW_14_1_7_25Apr05_v1-3d41c4bd01f1795c62fe47a112c1c01b/USER'
#config.Data.inputDataset = '/DStarKpipi/junseok-crab_RECO_MC_DStarKpipi_CMSSW_14_1_7_250113_v1-3d41c4bd01f1795c62fe47a112c1c01b/USER'
#config.Data.inputDataset = '/DStarKpipiPU/junseok-crab_RECO_MINIAOD_DStarKpipiPU_CMSSW_13_2_10_082724_v1-e7a893e470c0a14923ed410f031778e3/USER'
config.Data.inputDataset = '/IonPhysics[0-59]/pORun2025-PromptReco-v1/MINIAOD'
#config.Data.ignoreLocality = True
config.Data.outputPrimaryDataset = 'Dstar'
#config.Data.userInputFiles = open('my_file_list.txt').readlines()
#config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 1
config.Data.splitting = 'FileBased'
config.Data.outLFNDirBase = '/store/user/junseok/DStarMC/%s' % (config.General.requestName)
#config.Data.publication = True
config.Data.totalUnits = -1
#config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/HI/PromptReco/Cert_326381-327564_HI_PromptReco_Collisions18_JSON_HF_and_MuonPhys.txt'

config.section_('Site')
#config.Site.storageSite = 'T2_CH_CERN'
config.Site.storageSite = 'T3_KR_KNU'
config.Site.whitelist = [ 'T2_US_*', 'T2_IT_*', 'T2_KR_*' ]
config.Site.ignoreGlobalBlacklist=True
