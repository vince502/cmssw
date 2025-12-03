from datetime import datetime
from CRABClient.UserUtilities import config

config = config()

config.section_("General")
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'VCTree_PbPb2023_UPCDiMu_Express_cfg.py'
# config.JobType.inputFiles = ['HeavyIonRPRcd_PbPb2018_offline.db']
#config.JobType.numCores = 1
config.JobType.allowUndistributedCMSSW = True

config.section_('Data')
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.publication = False

config.section_('Site')
config.Data.ignoreLocality = True
config.Site.whitelist = ['T2_CH_CERN']
config.Site.storageSite = 'T2_CH_CERN'


#############################################################################################
#############################################################################################

request_name = "VCTree_Express_132X"
request_name += "_%s" % datetime.now().strftime("%y%m%d_%H%M%S")

config.General.requestName = request_name
config.Data.outputDatasetTag = config.General.requestName
config.Data.outLFNDirBase = '/store/group/phys_heavyions/jiazhao/Data_Run3/VCTree/%s' %  config.General.requestName
# config.JobType.maxMemoryMB = 2000
# config.JobType.maxJobRuntimeMin = 1000
# config.Data.inputDataset = dataset

#* Dataset **************************************************************
# dataset = "/CohJPsi_STARLIGHT_5p36TeV_2023Run3/phys_heavyions-CohJPsi_STARLIGHT_5p36TeV_2023Run3_RECO_20230812-0e0f7c8639a89ace3f1b7064824b102c/USER"
# config.Data.inputDataset = dataset
# print("Submitting CRAB job for: "+dataset)

#* FileList *************************************************************
input_filelist = "/afs/cern.ch/user/j/jiazhao/VCTreeProductions/CMSSW_13_2_5_patch1/src/VertexCompositeAnalysis/VertexCompositeProducer/test/fileList/Express_374345.txt"
config.Data.userInputFiles = open(input_filelist).readlines() 
print("Submitting CRAB job for: "+input_filelist)

print('OutputDirectory: '+config.Data.outLFNDirBase)
