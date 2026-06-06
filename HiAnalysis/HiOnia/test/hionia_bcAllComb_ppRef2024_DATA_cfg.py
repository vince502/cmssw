import FWCore.ParameterSet.Config as cms
import importlib.util
import os

# Wrapper on top of hioniaanalyzer_MuonElectron_PbPb2026_DATA_cfg.py
# Adds Bc semileptonic producer for all visible channels:
#   - BcToJpsiMuMuMuNu
#   - BcToJpsiMuMuENu
#   - BcToJpsiEEMuNu
#   - BcToJpsiEEENu

this_dir = os.path.dirname(os.path.abspath(__file__))
base_cfg = os.path.join(this_dir, "hioniaanalyzer_MuonElectron_PbPb2026_DATA_cfg.py")

spec = importlib.util.spec_from_file_location("hionia_base_cfg", base_cfg)
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)

process = base.process
options = base.options

# Requested threading setup.
process.options.numberOfThreads = cms.untracked.uint32(5)
process.options.numberOfStreams = cms.untracked.uint32(0)

from VertexCompositeAnalysis.VertexCompositeProducer.generalBcSemiLeptonicCandidates_cfi import (
    generalBcSemiLeptonicCandidatesMiniAOD,
)
from VertexCompositeAnalysis.VertexCompositeAnalyzer.patCompositeNtupleProducer_cfi import (
    bcSemiLepNtupleProducer,
)

process.bcSemiLepAll = generalBcSemiLeptonicCandidatesMiniAOD.clone(
    jpsiMuMuSrc=cms.InputTag("onia2MuMuPatGlbGlb"),
    jpsiEESrc=cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    muonSrc=cms.InputTag("patMuonsWithTrigger"),
    electronSrc=cms.InputTag("slimmedElectrons"),
    jpsiMassMin=cms.double(2.2),
    jpsiMassMax=cms.double(4.0),
    minMuonPt=cms.double(1.0),
    minElectronPt=cms.double(1.0),
    doJpsiMuMuMu=cms.bool(True),
    doJpsiMuMuE=cms.bool(True),
    doJpsiEEMu=cms.bool(True),
    doJpsiEEE=cms.bool(True),
    doGenMatching=cms.bool(False),
)

process.bcSemiLepTuple = bcSemiLepNtupleProducer.clone(
    candidateSrc=cms.InputTag("bcSemiLepAll", "BcToJpsiLepNu"),
    beamSpotSrc=cms.InputTag("offlineBeamSpot"),
    vertexSrc=cms.InputTag("offlineSlimmedPrimaryVertices"),
    genealogyInfo=cms.untracked.bool(False),
)

process.oniaTreeAna *= process.bcSemiLepAll
process.oniaTreeAna *= process.bcSemiLepTuple

edm_out = options.outputFile.replace(".root", "_BcAllCombEDM.root")
process.bcOut = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string(edm_out),
    outputCommands=cms.untracked.vstring(
        "drop *",
        "keep *_bcSemiLepAll_*_*",
        "keep *_onia2MuMuPatGlbGlb_*_*",
        "keep *_onia2ElectronElectronPatGlbGlb_*_*",
        "keep edmTriggerResults_TriggerResults_*_*",
    ),
)
process.bcOut_step = cms.EndPath(process.bcOut)
process.schedule.append(process.bcOut_step)

print("============================================================")
print("hionia + bcSemiLepAll wrapper ready")
print("  onia output   =", options.outputFile)
print("  edm output    =", edm_out)
print("  channels      = MuMuMu, MuMuE, EEMu, EEE (+ unions)")
print("============================================================")
