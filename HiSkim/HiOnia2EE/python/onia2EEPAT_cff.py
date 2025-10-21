import FWCore.ParameterSet.Config as cms

from HiSkim.HiOnia2EE.onia2EEPAT_cfi import *

electron_single_trigger_paths = [
    'HLT_HIEle10Gsf_v',
    'HLT_HIEle15Gsf_v',
    'HLT_HIEle20Gsf_v',
    'HLT_HIEle30Gsf_v',
    'HLT_HIEle40Gsf_v',
    'HLT_HIEle50Gsf_v'
]

electron_double_trigger_paths = [
    'HLT_HIDoubleEle10Gsf_v',
    'HLT_HIEle15Ele10Gsf_v',
    'HLT_HIEle15Ele10GsfMass50_v'
]

electron_trigger_paths = electron_single_trigger_paths + electron_double_trigger_paths

# J/psi -> e+e- selection
onia2ElectronElectronPatGlbGlb = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 4.0 && abs(eta) < 2.4"),
    lowerPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4"),
    dielectronSelection = cms.string("mass > 2.8 && mass < 3.4 && charge == 0"),
    doTriggerMatching = cms.bool(True),
    triggerPaths = cms.vstring(*electron_trigger_paths)
)

# Upsilon -> e+e- selection  
upsilon2ElectronElectronPatGlbGlb = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 4.0 && abs(eta) < 2.4"),
    lowerPuritySelection = cms.string("pt > 3.0 && abs(eta) < 2.4"),
    dielectronSelection = cms.string("mass > 8.0 && mass < 12.0 && charge == 0"),
    doTriggerMatching = cms.bool(True),
    triggerPaths = cms.vstring(*electron_trigger_paths)
)

# Loose selection for studies
onia2ElectronElectronPatLoose = onia2ElectronElectronPAT.clone(
    higherPuritySelection = cms.string("pt > 2.0 && abs(eta) < 2.5"),
    lowerPuritySelection = cms.string("pt > 1.0 && abs(eta) < 2.5"),
    dielectronSelection = cms.string("mass > 2.0 && mass < 15.0"),
    doTriggerMatching = cms.bool(True),
    triggerPaths = cms.vstring(*electron_trigger_paths)
)


def setupOnia2EEForMiniAOD(process, producer='onia2ElectronElectronPatGlbGlb'):
    if not hasattr(process, producer):
        return process

    process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
    process.load('HiAnalysis.HiOnia.unpackedTracksAndVertices_cfi')

    module = getattr(process, producer)
    module.srcTracks = cms.InputTag('unpackedTracksAndVertices')
    module.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')

    if hasattr(process, 'skim_path'):
        process.skim_path.insert(0, process.unpackedTracksAndVertices)

    return process
