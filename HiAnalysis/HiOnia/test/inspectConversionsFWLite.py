#!/usr/bin/env python3
# inspectConversionsFWLite.py - Inspect oniaPhotonCandidates using FWLite
# Usage: python3 inspectConversionsFWLite.py

import ROOT
ROOT.gROOT.SetBatch(True)

# Load FWLite libraries
ROOT.gSystem.Load("libFWCoreFWLite.so")
ROOT.FWLiteEnabler.enable()
ROOT.gSystem.Load("libDataFormatsFWLite.so")
ROOT.gSystem.Load("libDataFormatsPatCandidates.so")

from DataFormats.FWLite import Events, Handle

# Input file
inputFile = 'root://cms-xrd-global.cern.ch//store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root'

events = Events(inputFile)

# Handle for oniaPhotonCandidates
convHandle = Handle('std::vector<pat::CompositeCandidate>')
convLabel = ('oniaPhotonCandidates', 'conversions', 'RECO')

nEventsWithConv = 0
maxEvents = 100
maxShow = 3

for i, event in enumerate(events):
    if i >= maxEvents:
        break
    
    event.getByLabel(convLabel, convHandle)
    
    if convHandle.isValid():
        convCands = convHandle.product()
        
        if convCands.size() > 0:
            nEventsWithConv += 1
            
            if nEventsWithConv <= maxShow:
                print(f"\n=== Event {i} has {convCands.size()} conversion candidates ===")
                
                for j in range(convCands.size()):
                    cand = convCands.at(j)
                    print(f"  Cand {j}: pT={cand.pt():.3f} eta={cand.eta():.3f} phi={cand.phi():.3f} mass={cand.mass():.5f}")
                    print(f"    nDaughters: {cand.numberOfDaughters()}")
                    
                    # Print userFloat names
                    ufNames = cand.userFloatNames()
                    print(f"    userFloats ({len(ufNames)}): ", end="")
                    for name in ufNames:
                        print(f"{name}={cand.userFloat(name):.4f} ", end="")
                    print()
                    
                    # Print userInt names  
                    uiNames = cand.userIntNames()
                    print(f"    userInts ({len(uiNames)}): ", end="")
                    for name in uiNames:
                        print(f"{name}={cand.userInt(name)} ", end="")
                    print()

print(f"\nTotal events with conversions in first {maxEvents}: {nEventsWithConv}")
