# PbPb 2025 J/psi and psi(2S) MiniAOD setup

## Installation

Use a fresh `CMSSW_15_1_0_patch4` release and its standard architecture:

```bash
cmsrel CMSSW_15_1_0_patch4
cd CMSSW_15_1_0_patch4/src
cmsenv
git cms-init
git cms-merge-topic vince502:OniaPsiPbPb_151
scram b -j 4
```

This branch adds the validated muon-only HiOnia/HiOnia2MuMu packages and
filter-only HeavyIonsAnalysis/EventAnalysis package to the exact official
release tag. It does not modify PAT or generator/track matching algorithms.
Do not merge it indiscriminately into an existing customized OniaTree setup.

## Running

From a writable study output directory, with this release's runtime active:

```bash
cmsRun "$CMSSW_BASE/src/HiAnalysis/HiOnia/test/hioniaanalyzer_PbPb2025_Charmonia_cfg.py" \
  inputFiles=/store/PATH/TO/CERTIFIED_INPUT.root \
  maxEvents=2000 outputFile=charmonia_2025.root centralityVariation=Nominal
```

Replace the input placeholder with an appropriate 2025 data MiniAOD file.
VarParsing may append the event limit to the requested output filename.
The defaults use four threads and automatic streams. CRAB configuration and
submission are intentionally not included in this branch.

The configuration references the official certification JSON and year-specific
centrality DBs directly at their public CERN storage paths. These payloads are
not distributed here. Missing DBs fail explicitly; worker-node access to the
public AFS/EOS paths must be checked before production.

For a payload-deserialization check without input data events:

```bash
cmsRun "$CMSSW_BASE/src/HiAnalysis/HiOnia/test/test_charmonia_conditions_cfg.py" \
  year=2025 centralityVariation=Nominal
```

Repeat with `SysUp` and `SysDown` to check the other official tables.
Only the selected variation is stored in `Centrality`; nominal is the default.

## Selection and saved information

- Year-specific minimum-bias HLT, primary-vertex and official collision filters
  run before reconstruction on the sole tree-writing path.
- Muons require an inner track, tracker-muon status, TMOneStationTight,
  track-quality/impact-parameter cuts and the configured pT/eta acceptance.
- Opposite-sign dimuons: mass 2.4-4.2 GeV, pT < 50 GeV, |y| < 2.4,
  vertex probability > 0.01. No additional pair-pT >= 3 GeV cut is imposed.
- Single-muon information is retained, but a selected dimuon is required to
  save the event. This is not an inclusive event-counter tree.
- Event information, muons and dimuons share one tree entry. Use
  `(runNb, LS, eventNb)` to identify events; multistream entry order is not
  guaranteed. Use the stored daughter indices, not assumed list ordering.
- Reconstructed-muon indexing uses exact pT/eta/phi/charge keys, with explicit
  failures for ambiguous, missing or nonfinite keys and per-event eta reset.
- `SumET_HF`, `SumET_HFplus` and `SumET_HFminus` are ROOT doubles. Readers
  binding these branches must use doubles, not floats.
- Tracker/global, embedded soft/medium/tight ID decisions and ID-quality
  ingredients are saved. Hybrid Soft is a local implementation, not a claim
  of equivalence to every public Hybrid Soft definition. Rejected upstream
  muons, including pure standalone muons, cannot be recovered offline.
- Muonless PV production/use is disabled. Candidate-dependent PV association
  remains enabled. `Reco_QQ_vtx` stores the associated PV position, not the
  fitted dimuon decay vertex; full vertex covariance is not stored.
- `HLTriggers` stores 15 configured event-path decisions, not the full HLT
  menu. Bit i corresponds to hionia/hStats axis bin i+2. Missing and rejected
  paths both give zero; the virtual NoTrigger label is not a stored bit.
  The minimum-bias upstream OR is broader than the saved HF1AND path bit.
  PAT trigger-object matching is disabled.

## Validation scope

The isolated release was built and checked with all three official centrality
variations, including real EventSetup payload deserialization. Its nominal
2,000-source-event certified-input canary passed daughter-link, certification,
centrality, exact source-HF and selected event-HLT checks. The HF precision
change preserved the other 84 branches and seven histograms by event ID.

These are correctness canaries, not campaign-wide validation or a reliable
storage/runtime estimate. Campaign menu/GlobalTag applicability, prescales,
signal-rich psi(2S) retention, worker-node input/conditions access and a larger
one-thread versus four-thread regression remain production checks.
