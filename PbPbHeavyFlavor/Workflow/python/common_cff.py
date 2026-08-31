import FWCore.ParameterSet.Config as cms


DATA_GLOBAL_TAG = "132X_dataRun3_Prompt_v7"
MC_GLOBAL_TAG = "132X_mcRun3_2023_realistic_HI_v10"

DATA_CENTRALITY_TAG = (
    "CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_"
    "Run3v1302x04_Nominal_Offline"
)
MC_CENTRALITY_TAG = (
    "CentralityTable_HFtowers200_HydjetDrum5F_"
    "Run3v1302x04_Official_MC"
)


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
    process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
    process.GlobalTag.toGet.extend(
        [
            cms.PSet(
                record=cms.string("HeavyIonRcd"),
                tag=cms.string(
                    MC_CENTRALITY_TAG if isMC else DATA_CENTRALITY_TAG
                ),
                connect=cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
                label=cms.untracked.string("HFtowers"),
            )
        ]
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
    process.load("HeavyIonsAnalysis.EventAnalysis.hffilter_cfi")
    process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
    process.centralityBin.Centrality = cms.InputTag("hiCentrality")
    process.centralityBin.centralityVariable = cms.string("HFtowers")

    from HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi import (
        hiEvtAnalyzer as _hiEvtAnalyzer,
    )
    from HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi import (
        skimanalysis as _skimanalysis,
    )

    process.hiEvtAnalyzer = _hiEvtAnalyzer.clone(
        Vertex=cms.InputTag("offlineSlimmedPrimaryVertices"),
        doCentrality=cms.bool(True),
        doEvtPlane=cms.bool(False),
        doEvtPlaneFlat=cms.bool(False),
        doVertex=cms.bool(True),
        doMC=cms.bool(isMC),
        doHiMC=cms.bool(isMC),
        doHFfilters=cms.bool(True),
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
    process.hfCoincidenceSelectionPath = cms.Path(process.phfCoincFilter2Th4)
    process.eventInfoEndPath = cms.EndPath(
        process.hiEvtAnalyzer + process.skimanalysis
    )
    process.schedule = cms.Schedule(
        process.primaryVertexSelectionPath,
        process.clusterCompatibilitySelectionPath,
        process.hfCoincidenceSelectionPath,
        physicsPath,
        process.eventInfoEndPath,
    )
