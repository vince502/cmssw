from CRABClient.UserUtilities import config


# Edit this block for one production task.
isMC = False
collisionSystem = "OO"
requestName = "OxygenHeavyFlavor_D_DATA"
psetName = "../test/run_d_data_cfg.py"
inputDataset = "/PRIMARY/PROCESSED/TIER"
inputDBS = "global"
lumiMask = ""
outputFile = "Dmeson_OO_DATA.root"
storageSite = "T2_CH_CERN"
outLFNDirBase = "/store/user/USERNAME/OxygenHeavyFlavor"


config = config()
config.General.requestName = requestName
config.General.workArea = "crab_projects"
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = "Analysis"
config.JobType.psetName = psetName
config.JobType.pyCfgParams = [
    "collisionSystem=" + collisionSystem,
    "outputFile=" + outputFile,
]
config.JobType.outputFiles = [outputFile]
config.JobType.maxMemoryMB = 4000
config.JobType.numCores = 1

config.Data.inputDataset = inputDataset
config.Data.inputDBS = inputDBS
config.Data.publication = False
config.Data.outputDatasetTag = requestName
config.Data.outLFNDirBase = outLFNDirBase
config.Data.splitting = "FileBased" if isMC else "LumiBased"
config.Data.unitsPerJob = 1 if isMC else 50
if lumiMask:
    config.Data.lumiMask = lumiMask

config.Site.storageSite = storageSite
