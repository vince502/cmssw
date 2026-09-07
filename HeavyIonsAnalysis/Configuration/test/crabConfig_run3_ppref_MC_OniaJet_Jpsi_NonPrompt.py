from WMCore.Configuration import Configuration

config = Configuration()

config.section_('General')
config.General.requestName = 'NonPrompt_Psi1SJet_VertexCompsoite_MCPythia8_ppref_16Jul_v2'
config.General.workArea = 'crab_project'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'forest_miniAOD_run3_ppref_MC_Jpsi_OniaJet_tmp.py'
config.JobType.maxMemoryMB = 2500
config.JobType.numCores = 1

config.section_('Data')
config.Data.inputDataset = '/BToNonPromptJPsiToMuMu_inclusive_TuneCP5_5p36TeV_pythia8-evtgen/RunIIIpp5p36Winter24MiniAOD-141X_mcRun3_2024_realistic_ppRef5TeV_v7-v2/MINIAODSIM'
#config.Data.inputDBS = 'phys03'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.allowNonValidInputDataset = True
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
