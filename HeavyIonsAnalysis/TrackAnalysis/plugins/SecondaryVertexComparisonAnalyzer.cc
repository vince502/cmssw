// SecondaryVertexComparisonAnalyzer.cc
// Compares two secondary vertex collections (e.g., slimmedSecondaryVertices vs inclusiveCandidateSecondaryVertices)
// Fills TTree with per-SV info and per-event comparison statistics

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidateFwd.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include <vector>
#include <cmath>
#include <algorithm>

class SecondaryVertexComparisonAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit SecondaryVertexComparisonAnalyzer(const edm::ParameterSet&);
  ~SecondaryVertexComparisonAnalyzer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // Tokens
  edm::EDGetTokenT<reco::VertexCompositePtrCandidateCollection> svToken1_;
  edm::EDGetTokenT<reco::VertexCompositePtrCandidateCollection> svToken2_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;

  // Labels for output
  std::string label1_;
  std::string label2_;

  // Matching parameters
  double maxDeltaR_;
  double maxDeltaZ_;

  // Output tree
  TTree* tree_;

  // Event info
  unsigned int run_;
  unsigned int lumi_;
  unsigned long long event_;

  // Per-event comparison
  int nSV1_;
  int nSV2_;
  int nMatched_;
  float matchFraction1_;  // nMatched / nSV1
  float matchFraction2_;  // nMatched / nSV2

  // Collection 1 statistics (per event)
  float sv1_pt_mean_;
  float sv1_pt_std_;
  float sv1_mass_mean_;
  float sv1_mass_std_;
  float sv1_chi2_mean_;
  float sv1_ndof_mean_;
  float sv1_ntrk_mean_;
  float sv1_dxy_mean_;
  float sv1_dxySig_mean_;

  // Collection 2 statistics (per event)
  float sv2_pt_mean_;
  float sv2_pt_std_;
  float sv2_mass_mean_;
  float sv2_mass_std_;
  float sv2_chi2_mean_;
  float sv2_ndof_mean_;
  float sv2_ntrk_mean_;
  float sv2_dxy_mean_;
  float sv2_dxySig_mean_;

  // Matched pair differences (means per event)
  float matched_dPt_mean_;
  float matched_dMass_mean_;
  float matched_dR_mean_;
  float matched_dZ_mean_;

  // Per-SV vectors for Collection 1
  std::vector<float> sv1_pt_;
  std::vector<float> sv1_eta_;
  std::vector<float> sv1_phi_;
  std::vector<float> sv1_mass_;
  std::vector<float> sv1_x_;
  std::vector<float> sv1_y_;
  std::vector<float> sv1_z_;
  std::vector<float> sv1_chi2_;
  std::vector<float> sv1_ndof_;
  std::vector<int> sv1_ntracks_;
  std::vector<float> sv1_dxy_;
  std::vector<float> sv1_dxyErr_;
  std::vector<float> sv1_dxySig_;
  std::vector<int> sv1_matchIdx_;  // Index in collection 2, -1 if unmatched

  // Per-SV vectors for Collection 2
  std::vector<float> sv2_pt_;
  std::vector<float> sv2_eta_;
  std::vector<float> sv2_phi_;
  std::vector<float> sv2_mass_;
  std::vector<float> sv2_x_;
  std::vector<float> sv2_y_;
  std::vector<float> sv2_z_;
  std::vector<float> sv2_chi2_;
  std::vector<float> sv2_ndof_;
  std::vector<int> sv2_ntracks_;
  std::vector<float> sv2_dxy_;
  std::vector<float> sv2_dxyErr_;
  std::vector<float> sv2_dxySig_;
  std::vector<int> sv2_matchIdx_;  // Index in collection 1, -1 if unmatched

  // Histograms
  TH1F* h_nSV1_;
  TH1F* h_nSV2_;
  TH1F* h_nSVDiff_;
  TH2F* h_nSV_corr_;
  TH1F* h_matchedDR_;
  TH1F* h_matchedDPt_;
  TH1F* h_matchedDMass_;

  // Helper functions
  void clearVectors();
  float calculateMean(const std::vector<float>& v);
  float calculateStdDev(const std::vector<float>& v, float mean);
};

SecondaryVertexComparisonAnalyzer::SecondaryVertexComparisonAnalyzer(const edm::ParameterSet& iConfig)
    : svToken1_(consumes<reco::VertexCompositePtrCandidateCollection>(
          iConfig.getParameter<edm::InputTag>("svSrc1"))),
      svToken2_(consumes<reco::VertexCompositePtrCandidateCollection>(
          iConfig.getParameter<edm::InputTag>("svSrc2"))),
      pvToken_(consumes<reco::VertexCollection>(
          iConfig.getParameter<edm::InputTag>("pvSrc"))),
      label1_(iConfig.getParameter<std::string>("label1")),
      label2_(iConfig.getParameter<std::string>("label2")),
      maxDeltaR_(iConfig.getParameter<double>("maxDeltaR")),
      maxDeltaZ_(iConfig.getParameter<double>("maxDeltaZ")) {
  usesResource("TFileService");
}

void SecondaryVertexComparisonAnalyzer::beginJob() {
  edm::Service<TFileService> fs;

  tree_ = fs->make<TTree>("svTree", "Secondary Vertex Comparison Tree");

  // Event info
  tree_->Branch("run", &run_);
  tree_->Branch("lumi", &lumi_);
  tree_->Branch("event", &event_);

  // Per-event comparison
  tree_->Branch("nSV1", &nSV1_);
  tree_->Branch("nSV2", &nSV2_);
  tree_->Branch("nMatched", &nMatched_);
  tree_->Branch("matchFraction1", &matchFraction1_);
  tree_->Branch("matchFraction2", &matchFraction2_);

  // Collection 1 statistics
  tree_->Branch("sv1_pt_mean", &sv1_pt_mean_);
  tree_->Branch("sv1_pt_std", &sv1_pt_std_);
  tree_->Branch("sv1_mass_mean", &sv1_mass_mean_);
  tree_->Branch("sv1_mass_std", &sv1_mass_std_);
  tree_->Branch("sv1_chi2_mean", &sv1_chi2_mean_);
  tree_->Branch("sv1_ndof_mean", &sv1_ndof_mean_);
  tree_->Branch("sv1_ntrk_mean", &sv1_ntrk_mean_);
  tree_->Branch("sv1_dxy_mean", &sv1_dxy_mean_);
  tree_->Branch("sv1_dxySig_mean", &sv1_dxySig_mean_);

  // Collection 2 statistics
  tree_->Branch("sv2_pt_mean", &sv2_pt_mean_);
  tree_->Branch("sv2_pt_std", &sv2_pt_std_);
  tree_->Branch("sv2_mass_mean", &sv2_mass_mean_);
  tree_->Branch("sv2_mass_std", &sv2_mass_std_);
  tree_->Branch("sv2_chi2_mean", &sv2_chi2_mean_);
  tree_->Branch("sv2_ndof_mean", &sv2_ndof_mean_);
  tree_->Branch("sv2_ntrk_mean", &sv2_ntrk_mean_);
  tree_->Branch("sv2_dxy_mean", &sv2_dxy_mean_);
  tree_->Branch("sv2_dxySig_mean", &sv2_dxySig_mean_);

  // Matched pair differences
  tree_->Branch("matched_dPt_mean", &matched_dPt_mean_);
  tree_->Branch("matched_dMass_mean", &matched_dMass_mean_);
  tree_->Branch("matched_dR_mean", &matched_dR_mean_);
  tree_->Branch("matched_dZ_mean", &matched_dZ_mean_);

  // Per-SV vectors for Collection 1
  tree_->Branch("sv1_pt", &sv1_pt_);
  tree_->Branch("sv1_eta", &sv1_eta_);
  tree_->Branch("sv1_phi", &sv1_phi_);
  tree_->Branch("sv1_mass", &sv1_mass_);
  tree_->Branch("sv1_x", &sv1_x_);
  tree_->Branch("sv1_y", &sv1_y_);
  tree_->Branch("sv1_z", &sv1_z_);
  tree_->Branch("sv1_chi2", &sv1_chi2_);
  tree_->Branch("sv1_ndof", &sv1_ndof_);
  tree_->Branch("sv1_ntracks", &sv1_ntracks_);
  tree_->Branch("sv1_dxy", &sv1_dxy_);
  tree_->Branch("sv1_dxyErr", &sv1_dxyErr_);
  tree_->Branch("sv1_dxySig", &sv1_dxySig_);
  tree_->Branch("sv1_matchIdx", &sv1_matchIdx_);

  // Per-SV vectors for Collection 2
  tree_->Branch("sv2_pt", &sv2_pt_);
  tree_->Branch("sv2_eta", &sv2_eta_);
  tree_->Branch("sv2_phi", &sv2_phi_);
  tree_->Branch("sv2_mass", &sv2_mass_);
  tree_->Branch("sv2_x", &sv2_x_);
  tree_->Branch("sv2_y", &sv2_y_);
  tree_->Branch("sv2_z", &sv2_z_);
  tree_->Branch("sv2_chi2", &sv2_chi2_);
  tree_->Branch("sv2_ndof", &sv2_ndof_);
  tree_->Branch("sv2_ntracks", &sv2_ntracks_);
  tree_->Branch("sv2_dxy", &sv2_dxy_);
  tree_->Branch("sv2_dxyErr", &sv2_dxyErr_);
  tree_->Branch("sv2_dxySig", &sv2_dxySig_);
  tree_->Branch("sv2_matchIdx", &sv2_matchIdx_);

  // Histograms
  h_nSV1_ = fs->make<TH1F>("h_nSV1", (label1_ + " nSV;N_{SV};Events").c_str(), 50, 0, 50);
  h_nSV2_ = fs->make<TH1F>("h_nSV2", (label2_ + " nSV;N_{SV};Events").c_str(), 50, 0, 50);
  h_nSVDiff_ = fs->make<TH1F>("h_nSVDiff", "nSV difference;N_{SV}^{1} - N_{SV}^{2};Events", 41, -20.5, 20.5);
  h_nSV_corr_ = fs->make<TH2F>("h_nSV_corr", "nSV correlation;N_{SV}^{1};N_{SV}^{2}", 50, 0, 50, 50, 0, 50);
  h_matchedDR_ = fs->make<TH1F>("h_matchedDR", "Matched SV #DeltaR;#DeltaR;Pairs", 100, 0, 0.5);
  h_matchedDPt_ = fs->make<TH1F>("h_matchedDPt", "Matched SV #Deltap_{T};#Deltap_{T} [GeV];Pairs", 100, -5, 5);
  h_matchedDMass_ = fs->make<TH1F>("h_matchedDMass", "Matched SV #DeltaM;#DeltaM [GeV];Pairs", 100, -2, 2);
}

void SecondaryVertexComparisonAnalyzer::clearVectors() {
  sv1_pt_.clear();
  sv1_eta_.clear();
  sv1_phi_.clear();
  sv1_mass_.clear();
  sv1_x_.clear();
  sv1_y_.clear();
  sv1_z_.clear();
  sv1_chi2_.clear();
  sv1_ndof_.clear();
  sv1_ntracks_.clear();
  sv1_dxy_.clear();
  sv1_dxyErr_.clear();
  sv1_dxySig_.clear();
  sv1_matchIdx_.clear();

  sv2_pt_.clear();
  sv2_eta_.clear();
  sv2_phi_.clear();
  sv2_mass_.clear();
  sv2_x_.clear();
  sv2_y_.clear();
  sv2_z_.clear();
  sv2_chi2_.clear();
  sv2_ndof_.clear();
  sv2_ntracks_.clear();
  sv2_dxy_.clear();
  sv2_dxyErr_.clear();
  sv2_dxySig_.clear();
  sv2_matchIdx_.clear();
}

float SecondaryVertexComparisonAnalyzer::calculateMean(const std::vector<float>& v) {
  if (v.empty()) return 0;
  float sum = 0;
  for (const auto& x : v) sum += x;
  return sum / v.size();
}

float SecondaryVertexComparisonAnalyzer::calculateStdDev(const std::vector<float>& v, float mean) {
  if (v.size() < 2) return 0;
  float sumSq = 0;
  for (const auto& x : v) sumSq += (x - mean) * (x - mean);
  return std::sqrt(sumSq / (v.size() - 1));
}

void SecondaryVertexComparisonAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  clearVectors();

  // Event info
  run_ = iEvent.id().run();
  lumi_ = iEvent.luminosityBlock();
  event_ = iEvent.id().event();

  // Get collections
  edm::Handle<reco::VertexCompositePtrCandidateCollection> svs1;
  edm::Handle<reco::VertexCompositePtrCandidateCollection> svs2;
  edm::Handle<reco::VertexCollection> pvs;

  iEvent.getByToken(svToken1_, svs1);
  iEvent.getByToken(svToken2_, svs2);
  iEvent.getByToken(pvToken_, pvs);

  // Get primary vertex
  math::XYZPoint pv(0, 0, 0);
  if (pvs.isValid() && !pvs->empty()) {
    pv = pvs->front().position();
  }

  nSV1_ = svs1.isValid() ? svs1->size() : 0;
  nSV2_ = svs2.isValid() ? svs2->size() : 0;

  h_nSV1_->Fill(nSV1_);
  h_nSV2_->Fill(nSV2_);
  h_nSVDiff_->Fill(nSV1_ - nSV2_);
  h_nSV_corr_->Fill(nSV1_, nSV2_);

  // Fill collection 1 info
  std::vector<float> pt1_vec, mass1_vec, chi2_1_vec, ndof1_vec, ntrk1_vec, dxy1_vec, dxySig1_vec;
  
  if (svs1.isValid()) {
    for (size_t i = 0; i < svs1->size(); i++) {
      const auto& sv = (*svs1)[i];
      
      float dxy = std::sqrt((sv.vx() - pv.x()) * (sv.vx() - pv.x()) + 
                            (sv.vy() - pv.y()) * (sv.vy() - pv.y()));
      float dxyErr = std::sqrt(sv.vertexCovariance(0, 0) + sv.vertexCovariance(1, 1));
      float dxySig = dxyErr > 0 ? dxy / dxyErr : 0;

      sv1_pt_.push_back(sv.pt());
      sv1_eta_.push_back(sv.eta());
      sv1_phi_.push_back(sv.phi());
      sv1_mass_.push_back(sv.mass());
      sv1_x_.push_back(sv.vx());
      sv1_y_.push_back(sv.vy());
      sv1_z_.push_back(sv.vz());
      sv1_chi2_.push_back(sv.vertexChi2());
      sv1_ndof_.push_back(sv.vertexNdof());
      sv1_ntracks_.push_back(sv.numberOfDaughters());
      sv1_dxy_.push_back(dxy);
      sv1_dxyErr_.push_back(dxyErr);
      sv1_dxySig_.push_back(dxySig);
      sv1_matchIdx_.push_back(-1);  // Will be updated during matching

      pt1_vec.push_back(sv.pt());
      mass1_vec.push_back(sv.mass());
      chi2_1_vec.push_back(sv.vertexChi2());
      ndof1_vec.push_back(sv.vertexNdof());
      ntrk1_vec.push_back(sv.numberOfDaughters());
      dxy1_vec.push_back(dxy);
      dxySig1_vec.push_back(dxySig);
    }
  }

  // Fill collection 2 info
  std::vector<float> pt2_vec, mass2_vec, chi2_2_vec, ndof2_vec, ntrk2_vec, dxy2_vec, dxySig2_vec;
  
  if (svs2.isValid()) {
    for (size_t i = 0; i < svs2->size(); i++) {
      const auto& sv = (*svs2)[i];
      
      float dxy = std::sqrt((sv.vx() - pv.x()) * (sv.vx() - pv.x()) + 
                            (sv.vy() - pv.y()) * (sv.vy() - pv.y()));
      float dxyErr = std::sqrt(sv.vertexCovariance(0, 0) + sv.vertexCovariance(1, 1));
      float dxySig = dxyErr > 0 ? dxy / dxyErr : 0;

      sv2_pt_.push_back(sv.pt());
      sv2_eta_.push_back(sv.eta());
      sv2_phi_.push_back(sv.phi());
      sv2_mass_.push_back(sv.mass());
      sv2_x_.push_back(sv.vx());
      sv2_y_.push_back(sv.vy());
      sv2_z_.push_back(sv.vz());
      sv2_chi2_.push_back(sv.vertexChi2());
      sv2_ndof_.push_back(sv.vertexNdof());
      sv2_ntracks_.push_back(sv.numberOfDaughters());
      sv2_dxy_.push_back(dxy);
      sv2_dxyErr_.push_back(dxyErr);
      sv2_dxySig_.push_back(dxySig);
      sv2_matchIdx_.push_back(-1);

      pt2_vec.push_back(sv.pt());
      mass2_vec.push_back(sv.mass());
      chi2_2_vec.push_back(sv.vertexChi2());
      ndof2_vec.push_back(sv.vertexNdof());
      ntrk2_vec.push_back(sv.numberOfDaughters());
      dxy2_vec.push_back(dxy);
      dxySig2_vec.push_back(dxySig);
    }
  }

  // Calculate statistics for collection 1
  sv1_pt_mean_ = calculateMean(pt1_vec);
  sv1_pt_std_ = calculateStdDev(pt1_vec, sv1_pt_mean_);
  sv1_mass_mean_ = calculateMean(mass1_vec);
  sv1_mass_std_ = calculateStdDev(mass1_vec, sv1_mass_mean_);
  sv1_chi2_mean_ = calculateMean(chi2_1_vec);
  sv1_ndof_mean_ = calculateMean(ndof1_vec);
  sv1_ntrk_mean_ = calculateMean(ntrk1_vec);
  sv1_dxy_mean_ = calculateMean(dxy1_vec);
  sv1_dxySig_mean_ = calculateMean(dxySig1_vec);

  // Calculate statistics for collection 2
  sv2_pt_mean_ = calculateMean(pt2_vec);
  sv2_pt_std_ = calculateStdDev(pt2_vec, sv2_pt_mean_);
  sv2_mass_mean_ = calculateMean(mass2_vec);
  sv2_mass_std_ = calculateStdDev(mass2_vec, sv2_mass_mean_);
  sv2_chi2_mean_ = calculateMean(chi2_2_vec);
  sv2_ndof_mean_ = calculateMean(ndof2_vec);
  sv2_ntrk_mean_ = calculateMean(ntrk2_vec);
  sv2_dxy_mean_ = calculateMean(dxy2_vec);
  sv2_dxySig_mean_ = calculateMean(dxySig2_vec);

  // Matching: find closest SV in collection 2 for each SV in collection 1
  std::vector<float> matched_dPt, matched_dMass, matched_dR, matched_dZ;
  std::vector<bool> matched2(nSV2_, false);

  if (svs1.isValid() && svs2.isValid()) {
    for (size_t i = 0; i < svs1->size(); i++) {
      const auto& sv1 = (*svs1)[i];
      
      float bestDR = 999;
      int bestIdx = -1;
      
      for (size_t j = 0; j < svs2->size(); j++) {
        if (matched2[j]) continue;  // Already matched
        
        const auto& sv2 = (*svs2)[j];
        float dR = reco::deltaR(sv1.eta(), sv1.phi(), sv2.eta(), sv2.phi());
        float dZ = std::abs(sv1.vz() - sv2.vz());
        
        if (dR < maxDeltaR_ && dZ < maxDeltaZ_ && dR < bestDR) {
          bestDR = dR;
          bestIdx = j;
        }
      }
      
      if (bestIdx >= 0) {
        sv1_matchIdx_[i] = bestIdx;
        sv2_matchIdx_[bestIdx] = i;
        matched2[bestIdx] = true;
        
        const auto& sv2 = (*svs2)[bestIdx];
        float dPt = sv1.pt() - sv2.pt();
        float dMass = sv1.mass() - sv2.mass();
        float dZ = sv1.vz() - sv2.vz();
        
        matched_dPt.push_back(dPt);
        matched_dMass.push_back(dMass);
        matched_dR.push_back(bestDR);
        matched_dZ.push_back(dZ);
        
        h_matchedDR_->Fill(bestDR);
        h_matchedDPt_->Fill(dPt);
        h_matchedDMass_->Fill(dMass);
      }
    }
  }

  nMatched_ = matched_dPt.size();
  matchFraction1_ = nSV1_ > 0 ? float(nMatched_) / nSV1_ : 0;
  matchFraction2_ = nSV2_ > 0 ? float(nMatched_) / nSV2_ : 0;

  matched_dPt_mean_ = calculateMean(matched_dPt);
  matched_dMass_mean_ = calculateMean(matched_dMass);
  matched_dR_mean_ = calculateMean(matched_dR);
  matched_dZ_mean_ = calculateMean(matched_dZ);

  tree_->Fill();
}

void SecondaryVertexComparisonAnalyzer::endJob() {
  edm::LogInfo("SecondaryVertexComparisonAnalyzer") 
      << "Processed events. Check output histograms and tree.";
}

void SecondaryVertexComparisonAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("svSrc1", edm::InputTag("slimmedSecondaryVertices"));
  desc.add<edm::InputTag>("svSrc2", edm::InputTag("inclusiveCandidateSecondaryVertices"));
  desc.add<edm::InputTag>("pvSrc", edm::InputTag("offlineSlimmedPrimaryVertices"));
  desc.add<std::string>("label1", "slimmedSV");
  desc.add<std::string>("label2", "customSV");
  desc.add<double>("maxDeltaR", 0.3);
  desc.add<double>("maxDeltaZ", 1.0);
  descriptions.add("secondaryVertexComparisonAnalyzer", desc);
}

DEFINE_FWK_MODULE(SecondaryVertexComparisonAnalyzer);
