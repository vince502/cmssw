"""
CRAB configuration for B-jet flattening
Note: CRAB is designed for CMSSW jobs, not standalone ROOT macros.
For ROOT macro jobs, HTCondor is recommended.

This config is a template showing how you could wrap the macro in a CMSSW job.
"""

from CRABClient.UserUtilities import config
config = config()

# General settings
config.General.requestName = 'FlattenBJet_OO2025_v1'
config.General.workArea = 'crab_FlattenBJet'
config.General.transferOutputs = True
config.General.transferLogs = True

# Job type - use scriptExe for non-CMSSW jobs
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'dummy_pset.py'  # Minimal CMSSW config
config.JobType.scriptExe = 'flattenBJetMatch_crab.sh'
config.JobType.scriptArgs = []
config.JobType.inputFiles = ['flattenBJetMatch.C']
config.JobType.outputFiles = ['output_flat.root']
config.JobType.maxMemoryMB = 4000
config.JobType.maxJobRuntimeMin = 180

# Data settings - use userInputFiles for non-DBS files
config.Data.userInputFiles = open('filelist_OO_OniaForest.txt').read().splitlines()
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1  # 1 file per job for max parallelization
config.Data.outputPrimaryDataset = 'FlatBJet_OO2025'
config.Data.outLFNDirBase = '/store/user/soohwan/FlatBJet/'
config.Data.publication = False

# Site settings
config.Site.storageSite = 'T2_CH_CERN'
