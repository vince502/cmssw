"""One-PD CRAB configuration; importing this file does not submit a task.

Use the existing CMSSW_13_2_13 runtime on lxplus. Set config.Site.storageSite
directly to an approved, writable CMS storage site before submission.
Ten average files target roughly four hours including I/O headroom; this
is an estimate, not a runtime guarantee. The scheduler safety limit is 1000 minutes.
Native analyzer compression remains ZLIB1; no postprocessing script is used.
"""
from pathlib import Path

from CRABClient.UserUtilities import config

HERE = Path(__file__).resolve().parent
CERT = ('https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions23HI/'
        'Cert_Collisions2023HI_374288_375823_Muon.json')

config = config()
config.General.requestName = 'Onia2023_Prime0_pt2_m24to42_idx4_1'
config.General.workArea = str(HERE / 'crab_projects')
config.General.transferOutputs = True
config.General.transferLogs = False

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = str(HERE / 'hioniaanalyzer_PbPb2023_Prime0_cfg.py')
config.JobType.pyCfgParams = ['outputFile=onia.root', 'maxEvents=-1']
config.JobType.numCores = 4
config.JobType.maxMemoryMB = 4000
config.JobType.maxJobRuntimeMin = 1000
config.JobType.allowUndistributedCMSSW = True

config.Data.inputDataset = '/HIPhysicsRawPrime0/HIRun2023A-PromptReco-v2/MINIAOD'
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 10
config.Data.lumiMask = str(CERT)
config.Data.publication = False
config.Data.outLFNDirBase = '/store/user/soohwan/CharmoniaPbPbAudit/' + config.General.requestName

config.Site.storageSite = "T3_KR_KNU"
#config.Site.whitelist = ["T2_US_Vanderbilt"]
