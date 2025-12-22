#!/usr/bin/env python3
"""
Submit condor jobs to flatten hionia and hioniaElectrons trees
Usage: 
  python submitFlattenOniaTrees.py --input /eos/path/to/files --output /eos/path/to/output [--test] [--nFilesPerJob N]

Example:
  python submitFlattenOniaTrees.py \
    --input /eos/cms/store/group/phys_heavyions/soohwan/OniaForest/Run2025OO/OO_OniaForest_10kFilePromptReco_08Dec2025_v1/OxygenCombinedAna/crab_OO_OniaForest_10kFilePromptReco_08Dec2025_v1/251208_020603/0000 \
    --output /eos/cms/store/group/phys_heavyions/soohwan/OniaForest/Run2025OO/FlatOniaTrees_OO2025 \
    --test
"""

import os
import sys
import glob
import argparse
from datetime import datetime

def get_input_files(input_path, pattern="*.root"):
    """Get list of input ROOT files"""
    if os.path.isfile(input_path):
        # Single file
        return [input_path]
    elif os.path.isdir(input_path):
        # Directory - find all ROOT files
        files = glob.glob(os.path.join(input_path, pattern))
        # Also check subdirectories
        files += glob.glob(os.path.join(input_path, "*", pattern))
        return sorted(files)
    else:
        # Treat as glob pattern
        return sorted(glob.glob(input_path))

def create_job_script(job_dir, job_id, input_files, output_file, cmssw_base, macro_dir):
    """Create a single job script"""
    script_content = f"""#!/bin/bash
echo "=========================================="
echo "Starting job {job_id}"
echo "Host: $(hostname)"
echo "Date: $(date)"
echo "=========================================="

# Setup environment
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {cmssw_base}
eval `scramv1 runtime -sh`

# Create temp directory
TMPDIR=$(mktemp -d)
cd $TMPDIR
echo "Working in: $TMPDIR"

# Copy macro and header
cp {macro_dir}/flattenOniaTrees.C .
cp {macro_dir}/HiForestBranches.h .

"""
    
    if len(input_files) == 1:
        # Single file - process directly
        script_content += f"""
# Single file - process directly
INPUT_FILE="{input_files[0]}"
OUTPUT_FILE="FlatOniaTrees.root"

echo "Processing: $INPUT_FILE"
root -l -b -q 'flattenOniaTrees.C("'$INPUT_FILE'", "'$OUTPUT_FILE'")'
RETVAL=$?

if [ $RETVAL -ne 0 ]; then
    echo "ERROR: ROOT macro failed with return code $RETVAL"
    exit 1
fi
"""
    else:
        # Multiple files - process each and hadd
        script_content += f"""
# Multiple files - process each
NFILES={len(input_files)}
PROCESSED=0
FAILED=0

"""
        for i, f in enumerate(input_files):
            script_content += f"""
echo "Processing file {i+1}/{len(input_files)}: {os.path.basename(f)}"
root -l -b -q 'flattenOniaTrees.C("{f}", "flat_{i}.root")' 
if [ $? -eq 0 ]; then
    ((PROCESSED++))
else
    echo "WARNING: Failed to process {os.path.basename(f)}"
    ((FAILED++))
fi
"""
        
        script_content += f"""
echo "Processed: $PROCESSED/{len(input_files)}, Failed: $FAILED"

# Merge output files
OUTPUT_FILES=$(ls flat_*.root 2>/dev/null)
if [ -z "$OUTPUT_FILES" ]; then
    echo "ERROR: No output files to merge"
    exit 1
fi

echo "Merging output files..."
hadd -f FlatOniaTrees.root flat_*.root
RETVAL=$?

if [ $RETVAL -ne 0 ]; then
    echo "ERROR: hadd failed"
    exit 1
fi
"""
    
    script_content += f"""
# Check output
if [ ! -f FlatOniaTrees.root ]; then
    echo "ERROR: Output file not created"
    exit 1
fi

ls -lh FlatOniaTrees.root

# Copy output to EOS
echo "Copying output to {output_file}"
xrdcp -f FlatOniaTrees.root root://eoscms.cern.ch/{output_file}
RETVAL=$?

if [ $RETVAL -ne 0 ]; then
    echo "ERROR: xrdcp failed"
    # Try alternative copy
    cp FlatOniaTrees.root {output_file}
fi

# Cleanup
cd /
rm -rf $TMPDIR

echo "=========================================="
echo "Job {job_id} completed"
echo "Date: $(date)"
echo "=========================================="
"""
    
    script_path = os.path.join(job_dir, f"job_{job_id:04d}.sh")
    with open(script_path, 'w') as f:
        f.write(script_content)
    os.chmod(script_path, 0o755)
    return script_path

def create_condor_submit(job_dir, job_scripts, flavour="workday"):
    """Create condor submit file"""
    submit_content = f"""universe = vanilla
executable = $(script)
output = {job_dir}/logs/job_$(ProcId).out
error = {job_dir}/logs/job_$(ProcId).err
log = {job_dir}/logs/condor.log

+JobFlavour = "{flavour}"
request_memory = 4000
request_cpus = 1
transfer_output_files = ""

# For lxplus
requirements = (OpSysAndVer == "AlmaLinux9")
+AccountingGroup = "group_u_CMS.u_zh.users"

queue script from (
"""
    for script in job_scripts:
        submit_content += f"  {script}\n"
    submit_content += ")\n"
    
    submit_path = os.path.join(job_dir, "submit.sub")
    with open(submit_path, 'w') as f:
        f.write(submit_content)
    return submit_path

def main():
    parser = argparse.ArgumentParser(description='Submit flatten jobs for hionia/hioniaElectrons trees')
    parser.add_argument('--input', required=True, help='Input path (file, directory, or glob pattern)')
    parser.add_argument('--output', required=True, help='Output directory on EOS')
    parser.add_argument('--test', action='store_true', help='Test mode: only first 3 jobs')
    parser.add_argument('--nFilesPerJob', type=int, default=5, help='Files per job (default: 5)')
    parser.add_argument('--dryrun', action='store_true', help='Create scripts but do not submit')
    parser.add_argument('--flavour', default='workday', help='Job flavour (default: workday)')
    parser.add_argument('--pattern', default='*.root', help='File pattern (default: *.root)')
    args = parser.parse_args()
    
    # Get CMSSW environment
    cmssw_base = os.environ.get('CMSSW_BASE')
    if not cmssw_base:
        print("ERROR: CMSSW environment not set. Run 'cmsenv' first.")
        sys.exit(1)
    
    macro_dir = os.path.join(cmssw_base, "src/HiAnalysis/HiOnia/test")
    
    # Check macro exists
    if not os.path.exists(os.path.join(macro_dir, "flattenOniaTrees.C")):
        print(f"ERROR: flattenOniaTrees.C not found in {macro_dir}")
        sys.exit(1)
    
    if not os.path.exists(os.path.join(macro_dir, "HiForestBranches.h")):
        print(f"ERROR: HiForestBranches.h not found in {macro_dir}")
        sys.exit(1)
    
    # Get input files
    input_files = get_input_files(args.input, args.pattern)
    print(f"Found {len(input_files)} input files")
    
    if not input_files:
        print("ERROR: No input files found!")
        print(f"  Input path: {args.input}")
        print(f"  Pattern: {args.pattern}")
        sys.exit(1)
    
    # Show first few files
    print("First files:")
    for f in input_files[:3]:
        print(f"  {f}")
    if len(input_files) > 3:
        print(f"  ... and {len(input_files)-3} more")
    
    # Create job directory
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    job_dir = os.path.join(macro_dir, f"jobs_flattenOniaTrees_{timestamp}")
    os.makedirs(job_dir, exist_ok=True)
    os.makedirs(os.path.join(job_dir, "logs"), exist_ok=True)
    
    # Create output directory on EOS
    output_base = args.output
    if not output_base.startswith("/eos"):
        print(f"WARNING: Output path doesn't start with /eos: {output_base}")
    
    os.makedirs(output_base, exist_ok=True)
    
    # Split files into jobs
    n_files_per_job = args.nFilesPerJob
    job_file_lists = [input_files[i:i+n_files_per_job] for i in range(0, len(input_files), n_files_per_job)]
    
    if args.test:
        job_file_lists = job_file_lists[:3]
        print(f"TEST MODE: Only {len(job_file_lists)} jobs")
    
    print(f"\nCreating {len(job_file_lists)} jobs ({n_files_per_job} files per job)")
    
    # Create job scripts
    job_scripts = []
    for job_id, files in enumerate(job_file_lists):
        output_file = os.path.join(output_base, f"FlatOniaTrees_{job_id:04d}.root")
        script = create_job_script(job_dir, job_id, files, output_file, cmssw_base, macro_dir)
        job_scripts.append(script)
    
    # Create submit file
    submit_file = create_condor_submit(job_dir, job_scripts, args.flavour)
    
    # Save file list
    filelist_path = os.path.join(job_dir, "input_files.txt")
    with open(filelist_path, 'w') as f:
        for ff in input_files:
            f.write(ff + "\n")
    
    print(f"\n{'='*50}")
    print(f"Job directory: {job_dir}")
    print(f"Submit file:   {submit_file}")
    print(f"Total jobs:    {len(job_scripts)}")
    print(f"Output dir:    {output_base}")
    print(f"{'='*50}")
    
    if args.dryrun:
        print("\nDRY RUN - not submitting")
        print(f"\nTo submit manually:")
        print(f"  condor_submit {submit_file}")
    else:
        print("\nSubmitting jobs...")
        ret = os.system(f"condor_submit {submit_file}")
        if ret == 0:
            print("\nJobs submitted successfully!")
            print(f"Check status: condor_q")
            print(f"Check logs:   ls {job_dir}/logs/")
        else:
            print(f"\nERROR: condor_submit failed with code {ret}")

if __name__ == "__main__":
    main()
