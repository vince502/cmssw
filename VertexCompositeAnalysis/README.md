# VertexCompositeAnalysis

A CMSSW package for reconstructing and analyzing composite particle candidates (D mesons, B mesons) using `pat::CompositeCandidate`.

## Package Structure

```
VertexCompositeAnalysis/
├── VertexCompositeProducer/     # Candidate reconstruction
│   ├── interface/               # Header files
│   ├── plugins/                 # EDProducer plugins  
│   ├── python/                  # Python configurations
│   └── src/                     # Fitter implementations
│
└── VertexCompositeAnalyzer/     # Ntuple production
    ├── plugins/                 # EDAnalyzer plugins
    └── python/                  # Python configurations
```

## Supported Candidates

| Candidate | Producer | Decay Mode | Daughters |
|-----------|----------|------------|-----------|
| D0 | `D0Producer` | D0 → K⁻ π⁺ | 2-prong |
| D0 (4P) | `D04PProducer` | D0 → K⁻ π⁺ π⁺ π⁻ | 4-prong |
| D* | `DStarProducer` | D*⁺ → D0 π⁺ | D0 + slow π |
| D* (5P) | `DStar5PProducer` | D*⁺ → D0(4P) π⁺ | D0(4P) + slow π |
| B⁺ | `BDiLeptonProducer` | B⁺ → J/ψ K⁺ | J/ψ(→ℓℓ) + K |
| B⁰ | `BDiLeptonProducer` | B⁰ → J/ψ K*⁰ | J/ψ(→ℓℓ) + K* |
| Bc | `BDiLeptonProducer` | Bc → J/ψ π⁺ | J/ψ(→ℓℓ) + π |

## Key Features

### pat::CompositeCandidate Output
All producers output `pat::CompositeCandidateCollection`, which supports:
- `userFloat()` for storing analysis variables
- `userData()` for storing vertex objects
- Named daughters for easy access

### Stored UserFloats
Each candidate stores the following information as userFloats:

**Vertex Information:**
- `VtxChi2`, `VtxNdof` - Vertex fit quality
- `alpha2D`, `alpha3D` - Pointing angles
- `decaylength2D`, `decaylength3D` - Decay lengths
- `decaylengthsignif2D`, `decaylengthsignif3D` - Decay length significance
- `dca3D`, `dca3DErr` - 3D distance of closest approach
- `track3DDCA`, `track3DDCAErr` - Track DCA

**dE/dx Information:**
- `posDauDeDx`, `negDauDeDx` - Daughter track dE/dx

**D* Specific:**
- `slowPionDeDx` - Slow pion dE/dx
- `D0mva` - D0 MVA score (propagated from D0)
- `D0posDauDeDx`, `D0negDauDeDx` - D0 daughter dE/dx
- `deltaM` - Mass difference m(D*) - m(D0)

## Usage

### Running the Producers

```python
import FWCore.ParameterSet.Config as cms
from VertexCompositeAnalysis.VertexCompositeProducer.generalD0Candidates_cfi import generalD0CandidatesNew

process.d0Candidates = generalD0CandidatesNew.clone(
    trackRecoAlgorithm = cms.InputTag('generalTracks'),
    vertexRecoAlgorithm = cms.InputTag('offlinePrimaryVertices'),
)

process.d0Path = cms.Path(process.d0Candidates)
```

### Running the Analyzer

```python
from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import d0NtupleProducer

process.d0Ntuple = d0NtupleProducer.clone(
    candidateSrc = cms.InputTag('d0Candidates', 'D0'),
    isCentrality = cms.untracked.bool(True),  # For HI
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("output.root")
)

process.ana = cms.Path(process.d0Ntuple)
```

### Available Analyzer Configurations

Pre-configured analyzers in `patCompositeNtupleProducer_cfi.py`:
- `d0NtupleProducer` - D0 (2-prong)
- `d04pNtupleProducer` - D0 (4-prong)
- `dStarNtupleProducer` - D*
- `dStar5pNtupleProducer` - D* (5-prong)
- `bPlusNtupleProducer` - B⁺
- `bcNtupleProducer` - Bc

## Output Tree Branches

### Event Information
| Branch | Type | Description |
|--------|------|-------------|
| `RunNb` | uint | Run number |
| `LSNb` | uint | Lumi section |
| `EventNb` | uint | Event number |
| `nPV` | short | Number of primary vertices |
| `bestvtxX/Y/Z` | float | Primary vertex position |
| `candSize` | uint | Number of candidates |
| `centrality` | short | Centrality bin (HI only) |

### Candidate Information
| Branch | Type | Description |
|--------|------|-------------|
| `pT` | float | Candidate pT |
| `eta` | float | Candidate η |
| `phi` | float | Candidate φ |
| `mass` | float | Invariant mass |
| `y` | float | Rapidity |
| `pdgId` | int | PDG ID |
| `mva` | float | MVA score |
| `VtxProb` | float | Vertex probability |
| `alpha3D` | float | 3D pointing angle |
| `decayLength3D` | float | 3D decay length |
| `decayLengthSig3D` | float | 3D decay length significance |
| `dca3D` | float | 3D DCA |

### Daughter Information (D1, D2, ...)
| Branch | Type | Description |
|--------|------|-------------|
| `pTD{n}` | float | Daughter pT |
| `etaD{n}` | float | Daughter η |
| `massD{n}` | float | Daughter mass |
| `chargeD{n}` | short | Daughter charge |
| `dedxD{n}` | float | dE/dx |
| `dzSigD{n}` | float | z DCA significance |
| `dxySigD{n}` | float | xy DCA significance |
| `nhitD{n}` | float | Number of hits |
| `highPurityD{n}` | bool | High purity track |

### Two-Layer Decay (D*, B mesons)
| Branch | Type | Description |
|--------|------|-------------|
| `dauCand_mass` | float | Daughter composite mass (D0, J/ψ) |
| `dauCand_pt` | float | Daughter composite pT |
| `dauCand_vtxProb` | float | Daughter vertex probability |
| `dauCand_mva` | float | Daughter MVA |
| `deltaM` | float | Mass difference (D* only) |
| `pTGD{n}` | float | Grand-daughter pT |
| `dedxGD{n}` | float | Grand-daughter dE/dx |

### Muon-Specific (B mesons)
| Branch | Type | Description |
|--------|------|-------------|
| `isGlobalD{n}` | bool | Global muon |
| `isPFD{n}` | bool | PF muon |
| `isSoftD{n}` | bool | Soft muon ID |
| `isTightD{n}` | bool | Tight muon ID |
| `isHybridD{n}` | bool | Hybrid soft ID (HI) |
| `nMuonHitD{n}` | short | Muon hits |
| `nMatchedStationD{n}` | short | Matched stations |

## Dependencies

Required CMSSW packages:
- `FWCore/*`
- `DataFormats/PatCandidates`
- `DataFormats/VertexReco`
- `DataFormats/TrackReco`
- `DataFormats/HeavyIonEvent`
- `RecoVertex/KinematicFit`
- `TrackingTools/TransientTrack`
- `PhysicsTools/ONNXRuntime` (for MVA)

## Building

```bash
cd $CMSSW_BASE/src
cmsenv
scram b -j8
```

## Authors

- Soohwan Lee
- Based on original VertexCompositeAnalysis by Wei Li

## Notes

- All candidates use `pat::CompositeCandidate` for maximum flexibility
- dE/dx values are stored directly in userFloats (no external lookup needed)
- MVA evaluation uses ONNX Runtime for modern BDT inference
- Compatible with both pp and PbPb (heavy-ion) analyses
