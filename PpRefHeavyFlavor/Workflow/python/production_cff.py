import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

from PpRefHeavyFlavor.Workflow.common_cff import configureCommon, finalizeSchedule


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
    options.parseArguments()
    return options


def createProcess(isMC, defaultOutput):
    options = _makeOptions(defaultOutput)
    process = cms.Process("PPRHF", eras.Run3_2024_ppRef)
    configureCommon(process, options, isMC)

    from PpRefHeavyFlavor.Workflow.onia_cff import configureInclusiveOnia
    from PpRefHeavyFlavor.Workflow.bToJpsi_cff import configureBToJpsi

    unifiedSequence = configureInclusiveOnia(process, isMC)
    unifiedSequence += configureBToJpsi(process, isMC)
    process.unifiedOniaBPath = cms.Path(unifiedSequence)
    finalizeSchedule(process, process.unifiedOniaBPath)
    return process
