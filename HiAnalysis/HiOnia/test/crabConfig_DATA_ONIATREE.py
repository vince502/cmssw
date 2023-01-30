from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "OniaTree_MBPD2_Run327237_MuonSelAll"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_PbPbPrompt_12_3_X_DATA_cfg.py"
#config.JobType.maxMemoryMB = 2500         # request high memory machines.
#config.JobType.numCores = 4
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.section_("Data")
config.Data.inputDataset = '/HIMinimumBias2/HIRun2018A-PbPb18_MiniAODv1-v1/MINIAOD'
config.Data.inputDBS = 'global'
#config.Data.unitsPerJob = 10
#config.Data.totalUnits = -1
config.Data.splitting = "Automatic"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/user/jaebeom/%s' % (config.General.requestName)
config.Data.publication = False
config.Data.runRange = '327237-327237'#'327123-327564'#'326381-327122' or 326382 or 327564
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/HI/PromptReco/Cert_326381-327564_HI_PromptReco_Collisions18_JSON_HF_and_MuonPhys.txt'

config.section_("Site")
config.Site.storageSite = "T3_KR_KNU"
#config.Site.whitelist = ["T2_CH_CERN"]
