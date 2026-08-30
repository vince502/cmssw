# Source provenance

The merged workspace was created as a separate CMSSW 15.0.11 development
area. The source workspaces were not modified.

## Onia and D/B -> D0 K base

- Source: `OniaTree2025/JpsiToEETest/CMSSW_15_1_0_patch3/src`
- Branch: `from-CMSSW_15_1_0_patch3`
- Commit: `1b17e9d2bb93a397c1279a3ddf4ce7dbd8d6b8ee`
- Imported from the current dirty working tree because the B -> D0 K modules,
  current MVA files, and several 2025 configurations are untracked.
- Imported package roots: `HeavyIonsAnalysis`, `HiAnalysis`, `HiSkim`, and
  `VertexCompositeAnalysis`.
- Excluded generated content: ROOT files, logs, plots, CRAB work areas, job
  directories, caches, backups, and build artifacts.

## B -> J/psi graft

- Source: `BmesonAna/CMSSW_15_0_10/src/VertexCompositeAnalysis`
- Branch: `CMSSW_15_0_X`
- Commit: `b71f83df1523598fc70507a8f1390ac2f2bde438`
- Imported only `BDiMuMuProducer`, `BDiMuMuFitter`, `BDiMuMuNtuplizer`, and
  their Python `cfi`/`cff` fragments.
- Deleted legacy Bc and flat-tree implementation files were not restored.

These grafted BDiMuMu modules remain in the source area for provenance but
are no longer scheduled by `OxygenHeavyFlavor/Workflow`. The unified workflow
uses the Onia-base `BDiLeptonProducer` and `PATCompositeNtupleProducer`, which
already carry the original dimuon collection index as `jpsiIdx`.

The Onia versions of shared D0/D* producers, fitters, and
`PATCompositeNtupleProducer` remain authoritative. Producer and analyzer
`BuildFile.xml` dependencies were merged manually for the BDiMu modules.

## OO Upsilon OniaTree graft

- Source: `UpsilonOO/CMSSW_15_0_9_patch1/src/HiAnalysis/HiOnia`
- Branch: `from-CMSSW_15_0_9_patch1`
- Commit: `7a176af798796afb3786e6074d31c71d6fa9f231`
- Imported from the current dirty working tree because the OO detailed-track,
  two-particle Q-vector, PF-isolation, secondary-track, and unique-PV changes
  are local additions.
- Imported correction maps:
  `Eff_OO_2025_Hijing_MB_Centrality_fromHihfpf_NoPU_3D_Nominal_Official.root`
  and
  `Eff_OO_2025_PythiaHijing_QCD_pThat15_Centrality_fromHihfpf_NoPU_3D_Nominal_Official.root`.

The Upsilon features were merged additively into the newer HiOnia source.
The existing MET branches, B-candidate `Reco_QQ_collIdx` association, and
other current Onia changes remain in place. The OO workflow enables the
correction-dependent branches; NeNe uses only the detailed-track additions,
matching the legacy NeNe configuration. The 2024 ppRef Upsilon workflow was
not imported because it belongs to a separate CMSSW 14.1 release and
conditions setup.

## Unified Onia/B index contract

The framework `onia2MuMuPatGlbGlb` collection is now the only reconstructed
dimuon collection in the Onia/B path. `HiOniaAnalyzer` stores the original
collection index in `Reco_QQ_collIdx`; `BDiLeptonProducer` stores that same
index in each B candidate's `jpsiIdx`; `PATCompositeNtupleProducer` writes it
to the B trees. No additional index-matcher module was introduced.

Two small source extensions support the inclusive contract:

- `HiOniaAnalyzer` accepts an optional list of generated resonance PDG IDs
  and writes `Gen_QQ_pdgId`; an empty list retains the historical single
  `oniaPDG` behavior.
- `BDiLeptonProducer` can require an opposite-sign dilepton, preventing the
  Onia same-sign control pairs from entering B reconstruction.

`HiOniaAnalyzer` now preserves the complete legacy OO Upsilon muon schema,
including the selection mask, impact parameters and errors, track-fit quality,
segment compatibility, and inner/global-track momentum fields. It also writes
`runNb`, `LS`, and `eventNb` for MC and Data; `eventNb` is unsigned 64-bit.
The workflow explicitly routes `srcTracks` to the MiniAOD
`unpackedTracksAndVertices` product so `Ntracks`, the detailed track-count
matrix, and the two-particle quantities are populated.

`PATCompositeNtupleProducer` stores `EventNb` as unsigned 64-bit and adds an
event-level `genAll*` signal-decay collection to the B+, B0, and Bc trees.
This collection is filled independently of reconstructed `candSize` and uses
the existing strict daughter matcher to record `genAllRecoIdx` and the shared
Onia `genAllJpsiIdx`.

## Common event information

Focused changes were made in the imported
`HeavyIonsAnalysis/EventAnalysis` package:

- `FilterAnalyzer` now stores `run`, `lumi`, and `evt` in every `HltTree`.
- `HiEvtAnalyzer` resets its always-booked centrality and HF fields to `-1`
  for each event when centrality is disabled, preventing stale values.
- The unified OO configuration enables the retained `hiCentrality` and
  `centralityBin:HFtowers` MiniAOD products, so centrality, multiplicity, and
  HF activity fields are filled rather than left at sentinels.

The MiniAOD does not contain `reco::HFFilterInfo` or the `towerMaker` input
needed to rerun `HiHFFilterProducer`. The workflow consequently leaves
`doHFfilters` disabled. Exact tower-coincidence decisions must come from an
upstream product or a source format that retains calorimeter towers; they are
not inferred from PF candidates.

No source file in any original workspace was modified.
