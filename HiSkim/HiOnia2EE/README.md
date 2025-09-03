# HiOnia2EE - J/ψ → e+e- Reconstruction Package

This package provides tools for reconstructing J/ψ (and other onia states) decaying to electron-positron pairs in heavy-ion collisions, similar to the existing `HiOnia2MuMu` package for dimuon final states.

## Overview

The `HiOnia2EEPAT` producer creates PAT composite candidates from pairs of electrons, performing vertex fitting and applying selection criteria suitable for onia physics analysis in heavy-ion environments.

## Key Features

- **Electron pair reconstruction**: Creates dielectron candidates from PAT electrons
- **Vertex fitting**: Uses Kalman vertex fitter for precise vertex reconstruction
- **Conversion rejection**: Critical photon→e+e- conversion veto using ConversionTools
- **Electron identification**: H/E, σ(iηiη), ΔηΔφ, and track-cluster matching cuts
- **SuperCluster information**: Energy, position, and shower shape variables
- **Bremsstrahlung recovery**: fbrem and E/p ratio calculations
- **PF isolation**: Charged hadron, neutral hadron, and photon isolation
- **Quality cuts**: Configurable electron quality and kinematic selections
- **Heavy-ion compatibility**: Designed for CMSSW_14_1_X and Run3 PbPb data
- **PAT integration**: Full integration with PAT framework for analysis

## Main Components

### Core Classes
- `HiOnia2EEPAT`: Main EDProducer for dielectron candidate reconstruction

### Configuration Files
- `onia2EEPAT_cfi.py`: Basic configuration parameters
- `onia2EEPAT_cff.py`: Common analysis configurations (J/ψ, Υ, etc.)

### Test Configurations
- `onia2EEPAT_PbPb_DATA_MiniAOD_cfg.py`: Example configuration for PbPb data
- `SimpleJpsiToEESkim.py`: Simple J/ψ → e+e- skimming configuration

## Usage

### Basic Usage
```python
import FWCore.ParameterSet.Config as cms
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# For J/psi selection
process.jpsi_ee_path = cms.Path(process.onia2ElectronElectronPatGlbGlb)
```

### Key Parameters

#### Electron Selection
- `higherPuritySelection`: Tight electron selection (at least one electron must pass)
- `lowerPuritySelection`: Loose electron selection (both electrons must pass)

#### Dielectron Selection
- `dielectronSelection`: Mass and charge requirements for dielectron candidates

#### Vertex Options
- `addCommonVertex`: Enable vertex fitting (recommended: True)
- `resolvePileUpAmbiguity`: Handle multiple candidates per event

## Electron Quality and Identification

The package implements comprehensive electron identification similar to ggHiNtuplizer:

### Track Quality
- GSF tracks (appropriate for electrons) 
- Tracker layers with measurement > 5
- Pixel layers with measurement > 0  
- Impact parameter cuts (dxy < 0.3 cm, dz < 20 cm)

### Conversion Rejection
- **Critical**: Uses `ConversionTools::hasMatchedConversion()` to reject photon conversions
- Essential for reducing fake electron background in heavy-ion environment

### Electron ID Variables
- **H/E ratio**: Hadronic energy over electromagnetic energy < 0.15
- **Shower shape**: σ(iηiη) cuts for barrel (< 0.012) and endcap (< 0.035)  
- **Track-cluster matching**: Δη < 0.01, Δφ < 0.08 between track and SuperCluster
- **Bremsstrahlung**: fbrem and E/p ratio for energy loss characterization

### Stored Variables per Electron
Each dielectron candidate stores detailed information:
- Conversion veto results (`ele1ConvVeto`, `ele2ConvVeto`)
- SuperCluster variables (`ele1SCEta`, `ele1SCPhi`, `ele1SCEn`, etc.)
- ID variables (`ele1HoverE`, `ele1SigmaIEtaIEta`, `ele1DeltaEtaIn`, etc.)
- Isolation (`ele1PFChIso`, `ele1PFNeuIso`, `ele1PFPhoIso`)
- Bremsstrahlung (`ele1FBrem`, `ele1EoverP`)

## CMSSW Compatibility

This package is designed for **CMSSW_14_1_X** with Run3 heavy-ion data:
- Uses `Run3_pp_on_PbPb_2024` era
- Compatible with Global Tag `141X_dataRun3_Prompt_v3`
- Follows CMSSW_14_1_X EDProducer patterns

## Output Collections

- **Default collection**: All dielectron candidates passing selections
- **"trielectron"**: Placeholder for future trielectron analysis
- **"dieletrk"**: Placeholder for dielectron+track analysis

## Examples

### J/ψ → e+e- Selection
```python
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 2.8 && mass < 3.4 && charge == 0"
)
```

### Υ → e+e- Selection  
```python
process.upsilon2ElectronElectronPatGlbGlb.dielectronSelection = cms.string(
    "mass > 8.0 && mass < 12.0 && charge == 0"
)
```

## Important Notes

- **This package cannot be compiled locally** - requires lxplus or CMSSW computing environment
- Electron reconstruction is more challenging than muon reconstruction due to bremsstrahlung
- Consider using electron energy corrections and identification criteria appropriate for your analysis
- The package is based on the well-tested `HiOnia2MuMu` framework

## Related Packages

- `HiSkim/HiOnia2MuMu`: Equivalent package for dimuon final states
- `HeavyIonsAnalysis/EGMAnalysis`: Electron/photon analysis tools
- `HiAnalysis/HiOnia`: Higher-level onia analysis framework