from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "OniaTree_Combined_PbPb2025_EWSkim_28Nov_2025_v3"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_Combined_PbPb2025_DATA_cfg.py"
#config.JobType.maxMemoryMB = 2500         # request high memory machines.
config.JobType.numCores = 1
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.section_("Data")
config.Data.userInputFiles = open('fileEWSkim.txt').readlines()
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 7
#config.Data.totalUnits = -1
config.Data.splitting = "FileBased"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/PbPb2025/%s' % (config.General.requestName)
config.Data.publication = False

config.section_("Site")
config.Site.storageSite = "T2_CH_CERN"
#config.Site.whitelist = ["T2_CH_CERN"]
config.Site.whitelist = [ 'T2_US_*', 'T2_IT_*', 'T2_KR_*', 'T2_CH_CERN' ]
