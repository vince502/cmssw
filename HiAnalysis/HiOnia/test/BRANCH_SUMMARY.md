# Branch Summary for HiForest Onia/Composite Trees

## 1. hionia/myTree (J/psi → μμ)

### Event Info
| Branch | Type | Description |
|--------|------|-------------|
| eventNb, runNb, LS | UInt | Event identifiers |
| zVtx | Float | Primary vertex z |
| nPV | Short | Number of primary vertices |
| Ntracks | Short | Number of tracks |
| HLTriggers | ULong64 | Trigger bits |
| trigPrescale[11] | Int[] | Trigger prescales |

### Dimuon (Reco_QQ)
| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| Reco_QQ_size | Short | Number of dimuons | ✅ |
| Reco_QQ_4mom_pt/eta/phi/m | vector | 4-momentum | ✅ |
| Reco_QQ_type | Short[] | Dimuon type (GlbGlb, etc) | ✅ |
| Reco_QQ_sign | Short[] | Charge sign | ✅ |
| Reco_QQ_mupl_idx, mumi_idx | Short[] | Daughter muon indices | ✅ |
| Reco_QQ_trig | ULong64[] | Per-candidate trigger | ✅ |
| **Vertex & Lifetime** |||
| Reco_QQ_VtxProb | Float[] | Vertex probability | ✅ |
| Reco_QQ_vtx | TClonesArray | Vertex position (TVector3) | ✅ |
| Reco_QQ_ctau | Float[] | 2D ctau (mm) | ✅ |
| Reco_QQ_ctauErr | Float[] | 2D ctau error | ✅ |
| Reco_QQ_ctau3D | Float[] | 3D ctau | ✅ |
| Reco_QQ_ctauErr3D | Float[] | 3D ctau error | ✅ |
| Reco_QQ_cosAlpha | Float[] | 2D pointing angle | ✅ |
| Reco_QQ_cosAlpha3D | Float[] | 3D pointing angle | ✅ |
| Reco_QQ_dca | Float[] | DCA between tracks | ✅ |

### Single Muon (Reco_mu)
| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| Reco_mu_size | Short | Number of muons | ✅ |
| Reco_mu_4mom_pt/eta/phi/m | vector | 4-momentum | ✅ |
| Reco_mu_L1_4mom_* | vector | L1 matched 4-mom | ✅ |
| Reco_mu_type | Short[] | Muon type | ✅ |
| Reco_mu_charge | Short[] | Charge | ✅ |
| Reco_mu_trig | ULong64[] | Trigger match | ✅ |
| **Quality Flags** |||
| Reco_mu_highPurity | Bool[] | High purity track | ✅ |
| Reco_mu_TMOneStaTight | Bool[] | TM one station tight | ✅ |
| Reco_mu_isPF | Bool[] | PF muon | ✅ |
| Reco_mu_isTracker | Bool[] | Tracker muon | ✅ |
| Reco_mu_isGlobal | Bool[] | Global muon | ✅ |
| Reco_mu_isSoftCutBased | Bool[] | Soft cut-based ID | ✅ |
| Reco_mu_isHybridSoft | Bool[] | Hybrid soft ID | ✅ |
| Reco_mu_isMediumCutBased | Bool[] | Medium cut-based ID | ✅ |
| Reco_mu_isTightCutBased | Bool[] | Tight cut-based ID | ✅ |
| Reco_mu_softMvaRun3Value | Float[] | Soft MVA value | ✅ |
| **Track Hits** |||
| Reco_mu_nPixValHits | Int[] | Pixel valid hits | ✅ |
| Reco_mu_nMuValHits | Int[] | Muon valid hits | ✅ |
| Reco_mu_nTrkHits | Int[] | Tracker hits | ✅ |
| Reco_mu_nPixWMea | Int[] | Pixel layers w/ measurement | ✅ |
| Reco_mu_nTrkWMea | Int[] | Tracker layers w/ measurement | ✅ |
| Reco_mu_normChi2_inner | Float[] | Normalized chi2 | ✅ |
| **Acceptance** |||
| Reco_mu_InTightAcc | Bool[] | In tight acceptance | ✅ |
| Reco_mu_InLooseAcc | Bool[] | In loose acceptance | ✅ |

---

## 2. hioniaElectrons/eleTree (J/psi → ee)

### Dielectron (Reco_ee)
| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| Reco_ee_size | Int | Number of dielectrons | ✅ |
| Reco_ee_pt/eta/phi/y/mass | vector | Kinematics | ✅ |
| Reco_ee_charge | Short[] | Total charge | ✅ |
| Reco_ee_ele1Idx, ele2Idx | Short[] | Daughter indices | ✅ |
| Reco_ee_trigBits | ULong64[] | Trigger bits | ✅ |
| **Vertex & Lifetime** |||
| Reco_ee_vProb | Float[] | Vertex probability | ✅ |
| Reco_ee_chi2 | Float[] | Vertex chi2 | ✅ |
| Reco_ee_ndf | Float[] | Vertex NDF | ✅ |
| Reco_ee_ctau | Float[] | 2D ctau (mm) | ✅ (NEW) |
| Reco_ee_ctauErr | Float[] | 2D ctau error | ✅ (NEW) |
| Reco_ee_ctau3D | Float[] | 3D ctau | ✅ (NEW) |
| Reco_ee_ctauErr3D | Float[] | 3D ctau error | ✅ (NEW) |
| Reco_ee_cosAlpha | Float[] | 2D pointing angle | ✅ (NEW) |
| Reco_ee_cosAlpha3D | Float[] | 3D pointing angle | ✅ (NEW) |
| Reco_ee_dca | Float[] | DCA between tracks | ✅ (NEW) |
| **Missing** |||
| Reco_ee_vtx | - | Vertex position (x,y,z) | ❌ |

### Single Electron (Reco_ele)
| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| Reco_ele_size | Int | Number of electrons | ✅ |
| Reco_ele_pt/eta/phi/y/mass | vector | Kinematics | ✅ |
| Reco_ele_charge | Short[] | Charge | ✅ |
| **Track Quality** |||
| Reco_ele_trackPt/Eta/Phi | Float[] | GSF track kinematics | ✅ |
| Reco_ele_trackChi2 | Float[] | Track chi2 | ✅ |
| Reco_ele_trackNdof | Float[] | Track NDF | ✅ |
| Reco_ele_trackNormalizedChi2 | Float[] | Normalized chi2 | ✅ |
| Reco_ele_nValidHits | Short[] | Valid hits | ✅ |
| Reco_ele_nMissingHits | Short[] | Missing hits | ✅ |
| Reco_ele_trackerLayers | Short[] | Tracker layers | ✅ |
| Reco_ele_pixelLayers | Short[] | Pixel layers | ✅ |
| Reco_ele_dxy/dxyErr | Float[] | Transverse IP | ✅ |
| Reco_ele_dz/dzErr | Float[] | Longitudinal IP | ✅ |
| **ECAL/ID Variables** |||
| Reco_ele_isEB | Short[] | Barrel flag | ✅ |
| Reco_ele_energy | Float[] | Energy | ✅ |
| Reco_ele_corrEnergy/Err | Float[] | Corrected energy | ✅ |
| Reco_ele_EcalEnergy | Float[] | ECAL energy | ✅ |
| Reco_ele_scEta/Phi/En/RawEn | Float[] | SuperCluster info | ✅ |
| Reco_ele_r9 | Float[] | R9 | ✅ |
| Reco_ele_sigmaIEtaIEta | Float[] | σiηiη | ✅ |
| Reco_ele_sigmaIPhiIPhi | Float[] | σiφiφ | ✅ |
| Reco_ele_hOverE | Float[] | H/E | ✅ |
| Reco_ele_fbrem | Float[] | Bremsstrahlung fraction | ✅ |
| Reco_ele_eOverP | Float[] | E/p | ✅ |
| Reco_ele_eOverPInv | Float[] | 1/E - 1/p | ✅ |
| Reco_ele_deltaEtaIn | Float[] | ΔηSC-track | ✅ |
| Reco_ele_deltaPhiIn | Float[] | ΔφSC-track | ✅ |
| **Isolation** |||
| Reco_ele_pfChIso | Float[] | PF charged isolation | ✅ |
| Reco_ele_pfNeuIso | Float[] | PF neutral isolation | ✅ |
| Reco_ele_pfPhoIso | Float[] | PF photon isolation | ✅ |
| Reco_ele_pfPUIso | Float[] | PF PU isolation | ✅ |
| **MVA ID** |||
| Reco_ele_MVAIso | Float[] | MVA isolation score | ✅ |
| Reco_ele_MVAId | Float[] | MVA ID score | ✅ |
| Reco_ele_MVAIsoWP95/90/85/80 | Short[] | MVA iso WPs | ✅ |
| Reco_ele_MVAIdWP95/90/85/80 | Short[] | MVA ID WPs | ✅ |
| Reco_ele_CutIdWP95/90/80/70 | Short[] | Cut-based ID WPs | ✅ |
| Reco_ele_convVeto | Short[] | Conversion veto | ✅ |

---

## 3. d0Ntuplizer/candTree (D0 → Kπ)

| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| **Event Info** |||
| RunNb, LSNb, EventNb | UInt | Event identifiers | ✅ |
| nPV | Short | N primary vertices | ✅ |
| bestvtxX/Y/Z | Float | Best PV position | ✅ |
| centrality | Short | Centrality bin | ✅ |
| Ntrkoffline | Int | Offline track count | ✅ |
| candSize | UInt | Number of candidates | ✅ |
| **Candidate Kinematics** |||
| pT, eta, phi, mass, y | vector | 4-momentum | ✅ |
| pdgId | vector | PDG ID | ✅ |
| mva | vector | MVA score | ✅ |
| **Vertex Quality** |||
| VtxChi2, VtxNdof, VtxProb | vector | Vertex fit quality | ✅ |
| **Geometry** |||
| alpha2D, alpha3D | vector | Pointing angle | ✅ |
| decayLength2D/3D | vector | Decay length | ✅ |
| decayLengthSig2D/3D | vector | Decay length significance | ✅ |
| dca3D, dca3DErr | vector | 3D DCA to PV | ✅ |
| trackDCA, trackDCAErr | vector | DCA between daughters | ✅ |
| **Daughter Info (D=daughter)** |||
| pTD, etaD, phiD, massD | vector<vector> | Daughter kinematics | ✅ |
| chargeD | vector<vector> | Daughter charges | ✅ |
| dedxD | vector<vector> | dE/dx | ✅ |
| dzSigD, dxySigD | vector<vector> | Impact param significance | ✅ |
| nhitD | vector<vector> | N hits | ✅ |
| ptErrD | vector<vector> | pT error | ✅ |
| trkChi2D | vector<vector> | Track chi2/ndof | ✅ |
| highPurityD | vector<vector> | High purity flag | ✅ |
| **PV Matching** |||
| bestPVIdx | vector | Nearest PV index | ✅ (NEW) |
| bestPVDz/Err | vector | Dz to best PV | ✅ (NEW) |
| bestPVDxy/Err | vector | Dxy to best PV | ✅ (NEW) |

---

## 4. dStarNtuplizer/candTree (D* → D0π)

Same structure as D0, plus:
| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| dauCand_mass | vector | D0 daughter mass | ✅ |
| dauCand_pt | vector | D0 daughter pT | ✅ |
| deltaM | vector | m(D*) - m(D0) | ✅ |

---

## 5. vertexAnalyzer/vtxTree (Primary Vertices)

| Branch | Type | Description | Status |
|--------|------|-------------|--------|
| run, lumi, event | UInt/ULong64 | Event identifiers | ✅ |
| **BeamSpot** |||
| beamSpotX/Y/Z | Float | BeamSpot position | ✅ |
| beamSpotXErr/YErr/ZErr | Float | BeamSpot errors | ✅ |
| **Vertices** |||
| nVtx, nGoodVtx | Int | Vertex counts | ✅ |
| vtxX/Y/Z[maxVtx] | Float[] | Vertex positions | ✅ |
| vtxXErr/YErr/ZErr[maxVtx] | Float[] | Position errors | ✅ |
| vtxChi2/Ndof/Prob[maxVtx] | Float[] | Fit quality | ✅ |
| vtxNTracks[maxVtx] | Int[] | N tracks | ✅ |
| vtxSumPt/SumPt2[maxVtx] | Float[] | Track pT sums | ✅ |
| vtxIsGood[maxVtx] | Bool[] | Good vertex flag | ✅ |
| bestVtxIdx | Int | Best vertex index | ✅ |

---

## Summary of Recent Additions

1. **J/psi→ee ctau/DCA**: Added `Reco_ee_ctau`, `ctauErr`, `ctau3D`, `ctauErr3D`, `cosAlpha`, `cosAlpha3D`, `dca`
2. **D0/D* PV matching**: Added `bestPVIdx`, `bestPVDz/Err`, `bestPVDxy/Err`
3. **VertexAnalyzer**: New analyzer for detailed PV information

## Potential Missing Items

1. **Reco_ee_vtx**: Vertex position (x,y,z) as TVector3 or separate floats - currently not stored
2. **Daughter vertex info for D0**: Individual daughter track parameters at vertex
3. **Gen-level matching branches** (MC only): Currently disabled for data
