from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
from http.client import HTTPException

from CRABClient.UserUtilities import config
config = config()

userName = "User"
date = "11Jul2024"

config.section_("General")
config.General.workArea = 'crab_projects_v1/'+date
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
#config.JobType.inputFiles = ['HeavyIonRPRcd_PbPb2018_offline.db']
config.JobType.psetName = '../PbPb2023_DPlusMinus_MB_cfg_v0.py'

config.section_('Data')
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
#config.Data.lumiMask = 'Cert_Collisions2023HI_374288_375659_Muon.json'
config.Data.runRange = '374345-375823'
config.Data.unitsPerJob = 1
config.Data.totalUnits = -1# 
config.Data.publication = False
config.JobType.allowUndistributedCMSSW = True
config.Data.allowNonValidInputDataset = False

config.JobType.inputFiles = ["../CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374810.db"]

config.section_('Site')
#config.Data.ignoreLocality = True
#config.Site.whitelist = ['T2_US_Purdue', 'T2_US_MIT']
#config.Site.blacklist = ['T2_US_Vanderbilt']
#config.Data.outputDatasetTag = config.General.requestName
#config.Data.outLFNDirBase = '/store/user/soohwan/%s' % (config.Data.outputDatasetTag)
#
#config.section_('Site')
#config.Data.ignoreLocality = False
#config.Site.storageSite = 'T2_KR_KISTI'
config.Site.storageSite = 'T3_KR_KNU'

def submit(config):
    try:
        crabCommand('submit', config = config, dryrun=False)
    except HTTPException as hte:
        print("Failed submitting task: %s" % (hte.headers))
    except ClientException as cle:
        print("Failed submitting task: %s" % (cle))

#############################################################################################
## From now on that's what users should modify: this is the a-la-CRAB2 configuration part. ##
#############################################################################################

dataMap = {}

for i in range(0,1):
    dataMap[("HIPhysicsRawPrime"+str(i))] = { "PD": ("/HIPhysicsRawPrime"+str(i)+"/HIRun2023A-PromptReco-v2/MINIAOD"), "Units": 1, "Memory": 2400, "RunTime": 2700 }

## Submit the muon PDs
for key, val in dataMap.items():
    config.General.requestName = 'DPMPbPbTest_'+key+'_HIRun2023_PromptRecoV2_'+date + '_v3'
    config.Data.inputDataset = val["PD"]
    config.Data.unitsPerJob = val["Units"]
    config.JobType.maxMemoryMB = val["Memory"]
    config.JobType.maxJobRuntimeMin = val["RunTime"]
    config.Data.outputDatasetTag = config.General.requestName
    config.Data.outLFNDirBase = '/store/user/soohwan/Analysis/DStar_D0/%s/PbPb2023MinimumBias/%s' % (date, config.General.requestName)

    print("Submitting CRAB job for: "+val["PD"])
    submit(config)
