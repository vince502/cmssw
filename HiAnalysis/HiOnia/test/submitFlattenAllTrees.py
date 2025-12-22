#!/usr/bin/env python3
"""
Submit condor jobs to flatten HiForest files using flattenAllTrees.C
Usage: python submitFlattenAllTrees.py --filelist filelist_OO_OniaForest.txt --tag OO2025_FlatTrees --nFilesPerJob 100
"""

import os
import sys
import argparse
import shutil
from datetime import datetime

CMSSW_BASE = os.environ.get('CMSSW_BASE', '/afs/cern.ch/work/s/soohwan/private/Analysis/OniaTree2025/JpsiToEETest/CMSSW_15_1_0_patch3')
EOS_OUTPUT_BASE = "/eos/cms/store/group/phys_heavyions/soohwan"
MACRO_PATH = f"{CMSSW_BASE}/src/HiAnalysis/HiOnia/test/flattenAllTrees.C"

def get_files_from_list(filelist):
    """Get list of files from a text file"""
    with open(filelist, 'r') as f:
        files = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    return files

def create_job_script(job_dir, job_id, input_files, output_file, lepton_pt_cut, jet_pt_cut):
    """Create a single job script"""
    
    # Build file list for TChain
    files_str = '", "'.join(input_files)
    
    script_content = f"""#!/bin/bash
echo "Starting flatten job {job_id}"
echo "Host: $(hostname)"
echo "Date: $(date)"
echo "Processing {len(input_files)} input files"

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

# Create ROOT script to process multiple files with TChain
cat > runFlatten.C << 'ROOTSCRIPT'
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>

void runFlatten() {{
    // Input files
    std::vector<std::string> inputFiles = {{
        "{files_str}"
    }};
    
    // Load the flattener
    gROOT->ProcessLine(".L flattenAllTrees.C+");
    
    // Process each file and merge
    std::vector<std::string> tempOutputs;
    
    for (size_t i = 0; i < inputFiles.size(); i++) {{
        std::string inFile = inputFiles[i];
        std::string outFile = Form("temp_%zu.root", i);
        
        std::cout << "Processing file " << i+1 << "/" << inputFiles.size() << ": " << inFile << std::endl;
        
        // Run flattener
        gROOT->ProcessLine(Form("flattenAllTrees(\\"%s\\", \\"%s\\", {lepton_pt_cut}, {jet_pt_cut})", inFile.c_str(), outFile.c_str()));
        
        if (gSystem->AccessPathName(outFile.c_str()) == 0) {{
            tempOutputs.push_back(outFile);
        }}
    }}
    
    // Merge outputs
    if (tempOutputs.size() > 0) {{
        std::cout << "Merging " << tempOutputs.size() << " output files..." << std::endl;
        
        TFileMerger merger;
        merger.OutputFile("flat_output.root");
        for (const auto& f : tempOutputs) {{
            merger.AddFile(f.c_str());
        }}
        merger.Merge();
        
        std::cout << "Merged to flat_output.root" << std::endl;
    }}
}}
ROOTSCRIPT

# Run ROOT
root -l -b -q runFlatten.C

# Copy output
if [ -f flat_output.root ]; then
    echo "Output file created"
    ls -lh flat_output.root
    
    echo "Copying to EOS: {output_file}"
    xrdcp -f flat_output.root root://eoscms.cern.ch/{output_file}
    
    if [ $? -eq 0 ]; then
        echo "Successfully copied to EOS"
    else
        echo "ERROR: Failed to copy to EOS"
        exit 1
    fi
else
    echo "ERROR: Output file not found!"
    ls -la
    exit 1
fi

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

def create_condor_submit(job_dir, job_scripts, flavour="workday"):
    """Create condor submit file"""
    submit_content = f"""universe = vanilla
executable = $(script)
output = {job_dir}/logs/$(ClusterId).$(ProcId).out
error = {job_dir}/logs/$(ClusterId).$(ProcId).err
log = {job_dir}/logs/condor.log
+JobFlavour = "{flavour}"
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
    parser.add_argument('--filelist', required=True, help='Text file with input file list')
    parser.add_argument('--tag', required=True, help='Output tag (e.g., OO2025/FlatTrees)')
    parser.add_argument('--nFilesPerJob', type=int, default=100, help='Files per job (default: 100)')
    parser.add_argument('--nJobs', type=int, default=-1, help='Max number of jobs (default: all)')
    parser.add_argument('--leptonPtCut', type=float, default=2.0, help='Lepton pT cut in GeV (default: 2.0)')
    parser.add_argument('--jetPtCut', type=float, default=30.0, help='Jet pT cut in GeV (default: 30.0)')
    parser.add_argument('--flavour', default='workday', help='Condor job flavour (default: workday)')
    parser.add_argument('--dryrun', action='store_true', help='Dry run: create scripts but do not submit')
    args = parser.parse_args()
    
    # Check filelist
    if not os.path.exists(args.filelist):
        print(f"ERROR: File list not found: {args.filelist}")
        sys.exit(1)
    
    # Proxy is optional - files are on EOS accessible via AFS token
    proxy_file = None
    
    # Get input files
    input_files = get_files_from_list(args.filelist)
    print(f"Found {len(input_files)} input files")
    
    if not input_files:
        print("No input files found!")
        return
    
    # Create job directory
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    job_dir = os.path.abspath(f"jobs_flatten_{args.tag.replace('/', '_')}_{timestamp}")
    os.makedirs(job_dir, exist_ok=True)
    os.makedirs(os.path.join(job_dir, "logs"), exist_ok=True)
    
    
    # Create output directory on EOS
    output_eos_dir = f"{EOS_OUTPUT_BASE}/{args.tag}"
    print(f"Output EOS directory: {output_eos_dir}")
    os.makedirs(output_eos_dir, exist_ok=True)
    
    # Split files into jobs
    n_files_per_job = args.nFilesPerJob
    job_file_lists = [input_files[i:i+n_files_per_job] for i in range(0, len(input_files), n_files_per_job)]
    
    if args.nJobs > 0:
        job_file_lists = job_file_lists[:args.nJobs]
        print(f"Limited to {len(job_file_lists)} jobs")
    
    print(f"Creating {len(job_file_lists)} jobs ({n_files_per_job} files per job)")
    print(f"Lepton pT cut: {args.leptonPtCut} GeV")
    print(f"Jet pT cut: {args.jetPtCut} GeV")
    
    # Create job scripts
    job_scripts = []
    for job_id, files in enumerate(job_file_lists):
        output_file = f"{output_eos_dir}/FlatAllTrees_{job_id:04d}.root"
        script = create_job_script(job_dir, job_id, files, output_file, args.leptonPtCut, args.jetPtCut)
        job_scripts.append(script)
    
    # Create submit file
    submit_file = create_condor_submit(job_dir, job_scripts, args.flavour)
    
    print(f"\nJob directory: {job_dir}")
    print(f"Submit file: {submit_file}")
    print(f"Total jobs: {len(job_scripts)}")
    print(f"Output: {output_eos_dir}/FlatAllTrees_XXXX.root")
    
    if args.dryrun:
        print("\nDry run - not submitting")
        print(f"To submit: condor_submit {submit_file}")
    else:
        print("\nSubmitting jobs...")
        os.system(f"condor_submit {submit_file}")

if __name__ == "__main__":
    main()
