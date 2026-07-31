# L1TTrackStub: GTT regional stub-count emulation

## Scope

This package is a CMSSW proof of concept for transporting Outer Tracker
stub occupancy from the Track Finder boundary to a Phase-2 Global Trigger
decision. It provides a packed link contract, a receiver-side trigger
primitive, and a standard `P2GTAlgoBlockMap` decision.

The package does not contain a signal generator, physics-selection proposal,
HLS library, approved Apollo-to-Serenity protocol, or deployed FPGA design.
The default occupancy threshold is for software integration tests only.

`GTTStubCountWord` and the logical-link assignment isolate the provisional
format. They must be replaced if the Track Finder and GTT firmware teams adopt
a different carrier.

## Dataflow

```text
Phase-2 tracker digis
  -> TTCluster/TTStub builder
  -> ProducerDTC:{StubAccepted,StubLost}
  -> Track Finder: l1tTTTracksFromTrackletEmulation
  -> Apollo-side regional counter
       l1t::GTTStubCounts
  -> Apollo packer
       standard 18-link GTT track packet
       + nine tagged count words in reserved track slot 103
  -> Serenity unpacker
       l1t::GTTStubPrimitive
       + decoded Level1TTTracks
  -> standard GTT track selection and vertex emulation
  -> gttStubLowOccupancyPath
  -> standard L1GTAlgoBlockProducer
       P2GTAlgoBlockMap["L1_GTT_LowStubOccupancy"]
```

The receiver consumes only packed `tt::Streams`; it cannot access the original
`TTStub` or `TTDTC` collections. The test therefore crosses a serialization
boundary instead of recomputing the occupancy at the GTT endpoint. The path
does not use L1 Scouting.

The receiver also decodes every valid standard track word. The full-chain
customizer redirects `l1tGTTInputProducer` to those decoded tracks, allowing
the standard GTT track-selection and vertex-emulation modules to run from the
same serialized packet.

## Occupancy contract

The trigger quantity is the number of nonnull frames in
`ProducerDTC:StubAccepted`, summed over the nine Track Finder processing
regions. Region overlap can route one physical stub more than once, so this
quantity is not a unique-stub count.

The Apollo-side diagnostic object contains:

| Field | Meaning |
|---|---|
| `raw` | Physical accepted `TTStub` objects before DTC routing |
| `accepted` | Valid DTC frames delivered to all processing regions |
| `lost` | Frames removed by DTC bandwidth truncation |
| `acceptedByRegion` | Nine regional counts transported to Serenity |
| `lostByRegion` | Regional truncation diagnostic |
| `acceptedByLink`, `lostByLink` | Counts for every DTC channel |

Only the nine accepted regional counts and metadata-slot overflow flags cross
the emulated board boundary. `GTTStubPrimitive` exposes their sum, maximum,
active-region mask, track-overflow mask, count-overflow state, format version,
and structural-validity state.

## Provisional link format

Each processing region uses one 64-bit `GTTStubCountWord`:

| Bits | Field |
|---:|---|
| `[15:0]` | Accepted-frame count, saturated at 65,535 |
| `[19:16]` | Region index, 0 through 8 |
| `[20]` | Count overflow |
| `[21]` | Valid |
| `[25:22]` | Format version, currently 1 |
| `[26]` | Reserved metadata slot displaced a valid track |
| `[63:27]` | Reserved; required to be zero |

The standard GTT track codec produces 18 links, 104 96-bit track slots per
link, and 156 64-bit frames per link. This demonstrator writes the regional
words into bits `[63:0]` of slot 103 on links
`0, 2, 4, 6, 8, 10, 12, 14, 16`. Bits `[95:64]` are zero, so a standard track
decoder reads the carrier as invalid padding.

By default, the packer throws if slot 103 contains a valid track, preserving
the standard GTT track collection. Diagnostic studies can set
`throwOnTrackSlotOccupied=False`; in that mode the packer replaces the track
and sets bit 26. The unpacker collects these bits in
`GTTStubPrimitive::trackOverflowMask()`, and the decision filter rejects the
event by default.

The implemented carrier adds no line rate. It reserves:

- `1 / 104 = 0.96%` of the track-slot capacity on each affected link;
- `9 / (18 * 104) = 0.48%` of aggregate track-slot capacity.

A separate 64-bit word for each of nine regions would instead add 576 payload
bits per event, or 23.04 Gbit/s at 40 MHz before protocol framing and line
coding. The EMP gap frames are framing and are not treated as free payload.

## Configuration

Full defaults live in the package's `cfi` files:

| File | Responsibility |
|---|---|
| `gttStubCounts_cfi.py` | Count raw, accepted, and lost stubs |
| `gttStubPacker_cfi.py` | Assign regions to links and insert count words |
| `gttStubUnpacker_cfi.py` | Validate the packet and build the GTT primitive |
| `gttStubDecisionFilter_cfi.py` | Apply an inclusive occupancy window |
| `gttStubValidation_cfi.py` | Check packet, primitive, tracks, and decision |

`gttStubEmulation_cff.py` assembles the board-side sequences, low-occupancy
path, algorithm-block producer, accept filter, validation end path, and focused
event content.

`customiseGTTStubEmulation.py` appends the demonstrator to a normal Phase-2
`cmsDriver.py` process. When the production `L1P2GT` path is present, it adds
`L1_GTT_LowStubOccupancy` to the production algorithm map. Otherwise it runs a
focused clone of the same standard producer.

The neutral decision-filter `cfi` accepts the full representable count range.
The assembled demonstrator sets `maxStubCount=500`; this value is not a
physics-optimized threshold.

## Build and tests

From a configured CMSSW source area:

```bash
scram b -j 8
scram b code-checks
scram b \
  runtests_src_DataFormats_L1TrackTrigger_test \
  runtests_src_L1Trigger_L1TTrackStub_test
```

The registered tests cover:

- data-format packing, saturation, and structural validity;
- insertion and extraction in even and odd 96-bit track slots;
- preservation of the noncarrier half of a shared 64-bit frame;
- nominal decisions and rejection on count saturation, malformed packets, or
  displaced tracks;
- a five-event, two-stream `cmsRun` integration.

## Full-chain heavy-ion integration

The runner accepts a Phase-2 GEN-SIM input:

```bash
L1Trigger/L1TTrackStub/test/run_full_chain.sh \
  /absolute/path/to/step1.root \
  1 \
  L1Trigger/L1TTrackStub/test/results/full_chain
```

It generates and runs:

```text
DIGI:pdigi_hi,L1TrackTrigger,L1,L1P2GT,DIGI2RAW
```

using D121 geometry, `Phase2C22I13M9`, `phase2_pp_on_AA`, and `HiMixNoPU`.
The script writes its configuration, logs, EDM output, ROOT validation file,
and packet-check JSON below the selected results directory.

For an existing file containing retained Track Trigger products, the focused
replay rebuilds `ProducerDTC` and exercises the same packed boundary:

```bash
cmsRun L1Trigger/L1TTrackStub/test/runGTTStubCounting_cfg.py \
  inputFiles=file:step2.root \
  outputFile=gttStubEmulation.root \
  histogramFile=gttStubValidation.root \
  maxEvents=1 \
  maxStubCount=500
```

Persisted products can be checked independently:

```bash
L1Trigger/L1TTrackStub/scripts/check_gtt_stub_output.py \
  L1Trigger/L1TTrackStub/test/results/full_chain/gttStubFullChain.root
```

## Boundaries and failure modes

- `GTTStubCountWord` is a provisional software-emulation contract, not an
  approved firmware data format.
- The count is regional DTC-frame occupancy, not a unique physical-stub count.
- Count saturation and malformed packets reject the decision. An occupied
  metadata slot throws by default; permissive displacement mode marks the
  overflow and rejects the decision.
- Decoded track words do not recover the original `TTStubRef` objects.
- `DIGI2RAW` does not serialize the custom word or algorithm bit into a
  production FED payload; the demonstrator products remain in EDM.
- The occupancy reaches P2GT through a named path decision because the
  production GTT-to-GT object schema has no stub-occupancy object.
- A firmware implementation still requires a carrier agreement, link and
  time-multiplexing assignment, HLS or RTL implementation, latency and resource
  measurements, and bit-for-bit hardware comparison.

## Source map

- `DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h`: regional word
  layout.
- `DataFormats/L1TrackTrigger/interface/GTTStubCounts.h`: Apollo-side
  diagnostics.
- `DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h`: Serenity-side
  primitive.
- `L1Trigger/L1TTrackStub/plugins/GTTStubCountProducer.cc`: DTC-frame
  counting.
- `L1Trigger/L1TTrackStub/plugins/GTTStubPacker.cc`: standard track-packet
  generation and metadata insertion.
- `L1Trigger/L1TTrackStub/plugins/GTTStubUnpacker.cc`: metadata and track
  decoding.
- `L1Trigger/L1TTrackStub/plugins/GTTStubValidation.cc`: cross-boundary
  validation.
