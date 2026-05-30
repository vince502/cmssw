from WMCore.Configuration import Configuration
import os

config = Configuration()

here = os.path.dirname(os.path.abspath(__file__))
datasetFile = os.path.abspath(os.path.join(here, '..', '..', '..', 'datasets.txt'))

with open(datasetFile) as dataset_file:
    inputFiles = [
        line.strip()
        for line in dataset_file
        if line.strip() and not line.lstrip().startswith('#')
    ]

config.section_("General")
config.General.requestName = "OniaTree_MuonElectron_PbPb2026_DATA"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "hioniaanalyzer_MuonElectron_ppRef2024_DATA_cfg.py"
config.JobType.pyCfgParams = ['datasetFile=']
#config.JobType.maxMemoryMB = 2500         # request high memory machines.
config.JobType.numCores = 1
config.JobType.allowUndistributedCMSSW = True #Problems with slc7
#config.JobType.maxJobRuntimeMin = 1000 #2750    # request longer runtime, ~48 hours.

config.section_("Data")
config.Data.userInputFiles = inputFiles
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 2
config.Data.totalUnits = -1
config.Data.splitting = "FileBased"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/user/soohwan/Run2026/OniaTree'
config.Data.publication = False

config.section_("Site")
config.Site.storageSite = 'T2_CH_CERN'
config.Site.whitelist = ["T2_CH_CERN"]
