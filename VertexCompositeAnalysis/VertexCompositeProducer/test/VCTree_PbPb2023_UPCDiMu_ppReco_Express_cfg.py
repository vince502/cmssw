import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM',eras.Run3)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 200
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))
# process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True), SkipEvent = cms.untracked.vstring('ProductNotFound'))

# Define the input source
process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring('file:/eos/cms/store/group/phys_heavyions/jiazhao/public/reco_RAW2DIGI_L1Reco_RECO_PAT.root'),
   inputCommands=cms.untracked.vstring('keep *', 'drop *_hiEvtPlane_*_*'),
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('132X_dataRun3_Express_v4')


#* Add the VertexComposite producer
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDiMuCandidates_cff")
process.generalMuMuMassMin0CandidatesWrongSign = process.generalMuMuMassMin0Candidates.clone(isWrongSign = cms.bool(True))
from VertexCompositeAnalysis.VertexCompositeProducer.PATAlgos_cff import doPATMuons
doPATMuons(process, False)

#* Muon selection
process.twoMuons = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("muons"), minNumber = cms.uint32(2))
process.goodMuon = cms.EDFilter("MuonSelector",
            src = cms.InputTag("muons"),
            cut = process.generalMuMuMassMin0Candidates.muonSelection,
            )
process.twoGoodMuons = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("goodMuon"), minNumber = cms.uint32(2))
process.goodDimuon = cms.EDProducer("CandViewShallowCloneCombiner",
            cut = process.generalMuMuMassMin0Candidates.candidateSelection,
            checkCharge = cms.bool(False),
            decay = cms.string('goodMuon@+ goodMuon@-')
            )
process.oneGoodDimuon = cms.EDFilter("CandViewCountFilter", src = cms.InputTag("goodDimuon"), minNumber = cms.uint32(1))
process.dimuonEvtSel = cms.Sequence(process.twoMuons * process.goodMuon * process.twoGoodMuons * process.goodDimuon * process.oneGoodDimuon)

#* Add PbPb collision event selection
process.load('VertexCompositeAnalysis.VertexCompositeProducer.collisionEventSelection_cff')
process.colEvtSel = cms.Sequence( process.primaryVertexFilter )

#* Define the event selection sequence
process.eventFilter_HM = cms.Sequence(
    process.colEvtSel *
    process.dimuonEvtSel
)
process.eventFilter_HM_step = cms.Path( process.eventFilter_HM )

#* Define the analysis steps
process.dimurereco_step = cms.Path(process.eventFilter_HM * process.patMuonSequence * process.generalMuMuMassMin0Candidates)
process.dimurerecowrongsign_step = cms.Path(process.eventFilter_HM * process.patMuonSequence * process.generalMuMuMassMin0CandidatesWrongSign)


## Adding the VertexComposite tree ################################################################################################

# Add the VertexComposite ntuple
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dimuanalyzer_tree_cff")
process.dimucontana.selectEvents = cms.untracked.string("eventFilter_HM_step")
process.dimucontana.doMuonNtuple = cms.untracked.bool(True)
process.dimucontana.VertexCompositeCollection = cms.untracked.InputTag("generalMuMuMassMin0Candidates:DiMu")
process.dimucontana.isCentrality = cms.bool(False)
process.dimucontana.isEventPlane = cms.bool(False)
process.dimucontana_wrongsign = process.dimuana.clone(VertexCompositeCollection = cms.untracked.InputTag("generalMuMuMassMin0CandidatesWrongSign:DiMu"))
process.dimucontana_wrongsign.doMuonNtuple = cms.untracked.bool(False)

# Define the output
process.TFileService = cms.Service("TFileService", fileName = cms.string('dimuana.root'))
process.p = cms.EndPath(process.dimucontana * process.dimucontana_wrongsign)

#! Define the process schedule !!!!!!!!!!!!!!!!!!
process.schedule = cms.Schedule(
    process.eventFilter_HM_step,
    process.dimurereco_step,
    process.dimurerecowrongsign_step,
    process.p
)
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


## Add the event selection filters. Going to be used in evtSel ###############################################################
process.Flag_primaryVertexFilter = cms.Path(process.primaryVertexFilter)
eventFilterPaths = [ process.Flag_primaryVertexFilter ]

#! Adding the process schedule !!!!!!!!!!!!!!!!!!
for P in eventFilterPaths:
    process.schedule.insert(0, P)
