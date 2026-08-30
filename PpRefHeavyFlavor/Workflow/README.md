# Unified ppRef Onia + B ntuple

This package is the CMSSW 14.1.9 ppRef counterpart of the unified OO
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

- Era: `Run3_2024_ppRef`
- Data GlobalTag: `141X_dataRun3_Prompt_v3`
- MC GlobalTag: `141X_mcRun3_2024_realistic_ppRef5TeV_v7`
- Inclusive dimuon mass: 2.4--14 GeV
- B dimuon subset: opposite-sign, 2.9--3.3 GeV
- Bachelor-track threshold: pT > 0.3 GeV
- Detailed Ntrk thresholds: pT > 0.3, 0.4, 0.5, and 1.0 GeV for several
  |eta| ranges through 2.4
- Per-Onia q2 reference tracks: 0.4 < pT <= 3 GeV and |eta| <= 2.4, with the
  two candidate muons removed

The q-vector is intentionally uncorrected: no OO efficiency/fake-rate map is
applied to ppRef. Supply a validated ppRef tracking correction payload before
using it for a corrected v2 measurement.

Centrality is intentionally disabled. ppRef HF activity can be stored and
studied, but it must not be converted to a heavy-ion centrality percentile
without a separately validated ppRef definition and calibration.

## Local execution

From the CMSSW project root:

```bash
eval "$(scramv1 runtime -sh)"
cmsRun src/PpRefHeavyFlavor/Workflow/test/run_unified_mc_cfg.py \
  inputFiles=/store/mc/.../MINIAODSIM/...root \
  outputFile=UnifiedOniaB_ppRef_MC.root \
  maxEvents=100
```

For data, use `run_unified_data_cfg.py`. A local JSON may be passed as
`lumiMask=/path/to/certification.json`; in CRAB, `Data.lumiMask` is the
authoritative setting.

Run B and Upsilon MC as separate production tasks. They use the same schema
and producer configuration, while preserving independent dataset provenance
and normalization.

## Validation inputs

The local smoke tests used official RunIII ppRef MiniAOD samples:

- `Upsilon1SToMuMu_pThat-2_TuneCP5_5.36TeV_pythia8`
- `BToNonPromptJPsiToMuMu_inclusive_TuneCP5_5.36TeV_pythia8-evtgen`

The 200-event B test contains generated exclusive B+ decays, populated B+
candidates, a truth-matched B+, complete B0 daughter vectors, and exact Onia/B
index matches. The Upsilon test contains a truth-matched reconstructed
Upsilon(1S). All six trees retain identical event counts and identities.
