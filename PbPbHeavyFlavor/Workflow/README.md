# Unified PbPb Onia + B ntuple

This package is the CMSSW 17.0.0_pre4 profile of the unified PbPb Onia+B
workflow. One inclusive dimuon collection covers the J/psi and Upsilon mass
regions. The B producer consumes that same collection and stores `jpsiIdx` as
the original EDM collection index; match it to `hionia/myTree` through
`Reco_QQ_collIdx`.

The output contains aligned event entries in:

- `hionia/myTree`
- `bPlusToJpsiTree/candTree`
- `bZeroToJpsiTree/candTree`
- `bcToJpsiTree/candTree`
- `hiEvtAnalyzer/HiTree`
- `skimanalysis/HltTree`

The filter paths are recorded but do not veto the unified path, so entry-based
friendship is valid. Run, lumi, and event identifiers are also available for
explicit checks, with the event number stored as 64 bits in every tree.

## Configuration contract

- Era: `Run3_pp_on_PbPb_2026`
- Data GlobalTag: `auto:run3_data_HIon`
- MC GlobalTag: `auto:run3_mc_HIon`
- Default data centrality: `CentralityTable_HFtowers200_DataPbPb_offline_v0`
- Default MC centrality: `CentralityTable_HFtowers200_HydjetCello_v1401x0_official_MC2024`
- Inclusive dimuon mass: 2.4--14 GeV
- B dimuon subset: opposite-sign, 2.9--3.3 GeV
- Bachelor-track threshold: pT > 0.3 GeV
- Detailed Ntrk thresholds: pT > 0.3, 0.4, 0.5, and 1.0 GeV for several
  |eta| ranges through 2.4
- Per-Onia q2 reference tracks: 0.4 < pT <= 3 GeV and |eta| <= 2.4, with the
  two candidate muons removed
- Trigger map: the standard quarkonium paths plus the 2026 HI L1/L2 dimuon,
  single-muon, charge, and peripheral-centrality variants from `HLT_HIon`

For the B trees, `pTD2`, `etaD2`, and the other D2 fields describe the
bachelor K (B+) or pion (Bc); B0 additionally stores the K/pi pair in D2/D3.
The `mva` branch is intentionally `-999` because `useAnyMVA=False`. The
current common producer also leaves `decayLengthSig2D/3D` and `trackDCA*` at
their documented placeholder values; use the populated decay lengths,
uncertainties, vertex quantities, and daughter impact-parameter fields.

The q-vector is intentionally uncorrected: no OO efficiency/fake-rate map is
applied to PbPb. Supply a validated PbPb tracking correction payload before
using it for a corrected v2 measurement.

The Onia and event trees store `hiHF`, the official 200-bin `hiBin`, and the
HF-filter information. Centrality and the q-vector are event-level/reference
quantities; B candidates use the exact `jpsiIdx` to reach their Onia candidate
and its muon-excluded q-vector.

As of 31 August 2026, the production conditions database does not contain a
dedicated 2026 PbPb HFtowers calibration. The default data tag resolves to the
validated 2024 nominal payload for runs at and above 387853, and the current
2026 HI MC GlobalTag likewise carries the 2024 HydjetCello table. These
defaults make the workflow executable and preserve the complete branch
contract, but their `hiBin` values are provisional for 2026 physics. When the
2026 calibration is published, pass
`centralityTag=CentralityTable_HFtowers200_...` without changing the code or
tree schema. Raw HF sums remain stored independently.

## Local execution

From the CMSSW project root:

```bash
eval "$(scramv1 runtime -sh)"
cmsRun src/PbPbHeavyFlavor/Workflow/test/run_unified_mc_cfg.py \
  inputFiles=/store/mc/.../MINIAODSIM/...root \
  outputFile=UnifiedOniaB_PbPb_MC.root \
  maxEvents=100
```

For data, use `run_unified_data_cfg.py`. A local JSON may be passed as
`lumiMask=/path/to/certification.json`; in CRAB, `Data.lumiMask` is the
authoritative setting.

Run B and Upsilon MC as separate production tasks. They use the same schema
and producer configuration, while preserving independent dataset provenance
and normalization.

## Validation inputs

The original producer validation used separate official RunIII PbPb MiniAOD
samples:

- `Upsilon1SToMuMu_pThat-2_TuneCP5_5.36TeV_pythia8`
- `NonPromptJPsiToMuMu_Pthat2_TuneCP5_HydjetDrumMB_5p36TeV_pythia8-evtgen`

In 25 Upsilon events the output contains 25 generated Upsilon(1S) and 15
truth-matched reconstructed Upsilon candidates. In 100 nonprompt-J/psi events
it contains 11 generated exclusive B+/- -> J/psi K+/- decays and three
truth-matched reconstructed B+/- candidates. All 1,427 reconstructed B-family
candidates have valid Onia collection-index matches, all B0 D3 vector lengths
match `candSize`, and all six trees remain event-aligned. A one-event PbPb
data test also completed even though that event failed the monitored PV
filter; it was retained in every output tree as intended.

The CMSSW 17.0.0_pre4 port was exercised on 25 Upsilon events, 100
nonprompt-J/psi events, and one real 2026 PbPb PromptReco event (run 404926).
It reproduces 25 generated and 15 truth-matched reconstructed Upsilon(1S), 11
generated exclusive B+/- -> J/psi K+/- decays and three truth-matched B+/-
candidates. All 1,427 reconstructed B-family candidates have valid Onia
collection-index matches, all q-vector lengths and B0 D3 vector lengths are
consistent, and all six trees are event-aligned.
