// -*- C++ -*-
// ConversionComparisonAnalyzer.cc
// Analyzer to compare different conversion collections:
// - gsfTracksOpenConversions:gsfTracksOpenConversions
// - reducedEgamma:reducedConversions
// - reducedEgamma:reducedSingleLegConversions
// - oniaPhotonCandidates:conversions

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/EgammaCandidates/interface/ConversionFwd.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

class ConversionComparisonAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit ConversionComparisonAnalyzer(const edm::ParameterSet&);
  ~ConversionComparisonAnalyzer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;

  // Tokens
  edm::EDGetTokenT<reco::ConversionCollection> gsfOpenConvToken_;
  edm::EDGetTokenT<reco::ConversionCollection> reducedConvToken_;
  edm::EDGetTokenT<reco::ConversionCollection> reducedSingleLegConvToken_;
  edm::EDGetTokenT<std::vector<pat::CompositeCandidate>> oniaConvToken_;

  // Tree
  TTree* tree_;

  // Event info
  ULong64_t event_;
  UInt_t run_;
  UInt_t lumi_;

  // Counts
  int nGsfOpenConv_;
  int nReducedConv_;
  int nReducedSingleLegConv_;
  int nOniaConv_;

  // gsfTracksOpenConversions
  std::vector<float> gsfOpen_pt_;
  std::vector<float> gsfOpen_eta_;
  std::vector<float> gsfOpen_phi_;
  std::vector<float> gsfOpen_vtxX_;
  std::vector<float> gsfOpen_vtxY_;
  std::vector<float> gsfOpen_vtxZ_;
  std::vector<float> gsfOpen_vtxChi2_;
  std::vector<int> gsfOpen_nTracks_;
  std::vector<float> gsfOpen_pairMass_;
  std::vector<float> gsfOpen_pairCotThetaSep_;

  // reducedConversions
  std::vector<float> reduced_pt_;
  std::vector<float> reduced_eta_;
  std::vector<float> reduced_phi_;
  std::vector<float> reduced_vtxX_;
  std::vector<float> reduced_vtxY_;
  std::vector<float> reduced_vtxZ_;
  std::vector<float> reduced_vtxChi2_;
  std::vector<int> reduced_nTracks_;
  std::vector<float> reduced_pairMass_;
  std::vector<float> reduced_pairCotThetaSep_;

  // reducedSingleLegConversions
  std::vector<float> singleLeg_pt_;
  std::vector<float> singleLeg_eta_;
  std::vector<float> singleLeg_phi_;
  std::vector<float> singleLeg_vtxX_;
  std::vector<float> singleLeg_vtxY_;
  std::vector<float> singleLeg_vtxZ_;
  std::vector<int> singleLeg_nTracks_;

  // oniaPhotonCandidates:conversions (pat::CompositeCandidate)
  std::vector<float> onia_pt_;
  std::vector<float> onia_eta_;
  std::vector<float> onia_phi_;
  std::vector<float> onia_mass_;
  std::vector<float> onia_vtxX_;
  std::vector<float> onia_vtxY_;
  std::vector<float> onia_vtxZ_;
  std::vector<float> onia_vtxChi2_;
  std::vector<float> onia_vtxNdof_;
  std::vector<int> onia_nDaughters_;

  // Histograms
  TH1F* h_nGsfOpenConv_;
  TH1F* h_nReducedConv_;
  TH1F* h_nReducedSingleLegConv_;
  TH1F* h_nOniaConv_;
  TH2F* h2_nConv_gsfOpen_vs_reduced_;
  TH2F* h2_nConv_reduced_vs_onia_;
};

ConversionComparisonAnalyzer::ConversionComparisonAnalyzer(const edm::ParameterSet& iConfig) {
  usesResource("TFileService");

  gsfOpenConvToken_ = consumes<reco::ConversionCollection>(
      iConfig.getParameter<edm::InputTag>("gsfOpenConversions"));
  reducedConvToken_ = consumes<reco::ConversionCollection>(
      iConfig.getParameter<edm::InputTag>("reducedConversions"));
  reducedSingleLegConvToken_ = consumes<reco::ConversionCollection>(
      iConfig.getParameter<edm::InputTag>("reducedSingleLegConversions"));
  oniaConvToken_ = consumes<std::vector<pat::CompositeCandidate>>(
      iConfig.getParameter<edm::InputTag>("oniaConversions"));

  edm::Service<TFileService> fs;

  // Create tree
  tree_ = fs->make<TTree>("convTree", "Conversion Comparison Tree");

  tree_->Branch("event", &event_);
  tree_->Branch("run", &run_);
  tree_->Branch("lumi", &lumi_);

  tree_->Branch("nGsfOpenConv", &nGsfOpenConv_);
  tree_->Branch("nReducedConv", &nReducedConv_);
  tree_->Branch("nReducedSingleLegConv", &nReducedSingleLegConv_);
  tree_->Branch("nOniaConv", &nOniaConv_);

  // gsfTracksOpenConversions branches
  tree_->Branch("gsfOpen_pt", &gsfOpen_pt_);
  tree_->Branch("gsfOpen_eta", &gsfOpen_eta_);
  tree_->Branch("gsfOpen_phi", &gsfOpen_phi_);
  tree_->Branch("gsfOpen_vtxX", &gsfOpen_vtxX_);
  tree_->Branch("gsfOpen_vtxY", &gsfOpen_vtxY_);
  tree_->Branch("gsfOpen_vtxZ", &gsfOpen_vtxZ_);
  tree_->Branch("gsfOpen_vtxChi2", &gsfOpen_vtxChi2_);
  tree_->Branch("gsfOpen_nTracks", &gsfOpen_nTracks_);
  tree_->Branch("gsfOpen_pairMass", &gsfOpen_pairMass_);
  tree_->Branch("gsfOpen_pairCotThetaSep", &gsfOpen_pairCotThetaSep_);

  // reducedConversions branches
  tree_->Branch("reduced_pt", &reduced_pt_);
  tree_->Branch("reduced_eta", &reduced_eta_);
  tree_->Branch("reduced_phi", &reduced_phi_);
  tree_->Branch("reduced_vtxX", &reduced_vtxX_);
  tree_->Branch("reduced_vtxY", &reduced_vtxY_);
  tree_->Branch("reduced_vtxZ", &reduced_vtxZ_);
  tree_->Branch("reduced_vtxChi2", &reduced_vtxChi2_);
  tree_->Branch("reduced_nTracks", &reduced_nTracks_);
  tree_->Branch("reduced_pairMass", &reduced_pairMass_);
  tree_->Branch("reduced_pairCotThetaSep", &reduced_pairCotThetaSep_);

  // reducedSingleLegConversions branches
  tree_->Branch("singleLeg_pt", &singleLeg_pt_);
  tree_->Branch("singleLeg_eta", &singleLeg_eta_);
  tree_->Branch("singleLeg_phi", &singleLeg_phi_);
  tree_->Branch("singleLeg_vtxX", &singleLeg_vtxX_);
  tree_->Branch("singleLeg_vtxY", &singleLeg_vtxY_);
  tree_->Branch("singleLeg_vtxZ", &singleLeg_vtxZ_);
  tree_->Branch("singleLeg_nTracks", &singleLeg_nTracks_);

  // oniaPhotonCandidates branches
  tree_->Branch("onia_pt", &onia_pt_);
  tree_->Branch("onia_eta", &onia_eta_);
  tree_->Branch("onia_phi", &onia_phi_);
  tree_->Branch("onia_mass", &onia_mass_);
  tree_->Branch("onia_vtxX", &onia_vtxX_);
  tree_->Branch("onia_vtxY", &onia_vtxY_);
  tree_->Branch("onia_vtxZ", &onia_vtxZ_);
  tree_->Branch("onia_vtxChi2", &onia_vtxChi2_);
  tree_->Branch("onia_vtxNdof", &onia_vtxNdof_);
  tree_->Branch("onia_nDaughters", &onia_nDaughters_);

  // Histograms
  h_nGsfOpenConv_ = fs->make<TH1F>("h_nGsfOpenConv", "Number of gsfTracksOpenConversions;N;Events", 50, 0, 50);
  h_nReducedConv_ = fs->make<TH1F>("h_nReducedConv", "Number of reducedConversions;N;Events", 50, 0, 50);
  h_nReducedSingleLegConv_ = fs->make<TH1F>("h_nReducedSingleLegConv", "Number of reducedSingleLegConversions;N;Events", 50, 0, 50);
  h_nOniaConv_ = fs->make<TH1F>("h_nOniaConv", "Number of oniaPhotonCandidates:conversions;N;Events", 50, 0, 50);
  h2_nConv_gsfOpen_vs_reduced_ = fs->make<TH2F>("h2_nConv_gsfOpen_vs_reduced", 
      "N(gsfOpen) vs N(reduced);N(reducedConversions);N(gsfTracksOpenConversions)", 50, 0, 50, 50, 0, 50);
  h2_nConv_reduced_vs_onia_ = fs->make<TH2F>("h2_nConv_reduced_vs_onia",
      "N(reduced) vs N(onia);N(oniaPhotonCandidates);N(reducedConversions)", 50, 0, 50, 50, 0, 50);
}

void ConversionComparisonAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Clear vectors
  gsfOpen_pt_.clear(); gsfOpen_eta_.clear(); gsfOpen_phi_.clear();
  gsfOpen_vtxX_.clear(); gsfOpen_vtxY_.clear(); gsfOpen_vtxZ_.clear();
  gsfOpen_vtxChi2_.clear(); gsfOpen_nTracks_.clear();
  gsfOpen_pairMass_.clear(); gsfOpen_pairCotThetaSep_.clear();

  reduced_pt_.clear(); reduced_eta_.clear(); reduced_phi_.clear();
  reduced_vtxX_.clear(); reduced_vtxY_.clear(); reduced_vtxZ_.clear();
  reduced_vtxChi2_.clear(); reduced_nTracks_.clear();
  reduced_pairMass_.clear(); reduced_pairCotThetaSep_.clear();

  singleLeg_pt_.clear(); singleLeg_eta_.clear(); singleLeg_phi_.clear();
  singleLeg_vtxX_.clear(); singleLeg_vtxY_.clear(); singleLeg_vtxZ_.clear();
  singleLeg_nTracks_.clear();

  onia_pt_.clear(); onia_eta_.clear(); onia_phi_.clear(); onia_mass_.clear();
  onia_vtxX_.clear(); onia_vtxY_.clear(); onia_vtxZ_.clear();
  onia_vtxChi2_.clear(); onia_vtxNdof_.clear(); onia_nDaughters_.clear();

  // Event info
  event_ = iEvent.id().event();
  run_ = iEvent.id().run();
  lumi_ = iEvent.luminosityBlock();

  // Get collections
  edm::Handle<reco::ConversionCollection> gsfOpenConvH;
  iEvent.getByToken(gsfOpenConvToken_, gsfOpenConvH);

  edm::Handle<reco::ConversionCollection> reducedConvH;
  iEvent.getByToken(reducedConvToken_, reducedConvH);

  edm::Handle<reco::ConversionCollection> reducedSingleLegConvH;
  iEvent.getByToken(reducedSingleLegConvToken_, reducedSingleLegConvH);

  edm::Handle<std::vector<pat::CompositeCandidate>> oniaConvH;
  iEvent.getByToken(oniaConvToken_, oniaConvH);

  // Fill gsfTracksOpenConversions
  nGsfOpenConv_ = gsfOpenConvH.isValid() ? gsfOpenConvH->size() : 0;
  if (gsfOpenConvH.isValid()) {
    for (const auto& conv : *gsfOpenConvH) {
      gsfOpen_pt_.push_back(conv.refittedPair4Momentum().pt());
      gsfOpen_eta_.push_back(conv.refittedPair4Momentum().eta());
      gsfOpen_phi_.push_back(conv.refittedPair4Momentum().phi());
      gsfOpen_vtxX_.push_back(conv.conversionVertex().x());
      gsfOpen_vtxY_.push_back(conv.conversionVertex().y());
      gsfOpen_vtxZ_.push_back(conv.conversionVertex().z());
      gsfOpen_vtxChi2_.push_back(conv.conversionVertex().chi2());
      gsfOpen_nTracks_.push_back(conv.nTracks());
      gsfOpen_pairMass_.push_back(conv.pairInvariantMass());
      gsfOpen_pairCotThetaSep_.push_back(conv.pairCotThetaSeparation());
    }
  }

  // Fill reducedConversions
  nReducedConv_ = reducedConvH.isValid() ? reducedConvH->size() : 0;
  if (reducedConvH.isValid()) {
    for (const auto& conv : *reducedConvH) {
      reduced_pt_.push_back(conv.refittedPair4Momentum().pt());
      reduced_eta_.push_back(conv.refittedPair4Momentum().eta());
      reduced_phi_.push_back(conv.refittedPair4Momentum().phi());
      reduced_vtxX_.push_back(conv.conversionVertex().x());
      reduced_vtxY_.push_back(conv.conversionVertex().y());
      reduced_vtxZ_.push_back(conv.conversionVertex().z());
      reduced_vtxChi2_.push_back(conv.conversionVertex().chi2());
      reduced_nTracks_.push_back(conv.nTracks());
      reduced_pairMass_.push_back(conv.pairInvariantMass());
      reduced_pairCotThetaSep_.push_back(conv.pairCotThetaSeparation());
    }
  }

  // Fill reducedSingleLegConversions
  nReducedSingleLegConv_ = reducedSingleLegConvH.isValid() ? reducedSingleLegConvH->size() : 0;
  if (reducedSingleLegConvH.isValid()) {
    for (const auto& conv : *reducedSingleLegConvH) {
      singleLeg_pt_.push_back(conv.refittedPair4Momentum().pt());
      singleLeg_eta_.push_back(conv.refittedPair4Momentum().eta());
      singleLeg_phi_.push_back(conv.refittedPair4Momentum().phi());
      singleLeg_vtxX_.push_back(conv.conversionVertex().x());
      singleLeg_vtxY_.push_back(conv.conversionVertex().y());
      singleLeg_vtxZ_.push_back(conv.conversionVertex().z());
      singleLeg_nTracks_.push_back(conv.nTracks());
    }
  }

  // Fill oniaPhotonCandidates:conversions
  nOniaConv_ = oniaConvH.isValid() ? oniaConvH->size() : 0;
  if (oniaConvH.isValid()) {
    for (const auto& cand : *oniaConvH) {
      onia_pt_.push_back(cand.pt());
      onia_eta_.push_back(cand.eta());
      onia_phi_.push_back(cand.phi());
      onia_mass_.push_back(cand.mass());
      onia_vtxX_.push_back(cand.vertex().x());
      onia_vtxY_.push_back(cand.vertex().y());
      onia_vtxZ_.push_back(cand.vertex().z());
      onia_vtxChi2_.push_back(cand.hasUserFloat("vChi2") ? cand.userFloat("vChi2") : -999.);
      onia_vtxNdof_.push_back(cand.hasUserFloat("vNdof") ? cand.userFloat("vNdof") : -999.);
      onia_nDaughters_.push_back(cand.numberOfDaughters());
    }
  }

  // Fill histograms
  h_nGsfOpenConv_->Fill(nGsfOpenConv_);
  h_nReducedConv_->Fill(nReducedConv_);
  h_nReducedSingleLegConv_->Fill(nReducedSingleLegConv_);
  h_nOniaConv_->Fill(nOniaConv_);
  h2_nConv_gsfOpen_vs_reduced_->Fill(nReducedConv_, nGsfOpenConv_);
  h2_nConv_reduced_vs_onia_->Fill(nOniaConv_, nReducedConv_);

  tree_->Fill();
}

void ConversionComparisonAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("gsfOpenConversions", edm::InputTag("gsfTracksOpenConversions", "gsfTracksOpenConversions"));
  desc.add<edm::InputTag>("reducedConversions", edm::InputTag("reducedEgamma", "reducedConversions"));
  desc.add<edm::InputTag>("reducedSingleLegConversions", edm::InputTag("reducedEgamma", "reducedSingleLegConversions"));
  desc.add<edm::InputTag>("oniaConversions", edm::InputTag("oniaPhotonCandidates", "conversions"));
  descriptions.add("conversionComparisonAnalyzer", desc);
}

DEFINE_FWK_MODULE(ConversionComparisonAnalyzer);
