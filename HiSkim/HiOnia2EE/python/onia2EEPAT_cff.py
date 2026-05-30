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

def setElectronID(producer, idType="hardcoded", wp="", idName=""):
    """
    Configure electron ID for an onia2EE producer.
    
    Args:
        producer: The onia2EE producer module
        idType: "hardcoded" (default), "cutbased", "mva", "hiMVA", "none"
        wp: Working point. For cutbased: "veto/loose/medium/tight"
            For mva: "wp90/wp80". For hiMVA: "95/90/85/80"
        idName: Explicit MiniAOD ID name (overrides wp if set)
    
    Examples:
        setElectronID(process.onia2ElectronElectronPatGlbGlb, "cutbased", "loose")
        setElectronID(process.onia2ElectronElectronPatGlbGlb, "mva", "wp90")
        setElectronID(process.onia2ElectronElectronPatGlbGlb, "hiMVA", "90")
        setElectronID(process.onia2ElectronElectronPatGlbGlb, "cutbased", idName="cutBasedElectronID-RunIIIWinter22-V1-loose")
    """
    producer.electronIDType = cms.string(idType)
    producer.electronIDWP = cms.string(wp)
    producer.electronIDName = cms.string(idName)
    return producer

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


def setupOnia2EEForMiniAOD(process, producer='onia2ElectronElectronPatGlbGlb', runEnergyCorrections=True, runHIElectronID=True, isMC=False):
    """
    Setup Onia2EE for MiniAOD analysis with electron corrections and HI ID/ISO
    
    Args:
        process: CMSSW process
        producer: Name of the onia2EE producer (default: 'onia2ElectronElectronPatGlbGlb')
        runEnergyCorrections: Apply energy scale corrections (default: True)
        runHIElectronID: Apply HI-specific electron ID and isolation (default: True)
        isMC: True for MC, False for data (default: False)
    """
    if not hasattr(process, producer):
        return process

    process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')
    process.load('HeavyIonsAnalysis.TrackAnalysis.unpackedTracksAndVertices_cfi')

    module = getattr(process, producer)
    module.srcTracks = cms.InputTag('unpackedTracksAndVertices')
    module.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
    
    # Configure electron corrections and ID if requested
    if runEnergyCorrections or runHIElectronID:
        from HiSkim.HiOnia2EE.electronCorrectionAndID_cff import configureElectronCorrectionAndID, updateOnia2EEProducer
        
        # Setup correction and ID sequence
        configureElectronCorrectionAndID(process, isMC=isMC)
        
        # Update producer to use corrected electrons
        if runHIElectronID:
            updateOnia2EEProducer(process, producer)
        elif runEnergyCorrections:
            # Use corrected electrons without HI ID
            module.electrons = cms.InputTag('correctedElectrons')
        
        # Insert correction sequence before the producer
        if hasattr(process, 'skim_path'):
            # Find the producer in the path
            producerModule = getattr(process, producer)
            # Insert sequences at the beginning
            process.skim_path.insert(0, process.unpackedTracksAndVertices)
            process.skim_path.insert(1, process.electronCorrectionAndIDSequence)
        else:
            # If no skim_path, user needs to add these manually
            print("WARNING: No 'skim_path' found. Please add 'process.unpackedTracksAndVertices' and 'process.electronCorrectionAndIDSequence' to your path manually.")
    else:
        # Just add unpacked tracks without corrections
        if hasattr(process, 'skim_path'):
            process.skim_path.insert(0, process.unpackedTracksAndVertices)

    return process
