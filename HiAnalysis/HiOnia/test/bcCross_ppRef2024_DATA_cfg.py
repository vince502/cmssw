import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

# ----------------------------------------------------------------------------
# Bc cross-channel runner (DATA, ppRef 2024):
#   - Bc -> J/psi(mu mu) + e (+nu)
#   - Bc -> J/psi(e e)   + mu (+nu)
# ----------------------------------------------------------------------------

process = cms.Process('BcCrossDATA', eras.Run3_2024_ppRef)

options = VarParsing.VarParsing('analysis')
options.outputFile = 'BcCross_ppRef2024_DATA.root'
options.maxEvents = 1000
options.inputFiles = [
    '/store/data/Run2024J/PPRefZeroBiasPlusForward0/MINIAOD/PromptReco-v1/000/387/696/00000/0037fb37-713f-4df8-9668-a2ce4665a93c.root'
]
options.register(
    'requireCrossCand',
    True,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.bool,
    'Keep only events with >=1 Bc cross-channel candidate'
)
options.parseArguments()

# Standard services
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')

process.source = cms.Source(
    'PoolSource',
    fileNames=cms.untracked.vstring(options.inputFiles),
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.options.wantSummary = cms.untracked.bool(True)
process.options.numberOfThreads = cms.untracked.uint32(2)
process.options.numberOfStreams = cms.untracked.uint32(0)
process.MessageLogger.cerr.FwkReport.reportEvery = 500

# -----------------------------------------------------------------------------
# Onia producers (mu mu and e e)
# -----------------------------------------------------------------------------
from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import onia2MuMuPAT, changeToMiniAOD

onia2MuMuPAT(
    process,
    GlobalTag=process.GlobalTag.globaltag.value(),
    MC=False,
    HLT='HLT',
    Filter=False,
    useL1Stage2=True,
    doTrimuons=False,
    DimuonTrk=False,
    flipJpsiDir=0,
)

# changeToMiniAOD() expects process.hionia.isHI to exist.
process.hionia = cms.PSet(isHI=cms.bool(False))

changeToMiniAOD(process)
if hasattr(process, 'unpackedMuons'):
    process.unpackedMuons.addPropToMuonSt = cms.bool(True)

# J/psi + psi(2S) window for Bc semileptonic cross channel
process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string('mass > 2.2 && mass < 4.0 && charge==0')
process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string('pt > 1.0 && abs(eta) < 2.4 && isTrackerMuon')
process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string('')
process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string('userFloat("vProb")>0.001')
process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(False)

# Electron onia producer
process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

# J/psi + psi(2S) window for Bc semileptonic cross channel
process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string('mass > 2.2 && mass < 4.0 && charge == 0')
process.onia2ElectronElectronPatGlbGlb.higherPuritySelection = cms.string('pt > 1.5 && abs(eta) < 2.4')
process.onia2ElectronElectronPatGlbGlb.lowerPuritySelection = cms.string('pt > 1.0 && abs(eta) < 2.4')
process.onia2ElectronElectronPatGlbGlb.electrons = cms.InputTag('slimmedElectrons')
process.onia2ElectronElectronPatGlbGlb.srcTracks = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.primaryVertexTag = cms.InputTag('unpackedTracksAndVertices')
process.onia2ElectronElectronPatGlbGlb.conversions = cms.InputTag('reducedEgamma', 'reducedConversions')
process.onia2ElectronElectronPatGlbGlb.doTriggerMatching = cms.bool(False)
process.onia2ElectronElectronPatGlbGlb.electronIDType = cms.string('none')
process.onia2ElectronElectronPatGlbGlb.applyConversionVeto = cms.bool(False)

# Needed by onia2EE
process.load('TrackingTools.TransientTrack.TransientTrackBuilder_cfi')

# -----------------------------------------------------------------------------
# Bc semileptonic cross-channel producer
# -----------------------------------------------------------------------------
from VertexCompositeAnalysis.VertexCompositeProducer.generalBcSemiLeptonicCandidates_cfi import (
    bcToJpsiMuMuEAndEEMuCandidatesMiniAOD,
)

process.bcSemiLepCross = bcToJpsiMuMuEAndEEMuCandidatesMiniAOD.clone(
    jpsiMuMuSrc=cms.InputTag('onia2MuMuPatGlbGlb'),
    jpsiEESrc=cms.InputTag('onia2ElectronElectronPatGlbGlb'),
    # Keep J/psi + psi(2S) window consistent with the producers above.
    jpsiMassMin=cms.double(2.2),
    jpsiMassMax=cms.double(4.0),
    minMuonPt=cms.double(1.0),
    minElectronPt=cms.double(1.0),
)

process.bcCrossCandFilter = cms.EDFilter(
    'CandViewCountFilter',
    src=cms.InputTag('bcSemiLepCross', 'BcToJpsiCrossLepNu'),
    minNumber=cms.uint32(1),
)

process.bcCrossPath = cms.Path(
    process.patMuonSequence *
    process.onia2MuMuPatGlbGlb *
    process.onia2ElectronElectronPatGlbGlb *
    process.bcSemiLepCross *
    process.bcCrossCandFilter
)

# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------
select_events = cms.untracked.PSet()
if options.requireCrossCand:
    select_events = cms.untracked.PSet(SelectEvents=cms.vstring('bcCrossPath'))

process.out = cms.OutputModule(
    'PoolOutputModule',
    fileName=cms.untracked.string(options.outputFile),
    outputCommands=cms.untracked.vstring(
        'drop *',
        'keep *_onia2MuMuPatGlbGlb_*_*',
        'keep *_onia2ElectronElectronPatGlbGlb_*_*',
        'keep *_bcSemiLepCross_*_*',
        'keep *_offlineBeamSpot_*_*',
        'keep *_offlineSlimmedPrimaryVertices_*_*',
        'keep *Vert*_unpackedTracksAndVertices_*_*',
        'keep edmTriggerResults_TriggerResults_*_*',
    ),
    SelectEvents=select_events,
)

process.out_step = cms.EndPath(process.out)
process.schedule = cms.Schedule(process.bcCrossPath, process.out_step)

print('============================================================')
print('Bc cross DATA cfg ready')
print('  outputFile       =', options.outputFile)
print('  maxEvents        =', options.maxEvents)
print('  requireCrossCand =', options.requireCrossCand)
print('  channels         = BcToJpsiMuMuENu + BcToJpsiEEMuNu')
print('============================================================')
