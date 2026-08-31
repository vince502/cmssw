import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

from PbPbHeavyFlavor.Workflow.common_cff import configureCommon, finalizeSchedule


def _makeOptions(defaultOutput):
    options = VarParsing.VarParsing("analysis")
    options.outputFile = defaultOutput
    options.inputFiles = []
    options.maxEvents = -1
    options.register(
        "lumiMask",
        "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "Optional local JSON lumi mask; CRAB Data.lumiMask remains authoritative",
    )
    options.register(
        "centralityTag",
        "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "Override the default HFtowers HeavyIonRcd tag",
    )
    options.parseArguments()
    return options


def createProcess(isMC, defaultOutput):
    options = _makeOptions(defaultOutput)
    process = cms.Process("PBPBHF", eras.Run3_pp_on_PbPb_2026)
    configureCommon(process, options, isMC)

    from PbPbHeavyFlavor.Workflow.onia_cff import configureInclusiveOnia
    from PbPbHeavyFlavor.Workflow.bToJpsi_cff import configureBToJpsi

    unifiedSequence = configureInclusiveOnia(process, isMC)
    unifiedSequence += configureBToJpsi(process, isMC)
    process.unifiedOniaBPath = cms.Path(unifiedSequence)
    finalizeSchedule(process, process.unifiedOniaBPath)
    return process
