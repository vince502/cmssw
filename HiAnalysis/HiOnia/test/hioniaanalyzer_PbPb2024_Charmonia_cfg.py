from pathlib import Path
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from FWCore.PythonUtilities.LumiList import LumiList
from Configuration.Eras.Era_Run3_pp_on_PbPb_2024_cff import Run3_pp_on_PbPb_2024
from Configuration.AlCa.GlobalTag import GlobalTag

options = VarParsing("analysis")
options.outputFile = "charmonia_PbPb2024.root"
options.maxEvents = 1000
options.register(
    "globalTag", "141X_dataRun3_Prompt_v4", VarParsing.multiplicity.singleton,
    VarParsing.varType.string, "GlobalTag for the input data campaign.",
)
options.register(
    "lumiMask",
    "/eos/user/c/cmsdqm/www/CAF/certification/Collisions24HI/Cert_Collisions2024_HI_387853_388784_Muon.json",
    VarParsing.multiplicity.singleton, VarParsing.varType.string,
    "Official CMS DQM certification file on EOS; read directly, never copied.",
)
options.register(
    "centralityVariation", "Nominal", VarParsing.multiplicity.singleton,
    VarParsing.varType.string, "Data centrality table: Nominal, SysUp or SysDown.",
)
options.parseArguments()
if options.centralityVariation not in ("Nominal", "SysUp", "SysDown"):
    raise ValueError("centralityVariation must be Nominal, SysUp or SysDown")
if not options.inputFiles:
    raise ValueError("Supply inputFiles=<PbPb 2024 MINIAOD LFN or PFN>.")
if not options.lumiMask:
    raise ValueError("A certification lumiMask is required for this data configuration.")

process = cms.Process("CHARMONIA", Run3_pp_on_PbPb_2024)
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.GlobalTag = GlobalTag(process.GlobalTag, options.globalTag, "")
process.source = cms.Source(
    "PoolSource", fileNames=cms.untracked.vstring(options.inputFiles),
    lumisToProcess=cms.untracked.VLuminosityBlockRange(
        LumiList(filename=options.lumiMask).getCMSSWString().split(",")
    ),
)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.options = cms.untracked.PSet(
    numberOfThreads=cms.untracked.uint32(4),
    numberOfStreams=cms.untracked.uint32(0),
    wantSummary=cms.untracked.bool(True),
)
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.TFileService = cms.Service(
    "TFileService", fileName=cms.string(options.outputFile),
    closeFileFast=cms.untracked.bool(True),
)
process.load("HiAnalysis.HiOnia.charmoniaPbPbMiniAOD_cff")
# Read the supplied public conditions DB directly; never copy it locally.
centrality_db = Path(
    "/afs/cern.ch/work/n/nsaha/public/for_GO/DBfiles_2024/"
    "CentralityTable_HFtowers200_DataPbPb2024_periHYDJETshape_run3v140x01_offline_"
    + options.centralityVariation + ".db"
)
# All internal tag names omit '2024', unlike the supplied filenames.
centrality_tag = (
    "CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v140x01_offline_"
    + options.centralityVariation
)
if not centrality_db.is_file():
    raise FileNotFoundError("Provided centrality DB missing; a separately validated fallback is required: " + str(centrality_db))
process.GlobalTag.toGet.append(cms.PSet(
    record=cms.string("HeavyIonRcd"),
    tag=cms.string(centrality_tag),
    label=cms.untracked.string("HFtowers"),
    connect=cms.string("sqlite_file:" + str(centrality_db)),
))
process.hionia.histFileName = cms.string(options.outputFile)
process.load("HiAnalysis.HiOnia.charmoniaEventSelection_cff")
process.event_filters = cms.Sequence(process.eventFilters2024)
process.charmonia = cms.Path(process.event_filters * process.charmoniaSequence)
