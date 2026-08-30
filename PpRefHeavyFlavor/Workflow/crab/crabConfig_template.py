from CRABClient.UserUtilities import config


# Edit this block for one data, B MC, or Upsilon MC production task.
isMC = True
requestName = "UnifiedOniaB_ppRef_MC"
psetName = "../test/run_unified_mc_cfg.py"
inputDataset = "/PRIMARY/PROCESSED/MINIAODSIM"
inputDBS = "global"
lumiMask = ""
outputFile = "UnifiedOniaB_ppRef_MC.root"
storageSite = "T2_CH_CERN"
outLFNDirBase = "/store/user/USERNAME/PpRefHeavyFlavor"


config = config()
config.General.requestName = requestName
config.General.workArea = "crab_projects"
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = "Analysis"
config.JobType.psetName = psetName
config.JobType.pyCfgParams = ["outputFile=" + outputFile]
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
