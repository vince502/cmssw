from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "OniaTree_JpsiEE_AllZB_2024J_21Sep_2025_v2"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_ee_ppRef2024_PromptReco_cfg.py"
#config.JobType.maxMemoryMB = 2500         # request high memory machines.
config.JobType.numCores = 1
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.section_("Data")
#config.Data.inputDataset = '/PPRefZeroBiasPlusForward0/Run2024J-PromptReco-v1/MINIAOD'
config.Data.userInputFiles = open('dasfiles_ZB.txt').readlines()
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 25 
#config.Data.totalUnits = -1
config.Data.splitting = "FileBased"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/EE2024J/%s' % (config.General.requestName)
config.Data.publication = False

# for pPb
#config.Data.runRange = '285479-285832'
#config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/HI/Cert_285479-285832_HI8TeV_PromptReco_pPb_Collisions16_JSON_NoL1T_MuonPhys.txt'

# for Pbp
#config.Data.runRange = '285952-286496'
#config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/HI/Cert_285952-286496_HI8TeV_PromptReco_Pbp_Collisions16_JSON_NoL1T_MuonPhys.txt'

config.section_("Site")
config.Site.storageSite = "T2_CH_CERN"
#config.Site.whitelist = ["T2_CH_CERN"]
config.Site.whitelist = [ 'T2_US_*', 'T2_IT_*', 'T2_KR_*', 'T2_CH_*' ]
