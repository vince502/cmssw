import FWCore.ParameterSet.Config as cms

process = cms.Process("writeGBRForests")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1) # NB: needs to be set to 1 so that GBRForestWriter::analyze method gets called exactly once
)

process.source = cms.Source("EmptySource")

process.load('Configuration/StandardSequences/Services_cff')

process.gbrForestWriter = cms.EDAnalyzer("GBRForestWriter",
    jobs = cms.VPSet(
        cms.PSet(
            inputFileName = cms.FileInPath('/afs/cern.ch/work/s/soohwan/private/Analysis/DmesonAna/2023Analysis/DStarJunseok/el9/CMSSW_13_2_13/src/VertexCompositeAnalysis/VertexCompositeAnalyzer/test/xgb_model_Var_pT_y_cBin_19_v25Mar.xml'),
            inputFileType = cms.string("XML"),
            #inputVariables = cms.vstring( 'VtxProb', 'dca3D', 'v3DCosPointingAngle', 'v3DPointingAngle', 'v2DCosPointingAngle', 'v2DPointingAngle', 'v3DDecayLengthSignificance', 'v3DDecayLength', 'v2DDecayLengthSignificance', 'v2DDecayLength', 'pTD1', 'EtaD1', 'pTD2', 'EtaD2' ),
            inputVariables = cms.vstring( 
		#'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10', 'f11', 'f12', 'f13', 'f14', 'f15', 'f16', 'f17', 'f18', 'f19', 'f20', 'f21', 'f22'
		'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10', 'f11', 'f12', 'f13', 'f14', 'f15', 'f16', 'f17', 'f18'
		#'f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10', 'f11', 'f12', 'f13', 'f14'
	     ),
            spectatorVariables = cms.vstring(),
            methodName = cms.string("BDT"),
            gbrForestName = cms.string("D0InPbPbXGB"),
            outputFileType = cms.string("GBRForest"),
            outputFileName = cms.string("GBRForestfile_XGBDT_PromptD0InPbPb_pT_y_cBIN_19Params_v1_25Mar.root")
        )
    )
)



process.p = cms.Path(process.gbrForestWriter)
