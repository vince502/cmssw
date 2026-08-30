import FWCore.ParameterSet.Config as cms


DATA_GLOBAL_TAG = "150X_dataRun3_Prompt_v3"
OO_MC_GLOBAL_TAG = "150X_mcRun3_2025_forOO_realistic_v9"


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

    globalTag = OO_MC_GLOBAL_TAG if isMC else DATA_GLOBAL_TAG
    process.GlobalTag = GlobalTag(process.GlobalTag, globalTag, "")

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
    process.oxygenEventFilter = cms.Sequence(
        process.primaryVertexFilter + process.clusterCompatibilityFilter
    )
    process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")

    from HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi import (
        hiEvtAnalyzer as _hiEvtAnalyzer,
    )
    from HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi import (
        skimanalysis as _skimanalysis,
    )

    process.hiEvtAnalyzer = _hiEvtAnalyzer.clone(
        Vertex=cms.InputTag("offlineSlimmedPrimaryVertices"),
        # The OO MiniAOD retains hiCentrality and centralityBin:HFtowers, so
        # keep the validated HiEvtAnalyzer activity observables instead of
        # writing their disabled sentinel values.
        doCentrality=cms.bool(True),
        doEvtPlane=cms.bool(False),
        doEvtPlaneFlat=cms.bool(False),
        doVertex=cms.bool(True),
        doMC=cms.bool(isMC),
        doHiMC=cms.bool(False),
        # hiHFfilters consumes towerMaker, which is not retained in MiniAOD.
        # The centrality/PF HF observables above remain available; do not
        # silently substitute a different definition for the tower filters.
        doHFfilters=cms.bool(False),
        useHepMC=cms.bool(False),
    )
    process.skimanalysis = _skimanalysis.clone(
        hltresults=cms.InputTag("TriggerResults", "", process.name_()),
        superFilters=cms.vstring(),
    )


def finalizeSchedule(process, physicsPaths, monitorEventSelection):
    scheduledPaths = []
    if monitorEventSelection:
        process.primaryVertexSelectionPath = cms.Path(process.primaryVertexFilter)
        process.clusterCompatibilitySelectionPath = cms.Path(
            process.clusterCompatibilityFilter
        )
        scheduledPaths.extend(
            [
                process.primaryVertexSelectionPath,
                process.clusterCompatibilitySelectionPath,
            ]
        )

    scheduledPaths.extend(physicsPaths)
    process.eventInfoEndPath = cms.EndPath(
        process.hiEvtAnalyzer + process.skimanalysis
    )
    scheduledPaths.append(process.eventInfoEndPath)
    process.schedule = cms.Schedule(*scheduledPaths)
