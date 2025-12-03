import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM',eras.Run3_pp_on_PbPb)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 200
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# Define the input source
process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring('file:/eos/cms/store/group/phys_heavyions/wangj/RECO2023/aod_PhysicsHIForward0_374345/reco_run374345_ls0100_streamPhysicsHIForward0_StorageManager.root'),
   inputCommands=cms.untracked.vstring('keep *', 'drop *_hiEvtPlane_*_*')
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('132X_dataRun3_Express_v4')


## ##############################################################################################################################
## Variables Production #########################################################################################################

#* cent_seq: Add PbPb centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")
process.centralityBin.nonDefaultGlauberModel = cms.string("")
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("HeavyIonRcd"),
        tag = cms.string("CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run2v1033p1x01_offline"),
        connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
        label = cms.untracked.string("HFtowers")
        ),
    ])
process.cent_seq = cms.Sequence(process.centralityBin)

#* Add the VertexComposite producer
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDiMuCandidates_cff")
process.generalMuMuMassMin7CandidatesWrongSign = process.generalMuMuMassMin7Candidates.clone(isWrongSign = cms.bool(True))
from VertexCompositeAnalysis.VertexCompositeProducer.PATAlgos_cff import doPATMuons
doPATMuons(process, False)

#* Muon selection
process.twoMuons = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("muons"), minNumber = cms.uint32(2))
process.goodMuon = cms.EDFilter("MuonSelector",
            src = cms.InputTag("muons"),
            cut = process.generalMuMuMassMin7Candidates.muonSelection,
            )
process.twoGoodMuons = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("goodMuon"), minNumber = cms.uint32(2))
process.goodDimuon = cms.EDProducer("CandViewShallowCloneCombiner",
            cut = process.generalMuMuMassMin7Candidates.candidateSelection,
            checkCharge = cms.bool(False),
            decay = cms.string('goodMuon@+ goodMuon@-')
            )
process.oneGoodDimuon = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("goodDimuon"), minNumber = cms.uint32(1))
process.dimuonEvtSel = cms.Sequence(process.twoMuons * process.goodMuon * process.twoGoodMuons * process.goodDimuon * process.oneGoodDimuon)

# Add trigger selection
import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltFilter = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltFilter.andOr = cms.bool(True)
process.hltFilter.throw = cms.bool(False)
process.hltFilter.HLTPaths = [
    'HLT_HIUPC_ZeroBias_SinglePixelTrackLowPt_MaxPixelCluster400_v*',
    'HLT_HIUPC_ZeroBias_SinglePixelTrack_MaxPixelTrack_v*',
    'HLT_HIUPC_SingleMuOpen_NotMBHF2AND_v*',
    ]

#* Add PbPb collision event selection
#* hfCoincFilter2Th4: 2 HF towers above 4 GeV
process.load('VertexCompositeAnalysis.VertexCompositeProducer.collisionEventSelection_cff')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.clusterCompatibilityFilter_cfi')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.hfCoincFilter_cff')
process.colEvtSel = cms.Sequence(process.hfCoincFilter2Th4 * process.primaryVertexFilter * process.clusterCompatibilityFilter)

#* Define the event selection sequence
process.eventFilter_HM = cms.Sequence(
    process.hltFilter *
    process.dimuonEvtSel
)
process.eventFilter_HM_step = cms.Path( process.eventFilter_HM )

#* Define the analysis steps
process.pcentandep_step = cms.Path(process.eventFilter_HM * process.cent_seq)
process.dimurereco_step = cms.Path(process.eventFilter_HM * process.patMuonSequence * process.generalMuMuMassMin7Candidates)
process.dimurerecowrongsign_step = cms.Path(process.eventFilter_HM * process.patMuonSequence * process.generalMuMuMassMin7CandidatesWrongSign)


## Adding the VertexComposite tree ################################################################################################

# Add the VertexComposite ntuple
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dimuanalyzer_tree_cff")
process.dimucontana.selectEvents = cms.untracked.string("eventFilter_HM_step")
process.dimucontana_wrongsign.selectEvents = cms.untracked.string("eventFilter_HM_step")

# Define the output
process.TFileService = cms.Service("TFileService", fileName = cms.string('dimuana.root'))
process.p = cms.EndPath(process.dimucontana * process.dimucontana_wrongsign)

#! Define the process schedule !!!!!!!!!!!!!!!!!!
process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.pcentandep_step,
    process.dimurereco_step,
    process.dimurerecowrongsign_step,
    process.p
)
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

## Add the event selection filters ###############################################################################################
process.Flag_colEvtSel = cms.Path(process.eventFilter_HM * process.colEvtSel)
process.Flag_hfCoincFilter2Th4 = cms.Path(process.eventFilter_HM * process.hfCoincFilter2Th4)
process.Flag_primaryVertexFilter = cms.Path(process.eventFilter_HM * process.primaryVertexFilter)
process.Flag_clusterCompatibilityFilter = cms.Path(process.eventFilter_HM * process.clusterCompatibilityFilter)
eventFilterPaths = [ process.Flag_colEvtSel , process.Flag_hfCoincFilter2Th4 , process.Flag_primaryVertexFilter , process.Flag_clusterCompatibilityFilter ]

#! Adding the process schedule !!!!!!!!!!!!!!!!!!
for P in eventFilterPaths:
    process.schedule.insert(0, P)
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!