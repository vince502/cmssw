#!/usr/bin/env python3
"""
Submit condor jobs for HiForest cmsRun jobs
Usage: python submitForestCondor.py [options]

Example:
  python submitForestCondor.py --config forest_miniAOD_OniaBmeson_DATA.py --dataset /HIPhysicsRawPrime0/... --tag MyTag
  python submitForestCondor.py --config forest_miniAOD_OniaBmeson_DATA.py --filelist files.txt --tag MyTag
"""

import os
import sys
import glob
import argparse
from datetime import datetime

# Default configuration
CMSSW_BASE = os.environ.get('CMSSW_BASE', '/afs/cern.ch/work/s/soohwan/private/Analysis/OniaTree2025/JpsiToEETest/CMSSW_15_1_0_patch3')
EOS_OUTPUT_BASE = "/eos/cms/store/group/phys_heavyions/soohwan"  # Add year/system in --tag

def get_files_from_das(dataset, limit=-1):
    """Get list of files from DAS"""
    import subprocess
    cmd = f'dasgoclient -query="file dataset={dataset}"'
    if limit > 0:
        cmd += f' -limit={limit}'
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    files = [f.strip() for f in result.stdout.strip().split('\n') if f.strip()]
    return files

def get_files_from_list(filelist):
    """Get list of files from a text file"""
    with open(filelist, 'r') as f:
        files = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    return files

def format_input_file(filepath):
    """Format input file path for cmsRun (handle EOS, /store/, local files)"""
    if filepath.startswith('/eos/cms/'):
        # EOS path - use xrootd
        return f"root://eoscms.cern.ch/{filepath}"
    elif filepath.startswith('/eos/'):
        # Other EOS path
        return f"root://eosuser.cern.ch/{filepath}"
    elif filepath.startswith('/store/'):
        # Already in correct format for xrootd
        return filepath
    elif filepath.startswith('root://'):
        # Already xrootd URL
        return filepath
    elif filepath.startswith('file:'):
        # Already has file: prefix
        return filepath
    else:
        # Assume local file
        return f"file:{filepath}"

def create_modified_config(job_dir, config_file, input_files, output_file, max_events=-1):
    """Create a modified config file with specified input/output"""
    with open(config_file, 'r') as f:
        config_content = f.read()
    
    # Find and replace fileNames
    import re
    
    # Build new fileNames block (format paths properly)
    formatted_files = [format_input_file(f) for f in input_files]
    files_str = ',\n        '.join([f"'{f}'" for f in formatted_files])
    new_filenames = f"""fileNames = cms.untracked.vstring(
        {files_str}
    )"""
    
    # Replace existing fileNames
    config_content = re.sub(
        r'fileNames\s*=\s*cms\.untracked\.vstring\([^)]*\)',
        new_filenames,
        config_content,
        flags=re.DOTALL
    )
    
    # Replace output filename
    config_content = re.sub(
        r'fileName\s*=\s*cms\.string\([^)]*\)',
        f'fileName = cms.string("{output_file}")',
        config_content
    )
    
    # Replace maxEvents if specified
    if max_events != -1:
        config_content = re.sub(
            r'input\s*=\s*cms\.untracked\.int32\([^)]*\)',
            f'input = cms.untracked.int32({max_events})',
            config_content
        )
    
    return config_content

def create_job_script(job_dir, job_id, config_file, input_files, output_eos_path, max_events=-1):
    """Create a single job script"""
    
    local_output = f"output_{job_id}.root"
    
    # Create modified config content
    modified_config = create_modified_config(job_dir, config_file, input_files, local_output, max_events)
    
    # Write modified config
    job_config_path = os.path.join(job_dir, f"config_{job_id}.py")
    with open(job_config_path, 'w') as f:
        f.write(modified_config)
    
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

# Copy config
cp {job_config_path} config.py

# Run cmsRun
echo "Running cmsRun..."
cmsRun config.py 2>&1

# Check output
if [ -f {local_output} ]; then
    echo "Output file created: {local_output}"
    ls -lh {local_output}
    
    # Copy to EOS
    echo "Copying to EOS: {output_eos_path}"
    xrdcp -f {local_output} root://eoscms.cern.ch/{output_eos_path}
    
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

def create_condor_submit(job_dir, job_scripts, proxy_path, flavour="workday"):
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
x509userproxy = {proxy_path}
use_x509userproxy = true

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
    parser = argparse.ArgumentParser(description='Submit HiForest condor jobs')
    parser.add_argument('--config', required=True, help='CMSSW config file (e.g., forest_miniAOD_OniaBmeson_DATA.py)')
    parser.add_argument('--dataset', help='DAS dataset name (e.g., /HIPhysicsRawPrime0/...)')
    parser.add_argument('--filelist', help='Text file with input file list')
    parser.add_argument('--tag', required=True, help='Project tag for output directory')
    parser.add_argument('--nFilesPerJob', type=int, default=1, help='Files per job (default: 1)')
    parser.add_argument('--maxEvents', type=int, default=50000, help='Max events per job (default: 50000, -1 = all)')
    parser.add_argument('--test', action='store_true', help='Test mode: only 3 jobs')
    parser.add_argument('--dryrun', action='store_true', help='Dry run: create scripts but do not submit')
    parser.add_argument('--flavour', default='workday', help='Condor job flavour (default: workday)')
    parser.add_argument('--limit', type=int, default=-1, help='Limit number of files from DAS (default: all)')
    parser.add_argument('--proxy', default='myProxy', help='X509 proxy certificate file (default: myProxy)')
    args = parser.parse_args()
    
    # Validate inputs
    if not args.dataset and not args.filelist:
        print("ERROR: Must specify --dataset or --filelist")
        sys.exit(1)
    
    if not os.path.exists(args.config):
        print(f"ERROR: Config file not found: {args.config}")
        sys.exit(1)
    
    # Check proxy certificate
    proxy_file = args.proxy
    if not os.path.exists(proxy_file):
        # Try default locations
        default_proxy = f"/tmp/x509up_u{os.getuid()}"
        if os.path.exists(default_proxy):
            proxy_file = default_proxy
        else:
            print(f"ERROR: Proxy certificate not found: {args.proxy}")
            print(f"       Also checked: {default_proxy}")
            print("       Run: voms-proxy-init --voms cms --out myProxy")
            sys.exit(1)
    print(f"Using proxy: {proxy_file}")
    
    # Get input files
    if args.dataset:
        print(f"Querying DAS for dataset: {args.dataset}")
        input_files = get_files_from_das(args.dataset, args.limit)
    else:
        print(f"Reading file list from: {args.filelist}")
        input_files = get_files_from_list(args.filelist)
    
    print(f"Found {len(input_files)} input files")
    
    if not input_files:
        print("No input files found!")
        return
    
    # Create job directory
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    job_dir = os.path.abspath(f"jobs_{args.tag}_{timestamp}")
    os.makedirs(job_dir, exist_ok=True)
    os.makedirs(os.path.join(job_dir, "logs"), exist_ok=True)
    
    # Copy proxy to job directory
    import shutil
    job_proxy = os.path.join(job_dir, "x509proxy")
    shutil.copy2(proxy_file, job_proxy)
    print(f"Copied proxy to: {job_proxy}")
    
    # Create output directory on EOS
    output_eos_dir = f"{EOS_OUTPUT_BASE}/{args.tag}"
    print(f"Output EOS directory: {output_eos_dir}")
    os.makedirs(output_eos_dir, exist_ok=True)
    
    # Split files into jobs
    n_files_per_job = args.nFilesPerJob
    job_file_lists = [input_files[i:i+n_files_per_job] for i in range(0, len(input_files), n_files_per_job)]
    
    if args.test:
        job_file_lists = job_file_lists[:3]
        print(f"Test mode: only {len(job_file_lists)} jobs")
    
    print(f"Creating {len(job_file_lists)} jobs ({n_files_per_job} files per job)")
    
    # Create job scripts
    config_path = os.path.abspath(args.config)
    job_scripts = []
    for job_id, files in enumerate(job_file_lists):
        output_file = f"{output_eos_dir}/HiForest_{job_id:04d}.root"
        script = create_job_script(job_dir, job_id, config_path, files, output_file, args.maxEvents)
        job_scripts.append(script)
    
    # Create submit file
    submit_file = create_condor_submit(job_dir, job_scripts, job_proxy, args.flavour)
    
    print(f"\nJob directory: {job_dir}")
    print(f"Submit file: {submit_file}")
    print(f"Total jobs: {len(job_scripts)}")
    print(f"Output: {output_eos_dir}/HiForest_XXXX.root")
    
    if args.dryrun:
        print("\nDry run - not submitting")
        print(f"To submit: condor_submit {submit_file}")
    else:
        print("\nSubmitting jobs...")
        os.system(f"condor_submit {submit_file}")

if __name__ == "__main__":
    main()
