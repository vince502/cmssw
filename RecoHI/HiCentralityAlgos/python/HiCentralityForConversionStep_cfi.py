import FWCore.ParameterSet.Config as cms

from RecoHI.HiCentralityAlgos.HiCentrality_cfi import hiCentrality as _hiCentrality


hiCentralityForConversionStep = _hiCentrality.clone(
    produceHFhits = False,
    produceHFtowers = True,
    produceEcalhits = False,
    produceZDChits = False,
    produceETmidRapidity = False,
    producePixelhits = False,
    produceTracks = False,
    producePixelTracks = False,
    producePF = False,
    reUseCentrality = False,
)
