import FWCore.ParameterSet.Config as cms

from Configuration.EventContent.EventContent_cff import FEVTEventContent

skimFEVTContent = FEVTEventContent.clone()
skimFEVTContent.outputCommands.append("drop *_MEtoEDMConverter_*_*")
skimFEVTContent.outputCommands.append("drop *_*_*_SKIM")

#####################      

from Configuration.Skimming.Ion_MuonSkim_cff import *
muSkimIonPath = cms.Path( muSkimIonSequence )
SKIMStreamIonMuon = cms.FilteredStream(
    responsible = 'HI PAG',
    name = 'IonMuon',
    paths = (muSkimIonPath),
    content = skimFEVTContent.outputCommands,
    selectEvents = cms.untracked.PSet(),
    dataTier = cms.untracked.string('RAW-RECO')
    )

#####################      
