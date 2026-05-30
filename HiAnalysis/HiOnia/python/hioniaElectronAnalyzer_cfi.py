import FWCore.ParameterSet.Config as cms

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

hioniaElectrons = cms.EDAnalyzer(
    'HiOniaElectronAnalyzer',
    # input collections
    srcElectron=cms.InputTag('slimmedElectrons'),
    srcDielectron=cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    primaryVertexTag=cms.InputTag('offlineSlimmedPrimaryVertices'),
    beamSpotTag=cms.InputTag('offlineBeamSpot'),
    conversions=cms.InputTag('reducedEgamma', 'reducedConversions'),
    triggerResults=cms.InputTag('TriggerResults', '', 'HLT'),
    CentralitySrc=cms.InputTag(''),
    CentralityBinSrc=cms.InputTag(''),
    EvtPlane=cms.InputTag(''),
    genParticles=cms.InputTag('prunedGenParticles'),
    triggerPathNames=cms.vstring(*electron_trigger_paths),
    checkTriggerNames=cms.bool(True),
    storeGenInfo=cms.bool(False),
    fillTree=cms.bool(True),
    fillHistos=cms.bool(False),
    isHI=cms.untracked.bool(True),
    isMC=cms.untracked.bool(False),
    useEvtPlane=cms.untracked.bool(False)
)
