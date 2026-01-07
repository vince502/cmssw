### HiForest Configuration for Condor Submission
# Wrapper around forest_miniAOD_OniaBmeson_Unified_DATA_simple.py with command-line arguments
#
# Usage:
#   cmsRun forest_miniAOD_OniaBmeson_condor.py inputFiles=file.root outputFile=output.root maxEvents=-1

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

###############################################################################
# Command line options
###############################################################################
options = VarParsing('analysis')
options.outputFile = 'HiForest.root'
options.maxEvents = -1
options.parseArguments()

###############################################################################
# Execute the main config
###############################################################################
import os
config_dir = os.path.dirname(os.path.abspath(__file__))
config_file = os.path.join(config_dir, 'forest_miniAOD_OniaBmeson_Unified_DATA_simple.py')

# Read and execute the main config
with open(config_file, 'r') as f:
    exec(compile(f.read(), config_file, 'exec'))

###############################################################################
# Override settings from command line
###############################################################################
# Input files
process.source.fileNames = cms.untracked.vstring(options.inputFiles)

# Output file
process.TFileService.fileName = cms.string(options.outputFile)

# Max events
process.maxEvents.input = cms.untracked.int32(options.maxEvents)

# Single thread for condor (more reliable)
process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(0)
