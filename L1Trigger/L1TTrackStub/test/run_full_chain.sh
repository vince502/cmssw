#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
package_dir="$(cd "${script_dir}/.." && pwd)"
cmssw_src="$(cd "${package_dir}/../.." && pwd)"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /absolute/path/to/step1.root [max-events] [results-directory]" >&2
  exit 2
fi

input_file="$1"
max_events="${2:-1}"
results_dir="${3:-${script_dir}/results/full_chain}"

if [[ ! -f "${input_file}" ]]; then
  echo "Input file does not exist: ${input_file}" >&2
  exit 2
fi
input_file="$(cd "$(dirname "${input_file}")" && pwd)/$(basename "${input_file}")"

mkdir -p "${results_dir}"
results_dir="$(cd "${results_dir}" && pwd)"

# shellcheck source=/dev/null
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd "${cmssw_src}"
eval "$(scramv1 runtime -sh)"

cmsDriver.py step2 \
  --step DIGI:pdigi_hi,L1TrackTrigger,L1,L1P2GT,DIGI2RAW \
  --conditions auto:phase2_realistic_T35 \
  --datatier GEN-SIM-DIGI-RAW \
  --eventcontent FEVTDEBUGHLT \
  --geometry ExtendedRun4D121 \
  --era Phase2C22I13M9 \
  --procModifiers phase2_pp_on_AA \
  --pileup HiMixNoPU \
  --filein "file:${input_file}" \
  --fileout "file:${results_dir}/step2.root" \
  --python_filename "${results_dir}/step2_gtt_stub_cfg.py" \
  --customise L1Trigger/L1TTrackStub/customiseGTTStubEmulation.customiseGTTStubEmulation \
  --nThreads 1 \
  --nStreams 1 \
  --number "${max_events}" \
  --no_exec \
  >"${results_dir}/cmsDriver.log" 2>&1

cd "${results_dir}"
cmsRun step2_gtt_stub_cfg.py >cmsRun.log 2>&1
edmConfigDump step2_gtt_stub_cfg.py >step2_gtt_stub.dump.py
"${package_dir}/scripts/check_gtt_stub_output.py" \
  gttStubFullChain.root \
  >check_output.json
