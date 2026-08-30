import FWCore.ParameterSet.Config as cms


OO_DOUBLE_MUON_TRIGGERS = cms.vstring(
    "HLT_OxyL1DoubleMuOpen_v",
    "HLT_OxyL1DoubleMu0_v",
)

OO_SINGLE_MUON_TRIGGERS = cms.vstring(
    "HLT_OxyL1SingleMuOpen_v",
    "HLT_OxyL1SingleMu0_v",
    "HLT_OxyL1SingleMu3_v",
    "HLT_OxyL1SingleMu5_v",
    "HLT_OxyL1SingleMu7_v",
    "HLT_OxySingleMuCosmic_NotMBHF2AND_v",
    "HLT_OxySingleMuOpen_NotMBHF2AND_v",
    "HLT_MinimumBiasHF_OR_BptxAND_v",
    "HLT_MinimumBiasHF_AND_BptxAND_v",
    "HLT_MinimumBiasZDC1n_OR_BptxAND_v",
    "HLT_MinimumBiasZDC1n_OR_MinimumBiasHF_AND_BptxAND_v",
)

OO_DATA_TRACK_CORRECTIONS = (
    "HiAnalysis/HiOnia/data/"
    "Eff_OO_2025_Hijing_MB_Centrality_fromHihfpf_NoPU_3D_Nominal_Official.root"
)

OO_MC_TRACK_CORRECTIONS = (
    "HiAnalysis/HiOnia/data/"
    "Eff_OO_2025_PythiaHijing_QCD_pThat15_Centrality_fromHihfpf_NoPU_3D_"
    "Nominal_Official.root"
)


def configureInclusiveOnia(process, options, isMC):
    """Configure one framework HiOnia collection shared by Onia and B."""
    from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer

    triggerList = {
        "DoubleMuonTrigger": OO_DOUBLE_MUON_TRIGGERS,
        "SingleMuonTrigger": OO_SINGLE_MUON_TRIGGERS,
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

    # One collection covers the J/psi region used by B reconstruction and all
    # three Upsilon states. Same-sign pairs remain available to HiOnia as a
    # control sample; the B producer applies its own opposite-sign requirement.
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

    # Set the small-system mode before MiniAOD customization so the standard
    # helper disables PbPb-only packed-candidate muon selectors.
    process.hionia.isHI = cms.untracked.bool(False)
    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD

    changeToMiniAOD(process)
    process.unpackedMuons.addPropToMuonSt = cms.bool(True)
    process.unpackedMuons.muonSelectors = cms.vstring()

    isOxygen = options.collisionSystem == "OO"
    process.hionia.srcDimuon = cms.InputTag("onia2MuMuPatGlbGlb")
    process.hionia.oniaPDGs = cms.vint32(443, 553, 100553, 200553)
    process.hionia.muonSel = cms.string("All")
    process.hionia.primaryVertexTag = cms.InputTag("unpackedTracksAndVertices")
    # changeToMiniAOD does not reliably replace this InputTag when the Onia
    # sequence is assembled inside a helper.  Point explicitly at the
    # framework TrackAndVertexUnpacker used by the validated legacy job.
    process.hionia.srcTracks = cms.InputTag("unpackedTracksAndVertices")
    process.hionia.genParticles = cms.InputTag("prunedGenParticles")
    process.hionia.genealogyInfo = cms.bool(isMC)
    process.hionia.isMC = cms.untracked.bool(isMC)
    process.hionia.checkTrigNames = cms.bool(False)
    process.hionia.applyCuts = cms.bool(False)
    process.hionia.AtLeastOneCand = cms.bool(False)
    process.hionia.storeSameSign = cms.bool(True)
    process.hionia.SumETvariables = cms.bool(True)
    process.hionia.mom4format = cms.string("vector")
    process.hionia.useGeTracks = cms.untracked.bool(True)
    process.hionia.fillRecoTracks = cms.bool(False)
    process.hionia.useDetailEventInfo = cms.bool(True)
    process.hionia.ptTrkDetail = cms.vdouble(0.3, 0.4, 0.5, 1.0)
    process.hionia.absEtaTrkDetail = cms.vdouble(0.3, 0.5, 1.0, 1.6, 2.4)
    process.hionia.storeTwoParticleFlowQVec = cms.bool(isOxygen)
    process.hionia.twoParticleFlowQVecPtMin = cms.double(0.4)
    process.hionia.twoParticleFlowQVecPtMax = cms.double(3.0)
    process.hionia.twoParticleFlowQVecAbsEtaMax = cms.double(2.4)
    process.hionia.twoParticleFlowQVecOccupancy = cms.double(100.0)
    process.hionia.twoParticleFlowQVecCorrFile = cms.string(
        (OO_MC_TRACK_CORRECTIONS if isMC else OO_DATA_TRACK_CORRECTIONS)
        if isOxygen
        else ""
    )
    process.hionia.storeUniquePV = cms.bool(isOxygen and not isMC)
    process.hionia.maxAbsVz = cms.double(15.0)
    process.hionia.dblTriggerPathNames = OO_DOUBLE_MUON_TRIGGERS
    process.hionia.sglTriggerPathNames = OO_SINGLE_MUON_TRIGGERS

    return process.oniaTreeAna
