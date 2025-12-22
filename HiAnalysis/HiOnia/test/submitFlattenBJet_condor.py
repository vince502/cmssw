#!/usr/bin/env python3
"""
Condor submission script for mass production of flat BJet trees.

Usage (by files):
    python submitFlattenBJet_condor.py --input filelist.txt --output /eos/.../FlatBJet/OO2025_v1/ --filesPerJob 10
    
Usage (by events):
    python submitFlattenBJet_condor.py --input filelist.txt --output /eos/.../FlatBJet/OO2025_v1/ --eventsPerJob 100000
    
Or for EOS input:
    python submitFlattenBJet_condor.py --input /eos/cms/store/... --output /eos/user/... --eventsPerJob 50000
"""

import os
import sys
import argparse
import subprocess
from pathlib import Path

def get_files_from_directory(directory, pattern="*.root"):
    """Get list of ROOT files from a directory (supports EOS)."""
    if directory.startswith("/eos"):
        cmd = f"eos ls {directory}"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        files = [f"{directory}/{f.strip()}" for f in result.stdout.split('\n') if f.strip().endswith('.root')]
    else:
        files = list(Path(directory).glob(pattern))
        files = [str(f) for f in files]
    return sorted(files)

def get_files_from_filelist(filelist):
    """Read files from a text file."""
    files = []
    with open(filelist, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                files.append(line)
    return files

def split_files(files, n_per_job):
    """Split file list into chunks."""
    return [files[i:i + n_per_job] for i in range(0, len(files), n_per_job)]

def create_job_script(cmssw_base, macro_path, jet_tree, output_dir):
    """Create the executable script for condor jobs."""
    script = f"""#!/bin/bash
# Condor job script for flattenBJetTree

# Arguments
FILELIST=$1
JOBINDEX=$2
FIRSTEVENT=$3
MAXEVENTS=$4

echo "Starting job at $(date)"
echo "Running on $(hostname)"
echo "Input: $FILELIST"
echo "Job index: $JOBINDEX"
echo "First event: $FIRSTEVENT"
echo "Max events: $MAXEVENTS"
echo "Output dir: {output_dir}"

# Setup CMSSW
cd {cmssw_base}/src
source /cvmfs/cms.cern.ch/cmsset_default.sh
eval `scramv1 runtime -sh`

# For EOS access
export XRD_NETWORKSTACK=IPv4
export EOS_MGM_URL=root://eoscms.cern.ch

# Run flattening (output goes directly to EOS)
cd {os.path.dirname(macro_path)}
root -l -b -q 'flattenBJetTree.C("'$FILELIST'", "{output_dir}", '$JOBINDEX', true, "{jet_tree}", '$FIRSTEVENT', '$MAXEVENTS')'

# Check output on EOS
OUTPUT_FILE="{output_dir}/FlatBJetTree_$(printf '%04d' $JOBINDEX).root"
if eos ls $OUTPUT_FILE > /dev/null 2>&1; then
    echo "Success! Output: $OUTPUT_FILE"
    eos ls -l $OUTPUT_FILE
else
    echo "Error: Output not created at $OUTPUT_FILE"
    exit 1
fi

echo "Job finished at $(date)"
"""
    return script

def create_condor_submit(job_name, executable, log_dir, jobs_info, filelist_dir):
    """Create condor submit file with single queue statement."""
    
    # Write job parameters to a file
    args_file = f"{filelist_dir}/job_args.txt"
    with open(args_file, 'w') as f:
        for filelist, job_index, first_event, max_events in jobs_info:
            f.write(f"{filelist} {job_index} {first_event} {max_events}\n")
    
    submit = f"""# Condor submit file for {job_name}
universe = vanilla
executable = {executable}
output = {log_dir}/$(Cluster)_$(Process).out
error = {log_dir}/$(Cluster)_$(Process).err
log = {log_dir}/$(Cluster)_$(Process).log

+JobFlavour = "longlunch"
request_cpus = 1
request_memory = 4000

# For EOS access
+AccountingGroup = "group_u_CMS.CAF.PHYS"

arguments = $(filelist) $(jobindex) $(firstevent) $(maxevents)
queue filelist,jobindex,firstevent,maxevents from {args_file}
"""
    return submit

def main():
    parser = argparse.ArgumentParser(description='Submit condor jobs for BJet tree flattening')
    parser.add_argument('--input', required=True, help='Input filelist or directory')
    parser.add_argument('--output', required=True, help='Output directory (EOS supported)')
    parser.add_argument('--filesPerJob', type=int, default=1, help='Number of files per job (default: 1)')
    parser.add_argument('--maxEvents', type=int, default=-1, help='Max events per job, -1 = all (default: -1)')
    parser.add_argument('--jetTree', default='akCs4PFJetAnalyzer/t', help='Jet tree name')
    parser.add_argument('--dryRun', action='store_true', help='Do not submit, just prepare')
    parser.add_argument('--jobName', default='flattenBJet', help='Job name prefix')
    args = parser.parse_args()
    
    # Get CMSSW base
    cmssw_base = os.environ.get('CMSSW_BASE')
    if not cmssw_base:
        print("Error: CMSSW environment not set!")
        sys.exit(1)
    
    # Get input files
    if os.path.isfile(args.input):
        files = get_files_from_filelist(args.input)
    elif os.path.isdir(args.input) or args.input.startswith('/eos'):
        files = get_files_from_directory(args.input)
    else:
        print(f"Error: {args.input} not found!")
        sys.exit(1)
    
    print(f"Found {len(files)} input files")
    
    # Create output directory (handle EOS)
    if args.output.startswith('/eos'):
        os.system(f"eos mkdir -p {args.output}")
        # Use local dir for job files
        local_job_dir = f"{cmssw_base}/src/HiAnalysis/HiOnia/test/condor_jobs_{args.jobName}"
    else:
        os.makedirs(args.output, exist_ok=True)
        local_job_dir = f"{args.output}/condor_jobs"
    
    # Create job directory
    log_dir = f"{local_job_dir}/logs"
    filelist_dir = f"{local_job_dir}/filelists"
    os.makedirs(log_dir, exist_ok=True)
    os.makedirs(filelist_dir, exist_ok=True)
    
    # Split by files
    file_chunks = split_files(files, args.filesPerJob)
    print(f"Creating {len(file_chunks)} jobs ({args.filesPerJob} files each, maxEvents={args.maxEvents})")
    
    jobs_info = []
    for i, chunk in enumerate(file_chunks):
        filelist_path = f"{filelist_dir}/filelist_{i:04d}.txt"
        with open(filelist_path, 'w') as f:
            f.write('\n'.join(chunk))
        jobs_info.append((filelist_path, i, 0, args.maxEvents))  # (filelist, job_index, first_event, max_events)
    
    # Create executable
    macro_path = f"{cmssw_base}/src/HiAnalysis/HiOnia/test/flattenBJetTree.C"
    exec_script = create_job_script(cmssw_base, macro_path, args.jetTree, args.output)
    job_dir = local_job_dir
    exec_path = f"{job_dir}/run_flatten.sh"
    with open(exec_path, 'w') as f:
        f.write(exec_script)
    os.chmod(exec_path, 0o755)
    
    # Create submit file
    submit_content = create_condor_submit(args.jobName, exec_path, log_dir, jobs_info, filelist_dir)
    submit_path = f"{job_dir}/submit.sub"
    with open(submit_path, 'w') as f:
        f.write(submit_content)
    
    print(f"\nCreated:")
    print(f"  - Executable: {exec_path}")
    print(f"  - Submit file: {submit_path}")
    print(f"  - {len(jobs_info)} filelists in {filelist_dir}")
    
    if args.dryRun:
        print("\nDry run - not submitting. To submit, run:")
        print(f"  condor_submit {submit_path}")
    else:
        print("\nSubmitting to condor...")
        os.system(f"condor_submit {submit_path}")

if __name__ == '__main__':
    main()
