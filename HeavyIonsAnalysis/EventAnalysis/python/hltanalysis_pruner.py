import FWCore.ParameterSet.Config as cms

"""
HLT Analysis Pruner - Simple modifiers for hltanalysis configuration

Usage:
    from HeavyIonsAnalysis.EventAnalysis.hltanalysis_pruner import *
    
    # Remove specific HLT paths from dummy branches
    pruneHLTDummyBranches(process.hltanalysis, ['HLT_HIMinimumBias_part*', 'HLT_HIUPC_*'])
    
    # Keep only specific HLT paths  
    keepOnlyHLTDummyBranches(process.hltanalysis, ['HLT_HIL3DoubleMu*', 'HLT_HIL3Mu*'])
    
    # Clear all dummy branches
    clearHLTDummyBranches(process.hltanalysis)
    
    # Same for L1
    pruneL1DummyBranches(process.hltanalysis, ['L1_SingleJet*'])
"""

import fnmatch


def pruneHLTDummyBranches(hltanalyzer, patterns):
    """
    Remove HLT paths matching any of the given patterns from hltdummybranches
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module
        patterns: list of glob patterns (e.g., ['HLT_HIMinimumBias*', 'HLT_HIUPC_*'])
    """
    if not hasattr(hltanalyzer, 'hltdummybranches'):
        return
    
    current = list(hltanalyzer.hltdummybranches)
    filtered = []
    for path in current:
        if not any(fnmatch.fnmatch(path, p) for p in patterns):
            filtered.append(path)
    
    hltanalyzer.hltdummybranches = cms.vstring(filtered)
    print(f"[hltanalysis_pruner] Removed {len(current) - len(filtered)} HLT dummy branches")


def pruneL1DummyBranches(hltanalyzer, patterns):
    """
    Remove L1 paths matching any of the given patterns from l1dummybranches
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module
        patterns: list of glob patterns (e.g., ['L1_SingleJet*', 'L1_ETT*'])
    """
    if not hasattr(hltanalyzer, 'l1dummybranches'):
        return
    
    current = list(hltanalyzer.l1dummybranches)
    filtered = []
    for path in current:
        if not any(fnmatch.fnmatch(path, p) for p in patterns):
            filtered.append(path)
    
    hltanalyzer.l1dummybranches = cms.vstring(filtered)
    print(f"[hltanalysis_pruner] Removed {len(current) - len(filtered)} L1 dummy branches")


def keepOnlyHLTDummyBranches(hltanalyzer, patterns):
    """
    Keep only HLT paths matching any of the given patterns
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module
        patterns: list of glob patterns to KEEP (e.g., ['HLT_HIL3DoubleMu*'])
    """
    if not hasattr(hltanalyzer, 'hltdummybranches'):
        return
    
    current = list(hltanalyzer.hltdummybranches)
    filtered = []
    for path in current:
        if any(fnmatch.fnmatch(path, p) for p in patterns):
            filtered.append(path)
    
    hltanalyzer.hltdummybranches = cms.vstring(filtered)
    print(f"[hltanalysis_pruner] Kept {len(filtered)} HLT dummy branches (removed {len(current) - len(filtered)})")


def keepOnlyL1DummyBranches(hltanalyzer, patterns):
    """
    Keep only L1 paths matching any of the given patterns
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module  
        patterns: list of glob patterns to KEEP (e.g., ['L1_DoubleMu*'])
    """
    if not hasattr(hltanalyzer, 'l1dummybranches'):
        return
    
    current = list(hltanalyzer.l1dummybranches)
    filtered = []
    for path in current:
        if any(fnmatch.fnmatch(path, p) for p in patterns):
            filtered.append(path)
    
    hltanalyzer.l1dummybranches = cms.vstring(filtered)
    print(f"[hltanalysis_pruner] Kept {len(filtered)} L1 dummy branches (removed {len(current) - len(filtered)})")


def clearHLTDummyBranches(hltanalyzer):
    """Clear all HLT dummy branches"""
    if hasattr(hltanalyzer, 'hltdummybranches'):
        hltanalyzer.hltdummybranches = cms.vstring([])
        print("[hltanalysis_pruner] Cleared all HLT dummy branches")


def clearL1DummyBranches(hltanalyzer):
    """Clear all L1 dummy branches"""
    if hasattr(hltanalyzer, 'l1dummybranches'):
        hltanalyzer.l1dummybranches = cms.vstring([])
        print("[hltanalysis_pruner] Cleared all L1 dummy branches")


def addHLTDummyBranches(hltanalyzer, paths):
    """
    Add HLT paths to dummy branches
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module
        paths: list of HLT path names to add
    """
    if not hasattr(hltanalyzer, 'hltdummybranches'):
        hltanalyzer.hltdummybranches = cms.vstring([])
    
    current = list(hltanalyzer.hltdummybranches)
    for p in paths:
        if p not in current:
            current.append(p)
    
    hltanalyzer.hltdummybranches = cms.vstring(current)
    print(f"[hltanalysis_pruner] Added {len(paths)} HLT dummy branches")


def addL1DummyBranches(hltanalyzer, paths):
    """
    Add L1 paths to dummy branches
    
    Args:
        hltanalyzer: the hltanalysis EDAnalyzer module
        paths: list of L1 path names to add
    """
    if not hasattr(hltanalyzer, 'l1dummybranches'):
        hltanalyzer.l1dummybranches = cms.vstring([])
    
    current = list(hltanalyzer.l1dummybranches)
    for p in paths:
        if p not in current:
            current.append(p)
    
    hltanalyzer.l1dummybranches = cms.vstring(current)
    print(f"[hltanalysis_pruner] Added {len(paths)} L1 dummy branches")


def setHLTProcessName(hltanalyzer, processName):
    """Set the HLT process name (default: 'HLT')"""
    hltanalyzer.HLTProcessName = cms.string(processName)
    hltanalyzer.hltresults = cms.InputTag(f'TriggerResults::{processName}')
    print(f"[hltanalysis_pruner] Set HLT process name to '{processName}'")


def printHLTDummyBranches(hltanalyzer, maxPrint=20):
    """Print current HLT dummy branches"""
    if not hasattr(hltanalyzer, 'hltdummybranches'):
        print("[hltanalysis_pruner] No HLT dummy branches defined")
        return
    
    branches = list(hltanalyzer.hltdummybranches)
    print(f"[hltanalysis_pruner] HLT dummy branches ({len(branches)} total):")
    for i, b in enumerate(branches[:maxPrint]):
        print(f"  {b}")
    if len(branches) > maxPrint:
        print(f"  ... and {len(branches) - maxPrint} more")


def printL1DummyBranches(hltanalyzer, maxPrint=20):
    """Print current L1 dummy branches"""
    if not hasattr(hltanalyzer, 'l1dummybranches'):
        print("[hltanalysis_pruner] No L1 dummy branches defined")
        return
    
    branches = list(hltanalyzer.l1dummybranches)
    print(f"[hltanalysis_pruner] L1 dummy branches ({len(branches)} total):")
    for i, b in enumerate(branches[:maxPrint]):
        print(f"  {b}")
    if len(branches) > maxPrint:
        print(f"  ... and {len(branches) - maxPrint} more")


# =============================================================================
# Preset filter functions for common use cases
# =============================================================================

def keepOniaJetTriggers(hltanalyzer):
    """
    Keep only triggers relevant for Onia + Jet analysis:
    - MinimumBias (HLT_HIMinimumBias*)
    - Muon triggers (HLT_*Mu*, HLT_*mu*)
    - GED Photon (HLT_HIGEDPhoton*)
    - All jets (HLT_*Jet*, HLT_*jet*)
    - ZeroBias (HLT_*ZeroBias*)
    
    For L1:
    - MinimumBias (L1_MinimumBias*)
    - Muon (L1_*Mu*, L1_DoubleMu*)
    - Jets (L1_*Jet*)
    - ZeroBias (L1_ZeroBias*)
    """
    hlt_patterns = [
        'HLT_HIMinimumBias*',
        'HLT_*Mu*',
        'HLT_*mu*',
        'HLT_HIGEDPhoton*',
        'HLT_*Jet*',
        'HLT_*jet*',
        'HLT_*ZeroBias*',
        'HLT_HIZeroBias*',
    ]
    
    l1_patterns = [
        'L1_MinimumBias*',
        'L1_*Mu*',
        'L1_DoubleMu*',
        'L1_SingleMu*',
        'L1_*Jet*',
        'L1_ZeroBias*',
    ]
    
    keepOnlyHLTDummyBranches(hltanalyzer, hlt_patterns)
    keepOnlyL1DummyBranches(hltanalyzer, l1_patterns)
    print("[hltanalysis_pruner] Applied Onia+Jet trigger filter preset")
