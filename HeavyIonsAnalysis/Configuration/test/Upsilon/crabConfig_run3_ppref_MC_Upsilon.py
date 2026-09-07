from WMCore.Configuration import Configuration

config = Configuration()

config.section_('General')
#config.General.requestName = 'Psi2SJet_VertexCompsoite_MCOniaShowerPrivate_ppref_22Jan_v1'
config.General.requestName = 'UpsilonS_OniaTree_ppref_04May2025_v1'
config.General.workArea = 'crab_project'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'forest_miniAOD_run3_ppref_MC_Upsilon.py'
config.JobType.maxMemoryMB = 2000
config.JobType.numCores = 1

config.section_('Data')
#config.Data.inputDataset = '/Psi2S_OniaShower_PythiaOnly_07Nov_v1/soohwan-RECO_PAT_141X_PyhitaX_04Nov2024_v2-b77c20cd0a95b073265343812ed796c4/USER'
config.Data.inputDataset = '/Upsilon1SToMuMu_pThat-2_TuneCP5_5p36TeV_pythia8/RunIIIpp5p36Winter24MiniAOD-NoPU_141X_mcRun3_2024_realistic_ppRef5TeV_v7-v1/MINIAODSIM'
config.Data.inputDBS = 'global'
config.Data.allowNonValidInputDataset = True
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.totalUnits = -1

config.Data.publication = True
#config.Data.ignoreLocality = True

config.Data.outputDatasetTag = config.General.requestName
#config.Data.outLFNDirBase = '/store/user/soohwan/%s' % (config.Data.outputDatasetTag)
#config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/Analysis/%s' % (config.Data.outputDatasetTag)
config.Data.outLFNDirBase = '/store/user/soohwan/Run3_2024/MC/%s' % (config.Data.outputDatasetTag)

config.section_('Site')
config.Data.ignoreLocality = False
#config.Site.storageSite = 'T2_US_Vanderbilt'
#config.Site.storageSite = 'T2_CH_CERN'
config.Site.storageSite = 'T3_KR_KNU'
