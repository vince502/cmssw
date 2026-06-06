import FWCore.ParameterSet.Config as cms

hiOniaMuons = cms.EDProducer(
    "HiOniaMuonMVAProducer",
    muons=cms.InputTag("patMuonsWithTrigger"),
    pfCandidates=cms.InputTag("packedPFCandidates"),
    centrality=cms.InputTag("centralityBin", "HFtowers"),
    etaMap=cms.InputTag("hiFJRhoProducerFinerBins", "mapEtaEdges"),
    rhoMap=cms.InputTag("hiFJRhoProducerFinerBins", "mapToRho"),
    pf_maxAbsEta=cms.double(2.8),
    sk_radius=cms.double(0.4),
    muon_minPt=cms.double(0.0),
    iso_rVeto=cms.double(1.0e-3),
    iso_rCone=cms.double(0.3),
    file_isoModel=cms.FileInPath("HiAnalysis/HiOnia/data/Run3_2023_PbPb/muiso_BDT.ubj"),
    file_isoCorr=cms.FileInPath(
        "HeavyIonsAnalysis/Configuration/data/lepton_spectra_train_weights_Run3_2023_PbPb.json.gz"
    ),
)
