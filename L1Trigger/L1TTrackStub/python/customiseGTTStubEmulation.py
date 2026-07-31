import FWCore.ParameterSet.Config as cms


def customiseGTTStubEmulation(process):
    """Append the stub-occupancy chain to a standard Phase-2 cmsDriver job."""

    process.load("L1Trigger.L1TTrackStub.gttStubEmulation_cff")

    if not hasattr(process, "schedule"):
        raise RuntimeError("customiseGTTStubEmulation requires a cmsDriver process with a schedule")

    if hasattr(process, "l1tGTTInputProducer"):
        process.l1tGTTInputProducer.l1TracksInputTag = cms.InputTag(
            "serenityGTTStubUnpacker",
            "Level1TTTracks",
        )
        process.l1tGTTInputProducer.setTrackWordBits = False

    process.gttStubEmulationPath = cms.Path(process.gttStubEmulation)
    if not hasattr(process, "TFileService"):
        process.TFileService = cms.Service(
            "TFileService",
            fileName=cms.string("gttStubValidation.root"),
        )

    process.gttStubProducts = cms.OutputModule(
        "PoolOutputModule",
        fileName=cms.untracked.string("gttStubFullChain.root"),
        outputCommands=cms.untracked.vstring(
            "drop *",
            *process.gttStubEmulationEventContent.outputCommands,
            "keep *_TTStubsFromPhase2TrackerDigis_StubAccepted_*",
            "keep *_l1tTTTracksFromTrackletEmulation_Level1TTTracks_*",
            "keep *_l1tGTTInputProducer_Level1TTTracksConverted_*",
            "keep *_l1tTrackSelectionProducer_*_*",
            "keep *_l1tVertexFinderEmulator_L1VerticesEmulation_*",
            "keep *_ProducerDTC_StubAccepted_*",
            "keep *_ProducerDTC_StubLost_*",
            "keep *_l1tGTAlgoBlockProducer_*_*",
            "keep edmTriggerResults_TriggerResults_*_*",
        ),
    )
    process.gttStubProductsEndPath = cms.EndPath(process.gttStubProducts)

    production_algo_path = getattr(process, "Phase2L1GTAlgoBlockProducer", None)
    use_production_algo_map = (
        hasattr(process, "l1tGTAlgoBlockProducer")
        and production_algo_path is not None
        and any(path is production_algo_path for path in process.schedule)
    )

    if use_production_algo_map:
        algorithm_name = "L1_GTT_LowStubOccupancy"
        configured_algorithms = {
            algorithm.name.value(): algorithm
            for algorithm in process.l1tGTAlgoBlockProducer.algorithms
            if hasattr(algorithm, "name")
        }
        if algorithm_name in configured_algorithms:
            configured = configured_algorithms[algorithm_name]
            if (
                not hasattr(configured, "expression")
                or configured.expression.value() != "gttStubLowOccupancyPath"
            ):
                raise RuntimeError(
                    f"{algorithm_name} already exists with a different expression"
                )
        else:
            process.l1tGTAlgoBlockProducer.algorithms.append(
                cms.PSet(
                    name=cms.string(algorithm_name),
                    expression=cms.string("gttStubLowOccupancyPath"),
                    triggerTypes=cms.vint32(1),
                )
            )

        process.gttStubValidation.algoBlocks = cms.InputTag("l1tGTAlgoBlockProducer")
        process.gttStubAccept.algoBlocksTag = cms.InputTag("l1tGTAlgoBlockProducer")

        candidate_emulation_before = getattr(
            process,
            "L1simulation_step",
            production_algo_path,
        )
        emulation_before = (
            candidate_emulation_before
            if any(path is candidate_emulation_before for path in process.schedule)
            else production_algo_path
        )
        emulation_index = next(
            index
            for index, path in enumerate(process.schedule)
            if path is emulation_before
        )
        for path in (
            process.gttStubEmulationPath,
            process.gttStubLowOccupancyPath,
        ):
            process.schedule.insert(emulation_index, path)
            emulation_index += 1

        algo_index = next(
            index
            for index, path in enumerate(process.schedule)
            if path is production_algo_path
        )
        process.schedule.insert(algo_index + 1, process.gttStubAcceptPath)
    else:
        insertion_point = next(
            (
                index
                for index, path in enumerate(process.schedule)
                if isinstance(path, cms.EndPath)
            ),
            len(process.schedule),
        )
        for path in (
            process.gttStubEmulationPath,
            process.gttStubLowOccupancyPath,
            process.gttStubAlgoBlockPath,
            process.gttStubAcceptPath,
        ):
            process.schedule.insert(insertion_point, path)
            insertion_point += 1

    endpath_insertion_point = next(
        (
            index
            for index, path in enumerate(process.schedule)
            if isinstance(path, cms.EndPath)
        ),
        len(process.schedule),
    )
    for path in (
        process.gttStubValidationEndPath,
        process.gttStubProductsEndPath,
    ):
        process.schedule.insert(endpath_insertion_point, path)
        endpath_insertion_point += 1
    return process
