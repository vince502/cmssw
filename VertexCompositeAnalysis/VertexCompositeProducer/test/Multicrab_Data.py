from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
# from httplib import HTTPException
from http.client import HTTPException

from CRABClient.UserUtilities import config
config = config()

userName = "junseok"
date = "20250707"

config.section_("General")
config.General.workArea = 'crab_projects/'+date
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'pO2024_D0BothAndDStar_MB_cfg_v2.py'
config.section_('Data')
config.Data.inputDBS = 'global'
config.Data.splitting = 'Automatic'
# config.JobType.pyCfgParams = [ 'numThreads=1', 'resonance=Z', 'isFullAOD=False', 'isMC=False', 'globalTag=132X_dataRun3_Prompt_v4', 'era=Run2023HI', 'includeJets=False' ]
#config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/HI/PromptReco/Cert_326381-327564_HI_PromptReco_Collisions18_JSON_HF_and_MuonPhys.txt'
#config.Data.runRange = '326381-327564'
config.Data.publication = False
config.JobType.allowUndistributedCMSSW = True
config.Data.allowNonValidInputDataset = True

config.section_('Site')
#config.Data.ignoreLocality = True
#config.Site.whitelist = ['T2_US_Purdue', 'T2_US_MIT']
#config.Site.blacklist = ['T2_US_Vanderbilt']
config.Site.storageSite = 'T3_KR_KNU'

def submit(config):
    try:
        crabCommand('submit', config = config, dryrun=False)
    except HTTPException as hte:
        print ("Failed submitting task: %s" % (hte.headers))
    except ClientException as cle:
        print ("Failed submitting task: %s" % (cle))

#############################################################################################
## From now on that's what users should modify: this is the a-la-CRAB2 configuration part. ##
#############################################################################################

dataMap = {}

for i in range(0,60):
    dataMap[("IonPhysics"+str(i))] = { "PD": ("/IonPhysics"+str(i)+"/pORun2025-PromptReco-v1/MINIAOD"), "Units": 20, "Memory": 4000, "RunTime": 800 }

## Submit the muon PDs
for key, val in dataMap.items():
    config.General.requestName = 'DStarAnalysis_DstarToKpipi_CMSSW_15_0_9_'+key+'150X_dataRun3_Prompt'+date
    config.Data.inputDataset = val["PD"]
    #config.Data.unitsPerJob = val["Units"]
    config.JobType.maxMemoryMB = val["Memory"]
    # config.JobType.maxJobRuntimeMin = val["RunTime"]
    config.Data.outputDatasetTag = config.General.requestName
    config.Data.outLFNDirBase = '/store/user/junseok/DStarMC/%s/%s/%s' % (userName, date, config.General.requestName)
    print("Submitting CRAB job for: "+val["PD"])
    submit(config)
