import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('ANASKIM',eras.Run3_pp_on_PbPb)

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')

# Limit the output messages
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# Define the input source
process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring('file:/eos/cms/store/group/phys_heavyions/jiazhao/STARlight/2023Run3/Reco/STARlight_CohJpsiToMuMu_Reco_132X_230824_045129/CRAB_UserFiles/STARlight_CohJpsiToMuMu_Reco_132X_230824_045129/230824_025138/0000/step3_STARlight_Reco_10.root'),
   inputCommands=cms.untracked.vstring('keep *', 'drop *_hiEvtPlane_*_*')
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('132X_mcRun3_2023_realistic_HI_v1')


## ##############################################################################################################################
## Variables Production #########################################################################################################

#* cent_seq: Add PbPb centrality
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("HeavyIonRcd"),
        tag = cms.string("CentralityTable_HFtowers200_HydjetDrum5F_v1032x02_mc"),
        connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS"),
        label = cms.untracked.string("HFtowers")
        ),
    ])
process.cent_seq = cms.Sequence(process.centralityBin)

#* Add the VertexComposite producer
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalDiMuCandidates_cff")
process.generalDiMuCandidatesWrongSign = process.generalDiMuCandidates.clone(isWrongSign = cms.bool(True))
from VertexCompositeAnalysis.VertexCompositeProducer.PATAlgos_cff import doPATMuons
doPATMuons(process, True)	# Get patMuonSequence

#* Add PbPb collision event selection
#* hfCoincFilter2Th4: 2 HF towers above 4 GeV
process.load('VertexCompositeAnalysis.VertexCompositeProducer.collisionEventSelection_cff')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.clusterCompatibilityFilter_cfi')
process.load('VertexCompositeAnalysis.VertexCompositeProducer.hfCoincFilter_cff')
process.load("VertexCompositeAnalysis.VertexCompositeProducer.OfflinePrimaryVerticesRecovery_cfi")
process.colEvtSel = cms.Sequence(process.hfCoincFilter2Th4 * process.primaryVertexFilter * process.clusterCompatibilityFilter)

#* Define the analysis steps
process.pcentandep_step = cms.Path(process.offlinePrimaryVerticesRecovery * process.cent_seq)
process.dimurereco_step = cms.Path(process.offlinePrimaryVerticesRecovery * process.patMuonSequence * process.generalDiMuCandidates)
process.dimurerecowrongsign_step = cms.Path(process.offlinePrimaryVerticesRecovery * process.patMuonSequence * process.generalDiMuCandidatesWrongSign)


## Adding the VertexComposite tree ################################################################################################

process.TFileService = cms.Service("TFileService", fileName = cms.string('dimuana_mc.root'))

#* Using PATCompositeTreeProducer
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.dimuanalyzer_tree_cff")
process.p = cms.EndPath(process.dimucontana_mc * process.dimucontana_wrongsign_mc)
# process.dimucontana_mc.isGammaGamma = cms.untracked.bool(True)
process.dimucontana_mc.doMuonNtuple = cms.untracked.bool(True)
# process.dimucontana_wrongsign_mc.isGammaGamma = cms.untracked.bool(True)
process.dimucontana_wrongsign_mc.doMuonNtuple = cms.untracked.bool(True)

#! Define the process schedule !!!!!!!!!!!!!!!!!!
process.schedule = cms.Schedule(
    process.pcentandep_step,
    process.dimurereco_step,
    process.dimurerecowrongsign_step,
    process.p
)
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

## Add the event selection filters ###############################################################################################
process.Flag_colEvtSel = cms.Path(process.offlinePrimaryVerticesRecovery * process.colEvtSel)
process.Flag_hfCoincFilter2Th4 = cms.Path(process.offlinePrimaryVerticesRecovery * process.hfCoincFilter2Th4)
process.Flag_primaryVertexFilter = cms.Path(process.offlinePrimaryVerticesRecovery * process.primaryVertexFilter)
process.Flag_clusterCompatibilityFilter = cms.Path(process.offlinePrimaryVerticesRecovery * process.clusterCompatibilityFilter)
eventFilterPaths = [ process.Flag_colEvtSel , process.Flag_hfCoincFilter2Th4 , process.Flag_primaryVertexFilter , process.Flag_clusterCompatibilityFilter ]

#! Adding the process schedule !!!!!!!!!!!!!!!!!!
for P in eventFilterPaths:
    process.schedule.insert(0, P)
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# Add recovery for offline primary vertex
from HLTrigger.Configuration.CustomConfigs import massReplaceInputTag
process = massReplaceInputTag(process,"offlinePrimaryVertices","offlinePrimaryVerticesRecovery")
process.offlinePrimaryVerticesRecovery.oldVertexLabel = "offlinePrimaryVertices"