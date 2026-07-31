#!/usr/bin/env python3

import argparse
import json

import ROOT
from DataFormats.FWLite import Events, Handle


REGION_LINKS = (0, 2, 4, 6, 8, 10, 12, 14, 16)
NUM_LINKS = 18
METADATA_TRACK_SLOT = 103
FRAMES_PER_LINK = 156
COUNT_MASK = 0xFFFF
FORMAT_VERSION = 1
ALGORITHM_NAME = "L1_GTT_LowStubOccupancy"


def product(event, type_name, module):
    handle = Handle(type_name)
    event.getByLabel(module, handle)
    if not handle.isValid():
        raise RuntimeError(f"Missing {type_name} product from {module}")
    return handle.product()


def algorithm_block_from_modules(event, modules):
    for module in modules:
        handle = Handle("l1t::P2GTAlgoBlockMap")
        event.getByLabel(module, handle)
        if not handle.isValid():
            continue
        try:
            return handle.product().at(ALGORITHM_NAME), module
        except (IndexError, KeyError, RuntimeError):
            continue
    raise RuntimeError(
        f"Missing {ALGORITHM_NAME} in P2GT maps from {', '.join(modules)}"
    )


def metadata_word(stream):
    if len(stream) != FRAMES_PER_LINK:
        raise RuntimeError(f"Expected {FRAMES_PER_LINK} frames, got {len(stream)}")
    first_frame = 3 * (METADATA_TRACK_SLOT // 2)
    if METADATA_TRACK_SLOT % 2 == 0:
        upper = int(stream[first_frame + 1].to_ullong()) & 0xFFFFFFFF
        raw_word = int(stream[first_frame].to_ullong())
    else:
        upper = int(stream[first_frame + 2].to_ullong()) >> 32
        low = int(stream[first_frame + 1].to_ullong()) >> 32
        high = int(stream[first_frame + 2].to_ullong()) & 0xFFFFFFFF
        raw_word = low | (high << 32)
    if upper:
        raise RuntimeError("Reserved metadata carrier has nonzero bits [95:64]")
    return raw_word


def require_equal(label, actual, expected):
    if actual != expected:
        raise RuntimeError(f"{label} mismatch: {actual} != {expected}")


def main():
    parser = argparse.ArgumentParser(
        description="Check the persisted Apollo-to-Serenity stub-count chain."
    )
    parser.add_argument("input")
    parser.add_argument("--max-stub-count", type=int, default=500)
    args = parser.parse_args()

    summaries = []
    for event_number, event in enumerate(Events(args.input), start=1):
        counts = product(event, "l1t::GTTStubCounts", "apolloGTTStubCounts")
        streams = product(event, "tt::Streams", "apolloGTTStubPacker")
        primitive = product(
            event,
            "l1t::GTTStubPrimitive",
            "serenityGTTStubUnpacker",
        )
        algorithm, algo_blocks_module = algorithm_block_from_modules(
            event,
            ("l1tGTAlgoBlockProducer", "gttStubAlgoBlocks"),
        )

        region_counts = list(counts.acceptedByRegion())
        if len(region_counts) != len(REGION_LINKS):
            raise RuntimeError(f"Expected 9 regions, got {len(region_counts)}")
        if len(streams) != NUM_LINKS:
            raise RuntimeError(f"Expected {NUM_LINKS} links, got {len(streams)}")
        malformed_links = [
            link for link, stream in enumerate(streams) if len(stream) != FRAMES_PER_LINK
        ]
        if malformed_links:
            raise RuntimeError(
                f"Links with an invalid frame count: {malformed_links}"
            )

        transported_counts = []
        track_overflow_mask = 0
        count_overflow = False
        words = []
        for expected_region, link in enumerate(REGION_LINKS):
            raw_word = metadata_word(streams[link])
            word = ROOT.l1t.GTTStubCountWord(raw_word)
            if not word.structurallyValid() or word.region() != expected_region:
                raise RuntimeError(
                    f"Malformed word for region {expected_region}: 0x{raw_word:016x}"
                )
            transported_counts.append(int(word.count()))
            count_overflow |= bool(word.overflow())
            track_overflow_mask |= int(word.trackSlotOverflow()) << expected_region
            words.append(f"0x{raw_word:016x}")

        expected_transport = [min(count, COUNT_MASK) for count in region_counts]
        if transported_counts != expected_transport:
            raise RuntimeError(
                f"Regional transport mismatch: {transported_counts} != {expected_transport}"
            )

        regional_sum = sum(region_counts)
        transported_sum = sum(transported_counts)
        expected_count_overflow = any(count > COUNT_MASK for count in region_counts)
        expected_active_region_mask = sum(
            int(count != 0) << region
            for region, count in enumerate(transported_counts)
        )
        expected_max_region = max(
            range(len(transported_counts)),
            key=transported_counts.__getitem__,
        )
        expected_max_region_count = transported_counts[expected_max_region]

        require_equal("GTTStubCounts accepted total", int(counts.accepted()), regional_sum)
        require_equal(
            "Regional count-overflow flag",
            count_overflow,
            expected_count_overflow,
        )
        require_equal(
            "Serenity accepted-stub total",
            int(primitive.acceptedStubCount()),
            transported_sum,
        )
        require_equal(
            "Serenity maximum-region count",
            int(primitive.maxRegionCount()),
            expected_max_region_count,
        )
        require_equal(
            "Serenity maximum-region index",
            int(primitive.maxRegion()),
            expected_max_region,
        )
        require_equal(
            "Serenity active-region mask",
            int(primitive.activeRegionMask()),
            expected_active_region_mask,
        )
        require_equal(
            "Serenity track-overflow mask",
            int(primitive.trackOverflowMask()),
            track_overflow_mask,
        )
        require_equal(
            "Serenity track-overflow state",
            bool(primitive.trackOverflow()),
            track_overflow_mask != 0,
        )
        require_equal(
            "Serenity count-overflow state",
            bool(primitive.overflow()),
            expected_count_overflow,
        )
        require_equal("Serenity valid flag", bool(primitive.valid()), True)
        require_equal(
            "Serenity format version",
            int(primitive.formatVersion()),
            FORMAT_VERSION,
        )
        require_equal(
            "Serenity structural validity",
            bool(primitive.structurallyValid()),
            True,
        )

        expected_decision = (
            not expected_count_overflow
            and track_overflow_mask == 0
            and transported_sum <= args.max_stub_count
        )
        decisions = {
            "before BX mask and prescale": bool(
                algorithm.decisionBeforeBxMaskAndPrescale()
            ),
            "before prescale": bool(algorithm.decisionBeforePrescale()),
            "final": bool(algorithm.decisionFinal()),
        }
        for stage, decision in decisions.items():
            require_equal(f"P2GT {stage} decision", decision, expected_decision)

        summaries.append(
            {
                "event": event_number,
                "raw": int(counts.raw()),
                "accepted": int(counts.accepted()),
                "lost": int(counts.lost()),
                "regions": region_counts,
                "words": words,
                "serenity_sum": int(primitive.acceptedStubCount()),
                "max_region": int(primitive.maxRegion()),
                "max_region_count": int(primitive.maxRegionCount()),
                "track_overflow_mask": f"0x{int(primitive.trackOverflowMask()):03x}",
                "algo_blocks_module": algo_blocks_module,
                "decision": decisions["final"],
                "decisions": decisions,
            }
        )

    if not summaries:
        raise RuntimeError("Input contains no events")
    print(json.dumps(summaries, indent=2))


if __name__ == "__main__":
    main()
