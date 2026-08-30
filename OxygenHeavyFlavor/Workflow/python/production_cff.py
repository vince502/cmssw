import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

from OxygenHeavyFlavor.Workflow.common_cff import (
    configureCommon,
    finalizeSchedule,
)


def _makeOptions(defaultOutput):
    options = VarParsing.VarParsing("analysis")
    options.outputFile = defaultOutput
    options.inputFiles = []
    options.maxEvents = -1
    options.register(
        "collisionSystem",
        "OO",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "Collision system: OO or NeNe",
    )
    options.register(
        "lumiMask",
        "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "Optional local JSON lumi mask; CRAB Data.lumiMask remains authoritative",
    )
    options.register(
        "eventSelection",
        "legacy",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "Event selection policy: legacy, on, or off",
    )
    for name, default, valueType, description in (
        ("d0MvaCut", 0.5, VarParsing.VarParsing.varType.float, "B-to-D0K D0 MVA threshold"),
        ("d0DaughterPtCut", 0.5, VarParsing.VarParsing.varType.float, "B-to-D0K D0 daughter pT threshold"),
        ("d0EtaGapCut", 999.0, VarParsing.VarParsing.varType.float, "B-to-D0K D0 daughter eta-gap threshold"),
        ("d0PtCut", 0.0, VarParsing.VarParsing.varType.float, "B-to-D0K D0 pT threshold"),
        ("bachelorKaonPtCut", 0.8, VarParsing.VarParsing.varType.float, "B-to-D0K bachelor-kaon pT threshold"),
        ("minD0Candidates", 1, VarParsing.VarParsing.varType.int, "B-to-D0K minimum D0 candidates; zero disables"),
        ("minBToD0KCandidates", 0, VarParsing.VarParsing.varType.int, "Minimum B-to-D0K candidates; zero disables"),
    ):
        options.register(
            name,
            default,
            VarParsing.VarParsing.multiplicity.singleton,
            valueType,
            description,
        )
    options.parseArguments()
    return options


def _selectionEnabled(channel, isMC, policy):
    if policy == "on":
        return True
    if policy == "off":
        return False
    if policy != "legacy":
        raise ValueError("eventSelection must be one of: legacy, on, off")
    if channel == "d":
        return not isMC
    if channel == "bToD0K":
        return True
    if channel == "bToJpsi":
        return True
    if channel == "upsilon":
        return True
    raise ValueError("Unknown channel: " + channel)


def createProcess(channels, isMC, defaultOutput):
    options = _makeOptions(defaultOutput)
    collisionSystem = options.collisionSystem.strip()
    if collisionSystem not in ("OO", "NeNe"):
        raise ValueError("collisionSystem must be OO or NeNe")
    if isMC and collisionSystem != "OO":
        raise ValueError(
            "NeNe MC is disabled: no approved NeNe MC era and GlobalTag are documented"
        )
    process = cms.Process("OXYHF", eras.Run3_2025_OXY)
    configureCommon(process, options, isMC)

    physicsPaths = []
    selectionFlags = {
        channel: _selectionEnabled(channel, isMC, options.eventSelection)
        for channel in channels
    }

    # HiOnia owns one inclusive dimuon collection. B reconstruction consumes
    # that same collection, so jpsiIdx and Reco_QQ_collIdx are identical EDM
    # collection indices and no kinematic matcher is needed.
    oniaChannels = tuple(
        channel for channel in ("bToJpsi", "upsilon") if channel in channels
    )
    if oniaChannels:
        from OxygenHeavyFlavor.Workflow.upsilon_cff import configureInclusiveOnia

        oniaBSequence = configureInclusiveOnia(process, options, isMC)
        if "bToJpsi" in channels:
            from OxygenHeavyFlavor.Workflow.bToJpsi_cff import configureBToJpsi

            oniaBSequence += configureBToJpsi(process, isMC)

        if any(selectionFlags[channel] for channel in oniaChannels):
            # Default MC production keeps the unbiased generator denominator
            # and identical entry ordering across the event, Onia, and B
            # trees.  The standalone selection paths below still record both
            # filter decisions.  eventSelection=on retains the explicitly
            # requested filtering behavior.
            if isMC and options.eventSelection == "legacy":
                oniaBSequence = (
                    cms.ignore(process.primaryVertexFilter)
                    * cms.ignore(process.clusterCompatibilityFilter)
                    * oniaBSequence
                )
            else:
                oniaBSequence = process.oxygenEventFilter * oniaBSequence
        process.oniaBPath = cms.Path(oniaBSequence)
        physicsPaths.append(process.oniaBPath)
    if "d" in channels:
        from OxygenHeavyFlavor.Workflow.dMeson_cff import addDMeson

        physicsPaths.append(addDMeson(process, isMC, selectionFlags["d"]))
    if "bToD0K" in channels:
        from OxygenHeavyFlavor.Workflow.bToD0K_cff import addBToD0K

        physicsPaths.append(
            addBToD0K(process, options, isMC, selectionFlags["bToD0K"])
        )

    if not physicsPaths:
        raise ValueError("At least one production channel is required")
    finalizeSchedule(process, physicsPaths, any(selectionFlags.values()))
    return process
