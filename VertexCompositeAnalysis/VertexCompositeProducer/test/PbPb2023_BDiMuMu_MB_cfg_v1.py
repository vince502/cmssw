import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

process = cms.Process("BDiMuMuANA", eras.Run3_2024_ppRef)


# Setup VarParsing
options = VarParsing.VarParsing ('standard')
options.register('inputFiles',
                 '/store/user/davidlw/HIDoubleMuon/crab_PbPb2023_HIDoubleMuon_374810_HIPM_skim/231208_221043/0000/*.root',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Input files")
options.register('outputFile',
                 'BDiMuMu_PbPb2023.root',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Output file")
#options.register('maxEvents',
#                 1000,
#                 VarParsing.VarParsing.multiplicity.singleton,
#                 VarParsing.VarParsing.varType.int,
#                 "Maximum number of events")
options.parseArguments()



HLTProcess     = "HLT" # Name of HLT process
isMC           = False # if input is MONTECARLO: True or if it's DATA: False
muonSelection  = "All" # Single muon selection: All, Glb(isGlobal), GlbTrk(isGlobal&&isTracker), Trk(isTracker), GlbOrTrk, TwoGlbAmongThree (which requires two isGlobal for a trimuon, and one isGlobal for a dimuon) are available
applyEventSel  = False # Only apply Event Selection if the required collections are present
OnlySoftMuons  = False # Keep only isSoftMuon's (without highPurity, and without isGlobal which should be put in 'muonSelection' parameter) from the beginning of HiSkim. If you want the full SoftMuon selection, set this flag false and add 'isSoftMuon' in lowerPuritySelection. In any case, if applyCuts=True, isSoftMuon is required at HiAnalysis level for muons of selected dimuons.
applyCuts      = False # At HiAnalysis level, apply kinematic acceptance cuts + identification cuts (isSoftMuon (without highPurity) or isTightMuon, depending on TightGlobalMuon flag) for muons from selected di(tri)muons + hard-coded cuts on the di(tri)muon that you would want to add (but recommended to add everything in LateDimuonSelection, applied at the end of HiSkim)
SumETvariables = False  # Whether to write out SumET-related variables
SofterSgMuAcceptance = False # Whether to accept muons with a softer acceptance cuts than the usual (pt>3.5GeV at central eta, pt>1.5 at high |eta|). Applies when applyCuts=True
doTrimuons     = False # Make collections of trimuon candidates in addition to dimuons, and keep only events with >0 trimuons (if atLeastOneCand)
doDimuonTrk    = False # Make collections of Jpsi+track candidates in addition to dimuons
atLeastOneCand = False # Keep only events that have one selected dimuon (or at least one trimuon if doTrimuons = true). BEWARE this can cause trouble in .root output if no event is selected by onia2MuMuPatGlbGlbFilter!
OneMatchedHLTMu = -1   # Keep only di(tri)muons of which the one(two) muon(s) are matched to the HLT Filter of this number. You can get the desired number in the output of oniaTree. Set to -1 for no matching.
#############################################################################
keepExtraColl  = False # General Tracks + Stand Alone Muons + Converted Photon collections
miniAOD        = True # whether the input file is in miniAOD format (default is AOD)
UsePropToMuonSt = True # whether to use L1 propagated muons (works only for miniAOD now)
pdgId = 443 # J/Psi : 443, Y(1S) : 553
useMomFormat = "" # default "array" for TClonesArray of TLorentzVector. Use "vector" for std::vector<float> of pt, eta, phi, M


GT = '141X_dataRun3_Prompt_v3' if not isMC else 'auto:phase1_2024_realistic_ppRef5TeV'

# Load necessary conditions
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')

# Global tag for PbPb 2023
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, GT, '')

# Message logger configuration
process.MessageLogger.cerr.FwkReport.reportEvery = 100

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

## Load HiOnia2MuMu configuration for dimuon reconstruction
#from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import onia2MuMuPAT, changeToMiniAOD
#
#onia2MuMuPAT(process, GlobalTag=process.GlobalTag.globaltag, MC=isMC, HLT=HLTProName, Filter=False, useL1Stage2=(L1Stage==2), doTrimuons=doTrimu, DimuonTrk=doDimuTrk, flipJpsiDir=flipJpsiDir)
#
## Configure dimuon reconstruction
#process.onia2MuMuPatGlbGlb.muons = cms.InputTag("patMuonsWithTrigger")
#process.onia2MuMuPatGlbGlb.lowerPuritySelection = cms.string("isPFMuon && (isGlobalMuon && isTrackerMuon)")
#process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("")
#process.onia2MuMuPatGlbGlb.dimuonSelection = cms.string("mass > 2.9 && mass < 3.3 && charge = 0")
#process.onia2MuMuPatGlbGlb.addCommonVertex = cms.bool(True)
#process.onia2MuMuPatGlbGlb.resolvePileUpAmbiguity = cms.bool(True)

triggerList    = {
		# Double Muon Trigger List
		'DoubleMuonTrigger' : cms.vstring(
			      "HLT_PPRefL1DoubleMu0_Open_v",
            "HLT_PPRefL1DoubleMu0_v",
            "HLT_PPRefL1DoubleMu0_SQ_v",
            "HLT_PPRefL1DoubleMu2_v",
            "HLT_PPRefL1DoubleMu2_SQ_v",
            "HLT_PPRefL2DoubleMu0_Open_v",
            "HLT_PPRefL2DoubleMu0_v",
            "HLT_PPRefL3DoubleMu0_Open_v",
            "HLT_PPRefL3DoubleMu0_v"
            ),
        # Single Muon Trigger List
        'SingleMuonTrigger' : cms.vstring(
            "HLT_PPRefL1SingleMu7_v",
            "HLT_PPRefL1SingleMu12_v",
            "HLT_PPRefL2SingleMu7_v",
            "HLT_PPRefL2SingleMu12_v",
            "HLT_PPRefL2SingleMu15_v",
            #"HLT_PPRefL2SingleMu20_v",
            "HLT_PPRefL3SingleMu3_v",
            "HLT_PPRefL3SingleMu5_v",
            "HLT_PPRefL3SingleMu7_v",
            "HLT_PPRefL3SingleMu12_v",
            "HLT_PPRefL3SingleMu15_v",
            "HLT_PPRefL3SingleMu20_v",
			)
                }

from HiAnalysis.HiOnia.oniaTreeAnalyzer_cff import oniaTreeAnalyzer
oniaTreeAnalyzer(process,
                 muonTriggerList=triggerList, #HLTProName=HLTProcess,
                 muonSelection=muonSelection, L1Stage=2, isMC=isMC, pdgID=pdgId, outputFileName=options.outputFile, doTrimu=doTrimuons#, OnlySingleMuons=True
)

process.onia2MuMuPatGlbGlb.dimuonSelection       = cms.string("mass > 2 && mass < 4.2 && charge==0 && abs(daughter('muon1').innerTrack.dz - daughter('muon2').innerTrack.dz) < 25")
process.onia2MuMuPatGlbGlb.lowerPuritySelection  = cms.string("pt > 0 && abs(eta) < 2.4")
#process.onia2MuMuPatGlbGlb.higherPuritySelection = cms.string("") ## No need to repeat lowerPuritySelection in there, already included
if applyCuts:
  process.onia2MuMuPatGlbGlb.LateDimuonSel         = cms.string("userFloat(\"vProb\")>0.001")

process.onia2MuMuPatGlbGlb.onlySoftMuons         = cms.bool(OnlySoftMuons)
process.hionia.minimumFlag      = cms.bool(keepExtraColl)           #for Reco_trk_*
process.hionia.useGeTracks      = cms.untracked.bool(keepExtraColl) #for Reco_trk_*
process.hionia.fillRecoTracks   = cms.bool(keepExtraColl)           #for Reco_trk_*
#process.hionia.CentralitySrc    = cms.InputTag("hiCentrality")
#process.hionia.CentralityBinSrc = cms.InputTag("centralityBin","HFtowers")
#process.hionia.muonLessPV       = cms.bool(False)
process.hionia.SofterSgMuAcceptance = cms.bool(SofterSgMuAcceptance)
process.hionia.SumETvariables   = cms.bool(SumETvariables)
process.hionia.applyCuts        = cms.bool(applyCuts)
process.hionia.AtLeastOneCand   = cms.bool(atLeastOneCand)
process.hionia.OneMatchedHLTMu  = cms.int32(OneMatchedHLTMu)
process.hionia.checkTrigNames   = cms.bool(False)#change this to get the event-level trigger info in hStats output (but creates lots of warnings when fake trigger names are used)
process.hionia.mom4format       = cms.string(useMomFormat)
process.hionia.isHI = cms.untracked.bool(False)


# Load B meson reconstruction
process.load("VertexCompositeAnalysis.VertexCompositeProducer.generalBDiMuMuCandidates_cff")

# Configure B meson reconstruction for PbPb
process.generalBDiMuMuCandidates.dimuonCollection = cms.InputTag("onia2MuMuPatGlbGlb")
process.generalBDiMuMuCandidates.trackRecoAlgorithm = cms.InputTag("unpackedTracksAndVertices")
process.generalBDiMuMuCandidates.vertexRecoAlgorithm = cms.InputTag("unpackedTracksAndVertices")

# Adjust cuts for PbPb environment
process.generalBDiMuMuCandidates.tkPtCut = cms.double(0.3)         # Higher track pT
process.generalBDiMuMuCandidates.dimuonPtCut = cms.double(0.0)     # J/psi pT
process.generalBDiMuMuCandidates.bPtCut = cms.double(0.0)          # B meson pT
process.generalBDiMuMuCandidates.vtxProbCut = cms.double(0.01)     # Vertex quality
process.generalBDiMuMuCandidates.rVtxSigCut = cms.double(2.0)      # Decay length significance
process.generalBDiMuMuCandidates.lVtxSigCut = cms.double(2.0)

# Enable both B+ and B0 reconstruction
process.generalBDiMuMuCandidates.doBPlus = cms.bool(True)
process.generalBDiMuMuCandidates.doBZero = cms.bool(True)
process.generalBDiMuMuCandidates.doBc = cms.bool(True)
process.generalBDiMuMuCandidates.doJPsi = cms.bool(True)

# Load B meson reconstruction
process.load("VertexCompositeAnalysis.VertexCompositeAnalyzer.bDiMuMuNtuplizer_cff")
process.bDiMuMuNtuplizer.saveBPlus = cms.bool(True)
process.bDiMuMuNtuplizer.saveBZero = cms.bool(True)
process.bDiMuMuNtuplizer.saveBc = cms.bool(True)

# Output module
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

# Output content
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('BDiMuMu_output.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_generalBDiMuMuCandidates_*_*',
        'keep *_onia2MuMuPAT_*_*',
        'keep *_offlinePrimaryVertices_*_*',
        'keep *_generalTracks_*_*',
        'keep recoBeamSpot_offlineBeamSpot_*_*',
        'keep recoVertexs_offlinePrimaryVertices_*_*',
        'keep *_centralityBin_*_*',
        'keep *_hiCentrality_*_*'
    )
)

# Define the path
process.bDiMuMu_step = cms.Path(
    process.generalBDiMuMuCandidates
)

process.oniaTreeAna = cms.Path(process.oniaTreeAna)
if miniAOD:
  from HiSkim.HiOnia2MuMu.onia2MuMuPAT_cff import changeToMiniAOD
  changeToMiniAOD(process)
  process.unpackedMuons.addPropToMuonSt = cms.bool(UsePropToMuonSt)

  if applyEventSel:
    process.oniaTreeAna.replace(process.hionia, process.beamScrapingFilter * process.hionia ) # must be called after unpacking

process.oniaTreeAna.replace(process.hionia,  process.generalBDiMuMuCandidates* process.bDiMuMuNtuplizerSequence )# * process.hionia ) # must be called after unpacking

#process.analysis_step = cms.Sequence( process.Onia2MuMuPAT + process.bDiMuMu_step )

process.output_step = cms.EndPath(process.out)

# Schedule
process.schedule = cms.Schedule(
#    process.Onia2MuMuPAT,
    process.oniaTreeAna,
#    process.bDiMuMu_step,
#    process.output_step
)


import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions24/Cert_Collisions2024_ppref_387474_387721_Muon.json').getVLuminosityBlockRange()


# Add early deletion of temporary data products to reduce memory consumption
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
