"""Read the real centrality payload through EventSetup, without input data events.

Run from the study output directory:
  cmsRun <this file> year=2024 centralityVariation=Nominal
"""
import sys
from pathlib import Path

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing()
options.register("year", 2024, VarParsing.multiplicity.singleton,
                 VarParsing.varType.int, "PbPb data year (2023-2026).")
options.register("centralityVariation", "Nominal", VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, "Nominal, SysUp or SysDown.")
options.parseArguments()
if options.year not in (2023, 2024, 2025, 2026):
    raise ValueError("year must be 2023, 2024, 2025 or 2026")
cfg = Path(__file__).resolve().with_name(
    "hioniaanalyzer_PbPb%d_Charmonia_cfg.py" % options.year
)
sys.argv = [str(cfg), "inputFiles=file:/dev/null",
            "centralityVariation=" + options.centralityVariation]
namespace = {"__name__": "__main__", "__file__": str(cfg)}
exec(compile(cfg.read_text(), str(cfg), "exec"), namespace)
process = namespace["process"]
# The actual yearly era, GlobalTag and centrality override above are unchanged.
# Replace only the event source and active path for this conditions-only test.
process.source = cms.Source(
    "EmptySource", firstRun=cms.untracked.uint32(
        {2023: 375064, 2024: 387853, 2025: 400387, 2026: 404469}[options.year]
    ),
)
process.maxEvents.input = 1
process.options.numberOfThreads = 1
process.options.numberOfStreams = 1
process.centralityPayloadCheck = cms.EDAnalyzer(
    "EventSetupRecordDataGetter",
    toGet=cms.VPSet(cms.PSet(
        record=cms.string("HeavyIonRcd"),
        data=cms.vstring("CentralityTable/HFtowers"),
    )),
    verbose=cms.untracked.bool(True),
)
process.charmonia = cms.Path(process.centralityPayloadCheck)
del process.TFileService
