from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "OniaTree_RawPrime0"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_PbPbPrompt_13_2_X_DATA_cfg.py"
#config.JobType.maxMemoryMB = 2500         # request high memory machines.
config.JobType.numCores = 4
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.JobType.inputFiles = ["CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374289.db"]

config.section_("Data")
config.Data.inputDataset = '/HIPhysicsRawPrime0/HIRun2023A-PromptReco-v2/MINIAOD'
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 50
#config.Data.totalUnits = -1
config.Data.splitting = "LumiBased"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/user/fdamas/%s' % (config.General.requestName)
config.Data.publication = False
config.Data.runRange = '374345-374828'
config.Data.lumiMask = '/eos/cms/store/group/phys_heavyions/sayan/HIN_run3_pseudo_JSON/HIPhysicsRawPrime/Muon_Online_live.json'

config.section_("Site")
config.Site.storageSite = "T2_FR_GRIF_LLR"
#config.Site.whitelist = ["T2_CH_CERN"]
