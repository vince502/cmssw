import FWCore.ParameterSet.Config as cms


PBPB_DOUBLE_MUON_TRIGGERS = cms.vstring(
    "HLT_HIL1DoubleMu0_MaxDr3p5_Open_v",
    "HLT_HIL1DoubleMu0_v",
    "HLT_HIL1DoubleMu0_SQ_v",
    "HLT_HIL2DoubleMu0_Open_v",
    "HLT_HIL2DoubleMu0_M1p5to6_Open_v",
    "HLT_HIL2DoubleMu2p8_M1p5to6_Open_v",
    "HLT_HIL2DoubleMu0_M7to15_Open_v",
    "HLT_HIL2DoubleMu3_M7to15_Open_v",
    "HLT_HIL2DoubleMu0_SQ_v",
    "HLT_HIL2DoubleMuOpen_Centrality40to100_v",
    "HLT_HIL2DoubleMuOpen_OS_v",
    "HLT_HIL2DoubleMuOpen_SS_v",
    "HLT_HIL3DoubleMu0_M0toInf_Open_v",
    "HLT_HIL3DoubleMu0_Quarkonia_Open_v",
    "HLT_HIL3DoubleMu2_Quarkonia_Open_v",
    "HLT_HIL3DoubleMu0_M2to4p5_Open_v",
    "HLT_HIL3DoubleMu2_M2to4p5_Open_v",
    "HLT_HIL3DoubleMu0_M7to15_Open_v",
    "HLT_HIL3DoubleMu2_M7to15_Open_v",
)

PBPB_SINGLE_MUON_TRIGGERS = cms.vstring(
    "HLT_HIL1SingleMu0_Open_v",
    "HLT_HIL1SingleMu0_v",
    "HLT_HIL1SingleMu0_Centrality30to100_v",
    "HLT_HIL1SingleMu0_Centrality40to100_v",
    "HLT_HIL1SingleMuOpen_Centrality30to100_v",
    "HLT_HIL1SingleMuOpen_Centrality40to100_v",
    "HLT_HIL2SingleMu3_Open_v",
    "HLT_HIL2SingleMu5_v",
    "HLT_HIL2SingleMu7_v",
    "HLT_HIL2SingleMu12_v",
    "HLT_HIL2SingleMu0_Centrality30to100_v",
    "HLT_HIL2SingleMu0_Centrality40to100_v",
    "HLT_HIL2SingleMuOpen_Centrality30to100_v",
    "HLT_HIL3SingleMu3_Open_v",
    "HLT_HIL3SingleMu5_v",
    "HLT_HIL3SingleMu7_v",
    "HLT_HIL3SingleMu12_v",
    "HLT_HIMinimumBiasHF1AND_v",
    "HLT_HIMinimumBiasHF1ANDZDC2nOR_v",
    "HLT_HIMinimumBiasHF1ANDZDC1nOR_v",
)


def configureInclusiveOnia(process, isMC):
    from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer

    triggerList = {
        "DoubleMuonTrigger": PBPB_DOUBLE_MUON_TRIGGERS,
        "SingleMuonTrigger": PBPB_SINGLE_MUON_TRIGGERS,
    }
    oniaTreeAnalyzer(
        process,
        muonTriggerList=triggerList,
        HLTProName="HLT",
        muonSelection="All",
        L1Stage=2,
        isMC=isMC,
        pdgID=553,
        outputFileName=process.TFileService.fileName.value(),
        doTrimu=False,
        getObjectsBy="vector",
    )

    # The one framework collection covers B -> J/psi and all Upsilon states.
    process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string(
        "mass > 2.4 && mass < 14.0"
    )
    process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string(
        "pt > 0.0 && abs(eta) < 2.4"
    )
    process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
    process.onia2MuMuPatGlbGlb.onlySoftMuons = cms.bool(False)
    process.onia2MuMuPatGlbGlb.LateDimuonSel = cms.string(
        'userFloat("vProb") > 0.0'
    )

    process.hionia.isHI = cms.untracked.bool(True)
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD

    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(True)
    process.unpackedMuons.muonSelectors = cms.vstring()

    process.hionia.srcDimuon = cms.InputTag("onia2MuMuPatGlbGlb")
    process.hionia.oniaPDGs = cms.vint32(443, 553, 100553, 200553)
    process.hionia.muonSel = cms.string("All")
    process.hionia.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
    process.hionia.srcTracks = cms.InputTag("unpackedTracksAndVertices")
    process.hionia.genParticles = cms.InputTag("prunedGenParticles")
    process.hionia.genealogyInfo = cms.bool(isMC)
    process.hionia.isMC = cms.untracked.bool(isMC)
    process.hionia.checkTrigNames = cms.bool(False)
    process.hionia.applyCuts = cms.bool(False)
    process.hionia.AtLeastOneCand = cms.bool(False)
    process.hionia.storeSameSign = cms.bool(True)
    process.hionia.CentralitySrc = cms.InputTag("hiCentrality")
    process.hionia.CentralityBinSrc = cms.InputTag("centralityBin", "HFtowers")
    process.hionia.SumETvariables = cms.bool(True)
    process.hionia.mom4format = cms.string("vector")
    process.hionia.useGeTracks = cms.untracked.bool(True)
    process.hionia.fillRecoTracks = cms.bool(False)
    process.hionia.useDetailEventInfo = cms.bool(True)
    process.hionia.ptTrkDetail = cms.vdouble(0.3, 0.4, 0.5, 1.0)
    process.hionia.absEtaTrkDetail = cms.vdouble(0.3, 0.5, 1.0, 1.6, 2.4)

    # Store an uncorrected PbPb reference Q-vector. A PbPb tracking payload
    # can be supplied later without changing the branch contract.
    process.hionia.storeTwoParticleFlowQVec = cms.bool(True)
    process.hionia.twoParticleFlowQVecPtMin = cms.double(0.4)
    process.hionia.twoParticleFlowQVecPtMax = cms.double(3.0)
    process.hionia.twoParticleFlowQVecAbsEtaMax = cms.double(2.4)
    process.hionia.twoParticleFlowQVecOccupancy = cms.double(100.0)
    process.hionia.twoParticleFlowQVecCorrFile = cms.string("")

    process.hionia.storeUniquePV = cms.bool(not isMC)
    process.hionia.maxAbsVz = cms.double(15.0)
    process.hionia.dblTriggerPathNames = PBPB_DOUBLE_MUON_TRIGGERS
    process.hionia.sglTriggerPathNames = PBPB_SINGLE_MUON_TRIGGERS

    process.oniaTreeAna.replace(
        process.hionia, process.centralityBin * process.hionia
    )
    return process.oniaTreeAna
