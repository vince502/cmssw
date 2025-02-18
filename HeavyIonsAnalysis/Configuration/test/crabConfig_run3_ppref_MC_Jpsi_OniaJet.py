from WMCore.Configuration import Configuration

config = Configuration()

config.section_('General')
#config.General.requestName = 'Psi2SJet_VertexCompsoite_MCOniaShowerPrivate_ppref_22Jan_v1'
config.General.requestName = 'Psi1SJet_VertexCompsoite_MCOniaPrivate_ppref_17Feb_v1'
config.General.workArea = 'crab_project'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'forest_miniAOD_run3_ppref_MC_Jpsi_OniaJet_tmp.py'
config.JobType.maxMemoryMB = 4000
config.JobType.numCores = 1

config.section_('Data')
#config.Data.inputDataset = '/Psi2S_OniaShower_PythiaOnly_07Nov_v1/soohwan-RECO_PAT_141X_PyhitaX_04Nov2024_v2-b77c20cd0a95b073265343812ed796c4/USER'
config.Data.inputDataset = '/Jpsi_PythiaCP5_Noemb_ppRef5p36_13Dec_v1/soohwan-PAT_MC_PythiaJPsi_CMSW_14_1_6_16Dec2024_v2_FixHLT_t2-932a5f8359956d6b7fe360a7c9b5e150/USER'
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 2
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
