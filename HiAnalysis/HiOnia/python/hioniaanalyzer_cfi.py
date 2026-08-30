import FWCore.ParameterSet.Config as cms

hionia = cms.EDAnalyzer('HiOniaAnalyzer',
                        #-- Collections
                        srcMuon          = cms.InputTag("patMuonsWithTrigger"),
                        srcMuonNoTrig    = cms.InputTag("patMuonsWithoutTrigger"),
                        srcDimuon        = cms.InputTag("onia2MuMuPatGlbGlb"),
                        srcTrimuon       = cms.InputTag("onia2MuMuPatGlbGlb","trimuon"),      # Name of Onia Skim Collection for trimuons
                        srcDimuTrk       = cms.InputTag("onia2MuMuPatGlbGlb","dimutrk"),      # Name of Onia Skim Collection for Jpsi+track
                        srcSV            = cms.InputTag("inclusiveSecondaryVerticesLoose",""), # Name of SV collection
                        srcTracks        = cms.InputTag("generalTracks"),
                        genParticles     = cms.InputTag("genParticles"),
                        EvtPlane         = cms.InputTag("hiEvtPlaneFlat"),
                        primaryVertexTag = cms.InputTag("offlinePrimaryVertices"),

                        triggerResultsLabel = cms.InputTag("TriggerResults","","HLT"),

                        CentralitySrc    = cms.InputTag(""),
                        CentralityBinSrc = cms.InputTag(""),

                        #-- Reco Details
                        useBeamSpot = cms.bool(False),
                        useRapidity = cms.bool(True),

                        #--
                        maxAbsZ = cms.double(24.0),

                        pTBinRanges = cms.vdouble(0.0, 6.0, 8.0, 9.0, 10.0, 12.0, 15.0, 40.0),
                        etaBinRanges = cms.vdouble(0.0, 2.5),
                        centralityRanges = cms.vdouble(20,40,100),

                        onlyTheBest = cms.bool(False),
                        applyCuts = cms.bool(False),
			selTightGlobalMuon = cms.bool(False),
                        storeEfficiency = cms.bool(False),
                        SofterSgMuAcceptance = cms.bool(False),
                        SumETvariables = cms.bool(True),
                        OneMatchedHLTMu = cms.int32(-1),
                        storeSameSign = cms.bool(False),
                        AtLeastOneCand = cms.bool(False),

    doTrimuons = cms.bool(False),
    flipJpsiDirection = cms.int32(0),
    genealogyInfo = cms.bool(False),
    doMet = cms.bool(False),
    metInputTag = cms.InputTag("slimmedMETs"),
    miniAODcut = cms.bool(False),
    removeSignalEvents = cms.untracked.bool(False),
                        removeTrueMuons = cms.untracked.bool(False),
                        checkTrigNames     = cms.bool(True),  # Whether to names of the triggers given in the config

                        muonLessPV = cms.bool(False),
                        useSVfinder = cms.bool(False),

                        #-- Gen Details
                        BcPDG = cms.int32(541),
                        oniaPDG = cms.int32(443),
                        oniaPDGs = cms.vint32(),
                        muonSel = cms.string("GlbGlb"),
                        isHI = cms.untracked.bool(True),
                        isPA = cms.untracked.bool(False),
                        isMC = cms.untracked.bool(False),
                        isPromptMC = cms.untracked.bool(True),
                        useEvtPlane = cms.untracked.bool(False),
                        useGeTracks = cms.untracked.bool(False),
                        genOnly     = cms.bool(False),  # fill only generated info

                        #-- OO small-system event and two-particle-correlation details
                        useDetailEventInfo = cms.bool(False),
                        ptTrkDetail = cms.vdouble(0.5, 1.0, 2.0),
                        absEtaTrkDetail = cms.vdouble(1.0, 2.0, 2.4),
                        storeTwoParticleFlowQVec = cms.bool(False),
                        twoParticleFlowQVecPtMin = cms.double(0.4),
                        twoParticleFlowQVecPtMax = cms.double(3.0),
                        twoParticleFlowQVecAbsEtaMax = cms.double(2.4),
                        twoParticleFlowQVecOccupancy = cms.double(100.0),
                        twoParticleFlowQVecCorrFile = cms.string(""),

                        #-- Optional storage of unique, good primary vertices
                        storeUniquePV = cms.bool(False),
                        maxAbsVz = cms.double(15.0),

                        #-- Histogram configuration
                        combineCategories = cms.bool(False),
                        fillRooDataSet = cms.bool(False),
                        fillTree = cms.bool(True),
                        fillHistos = cms.bool(False),
                        minimumFlag = cms.bool(False),
                        fillSingleMuons = cms.bool(True),
                        onlySingleMuons = cms.bool(False),
                        fillRecoTracks = cms.bool(False),
                        histFileName = cms.string("Jpsi_Histos.root"),
                        dataSetName = cms.string("Jpsi_DataSet.root"),
                        mom4format = cms.string("vector"), # "vector" for flat tree, "array" for TClonesArray of LorentzVectors

                        #--
                        dblTriggerPathNames = cms.vstring(),
                        sglTriggerPathNames = cms.vstring(),
                        stageL1Trigger = cms.uint32(True)
                        )
