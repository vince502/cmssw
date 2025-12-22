// UnpackedTrackComparisonAnalyzer: Compare unpackedTracksAndVertices with its major input (packedPFCandidates)
// Validates track unpacking by comparing kinematics between input and output
//
// Compares:
//   - packedPFCandidates (input to unpacker)
//   - unpackedTracksAndVertices (output of unpacker)
//   - offlineSlimmedPrimaryVertices (vertices)
//
// Does NOT include lostTracks in comparison

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include <vector>
#include <cmath>

class UnpackedTrackComparisonAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit UnpackedTrackComparisonAnalyzer(const edm::ParameterSet&);
  ~UnpackedTrackComparisonAnalyzer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // Tokens
  edm::EDGetTokenT<pat::PackedCandidateCollection> packedPFCandToken_;
  edm::EDGetTokenT<reco::TrackCollection> unpackedTrackToken_;
  edm::EDGetTokenT<reco::VertexCollection> unpackedVertexToken_;
  edm::EDGetTokenT<reco::VertexCollection> slimmedVertexToken_;

  // Output tree
  TTree* tree_;
  
  // Event info
  unsigned long long event_;
  int run_;
  int lumi_;
  
  // Counts
  int nPackedPF_;
  int nPackedPFWithTrack_;
  int nUnpackedTracks_;
  int nSlimmedVertices_;
  int nUnpackedVertices_;
  
  // Matched track info (for detailed comparison)
  std::vector<float> packed_pt_;
  std::vector<float> packed_eta_;
  std::vector<float> packed_phi_;
  std::vector<int> packed_charge_;
  std::vector<float> packed_dxy_;
  std::vector<float> packed_dz_;
  std::vector<int> packed_hasTrackDetails_;
  std::vector<int> packed_highPurity_;
  
  std::vector<float> unpacked_pt_;
  std::vector<float> unpacked_eta_;
  std::vector<float> unpacked_phi_;
  std::vector<int> unpacked_charge_;
  std::vector<float> unpacked_dxy_;
  std::vector<float> unpacked_dz_;
  std::vector<float> unpacked_chi2_;
  std::vector<int> unpacked_nHits_;
  std::vector<int> unpacked_highPurity_;
  
  // Histograms for quick validation
  TH1F* h_nPackedPF_;
  TH1F* h_nPackedPFWithTrack_;
  TH1F* h_nUnpackedTracks_;
  TH1F* h_deltaPt_;
  TH1F* h_deltaEta_;
  TH1F* h_deltaPhi_;
  TH2F* h_pt_packed_vs_unpacked_;
};

UnpackedTrackComparisonAnalyzer::UnpackedTrackComparisonAnalyzer(const edm::ParameterSet& iConfig)
    : packedPFCandToken_(consumes<pat::PackedCandidateCollection>(
          iConfig.getParameter<edm::InputTag>("packedPFCandidates"))),
      unpackedTrackToken_(consumes<reco::TrackCollection>(
          iConfig.getParameter<edm::InputTag>("unpackedTracks"))),
      unpackedVertexToken_(consumes<reco::VertexCollection>(
          iConfig.getParameter<edm::InputTag>("unpackedVertices"))),
      slimmedVertexToken_(consumes<reco::VertexCollection>(
          iConfig.getParameter<edm::InputTag>("slimmedVertices"))) {
  usesResource("TFileService");
}

void UnpackedTrackComparisonAnalyzer::beginJob() {
  edm::Service<TFileService> fs;
  
  // Create tree
  tree_ = fs->make<TTree>("TrackComparison", "Packed vs Unpacked Track Comparison");
  
  tree_->Branch("event", &event_);
  tree_->Branch("run", &run_);
  tree_->Branch("lumi", &lumi_);
  
  tree_->Branch("nPackedPF", &nPackedPF_);
  tree_->Branch("nPackedPFWithTrack", &nPackedPFWithTrack_);
  tree_->Branch("nUnpackedTracks", &nUnpackedTracks_);
  tree_->Branch("nSlimmedVertices", &nSlimmedVertices_);
  tree_->Branch("nUnpackedVertices", &nUnpackedVertices_);
  
  tree_->Branch("packed_pt", &packed_pt_);
  tree_->Branch("packed_eta", &packed_eta_);
  tree_->Branch("packed_phi", &packed_phi_);
  tree_->Branch("packed_charge", &packed_charge_);
  tree_->Branch("packed_dxy", &packed_dxy_);
  tree_->Branch("packed_dz", &packed_dz_);
  tree_->Branch("packed_hasTrackDetails", &packed_hasTrackDetails_);
  tree_->Branch("packed_highPurity", &packed_highPurity_);
  
  tree_->Branch("unpacked_pt", &unpacked_pt_);
  tree_->Branch("unpacked_eta", &unpacked_eta_);
  tree_->Branch("unpacked_phi", &unpacked_phi_);
  tree_->Branch("unpacked_charge", &unpacked_charge_);
  tree_->Branch("unpacked_dxy", &unpacked_dxy_);
  tree_->Branch("unpacked_dz", &unpacked_dz_);
  tree_->Branch("unpacked_chi2", &unpacked_chi2_);
  tree_->Branch("unpacked_nHits", &unpacked_nHits_);
  tree_->Branch("unpacked_highPurity", &unpacked_highPurity_);
  
  // Histograms
  h_nPackedPF_ = fs->make<TH1F>("h_nPackedPF", "Number of PackedPFCandidates;N;Events", 500, 0, 5000);
  h_nPackedPFWithTrack_ = fs->make<TH1F>("h_nPackedPFWithTrack", "PackedPF with track details;N;Events", 500, 0, 5000);
  h_nUnpackedTracks_ = fs->make<TH1F>("h_nUnpackedTracks", "Number of Unpacked Tracks;N;Events", 500, 0, 5000);
  
  h_deltaPt_ = fs->make<TH1F>("h_deltaPt", "#Delta p_{T} (unpacked - packed);#Delta p_{T} [GeV];Tracks", 200, -0.1, 0.1);
  h_deltaEta_ = fs->make<TH1F>("h_deltaEta", "#Delta #eta (unpacked - packed);#Delta #eta;Tracks", 200, -0.01, 0.01);
  h_deltaPhi_ = fs->make<TH1F>("h_deltaPhi", "#Delta #phi (unpacked - packed);#Delta #phi;Tracks", 200, -0.01, 0.01);
  h_pt_packed_vs_unpacked_ = fs->make<TH2F>("h_pt_packed_vs_unpacked", 
      "p_{T} comparison;Packed p_{T} [GeV];Unpacked p_{T} [GeV]", 100, 0, 50, 100, 0, 50);
}

void UnpackedTrackComparisonAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Clear vectors
  packed_pt_.clear();
  packed_eta_.clear();
  packed_phi_.clear();
  packed_charge_.clear();
  packed_dxy_.clear();
  packed_dz_.clear();
  packed_hasTrackDetails_.clear();
  packed_highPurity_.clear();
  
  unpacked_pt_.clear();
  unpacked_eta_.clear();
  unpacked_phi_.clear();
  unpacked_charge_.clear();
  unpacked_dxy_.clear();
  unpacked_dz_.clear();
  unpacked_chi2_.clear();
  unpacked_nHits_.clear();
  unpacked_highPurity_.clear();
  
  // Event info
  event_ = iEvent.id().event();
  run_ = iEvent.id().run();
  lumi_ = iEvent.luminosityBlock();
  
  // Get collections
  edm::Handle<pat::PackedCandidateCollection> packedPFCands;
  edm::Handle<reco::TrackCollection> unpackedTracks;
  edm::Handle<reco::VertexCollection> unpackedVertices;
  edm::Handle<reco::VertexCollection> slimmedVertices;
  
  iEvent.getByToken(packedPFCandToken_, packedPFCands);
  iEvent.getByToken(unpackedTrackToken_, unpackedTracks);
  iEvent.getByToken(unpackedVertexToken_, unpackedVertices);
  iEvent.getByToken(slimmedVertexToken_, slimmedVertices);
  
  // Count and fill packed PF candidates
  nPackedPF_ = 0;
  nPackedPFWithTrack_ = 0;
  
  if (packedPFCands.isValid()) {
    nPackedPF_ = packedPFCands->size();
    
    for (const auto& cand : *packedPFCands) {
      if (cand.hasTrackDetails()) {
        nPackedPFWithTrack_++;
        
        const auto& track = cand.pseudoTrack();
        packed_pt_.push_back(track.pt());
        packed_eta_.push_back(track.eta());
        packed_phi_.push_back(track.phi());
        packed_charge_.push_back(cand.charge());
        packed_dxy_.push_back(cand.dxy());
        packed_dz_.push_back(cand.dz());
        packed_hasTrackDetails_.push_back(1);
        packed_highPurity_.push_back(cand.trackHighPurity() ? 1 : 0);
      }
    }
  }
  
  // Count and fill unpacked tracks
  nUnpackedTracks_ = 0;
  
  if (unpackedTracks.isValid()) {
    nUnpackedTracks_ = unpackedTracks->size();
    
    for (const auto& track : *unpackedTracks) {
      unpacked_pt_.push_back(track.pt());
      unpacked_eta_.push_back(track.eta());
      unpacked_phi_.push_back(track.phi());
      unpacked_charge_.push_back(track.charge());
      unpacked_dxy_.push_back(track.dxy());
      unpacked_dz_.push_back(track.dz());
      unpacked_chi2_.push_back(track.normalizedChi2());
      unpacked_nHits_.push_back(track.numberOfValidHits());
      unpacked_highPurity_.push_back(track.quality(reco::TrackBase::highPurity) ? 1 : 0);
    }
  }
  
  // Vertices
  nSlimmedVertices_ = slimmedVertices.isValid() ? slimmedVertices->size() : 0;
  nUnpackedVertices_ = unpackedVertices.isValid() ? unpackedVertices->size() : 0;
  
  // Fill histograms
  h_nPackedPF_->Fill(nPackedPF_);
  h_nPackedPFWithTrack_->Fill(nPackedPFWithTrack_);
  h_nUnpackedTracks_->Fill(nUnpackedTracks_);
  
  // Match tracks by deltaR and compare (simple nearest neighbor)
  // This is for validation - packed tracks with details should match unpacked tracks
  if (packedPFCands.isValid() && unpackedTracks.isValid()) {
    for (const auto& cand : *packedPFCands) {
      if (!cand.hasTrackDetails()) continue;
      
      const auto& packedTrack = cand.pseudoTrack();
      float packedPt = packedTrack.pt();
      float packedEta = packedTrack.eta();
      float packedPhi = packedTrack.phi();
      
      // Find best matching unpacked track
      float bestDR = 999;
      float bestDPt = 999;
      const reco::Track* bestMatch = nullptr;
      
      for (const auto& track : *unpackedTracks) {
        float dEta = track.eta() - packedEta;
        float dPhi = track.phi() - packedPhi;
        while (dPhi > M_PI) dPhi -= 2*M_PI;
        while (dPhi < -M_PI) dPhi += 2*M_PI;
        float dR = std::sqrt(dEta*dEta + dPhi*dPhi);
        
        if (dR < bestDR && dR < 0.01) {  // Require very close match
          bestDR = dR;
          bestDPt = track.pt() - packedPt;
          bestMatch = &track;
        }
      }
      
      if (bestMatch) {
        h_deltaPt_->Fill(bestDPt);
        h_deltaEta_->Fill(bestMatch->eta() - packedEta);
        h_deltaPhi_->Fill(bestMatch->phi() - packedPhi);
        h_pt_packed_vs_unpacked_->Fill(packedPt, bestMatch->pt());
      }
    }
  }
  
  tree_->Fill();
}

void UnpackedTrackComparisonAnalyzer::endJob() {
  edm::LogInfo("UnpackedTrackComparisonAnalyzer") 
      << "Analysis complete. Check TrackComparison tree and histograms for validation.";
}

void UnpackedTrackComparisonAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("packedPFCandidates", edm::InputTag("packedPFCandidates"));
  desc.add<edm::InputTag>("unpackedTracks", edm::InputTag("unpackedTracksAndVertices"));
  desc.add<edm::InputTag>("unpackedVertices", edm::InputTag("unpackedTracksAndVertices"));
  desc.add<edm::InputTag>("slimmedVertices", edm::InputTag("offlineSlimmedPrimaryVertices"));
  descriptions.add("unpackedTrackComparison", desc);
}

DEFINE_FWK_MODULE(UnpackedTrackComparisonAnalyzer);
