#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "PhysicsTools/XGBoost/interface/XGBooster.h"
#include "correction.h"
#include "fastjet/contrib/SoftKiller.hh"

namespace {
  enum class WP { WP95, WP90, WP85, WP80 };

  bool passMVAIso(const double mva, const double cent, const WP wp) {
    double cut = 10.;
    const auto cen = cent > 90. ? 90. : cent;
    const auto cen2 = cen * cen;
    const auto cen3 = cen * cen * cen;

    if (wp == WP::WP95)
      cut = 7.978478076287510e-07 * cen3 + -0.00010197402752356007 * cen2 +
            0.00073749187425983740 * cent + 0.44973546555978620;
    else if (wp == WP::WP90)
      cut = 5.023194760398722e-07 * cen3 + -6.386564313645383e-05 * cen2 +
            -0.00030034696427764694 * cent + 0.26733467400525280;
    else if (wp == WP::WP85)
      cut = 3.642678187960558e-07 * cen3 + -4.4289339403249526e-05 * cen2 +
            -0.00038178775816005510 * cent + 0.17242030428600790;
    else if (wp == WP::WP80)
      cut = 2.792961957599443e-07 * cen3 + -3.314677611344172e-05 * cen2 +
            -0.00028826679894283433 * cent + 0.11887071187630002;

    return mva < cut;
  }
}  // namespace

namespace pat {

  class HiOniaMuonMVAProducer : public edm::global::EDProducer<> {
  public:
    explicit HiOniaMuonMVAProducer(const edm::ParameterSet& iConfig)
        : muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"))),
          pfCandidateToken_(consumes<reco::CandidateView>(iConfig.getParameter<edm::InputTag>("pfCandidates"))),
          centralityToken_(consumes<int>(iConfig.getParameter<edm::InputTag>("centrality"))),
          etaToken_(consumes<std::vector<double>>(iConfig.getParameter<edm::InputTag>("etaMap"))),
          rhoToken_(consumes<std::vector<double>>(iConfig.getParameter<edm::InputTag>("rhoMap"))),
          patMuonPutToken_(produces<pat::MuonCollection>()),
          pfMaxEta_(iConfig.getParameter<double>("pf_maxAbsEta")),
          skRadius_(iConfig.getParameter<double>("sk_radius")),
          muonMinPt_(iConfig.getParameter<double>("muon_minPt")),
          rVeto_(iConfig.getParameter<double>("iso_rVeto")),
          rCone_(iConfig.getParameter<double>("iso_rCone")),
          isoCorr_(correction::CorrectionSet::from_file(iConfig.getParameter<edm::FileInPath>("file_isoCorr").fullPath())
                       ->at("iso_rho_correction")),
          isoModel_(std::make_unique<XGBooster>(iConfig.getParameter<edm::FileInPath>("file_isoModel").fullPath())) {
      for (int i = 0; i < 8; ++i)
        isoModel_->addFeature(std::to_string(i));
    }

    void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const override {
      const auto& muons = iEvent.get(muonToken_);
      const auto& pfCandidates = iEvent.get(pfCandidateToken_);
      const auto& etaMap = iEvent.get(etaToken_);
      const auto& rhoMap = iEvent.get(rhoToken_);
      const double cent = iEvent.get(centralityToken_) / 2.0;

      std::vector<std::tuple<double, double, double, int, int, double>> selPFCands;
      if (etaMap.size() > 1) {
        selPFCands.reserve(pfCandidates.size());
        std::vector<std::vector<fastjet::PseudoJet>> particlesForSK(etaMap.size() - 1);
        for (const auto& pf : pfCandidates) {
          int ieta = -1;
          for (size_t i = 1; i < etaMap.size(); ++i) {
            if (pf.eta() >= etaMap[i - 1] && pf.eta() < etaMap[i]) {
              ieta = i - 1;
              break;
            }
          }
          if (ieta < 0)
            continue;

          particlesForSK[ieta].emplace_back(pf.px(), pf.py(), pf.pz(), pf.energy());
          const auto& id = convert_.translatePdgIdToType(pf.pdgId());
          if (id > 0 && id <= 5 && std::abs(pf.eta()) <= pfMaxEta_)
            selPFCands.emplace_back(pf.pt(), pf.eta(), pf.phi(), id, ieta, 0.0);
        }

        std::vector<double> skThrs(etaMap.size() - 1);
        for (size_t i = 0; i < particlesForSK.size(); ++i) {
          const auto& particles = particlesForSK[i];
          if (!particles.empty()) {
            fastjet::contrib::SoftKiller softKiller(etaMap[i], etaMap[i + 1], skRadius_, skRadius_);
            std::vector<fastjet::PseudoJet> softKilledEvent;
            softKiller.apply(particles, softKilledEvent, skThrs[i]);
          }
        }

        for (auto& cand : selPFCands)
          std::get<5>(cand) = skThrs[std::get<4>(cand)];
      }

      pat::MuonCollection output(muons);
      for (auto& muon : output) {
        if (muon.pt() < muonMinPt_)
          continue;

        double rho = -1.;
        for (size_t i = 1; i < etaMap.size(); ++i) {
          if (muon.eta() >= etaMap[i - 1] && muon.eta() < etaMap[i]) {
            rho = rhoMap[i - 1];
            break;
          }
        }
        if (rho < 0)
          continue;

        const auto ip3DSig = std::abs(muon.dB(pat::Muon::PV3D)) / muon.edB(pat::Muon::PV3D);

        double skPFChIso = 0.;
        double skPFNeuIso = 0.;
        double skPFPhoIso = 0.;
        for (const auto& cand : selPFCands) {
          const auto& [pt, eta, phi, id, ieta, skThr] = cand;
          const auto dR2 = reco::deltaR2(muon.eta(), muon.phi(), eta, phi);
          if (dR2 >= rVeto_ * rVeto_ && dR2 <= rCone_ * rCone_)
            (id == 5 ? skPFNeuIso : (id == 4 ? skPFPhoIso : skPFChIso)) += pt * (pt > skThr);
        }

        const auto skPFIso = skPFChIso + skPFNeuIso + skPFPhoIso;
        const auto pfChIso = muon.pfIsolationR04().sumChargedHadronPt;
        const auto pfNeuIso = muon.pfIsolationR04().sumNeutralHadronEt;
        const auto pfPhoIso = muon.pfIsolationR04().sumPhotonEt;
        const auto pfIso = pfChIso + pfNeuIso + pfPhoIso;

        const auto pfRelIso = (pfIso - isoCorr_->evaluate({{"PFIso", "mu", rho}})) / muon.pt();
        const auto pfChRelIso = (pfChIso - isoCorr_->evaluate({{"PFChIso", "mu", rho}})) / muon.pt();
        const auto skPFRelIso = (skPFIso - isoCorr_->evaluate({{"skPFIso", "mu", rho}})) / muon.pt();
        const auto skPFChRelIso = (skPFChIso - isoCorr_->evaluate({{"skPFChIso", "mu", rho}})) / muon.pt();

        const std::vector<double> inputs{
            std::abs(muon.eta()), muon.phi(), rho, ip3DSig, pfRelIso, pfChRelIso, skPFRelIso, skPFChRelIso};
        const std::vector<float> features(inputs.begin(), inputs.end());
        const auto isoValue = 1. - isoModel_->predict(features);

        muon.addUserFloat("hiMVAIso", isoValue);
        muon.addUserInt("hiMVAIsoWP95", passMVAIso(isoValue, cent, WP::WP95));
        muon.addUserInt("hiMVAIsoWP90", passMVAIso(isoValue, cent, WP::WP90));
        muon.addUserInt("hiMVAIsoWP85", passMVAIso(isoValue, cent, WP::WP85));
        muon.addUserInt("hiMVAIsoWP80", passMVAIso(isoValue, cent, WP::WP80));
      }

      iEvent.emplace(patMuonPutToken_, std::move(output));
    }

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
      edm::ParameterSetDescription desc;
      desc.add<edm::InputTag>("muons", edm::InputTag("patMuonsWithTrigger"));
      desc.add<edm::InputTag>("pfCandidates", edm::InputTag("packedPFCandidates"));
      desc.add<edm::InputTag>("centrality", edm::InputTag("centralityBin", "HFtowers"));
      desc.add<edm::InputTag>("etaMap", edm::InputTag("hiFJRhoProducerFinerBins", "mapEtaEdges"));
      desc.add<edm::InputTag>("rhoMap", edm::InputTag("hiFJRhoProducerFinerBins", "mapToRho"));
      desc.add<double>("pf_maxAbsEta", 2.8);
      desc.add<double>("sk_radius", 0.4);
      desc.add<double>("muon_minPt", 0.0);
      desc.add<double>("iso_rVeto", 1.e-3);
      desc.add<double>("iso_rCone", 0.3);
      desc.add<edm::FileInPath>("file_isoModel",
                                edm::FileInPath("HiAnalysis/HiOnia/data/Run3_2023_PbPb/muiso_BDT.ubj"));
      desc.add<edm::FileInPath>(
          "file_isoCorr",
          edm::FileInPath("HeavyIonsAnalysis/Configuration/data/lepton_spectra_train_weights_Run3_2023_PbPb.json.gz"));
      descriptions.add("hiOniaMuons", desc);
    }

  private:
    const edm::EDGetTokenT<pat::MuonCollection> muonToken_;
    const edm::EDGetTokenT<reco::CandidateView> pfCandidateToken_;
    const edm::EDGetTokenT<int> centralityToken_;
    const edm::EDGetTokenT<std::vector<double>> etaToken_;
    const edm::EDGetTokenT<std::vector<double>> rhoToken_;
    const edm::EDPutTokenT<pat::MuonCollection> patMuonPutToken_;
    const reco::PFCandidate convert_;
    const double pfMaxEta_;
    const double skRadius_;
    const double muonMinPt_;
    const double rVeto_;
    const double rCone_;
    const std::shared_ptr<const correction::Correction> isoCorr_;
    const std::unique_ptr<XGBooster> isoModel_;
  };

}  // namespace pat

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(HiOniaMuonMVAProducer);
