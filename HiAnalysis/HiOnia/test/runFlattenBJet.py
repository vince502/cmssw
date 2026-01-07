#!/usr/bin/env python3
"""
runFlattenBJet.py - Run B-jet flattening on input files
Usage:
  python runFlattenBJet.py input.root output.root [maxEvents]
  python runFlattenBJet.py filelist.txt outputDir/ [maxEvents]
"""

import os
import sys
import ROOT

def run_flatten(input_file, output_file, max_events=-1):
    """Run the flattening macro on a single file"""
    
    # Load the macro
    script_dir = os.path.dirname(os.path.abspath(__file__))
    macro_path = os.path.join(script_dir, "flattenBJetMatch.C")
    
    if not os.path.exists(macro_path):
        print(f"ERROR: Cannot find macro at {macro_path}")
        return False
    
    # Compile and load the macro
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.LoadMacro(macro_path + "+")  # + for ACLiC compilation
    
    print(f"Processing: {input_file}")
    print(f"Output: {output_file}")
    print(f"Max events: {max_events}")
    
    # Run the function
    ROOT.flattenBJetMatch(input_file, output_file, max_events)
    
    return True

def process_filelist(filelist, output_dir, max_events=-1):
    """Process multiple files from a text file list"""
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    with open(filelist, 'r') as f:
        files = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    
    print(f"Found {len(files)} files to process")
    
    for i, input_file in enumerate(files):
        basename = os.path.basename(input_file).replace('.root', '_flat.root')
        output_file = os.path.join(output_dir, basename)
        
        print(f"\n[{i+1}/{len(files)}] Processing {input_file}")
        run_flatten(input_file, output_file, max_events)
    
    print(f"\nDone! Processed {len(files)} files")

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    max_events = int(sys.argv[3]) if len(sys.argv) > 3 else -1
    
    if input_path.endswith('.txt'):
        # Process file list
        process_filelist(input_path, output_path, max_events)
    else:
        # Process single file
        run_flatten(input_path, output_path, max_events)

if __name__ == "__main__":
    main()
