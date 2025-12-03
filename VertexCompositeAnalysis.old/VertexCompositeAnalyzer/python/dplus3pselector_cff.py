import FWCore.ParameterSet.Config as cms

from VertexCompositeAnalysis.VertexCompositeAnalyzer.dplus3pselector_cfi import *

dplus3pselectorBDTPreCut = dplus3pselector.clone(
  useAnyMVA = cms.bool(True),

  trkPtMin = cms.untracked.double(0.7),
  trkPtSumMin = cms.untracked.double(1.6),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkNHitMin = cms.untracked.int32(11),

  cand3DPointingAngleMax = cms.untracked.double(1.0),
  cand2DPointingAngleMax = cms.untracked.double(1.0),
)

dplus3pselectorBDTNonPrompt = dplus3pselectorBDTPreCut.clone(
  useAnyMVA = cms.bool(True),
  BDTCutFileName = cms.string('BDTCuts_NonPrompLamC3P_HM185.root')
#  mvaCuts = cms.vdouble(4.428e-01,-2.639e-04,-2.610e-02,0.0,2.997e-01,-8.948e-02,1.145e-02,-5.321e-04)
#  mvaCuts = cms.vdouble(0.45,-0.0047,-0.023,0.23,-0.087,0.011,-0.0005)
#  mvaCuts = cms.vdouble(0.44,0.0,-0.026,0.3,-0.09,0.011,0.0)
)

dplus3pselectorBDTPrompt = dplus3pselectorBDTPreCut.clone(
  useAnyMVA = cms.bool(True),
  BDTCutFileName = cms.string('BDTCuts_PrompLamC3P_HM185.root')
#  mvaCuts = cms.vdouble(0.4,0.19,-0.088,0.14,-0.0054,-0.0016,0.0001)
#  mvaCuts = cms.vdouble(3.999e-01,1.872e-01,-8.781e-02,1.397e-01,-5.427e-03,-1.607e-03,1.252e-04) v11
#  mvaCuts = cms.vdouble(4.185e-01,-4.441e-02,1.322e-01,-5.939e-02,1.577e-01,-1.177e-02,-6.253e-04,7.098e-05) #v12
)

dplus3pselectorMCBDTPreCut = dplus3pselectorMC.clone(
  useAnyMVA = cms.bool(False),

  trkPtMin = cms.untracked.double(0.7),
  trkPtSumMin = cms.untracked.double(1.6),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkNHitMin = cms.untracked.int32(11),
)

dplus3pselectorMCBDTNonPrompt = dplus3pselectorMCBDTPreCut.clone(
  useAnyMVA = cms.bool(True),
  BDTCutFileName = cms.string('BDTCuts_NonPrompLamC3P_HM185.root')
#  mvaCuts = cms.vdouble(4.428e-01,-2.639e-04,-2.610e-02,0.0,2.997e-01,-8.948e-02,1.145e-02,-5.321e-04)
)

dplus3pselectorMCBDTPrompt = dplus3pselectorMCBDTPreCut.clone(
  useAnyMVA = cms.bool(True),
  BDTCutFileName = cms.string('BDTCuts_PrompLamC3P_HM185.root')
#  mvaCuts = cms.vdouble(4.185e-01,-4.441e-02,1.322e-01,-5.939e-02,1.577e-01,-1.177e-02,-6.253e-04,7.098e-05) #v12
)     

dplus3pselectorPID = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),
  userPID = cms.bool(True),

  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorPID2 = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),
  userPID = cms.bool(True),

  trkPtSumMin = cms.untracked.double(1.6),
  trkEtaDiffMax = cms.untracked.double(1.2),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCut = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtMin = cms.untracked.double(0.7),
  trkEtaMax = cms.untracked.double(1.5),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutNew = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtMin = cms.untracked.double(0.7),
  trkPtSumMin = cms.untracked.double(1.6),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutNew2 = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtSumMin = cms.untracked.double(1.6),
  trkPtMin = cms.untracked.double(0.6),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)


dplus3pselectorCutNew3 = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtSumMin = cms.untracked.double(1.6),
  trkPtMin = cms.untracked.double(0.7),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutNew4 = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtSumMin = cms.untracked.double(1.6),
  trkPtMin = cms.untracked.double(0.8),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutNew5 = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),
  
  trkPtSumMin = cms.untracked.double(1.6),
  trkPtMin = cms.untracked.double(0.7),
  trkEtaMax = cms.untracked.double(1.5),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutMC = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),
  
  trkPtMin = cms.untracked.double(0.7),
  trkEtaMax = cms.untracked.double(1.5),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorCutNewMC = dplus3pselector.clone(
  useAnyMVA = cms.bool(False),

  trkPtMin = cms.untracked.double(0.7),
  trkPtSumMin = cms.untracked.double(1.6),
  trkEtaDiffMax = cms.untracked.double(1.),
  trkPtErrMax = cms.untracked.double(0.1),
  trkNHitMin = cms.untracked.int32(11),
  cand3DDecayLengthSigMin = cms.untracked.double(3.5),
  cand3DPointingAngleMax = cms.untracked.double(0.15),
  candVtxProbMin = cms.untracked.double(0.15)
)

dplus3pselectorWS = dplus3pselector.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generaldplus3PCandidatesNewWrongSign:dplus3P"),
  MVACollection = cms.InputTag("generaldplus3PCandidatesNewWrongSign:MVAValues")
)

dplus3pselectorWSMC = dplus3pselectorMC.clone(
  VertexCompositeCollection = cms.untracked.InputTag("generaldplus3PCandidatesNewWrongSign:dplus3P"),
  MVACollection = cms.InputTag("generaldplus3PCandidatesNewWrongSign:MVAValues")
)

dplus3pselectorMCGenMatch = dplus3pselectorMC.clone(
  selectGenMatch = cms.untracked.bool(True)
)

dplus3pselectorMCGenUnMatch = dplus3pselectorMC.clone(
  selectGenUnMatch = cms.untracked.bool(True)
)

dplus3pselectorMCGenMatchSwap = dplus3pselectorMC.clone(
  selectGenMatchSwap = cms.untracked.bool(True)
)

dplus3pselectorMCGenMatchUnSwap = dplus3pselectorMC.clone(
  selectGenMatchUnSwap = cms.untracked.bool(True)
)
