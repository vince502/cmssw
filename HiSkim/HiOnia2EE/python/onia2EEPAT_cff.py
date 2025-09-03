import FWCore.ParameterSet.Config as cms

from HiSkim.HiOnia2EE.onia2EEPAT_cfi import *

# J/psi -> e+e- selection
onia2ElectronElectronPatGlbGlb = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 4.0 && abs(eta) < 2.4"),
    lowerPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4"),
    dielectronSelection = cms.string("mass > 2.8 && mass < 3.4 && charge == 0")
)

# Upsilon -> e+e- selection  
upsilon2ElectronElectronPatGlbGlb = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 4.0 && abs(eta) < 2.4"),
    lowerPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4"),
    dielectronSelection = cms.string("mass > 8.0 && mass < 12.0 && charge == 0")
)

# Loose selection for studies
onia2ElectronElectronPatLoose = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.5"),
    lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.5"),
    dielectronSelection = cms.string("mass > 2.0 && mass < 15.0")
)