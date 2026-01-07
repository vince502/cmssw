from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "PPrefPromptD0_MVATight_05Jan2026_v2_CandFilter"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "ppRefDmeson_DATA_cfg.py"
config.JobType.maxMemoryMB = 3000         # request high memory machines.
config.JobType.numCores = 1
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.section_("Data")
#config.Data.inputDataset = '/PPRefZeroBiasPlusForward0/Run2024J-PromptReco-v1/MINIAOD'
config.Data.outputPrimaryDataset = 'ppRefCombinedAna'
config.Data.userInputFiles = open('zeroBias.txt').readlines()
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 6
config.Data.totalUnits = -1
config.Data.splitting = "FileBased"
config.Data.allowNonValidInputDataset = True

#config.Data.outLFNDirBase = '/store/user/soohwan/DmesonAna/Run2024ppRef/%s' % (config.General.requestName)
config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/DmesonAna/Run2024ppRef/%s' % (config.General.requestName)
config.Data.publication = False

# for pPb
#config.Data.runRange = '285479-285832'
#config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/HI/Cert_285479-285832_HI8TeV_PromptReco_pPb_Collisions16_JSON_NoL1T_MuonPhys.txt'

# for Pbp
#config.Data.runRange = '285952-286496'
#config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/HI/Cert_285952-286496_HI8TeV_PromptReco_Pbp_Collisions16_JSON_NoL1T_MuonPhys.txt'

config.section_("Site")
#config.Site.storageSite = "T3_KR_KNU"
config.Site.storageSite = "T2_CH_CERN"
config.Site.whitelist = ["T2_CH_CERN", "T2_US_*", "T2_KR_*", "T3_KR_*"]
