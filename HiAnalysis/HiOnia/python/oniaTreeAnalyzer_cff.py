import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.tools.helpers import *

def oniaTreeAnalyzer(process,
                     muonTriggerList=[[], [], [], []],
                     HLTProName='HLT',
                     muonSelection="Trk",
                     L1Stage=2,
                     isMC=True,
                     pdgID=443,
                     outputFileName="OniaTree.root",
                     muonlessPV=False,
                     doTrimu=False,
                     doDimuTrk=False,
                     flipJpsiDir=0,
                     OnlySingleMuons=False,
                     getObjectsBy="array",
                     doElectrons=False,
                     electronTriggerList=None):

    process.load("FWCore.MessageService.MessageLogger_cfi")
    process.MessageLogger.cerr.FwkReport.reportEvery = 1000

###################### Onia Skim Producer #################################################

    from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import onia2MuMuPAT
    onia2MuMuPAT(process, GlobalTag=process.GlobalTag.globaltag, MC=isMC, HLT=HLTProName, Filter=False, useL1Stage2=(L1Stage==2), doTrimuons=doTrimu, DimuonTrk=doDimuTrk, flipJpsiDir=flipJpsiDir)

##### Onia2MuMuPAT input collections/options
    process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 0")
    process.onia2MuMuPatGlbGlb.onlySingleMuons = cms.bool(OnlySingleMuons)
    process.onia2MuMuPatGlbGlb.resolvePileUpAmbiguity = cms.bool(True)
    process.onia2MuMuPatGlbGlb.srcTracks = cms.InputTag("generalTracks")
    process.onia2MuMuPatGlbGlb.primaryVertexTag = cms.InputTag("offlinePrimaryVertices")
    process.patMuonsWithoutTrigger.pvSrc = cms.InputTag("offlinePrimaryVertices")
# Adding muonLessPV gives you lifetime values wrt. muonLessPV only
    process.onia2MuMuPatGlbGlb.addMuonlessPrimaryVertex = muonlessPV
    if isMC:
        process.genMuons.src = "genParticles"
        process.onia2MuMuPatGlbGlb.genParticles = "genParticles"

##### Dimuon pair selection
    commonP1 = "|| (innerTrack.isNonnull && genParticleRef(0).isNonnull)"
    commonP2 = " && abs(innerTrack.dxy)<4 && abs(innerTrack.dz)<35"
    if muonSelection == "Glb":
        highP = "isGlobalMuon"; # At least one muon must pass this selection. No need to repeat the lowerPuritySelection cuts.
        process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
        lowP = "isGlobalMuon"; # BOTH muons must pass this selection
        process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("("+lowP+commonP1+")"+commonP2)
    elif muonSelection == "GlbTrk":
        highP = "(isGlobalMuon && isTrackerMuon)";
        process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
        lowP = "(isGlobalMuon && isTrackerMuon)";
        process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("("+lowP+commonP1+")"+commonP2)
    elif (muonSelection == "GlbOrTrk" or muonSelection == "TwoGlbAmongThree"):
        highP = "(isGlobalMuon || isTrackerMuon)";
        process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
        lowP = "(isGlobalMuon || isTrackerMuon)";
        process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("("+lowP+commonP1+")"+commonP2)
    elif muonSelection == "Trk":
        highP = "isTrackerMuon";
        process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
        lowP = "isTrackerMuon";
        process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("("+lowP+commonP1+")"+commonP2)
    elif muonSelection == "All":
        process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
        process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("pt > 0")
    else:
        print("ERROR: Incorrect muon selection " + muonSelection + " . Valid options are: Glb, Trk, GlbTrk");

###################### HiOnia Analyzer #################################################

    from HiAnalysis.HiOnia.hioniaanalyzer_cfi import hionia
    process.hionia = hionia.clone(
            triggerResultsLabel = cms.InputTag("TriggerResults","",HLTProName), # Label of Trigger Results
            muonLessPV          = cms.bool(muonlessPV),
            doTrimuons          = cms.bool(doTrimu),  # Whether to produce trimuon objects
            DimuonTrk           = cms.bool(doDimuTrk),  # Whether to produce Jpsi+track objects
            flipJpsiDirection   = cms.int32(flipJpsiDir), # Whether to flip the Jpsi momentum direction
            oniaPDG             = cms.int32(pdgID),
            muonSel             = cms.string(muonSelection),
            isMC                = cms.untracked.bool(isMC),
            onlySingleMuons     = cms.bool(OnlySingleMuons),
            histFileName        = cms.string(outputFileName),
            mom4format          = cms.string(getObjectsBy), # "vector" for flat tree, "array" for TClonesArray of LorentzVectors
            dblTriggerPathNames = muonTriggerList['DoubleMuonTrigger'],
            sglTriggerPathNames = muonTriggerList['SingleMuonTrigger'],
            stageL1Trigger      = cms.uint32(L1Stage)
    )

    process.oniaTreeAna = cms.Sequence(process.patMuonSequence * process.onia2MuMuPatGlbGlb * process.hionia)

    if doElectrons:
        if electronTriggerList is None:
            electronTriggerList = {
                'SingleElectronTrigger': [],
                'DoubleElectronTrigger': []
            }

        if not hasattr(process, 'onia2ElectronElectronPatGlbGlb'):
            process.load('HiSkim.HiOnia2EE.onia2EEPAT_cff')

        from HiSkim.HiOnia2EE.onia2EEPAT_cff import setupOnia2EEForMiniAOD
        setupOnia2EEForMiniAOD(process, 'onia2ElectronElectronPatGlbGlb')

        process.onia2ElectronElectronPatGlbGlb.dielectronSelection = cms.string("mass > 0")
        process.onia2ElectronElectronPatGlbGlb.resolvePileUpAmbiguity = cms.bool(True)

        electronPathsList = []
        for key in ['DoubleElectronTrigger', 'SingleElectronTrigger']:
            if key in electronTriggerList and electronTriggerList[key]:
                electronPathsList.extend(list(electronTriggerList[key]))
        electronPaths = cms.vstring(*electronPathsList)

        from HiAnalysis.HiOnia.hioniaElectronAnalyzer_cfi import hioniaElectrons
        process.hioniaElectrons = hioniaElectrons.clone(
            triggerPathNames=electronPaths,
            triggerResults=cms.InputTag('TriggerResults', '', HLTProName),
            storeGenInfo=cms.bool(isMC),
            isHI=cms.untracked.bool(process.hionia.isHI.value()),
            isMC=cms.untracked.bool(isMC),
            useEvtPlane=cms.untracked.bool(process.hionia.useEvtPlane.value()),
            CentralitySrc=process.hionia.CentralitySrc,
            CentralityBinSrc=process.hionia.CentralityBinSrc,
            EvtPlane=process.hionia.EvtPlane,
            srcElectron=process.onia2ElectronElectronPatGlbGlb.electrons,
            srcDielectron=cms.InputTag('onia2ElectronElectronPatGlbGlb'),
            primaryVertexTag=process.onia2ElectronElectronPatGlbGlb.primaryVertexTag,
            beamSpotTag=process.onia2ElectronElectronPatGlbGlb.beamSpotTag,
            conversions=process.onia2ElectronElectronPatGlbGlb.conversions,
            genParticles=cms.InputTag('prunedGenParticles') if isMC else cms.InputTag('')
        )

        electronSequence = cms.Sequence(process.onia2ElectronElectronPatGlbGlb * process.hioniaElectrons)
        if hasattr(process, 'unpackedTracksAndVertices'):
            electronSequence = cms.Sequence(process.unpackedTracksAndVertices * electronSequence)

        process.oniaTreeAna += electronSequence
