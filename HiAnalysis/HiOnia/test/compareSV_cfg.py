### Compare Secondary Vertex Collections
# This config:
#   1. Loads MiniAOD input
#   2. Re-runs SV finding to create inclusiveCandidateSecondaryVertices
#   3. Compares slimmedSecondaryVertices (MiniAOD) vs inclusiveCandidateSecondaryVertices (custom)
#
# Usage: 
#   OO 2025:   cmsRun compareSV_cfg.py maxEvents=1000
#   PbPb 2024: cmsRun compareSV_cfg.py maxEvents=1000 era=PbPb globalTag=151X_dataRun3_Prompt_v2

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
from FWCore.ParameterSet.VarParsing import VarParsing

# Command line options
options = VarParsing('analysis')
options.maxEvents = 1000
options.outputFile = 'SVComparison.root'
options.register('era', 'OO',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Era: OO (Run3_2025_OXY), PbPb (Run3_pp_on_PbPb_2024)"
)
options.register('globalTag', '',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Global tag (auto-selected if empty)"
)
options.parseArguments()

###############################################################################
# Era and Global Tag Configuration
###############################################################################
# Available eras:
#   OO   -> Run3_2025_OXY      (OO 2025)
#   PbPb -> Run3_pp_on_PbPb_2024 (PbPb 2024)
#   pp   -> Run3               (pp reference)
#
# Default Global Tags:
#   OO:   150X_dataRun3_Prompt_v3
#   PbPb: 151X_dataRun3_Prompt_v2
#   pp:   150X_dataRun3_Prompt_v3

eraConfig = {
    'OO':   {'era': eras.Run3_2025_OXY,       'gt': '150X_dataRun3_Prompt_v3'},
    'PbPb': {'era': eras.Run3_pp_on_PbPb_2025, 'gt': '151X_dataRun3_Prompt_v1'},
    'pp':   {'era': eras.Run3,                'gt': '150X_dataRun3_Prompt_v3'},
}

if options.era not in eraConfig:
    raise ValueError(f"Unknown era '{options.era}'. Choose from: {list(eraConfig.keys())}")

selectedEra = eraConfig[options.era]['era']
selectedGT = options.globalTag if options.globalTag else eraConfig[options.era]['gt']

print(f"=== Configuration ===")
print(f"Era: {options.era}")
print(f"Global Tag: {selectedGT}")
print(f"=====================")

process = cms.Process('SVComparison', selectedEra)

###############################################################################
# Input
###############################################################################
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
#        '/store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root',
'/store/hidata/HIRun2025A/HIPhysicsRawPrime0/MINIAOD/PromptReco-v1/000/399/661/00000/df4f3809-067a-4f7a-bf02-f3900a58523f.root',
    ),
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(12),
    numberOfStreams = cms.untracked.uint32(6),
    wantSummary = cms.untracked.bool(True),
)

###############################################################################
# Load Global Tag, geometry, etc.
###############################################################################
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, selectedGT, '')

###############################################################################
# TransientTrack Builder (needed for vertex fitting)
###############################################################################
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")

###############################################################################
# Event Filters (remove garbage events)
###############################################################################
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')

# Combined filter sequence
process.eventFilter = cms.Sequence(
    process.primaryVertexFilter +
    process.clusterCompatibilityFilter
)

###############################################################################
# Output
###############################################################################
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

###############################################################################
# Secondary Vertex Re-creation from MiniAOD tracks
###############################################################################
# This recreates the full SV chain from packed candidates
# Using same settings as forest (setupJets_PbPb_cff.py candidateBtaggingMiniAOD)

from RecoVertex.AdaptiveVertexFinder.inclusiveVertexing_cff import (
    inclusiveCandidateVertexFinder, 
    candidateVertexMerger, 
    candidateVertexArbitrator, 
    inclusiveCandidateSecondaryVertices
)

# Step 1: Inclusive Candidate Vertex Finder (same as forest)
process.inclusiveCandidateVertexFinder = inclusiveCandidateVertexFinder.clone(
    tracks = "packedPFCandidates",
    primaryVertices = "offlineSlimmedPrimaryVertices",
    minHits = 0,   # Same as forest
    minPt = 0.8    # Same as forest
)

# Step 2: Candidate Vertex Merger (default)
process.candidateVertexMerger = candidateVertexMerger.clone()

# Step 3: Candidate Vertex Arbitrator (same as forest)
process.candidateVertexArbitrator = candidateVertexArbitrator.clone(
    tracks = "packedPFCandidates",
    primaryVertices = "offlineSlimmedPrimaryVertices"
)

# Step 4: Inclusive Candidate Secondary Vertices (default)
process.inclusiveCandidateSecondaryVertices = inclusiveCandidateSecondaryVertices.clone()

# SV Task
process.svTask = cms.Task(
    process.inclusiveCandidateVertexFinder,
    process.candidateVertexMerger,
    process.candidateVertexArbitrator,
    process.inclusiveCandidateSecondaryVertices
)

###############################################################################
# SV Comparison Analyzer
###############################################################################
process.load('HeavyIonsAnalysis.TrackAnalysis.secondaryVertexComparison_cfi')

# Configure the comparison
process.secondaryVertexComparison.svSrc1 = cms.InputTag("slimmedSecondaryVertices")
process.secondaryVertexComparison.svSrc2 = cms.InputTag("inclusiveCandidateSecondaryVertices")
process.secondaryVertexComparison.pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices")
process.secondaryVertexComparison.label1 = cms.string("MiniAOD_slimmed")
process.secondaryVertexComparison.label2 = cms.string("Custom_remade")
process.secondaryVertexComparison.maxDeltaR = cms.double(0.3)
process.secondaryVertexComparison.maxDeltaZ = cms.double(1.0)

###############################################################################
# Path and Schedule
###############################################################################
process.svPath = cms.Path(
    process.eventFilter +
    process.secondaryVertexComparison
)
process.svPath.associate(process.svTask)

process.schedule = cms.Schedule(process.svPath)

###############################################################################
# Summary
###############################################################################
# Output tree (secondaryVertexComparison/svTree):
#
# Event info: run, lumi, event
#
# Per-event comparison:
#   nSV1, nSV2          - Number of SVs in each collection
#   nMatched            - Number of matched pairs
#   matchFraction1/2    - Fraction of SVs that matched
#
# Collection 1 (MiniAOD) statistics:
#   sv1_pt_mean, sv1_pt_std, sv1_mass_mean, sv1_mass_std
#   sv1_chi2_mean, sv1_ndof_mean, sv1_ntrk_mean
#   sv1_dxy_mean, sv1_dxySig_mean
#
# Collection 2 (Custom) statistics:
#   sv2_pt_mean, sv2_pt_std, sv2_mass_mean, sv2_mass_std
#   sv2_chi2_mean, sv2_ndof_mean, sv2_ntrk_mean
#   sv2_dxy_mean, sv2_dxySig_mean
#
# Matched pair differences (means):
#   matched_dPt_mean, matched_dMass_mean, matched_dR_mean, matched_dZ_mean
#
# Per-SV vectors for both collections:
#   sv{1,2}_pt, sv{1,2}_eta, sv{1,2}_phi, sv{1,2}_mass
#   sv{1,2}_x, sv{1,2}_y, sv{1,2}_z
#   sv{1,2}_chi2, sv{1,2}_ndof, sv{1,2}_ntracks
#   sv{1,2}_dxy, sv{1,2}_dxyErr, sv{1,2}_dxySig
#   sv{1,2}_matchIdx (-1 if unmatched, index in other collection if matched)
#
# Histograms:
#   h_nSV1, h_nSV2      - nSV distributions
#   h_nSVDiff           - nSV1 - nSV2
#   h_nSV_corr          - 2D correlation
#   h_matchedDR         - DeltaR of matched pairs
#   h_matchedDPt        - DeltaPt of matched pairs
#   h_matchedDMass      - DeltaMass of matched pairs
###############################################################################
