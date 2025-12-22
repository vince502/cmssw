#!/usr/bin/env python3
"""
Submit condor jobs to flatten OniaTree files (hioniaCombined)
Usage: python submitFlattenOnia.py [--test] [--nFilesPerJob N]
"""

import os
import sys
import glob
import argparse
from datetime import datetime

# Configuration for OO 2025
INPUT_BASE = "/eos/cms/store/group/phys_heavyions/soohwan/OO2025/Test"  # Update this to your OO HiForest output path
OUTPUT_BASE = "/eos/cms/store/group/phys_heavyions/soohwan/OO2025/FlatOniaTree_OO2025"
CMSSW_BASE = os.environ.get('CMSSW_BASE', '/afs/cern.ch/work/s/soohwan/private/Analysis/OniaTree2025/JpsiToEETest/CMSSW_15_1_0_patch3')
MACRO_PATH = f"{CMSSW_BASE}/src/HiAnalysis/HiOnia/test/flattenOniaTreeCombined.C"

def get_input_files():
    """Get list of input ROOT files"""
    pattern = f"{INPUT_BASE}/*/*.root"
    files = glob.glob(pattern)
    return sorted(files)

def create_job_script(job_dir, job_id, input_files, output_file):
    """Create a single job script"""
    script_content = f"""#!/bin/bash
echo "Starting job {job_id}"
echo "Host: $(hostname)"
echo "Date: $(date)"

# Setup environment
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {CMSSW_BASE}
eval `scramv1 runtime -sh`

# Create temp directory
TMPDIR=$(mktemp -d)
cd $TMPDIR
echo "Working in: $TMPDIR"

# Copy macro
cp {MACRO_PATH} .

# Process files
"""
    
    # Add hadd if multiple files, otherwise direct processing
    if len(input_files) == 1:
        script_content += f"""
# Single file - process directly
INPUT_FILE="{input_files[0]}"
OUTPUT_FILE="flat_output.root"

root -l -b -q 'flattenOniaTreeCombined.C("'$INPUT_FILE'", "'$OUTPUT_FILE'")'
"""
    else:
        # Multiple files - process each and hadd
        script_content += """
# Multiple files - process each
"""
        for i, f in enumerate(input_files):
            script_content += f"""
echo "Processing file {i+1}/{len(input_files)}: {f}"
root -l -b -q 'flattenOniaTreeCombined.C("{f}", "flat_{i}.root")' || echo "Failed: {f}"
"""
        
        script_content += f"""
# Merge output files
echo "Merging {len(input_files)} output files..."
hadd -f flat_output.root flat_*.root
"""
    
    script_content += f"""
# Copy output
echo "Copying output to {output_file}"
xrdcp -f flat_output.root root://eoscms.cern.ch/{output_file}

# Cleanup
cd /
rm -rf $TMPDIR

echo "Job {job_id} completed"
date
"""
    
    script_path = os.path.join(job_dir, f"job_{job_id}.sh")
    with open(script_path, 'w') as f:
        f.write(script_content)
    os.chmod(script_path, 0o755)
    return script_path

def create_condor_submit(job_dir, job_scripts):
    """Create condor submit file"""
    submit_content = f"""universe = vanilla
executable = $(script)
output = {job_dir}/logs/$(ClusterId).$(ProcId).out
error = {job_dir}/logs/$(ClusterId).$(ProcId).err
log = {job_dir}/logs/condor.log
+JobFlavour = "workday"
transfer_output_files = ""
request_memory = 4000
request_cpus = 1
requirements = (OpSysAndVer == "AlmaLinux9")
+AccountingGroup = "group_u_CMS.u_zh.users"

queue script from (
"""
    for script in job_scripts:
        submit_content += f"    {script}\n"
    submit_content += ")\n"
    
    submit_path = os.path.join(job_dir, "submit.sub")
    with open(submit_path, 'w') as f:
        f.write(submit_content)
    return submit_path

def main():
    parser = argparse.ArgumentParser(description='Submit flatten jobs')
    parser.add_argument('--test', action='store_true', help='Test mode: only 5 jobs')
    parser.add_argument('--nFilesPerJob', type=int, default=10, help='Files per job (default: 10)')
    parser.add_argument('--dryrun', action='store_true', help='Dry run: create scripts but do not submit')
    args = parser.parse_args()
    
    # Get input files
    input_files = get_input_files()
    print(f"Found {len(input_files)} input files")
    
    if not input_files:
        print("No input files found!")
        return
    
    # Create job directory
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    job_dir = f"jobs_flattenOnia_{timestamp}"
    os.makedirs(job_dir, exist_ok=True)
    os.makedirs(os.path.join(job_dir, "logs"), exist_ok=True)
    
    # Create output directory
    os.makedirs(OUTPUT_BASE, exist_ok=True)
    
    # Split files into jobs
    n_files_per_job = args.nFilesPerJob
    job_file_lists = [input_files[i:i+n_files_per_job] for i in range(0, len(input_files), n_files_per_job)]
    
    if args.test:
        job_file_lists = job_file_lists[:5]
        print(f"Test mode: only {len(job_file_lists)} jobs")
    
    print(f"Creating {len(job_file_lists)} jobs ({n_files_per_job} files per job)")
    
    # Create job scripts
    job_scripts = []
    for job_id, files in enumerate(job_file_lists):
        output_file = f"{OUTPUT_BASE}/FlatOnia_{job_id:04d}.root"
        script = create_job_script(job_dir, job_id, files, output_file)
        job_scripts.append(script)
    
    # Create submit file
    submit_file = create_condor_submit(job_dir, job_scripts)
    
    print(f"\nJob directory: {job_dir}")
    print(f"Submit file: {submit_file}")
    print(f"Total jobs: {len(job_scripts)}")
    
    if args.dryrun:
        print("\nDry run - not submitting")
        print(f"To submit: condor_submit {submit_file}")
    else:
        print("\nSubmitting jobs...")
        os.system(f"condor_submit {submit_file}")

if __name__ == "__main__":
    main()
