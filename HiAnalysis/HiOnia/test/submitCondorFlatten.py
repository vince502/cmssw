#!/usr/bin/env python3
"""
submitCondorFlatten.py - Submit B-jet flattening jobs to HTCondor
Usage:
  python submitCondorFlatten.py filelist.txt outputDir/ [--submit]
  
This creates one job per input file for maximum parallelization.
"""

import os
import sys
import argparse
from datetime import datetime

def generate_condor_jobs(filelist, output_dir, job_dir, submit=False):
    """Generate and optionally submit condor jobs"""
    
    # Read file list
    with open(filelist, 'r') as f:
        files = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    
    print(f"Found {len(files)} files to process")
    
    # Create directories
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(job_dir, exist_ok=True)
    log_dir = os.path.join(job_dir, "logs")
    os.makedirs(log_dir, exist_ok=True)
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    wrapper_script = os.path.join(script_dir, "flattenBJetMatch_condor.sh")
    
    # Generate job submission file
    sub_file = os.path.join(job_dir, "submit_all.sub")
    dag_file = os.path.join(job_dir, "submit_all.dag")
    
    # Create individual job configs and DAG
    with open(dag_file, 'w') as dag:
        dag.write(f"# DAG file generated on {datetime.now()}\n")
        dag.write(f"# {len(files)} jobs\n\n")
        
        for i, input_file in enumerate(files):
            job_name = f"job_{i:04d}"
            basename = os.path.basename(input_file).replace('.root', '_flat.root')
            output_file = os.path.join(output_dir, basename)
            
            # Individual submit file
            job_sub = os.path.join(job_dir, f"{job_name}.sub")
            with open(job_sub, 'w') as jsub:
                jsub.write(f"""# Job {i}: {basename}
universe = vanilla
executable = {wrapper_script}
arguments = {input_file} {output_file}

output = {log_dir}/{job_name}.out
error = {log_dir}/{job_name}.err
log = {log_dir}/{job_name}.log

+JobFlavour = "longlunch"
request_memory = 4000
request_cpus = 1

should_transfer_files = NO

queue 1
""")
            
            dag.write(f"JOB {job_name} {job_sub}\n")
    
    # Also create a simple submit-all script
    with open(sub_file, 'w') as f:
        f.write(f"""# Submit all jobs
universe = vanilla
executable = {wrapper_script}

output = {log_dir}/job_$(Process).out
error = {log_dir}/job_$(Process).err
log = {log_dir}/job_$(Process).log

+JobFlavour = "longlunch"
request_memory = 4000
request_cpus = 1

should_transfer_files = NO

""")
        for i, input_file in enumerate(files):
            basename = os.path.basename(input_file).replace('.root', '_flat.root')
            output_file = os.path.join(output_dir, basename)
            f.write(f"arguments = {input_file} {output_file}\n")
            f.write("queue 1\n\n")
    
    # Create job list for tracking
    job_list = os.path.join(job_dir, "job_list.txt")
    with open(job_list, 'w') as f:
        for i, input_file in enumerate(files):
            basename = os.path.basename(input_file).replace('.root', '_flat.root')
            output_file = os.path.join(output_dir, basename)
            f.write(f"{i}\t{input_file}\t{output_file}\n")
    
    print(f"\n=== Generated files ===")
    print(f"DAG file: {dag_file}")
    print(f"Submit file: {sub_file}")
    print(f"Job list: {job_list}")
    print(f"Log dir: {log_dir}")
    
    if submit:
        print(f"\n=== Submitting jobs ===")
        os.system(f"condor_submit_dag {dag_file}")
    else:
        print(f"\n=== To submit ===")
        print(f"Option 1 (DAG): condor_submit_dag {dag_file}")
        print(f"Option 2 (All): condor_submit {sub_file}")

def main():
    parser = argparse.ArgumentParser(description="Submit B-jet flattening jobs to HTCondor")
    parser.add_argument("filelist", help="Input file list (txt)")
    parser.add_argument("output_dir", help="Output directory for flat ROOT files")
    parser.add_argument("--job-dir", default=None, help="Directory for job files (default: jobs_<timestamp>)")
    parser.add_argument("--submit", action="store_true", help="Actually submit jobs")
    
    args = parser.parse_args()
    
    if args.job_dir is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        args.job_dir = f"jobs_flatten_{timestamp}"
    
    generate_condor_jobs(args.filelist, args.output_dir, args.job_dir, args.submit)

if __name__ == "__main__":
    main()
