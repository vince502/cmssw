import FWCore.ParameterSet.Config as cms


pfCandComposites = cms.EDProducer('PFCandCompositeProducer',
                                  pfCandTag    = cms.InputTag('packedPFCandidates'),
                                  compositeTag = cms.InputTag('onia2MuMuPatGlbGlb'),
                                  jpsiTrigFilter = cms.string("hltL1fL1sDoubleMu0L1Filtered0"),
                                  isHI = cms.bool(False),
                                  replaceOniaMM = cms.bool(True),
                                  massOnia = cms.double(3.096900),
                                  replaceDKPi = cms.bool(False)
                                  )
