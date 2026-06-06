from WMCore.Configuration import Configuration

config = Configuration()

config.section_('General')
config.General.requestName = 'OniaTree_ppRef2024_DATA_BcAllCombNoEDM_05Mar2025_v1'
config.General.workArea = 'crab_project'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'hionia_bcAllCombNoEDM_ppRef2024_DATA_slimmed_dimuon_cfg.py'
config.JobType.maxMemoryMB = 2800
config.JobType.numCores = 1

config.section_('Data')
config.Data.outputPrimaryDataset = 'PPRefDoubleMuonAll'
#config.Data.inputDataset = '/Psi2S_OniaShower_PythiaOnly_07Nov_v1/soohwan-GEN_MC_PythiaPsi_pTHat10_OniaShower_CMSW_14_1_4_07Nov2024_v1-a09dacb3b0eda2f4a8d5121c000dea3d/USER'
config.Data.userInputFiles = open('dbmu2024J.txt').readlines()
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 4
config.Data.totalUnits = -1

config.Data.publication = False
#config.Data.ignoreLocality = True

config.Data.outputDatasetTag = config.General.requestName
#config.Data.outLFNDirBase = '/store/user/soohwan/%s' % (config.Data.outputDatasetTag)
# config.Data.outLFNDirBase = '/store/user/soohwan/Run3_2024/MC/%s' % (config.Data.outputDatasetTag)
config.Data.outLFNDirBase = '/store/user/soohwan/Run3_2024/Data/%s' % (config.Data.outputDatasetTag)
# config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/Analysis/%s' % (config.Data.outputDatasetTag)

config.section_('Site')
config.Data.ignoreLocality = False
#config.Site.storageSite = 'T2_US_Vanderbilt'
# config.Site.storageSite = 'T2_CH_CERN'
config.Site.storageSite = 'T3_KR_KNU'
config.Site.whitelist = [ 'T2_US_*', 'T2_IT_*', 'T2_KR_*', 'T3_KR_*' ]
