import FWCore.ParameterSet.Config as cms

from HiAnalysis.HiOnia.hioniaanalyzer_cfi import hionia
from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cfi import onia2MuMuPAT
from RecoHI.HiCentralityAlgos.CentralityBin_cfi import centralityBin

# Nominal Run-3 PbPb tracker-muon selection from the chi-c data setup.
# No photon, electron, conversion, or chi-c modules are imported.
charmoniaSelectedMuons = cms.EDFilter(
    "PATMuonSelector",
    src=cms.InputTag("slimmedMuons"),
    cut=cms.string(
        'innerTrack.isNonnull && muonID("AllTrackerMuons")'
        ' && muonID("TMOneStationTight")'
        " && abs(innerTrack.dxy) < 0.3 && abs(innerTrack.dz) < 20."
        " && innerTrack.hitPattern.trackerLayersWithMeasurement > 5"
        " && innerTrack.hitPattern.pixelLayersWithMeasurement > 0"
        " && pt > 2.0 && abs(eta) <= 2.4"
        " && ((abs(eta) < 1.0 && pt > 3.3)"
        " || (abs(eta) >= 1.0 && abs(eta) < 1.3"
        "     && pt > 3.3 - 4.0 * (abs(eta) - 1.0))"
        " || (abs(eta) >= 1.3 && abs(eta) < 1.7"
        "     && pt > 1.53 - 1.325 * (abs(eta) - 1.3))"
        " || (abs(eta) >= 1.7 && abs(eta) <= 2.4 && pt > 1.0))"
    ),
    filter=cms.bool(False),
)

charmoniaDimuons = onia2MuMuPAT.clone(
    muons=cms.InputTag("charmoniaSelectedMuons"),
    primaryVertexTag=cms.InputTag("offlineSlimmedPrimaryVertices"),
    beamSpotTag=cms.InputTag("offlineBeamSpot"),
    genParticles=cms.InputTag("prunedGenParticles"),
    higherPuritySelection=cms.string(""),
    lowerPuritySelection=cms.string(""),
    # Widen the chi-c J/psi window to retain J/psi and psi(2S), plus sidebands.
    dimuonSelection=cms.string(
        "mass >= 2.4 && mass <= 4.2 && pt < 50. && abs(y) < 2.4 && charge == 0"
    ),
    LateDimuonSel=cms.string("userFloat('vProb') > 0.01"),
    addCommonVertex=cms.bool(True),
    onlySoftMuons=cms.bool(False),
    doTrimuons=cms.bool(False),
    DimuonTrk=cms.bool(False),
)

charmoniaDimuonCounter = cms.EDFilter(
    "CandViewCountFilter", src=cms.InputTag("charmoniaDimuons"), minNumber=cms.uint32(1)
)

charmoniaCentralityBin = centralityBin.clone(
    Centrality=cms.InputTag("hiCentrality"),
    centralityVariable=cms.string("HFtowers"),
    nonDefaultGlauberModel=cms.string(""),
)

hionia = hionia.clone(
    srcMuon=cms.InputTag("charmoniaSelectedMuons"),
    srcMuonNoTrig=cms.InputTag("charmoniaSelectedMuons"),
    srcDimuon=cms.InputTag("charmoniaDimuons"),
    primaryVertexTag=cms.InputTag("offlineSlimmedPrimaryVertices"),
    triggerResultsLabel=cms.InputTag("TriggerResults", "", "HLT"),
    CentralitySrc=cms.InputTag("hiCentrality"),
    CentralityBinSrc=cms.InputTag("charmoniaCentralityBin", "HFtowers"),
    stageL1Trigger=cms.uint32(2),
    maxAbsZ=cms.double(1.0e6),
    muonSel=cms.string("All"),
    applyCuts=cms.bool(False),
    miniAODcut=cms.bool(False),
    isHI=cms.untracked.bool(True),
    isPA=cms.untracked.bool(False),
    isMC=cms.untracked.bool(False),
    SumETvariables=cms.bool(True),
    doMuonTriggerMatching=cms.bool(False),
    checkTrigNames=cms.bool(False),
    fillSingleMuons=cms.bool(True),
    AtLeastOneCand=cms.bool(True),
    fillRecoTracks=cms.bool(False),
    useGeTracks=cms.untracked.bool(False),
    useEvtPlane=cms.untracked.bool(False),
    mom4format=cms.string("vector"),
    # Event-level bits only; no PAT muon-object matching.
    # Active HLT/event selection is configured in charmoniaEventSelection_cff.
    dblTriggerPathNames=cms.vstring(
        "HLT_HIL1DoubleMu0_MaxDr3p5_Open_v", "HLT_HIL1DoubleMu0_v",
        "HLT_HIL1DoubleMu0_SQ_v", "HLT_HIL2DoubleMu0_Open_v",
        "HLT_HIL2DoubleMu0_M1p5to6_Open_v", "HLT_HIL2DoubleMu2p8_M1p5to6_Open_v",
        "HLT_HIL2DoubleMu0_M7to15_Open_v", "HLT_HIL2DoubleMu3_M7to15_Open_v",
    ),
    sglTriggerPathNames=cms.vstring(
        "HLT_HIMinimumBiasHF1AND_v", "HLT_HIL1SingleMu0_Open_v",
        "HLT_HIL1SingleMu0_v", "HLT_HIL2SingleMu3_Open_v",
        "HLT_HIL2SingleMu5_v", "HLT_HIL2SingleMu7_v", "HLT_HIL2SingleMu12_v",
    ),
)

charmoniaSequence = cms.Sequence(
    charmoniaSelectedMuons * charmoniaDimuons * charmoniaDimuonCounter
    * charmoniaCentralityBin * hionia
)
