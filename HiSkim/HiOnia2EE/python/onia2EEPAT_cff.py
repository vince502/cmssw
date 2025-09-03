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

def changeToMiniAOD(process):

    process.load('HeavyIonsAnalysis.TrackAnalysis.unpackedTracksAndVertices_cfi')
    if hasattr(process, "patMuonsWithTrigger"):
        from MuonAnalysis.MuonAssociators.patMuonsWithTrigger_cff import useExistingPATMuons
        useExistingPATMuons(process, newPatMuonTag=cms.InputTag("unpackedMuons"), addL1Info=False)

        process.patTriggerFull = cms.EDProducer("PATTriggerObjectStandAloneUnpacker",
            patTriggerObjectsStandAlone = cms.InputTag('slimmedPatTrigger'),
            triggerResults              = cms.InputTag('TriggerResults::HLT'),
            unpackFilterLabels          = cms.bool(True)
        )
        process.patMuonSequence.insert(0, process.unpackedTracksAndVertices)
        process.load('HiAnalysis.HiOnia.unpackedMuons_cfi')
        if not process.hionia.isHI:
            process.unpackedMuons.muonSelectors = []
        process.patMuonSequence.insert(1, process.unpackedMuons)

        process.outOnia2MuMu.outputCommands.append('keep *Vert*_unpackedTracksAndVertices_*_*')
        process.outOnia2MuMu.outputCommands.append('keep patMuons_unpackedMuons_*_*')
        process.outOnia2MuMu.outputCommands.append('drop patMuons_patMuonsWith*_*_*')

        if hasattr(process, "muonMatch"):
            from MuonAnalysis.MuonAssociators.patMuonsWithTrigger_cff import changeRecoMuonInput
            changeRecoMuonInput(process, recoMuonCollectionTag=cms.InputTag("unpackedMuonsWithGenMatch"), oldRecoMuonCollectionTag=cms.InputTag("unpackedMuons"))
            process.load('HiAnalysis.HiOnia.unpackedMuonsWithGenMatch_cfi')
            process.patMuonsWithTriggerSequence.insert(1, process.unpackedMuonsWithGenMatch)

            process.onia2MuMuPatGlbGlb.genParticles = "prunedGenParticles"
            process.genMuons.src = "prunedGenParticles"
            process.muonMatch.src = "unpackedMuons"

            process.outOnia2MuMu.outputCommands.append('keep *_prunedGenParticles_*_*')
            process.outOnia2MuMu.outputCommands.append('keep patMuons_unpackedMuonsWithGenMatch_*_*')
            process.outOnia2MuMu.outputCommands.append('drop patMuons_unpackedMuons_*_*')


    from HLTrigger.Configuration.CustomConfigs import massReplaceInputTag
    process = massReplaceInputTag(process,"offlinePrimaryVertices","unpackedTracksAndVertices")
    process = massReplaceInputTag(process,"generalTracks","unpackedTracksAndVertices")
    process = massReplaceInputTag(process,"genParticles","prunedGenParticles")
