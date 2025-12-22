from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
#config.General.requestName = "Ntuplizer_test_AOD_Z_Run2018HI_Data"
#config.General.requestName = "OniaTree_OODimuon_MINIAOD_Run2025OO_PromptReco_v1_11Dec"
config.General.requestName = "DStarOO_MINIAOD_Run2025OO_PromptRecoMVA_v1_20Dec"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.allowUndistributedCMSSW = True
config.JobType.pluginName = "Analysis"
#config.JobType.psetName = "testChiX_DATA_cfg.py" 
config.JobType.psetName = "testDmeson_DATA_cfg.py" 
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2800         # request high memory machines.
#config.JobType.maxJobRuntimeMin = 2750    # request longer runtime, ~48 hours.

config.section_("Data")
#config.Data.inputDataset = '/HIDoubleMuon/HIRun2018A-04Apr2019-v1/AOD'
#config.Data.inputDataset = '/HIPhysicsRawPrime0/HIRun2023A-PromptReco-v1/MINIAOD'
#config.Data.userInputFiles = open('OODimuSkim.txt').readlines()
config.Data.userInputFiles = open('dasfiles_OO.txt').readlines()
config.Data.ignoreLocality = False
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 10
config.Data.splitting = 'FileBased'
#config.Data.outLFNDirBase = '/store/user/soohwan/OniaTree/Run2025OOV1/%s' % (config.General.requestName)
config.Data.outLFNDirBase = '/store/group/phys_heavyions/soohwan/OO2025/%s' % (config.General.requestName)
config.Data.publication = False
config.Data.totalUnits = 10000
#config.Data.runRange = '374345-375828'
#config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions23HI/Cert_Collisions2023HI_374288_375823_Muon.json'

config.section_('Site')
#config.Site.storageSite = 'T3_CH_CERNBOX'
config.Site.storageSite = 'T2_CH_CERN'
#config.Site.storageSite = 'T2_KR_KISTI'
config.Site.whitelist = [ 'T2_CH_CERN', 'T2_US_*', 'T2_IT_*', 'T2_KR_*' ]


