"""Construct a yearly pset without processing events; check filters and DB tags."""
import re
import sqlite3
import sys
from pathlib import Path

cfg = Path(sys.argv[1]).resolve()
year = int(re.search(r"PbPb(202[3456])_", cfg.name).group(1))
sys.argv = [str(cfg), "inputFiles=file:/dev/null"] + sys.argv[2:]
namespace = {"__file__": str(cfg), "__name__": "__main__"}
exec(compile(cfg.read_text(), str(cfg), "exec"), namespace)
process = namespace["process"]


class ModuleOrder:
    def __init__(self):
        self.names = []

    def enter(self, node):
        if hasattr(node, "type_") and hasattr(node, "label_"):
            self.names.append(node.label_())

    def leave(self, node):
        pass


def ordered_modules(sequence):
    visitor = ModuleOrder()
    sequence.visit(visitor)
    return visitor.names


assert set(process.paths_()) == {"charmonia"}, list(process.paths_())
expected = ["hltfilter", "primaryVertexFilter"]
if year in (2023, 2024):
    expected.append("clusterCompatibilityFilter")
expected += [
    "phfCoincFilter2Th4" if year == 2023 else "phfCoincFilter3Th5",
    "charmoniaSelectedMuons", "charmoniaDimuons",
    "charmoniaDimuonCounter", "charmoniaCentralityBin", "hionia",
]
assert ordered_modules(process.charmonia) == expected, ordered_modules(process.charmonia)
assert list(process.hltfilter.HLTPaths) == ["HLT_HIMinimumBias*"]
assert process.hltfilter.TriggerResultsTag.value() == "TriggerResults::HLT"
assert process.hltfilter.throw.value() and process.hltfilter.andOr.value()
assert process.hionia.SumETvariables.value()
assert process.hionia.triggerResultsLabel.value() == "TriggerResults::HLT"
assert not process.hionia.doMuonTriggerMatching.value()
assert process.phfCoincFilter3Th5.threshold.value() == 5
assert process.phfCoincFilter3Th5.minnumtowers.value() == 3
assert process.phfCoincFilter2Th4.threshold.value() == 4
assert process.phfCoincFilter2Th4.minnumtowers.value() == 2
assert process.primaryVertexFilter.src.value() == "offlineSlimmedPrimaryVertices"
assert process.primaryVertexFilter.filter.value()
assert process.phfCoincFilter3Th5.HFfilters.value() == "hiHFfilters:hiHFfilters"
assert ordered_modules(process.eventFilters2023) == [
    "hltfilter", "primaryVertexFilter", "clusterCompatibilityFilter", "phfCoincFilter2Th4",
]
assert process.clusterCompatibilityFilter.cluscomSrc.value() == "hiClusterCompatibility"
print("PASS", year, "active path:", " -> ".join(expected))
print("PASS: 2023 reference sequence, HF thresholds/tower counts, MiniAOD sources, strict HLT and one active path")
overrides = [item for item in process.GlobalTag.toGet if item.record.value() == "HeavyIonRcd"]
assert len(overrides) == 1
conditions = overrides[0]
assert conditions.label.value() == "HFtowers"
assert conditions.connect.value().startswith("sqlite_file:/afs/cern.ch/")
db_path = Path(conditions.connect.value().removeprefix("sqlite_file:"))
with sqlite3.connect(db_path.as_uri() + "?mode=ro&immutable=1", uri=True) as connection:
    tags = connection.execute(
        "SELECT OBJECT_TYPE, TIME_TYPE FROM TAG WHERE NAME = ?", (conditions.tag.value(),)
    ).fetchall()
    assert tags == [("CentralityTable", "Run")], tags
    iovs = connection.execute(
        "SELECT SINCE, PAYLOAD_HASH FROM IOV WHERE TAG_NAME = ?", (conditions.tag.value(),)
    ).fetchall()
    assert len(iovs) == 1 and iovs[0][0] == 1, iovs
assert namespace["options"].centralityVariation in db_path.stem
print("PASS: direct DB, HeavyIonRcd/HFtowers, verified internal tag and IOV:",
      db_path, conditions.tag.value(), iovs)
