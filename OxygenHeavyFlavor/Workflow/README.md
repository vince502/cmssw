# OxygenHeavyFlavor workflow

This package contains the CMSSW 15.0.11 OO/NeNe D- and B-meson production
entry points and the integrated Upsilon OniaTree. Run from the release `src`
directory after `cmsenv`.

Available runners are in `test/`: individual D, B -> D0 K, B -> J/psi, and
Upsilon Data/MC configurations; combined B configurations; and integrated
D/B/Upsilon configurations. All accept standard `inputFiles`, `outputFile`,
and `maxEvents` arguments plus:

- `collisionSystem=OO|NeNe`
- `lumiMask=/path/to/certification.json`
- `eventSelection=legacy|on|off`

The legacy selection policy retains the source behavior except that default
Onia/B MC monitors the primary-vertex and cluster-compatibility filters without
vetoing events. This keeps its generator denominator and every output tree
entry-aligned; use `eventSelection=on` to apply the veto explicitly. Data and
both B -> D0 K modes apply their configured filters, while D MC does not. Every runner writes
`hiEvtAnalyzer/HiTree` and `skimanalysis/HltTree` for event identity and path
decisions. NeNe MC is rejected until its campaign conditions are documented.

## Shared Onia and B contract

One standard `onia2MuMuPatGlbGlb` collection covers 2.4--14 GeV. The standard
HiOnia analyzer writes it to `hionia/myTree`, and the framework
`BDiLeptonProducer` reads that same collection for the 2.9--3.3 GeV J/psi
subset:

```text
onia2MuMuPatGlbGlb
  +-- hionia/myTree: Reco_QQ_collIdx
  +-- BDiLeptonProducer: jpsiIdx
        +-- bPlusToJpsiTree/candTree
        +-- bZeroToJpsiTree/candTree
        +-- bcToJpsiTree/candTree
```

For a B candidate, `jpsiIdx` is the original EDM collection index. Match it
to the Onia candidate satisfying
`jpsiIdx == Reco_QQ_collIdx` in the same run/lumi/event. No kinematic matcher
is used. The B-only runners also write the complete OniaTree. In default MC
production, the event, Onia, and B trees retain the same events in the same
order, so existing entry-based friends remain valid. The common event key is
also stored in every tree and should be used for explicit joins:

- `skimanalysis/HltTree`: `run`, `lumi`, `evt`
- `hiEvtAnalyzer/HiTree`: `run`, `lumi`, `evt`
- `hionia/myTree`: `runNb`, `LS`, `eventNb`
- B candidate trees: `RunNb`, `LSNb`, `EventNb`

Every event number is stored as an unsigned 64-bit value.

The OniaTree retains every branch in the legacy OO Upsilon tree, including
the muon selection mask, impact parameters and errors, track-fit quality,
segment compatibility, inner/global-track momenta, and detailed event track
counts. The MiniAOD track source is the standard
`unpackedTracksAndVertices` product. Generated J/psi, Y(1S), Y(2S), and Y(3S)
are stored together and identified by `Gen_QQ_pdgId`. Additive fields include
`runNb`, `LS`, `Reco_QQ_MassErr`, `Reco_QQ_collIdx`, and the generated-state
identity fields. OO additionally stores efficiency-corrected two-particle
Q-vectors; OO Data stores unique primary vertices. NeNe keeps the
detailed-track branches but does not apply the OO correction maps.

Each MC B -> J/psi candidate tree also owns an event-level generated-signal
collection. `genAllSize` counts every generated decay matching that tree's
signal definition, including events with no reconstructed B candidate. The
parallel `genAll{Role}{Pt,Eta,Phi,Mass,Y,PdgId,Status}` vectors use the roles
`B`, `Jpsi`, `Hadron1`, `Hadron2`, `MuPlus`, and `MuMinus`. `Hadron1` is the
bachelor kaon for B+, the kaon for B0, and the pion for Bc; `Hadron2` is the
B0 pion and is otherwise unused. `genAllRecoIdx` is the matched reconstructed
B-candidate index. `genAllJpsiIdx` is that candidate's reconstructed Onia
collection index, not an index into `hionia/Gen_QQ`; both are therefore `-1`
when the generated decay has no reconstructed B match. These branches provide
the generator denominator independently of `candSize`.

In `bPlusToJpsiTree/candTree`, daughter `D1` is the J/psi composite and `D2`
is the bachelor track under the kaon-mass hypothesis. The `pTD2`, `etaD2`,
`phiD2`, `chargeD2`, `nhitD2`, `highPurityD2`, `trkChi2D2`, `ptErrD2`,
`dxySigD2`, and `dzSigD2` fields are therefore the bachelor-kaon observables.
In `bZeroToJpsiTree/candTree`, the nested EDM decay is flattened as
`D1=J/psi`, `D2=K`, and `D3=pi`; every daughter vector has exactly `candSize`
elements.

The B -> J/psi configuration sets `useAnyMVA=False`. Consequently, the
generic B-tree `mva` field is the producer's `-999` missing-value sentinel,
not a trained BDT score or an analysis working point.

`hiEvtAnalyzer/HiTree` reads the retained `hiCentrality` and
`centralityBin:HFtowers` products, so its centrality, multiplicity, and HF
activity values are populated. The exact `hiHFfilters` tower-coincidence
product cannot be regenerated from MiniAOD because `towerMaker` is absent;
`doHFfilters` therefore remains disabled instead of substituting a different
filter definition.

Example:

```bash
cmsRun OxygenHeavyFlavor/Workflow/test/run_upsilon_mc_cfg.py \
  inputFiles=/store/.../Y2S_MINIAOD.root maxEvents=100
```

Use `crab/crabConfig_template.py` as the single CRAB template. Set the runner,
dataset, DBS instance, output file, storage site, and output LFN in its edit
block before a dry run or submission.

`VarParsing("analysis")` appends `_numEventN` to a local output name when
`maxEvents=N` is supplied on the command line. CRAB does not pass that local
test option, so `config.JobType.outputFiles` must keep the unsuffixed
`outputFile` value from the template.

The package builds in CMSSW 15.0.11 and every runner passes `edmConfigDump`.
The unified path processed ten 2025 OO PromptReco Data events, a complete
1,798-event Y(1S) MC file, a complete 875-event B+ -> J/psi K signal file, and
a one-event all-channel MC job. The merged MC validation file has 2,673 aligned
entries in every event, Onia, and B tree. In the signal check, event 5010006005
had one B candidate with `jpsiIdx=0`, `genAllRecoIdx=0`, and
`genAllJpsiIdx=0`. A branch-set comparison
against the 122-branch legacy OO Upsilon MC tree found no missing branch; the
unified MC OniaTree has 133 branches. The OO MC settings follow `MC.md`:
`Run3_2025_OXY` with `150X_mcRun3_2025_forOO_realistic_v9`. The tests use
CMSSW 15.0.11-compatible OO MiniAOD, including the official
`HINOOSpring25MiniAOD` v9 campaign and the private Y(1S) and B+ signal samples.

Do not test this area with a MINIAODSIM written by a newer CMSSW release. In
particular, the available private B -> D0 K signal file produced by CMSSW
15.1.0_patch3 is forward-incompatible with CMSSW 15.0.11 and is correctly
rejected before event processing.

The 2024 ppRef Upsilon analysis is not folded into this package; it remains
under `OniaJetRun3/ppRef2024` with its CMSSW 14.1 conditions and workflow.
