import FWCore.ParameterSet.Config as cms


DATA_GLOBAL_TAG = "141X_dataRun3_Prompt_v3"
MC_GLOBAL_TAG = "141X_mcRun3_2024_realistic_ppRef5TeV_v7"


def configureCommon(process, options, isMC):
    process.load("Configuration.StandardSequences.Reconstruction_cff")
    process.load("Configuration.StandardSequences.Services_cff")
    process.load("Configuration.Geometry.GeometryDB_cff")
    process.load("Configuration.StandardSequences.MagneticField_38T_cff")
    process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
    process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
    process.load("FWCore.MessageService.MessageLogger_cfi")
    process.MessageLogger.cerr.FwkReport.reportEvery = 100

    from Configuration.AlCa.GlobalTag import GlobalTag

    process.GlobalTag = GlobalTag(
        process.GlobalTag, MC_GLOBAL_TAG if isMC else DATA_GLOBAL_TAG, ""
    )
    process.source = cms.Source(
        "PoolSource",
        duplicateCheckMode=cms.untracked.string("noDuplicateCheck"),
        fileNames=cms.untracked.vstring(options.inputFiles),
    )
    process.maxEvents = cms.untracked.PSet(
        input=cms.untracked.int32(options.maxEvents)
    )
    process.options = cms.untracked.PSet(
        wantSummary=cms.untracked.bool(True),
        numberOfThreads=cms.untracked.uint32(1),
        numberOfStreams=cms.untracked.uint32(1),
    )
    process.TFileService = cms.Service(
        "TFileService", fileName=cms.string(options.outputFile)
    )

    if options.lumiMask:
        from FWCore.PythonUtilities.LumiList import LumiList

        process.source.lumisToProcess = LumiList(
            filename=options.lumiMask
        ).getVLuminosityBlockRange()

    process.load("HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff")

    from HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi import (
        hiEvtAnalyzer as _hiEvtAnalyzer,
    )
    from HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi import (
        skimanalysis as _skimanalysis,
    )

    process.hiEvtAnalyzer = _hiEvtAnalyzer.clone(
        Vertex=cms.InputTag("offlineSlimmedPrimaryVertices"),
        doCentrality=cms.bool(False),
        doEvtPlane=cms.bool(False),
        doEvtPlaneFlat=cms.bool(False),
        doVertex=cms.bool(True),
        doMC=cms.bool(isMC),
        doHiMC=cms.bool(False),
        doHFfilters=cms.bool(False),
        useHepMC=cms.bool(False),
    )
    process.skimanalysis = _skimanalysis.clone(
        hltresults=cms.InputTag("TriggerResults", "", process.name_()),
        superFilters=cms.vstring(),
    )


def finalizeSchedule(process, physicsPath):
    # These paths record the standard filter decisions but do not veto the
    # unified output. This keeps HiEvt, skim, Onia, and B entries aligned.
    process.primaryVertexSelectionPath = cms.Path(process.primaryVertexFilter)
    process.clusterCompatibilitySelectionPath = cms.Path(
        process.clusterCompatibilityFilter
    )
    process.eventInfoEndPath = cms.EndPath(
        process.hiEvtAnalyzer + process.skimanalysis
    )
    process.schedule = cms.Schedule(
        process.primaryVertexSelectionPath,
        process.clusterCompatibilitySelectionPath,
        physicsPath,
        process.eventInfoEndPath,
    )
