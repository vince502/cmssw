// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BcSemiLeptonicProducer
//
/**\class BcSemiLeptonicProducer
 *
 * Description: Build visible Bc semileptonic candidates from J/psi and an extra lepton.
 *
 * Channels:
 *   0 = J/psi(mu mu) + mu (+nu, missing)
 *   1 = J/psi(mu mu) + e  (+nu, missing)
 *   2 = J/psi(e  e ) + mu (+nu, missing)
 *   3 = J/psi(e  e ) + e  (+nu, missing)
 *
 * Notes:
 *   - Neutrino is not reconstructed.
 *   - MET is intentionally not used here.
 */

#ifndef VertexCompositeAnalysis_BcSemiLeptonicProducer_h
#define VertexCompositeAnalysis_BcSemiLeptonicProducer_h

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

namespace edm {
class ConfigurationDescriptions;
}

class BcSemiLeptonicProducer : public edm::stream::EDProducer<> {
public:
  using CCC = pat::CompositeCandidateCollection;

  explicit BcSemiLeptonicProducer(const edm::ParameterSet&);
  ~BcSemiLeptonicProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  enum class ChannelId : int {
    MuMuMu = 0,
    MuMuE = 1,
    EEMu = 2,
    EEE = 3
  };

  struct GenLeptonInfo {
    reco::Candidate::LorentzVector p4;
    int pdgId = 0;
    int charge = 0;
  };

  struct GenBcTruthInfo {
    int channelId = -1;
    int bcPdgId = 0;
    int jpsiPdgId = 0;
    GenLeptonInfo jpsiLep1;
    GenLeptonInfo jpsiLep2;
    GenLeptonInfo wLep;
  };

  struct RecoLeptonInfo {
    reco::Candidate::LorentzVector p4;
    int charge = 0;
  };

  struct MatchResult {
    int genIndex = -1;
    int nMatched = 0;
    int isMatched = 0;
    std::array<float, 3> dR{{-1.f, -1.f, -1.f}};
    std::array<int, 3> recoToGen{{-1, -1, -1}};
  };

  void produce(edm::Event&, const edm::EventSetup&) override;

  bool passJpsi(const pat::CompositeCandidate&) const;
  bool passMuon(const pat::Muon&) const;
  bool passElectron(const pat::Electron&) const;
  bool passVisible(const pat::CompositeCandidate&) const;

  bool overlapsWithJpsiDaughters(const reco::Candidate& lep, const pat::CompositeCandidate& jpsi) const;

  pat::CompositeCandidate buildVisibleCandidate(const pat::CompositeCandidate& jpsi,
                                                const reco::Candidate& lep,
                                                ChannelId channel,
                                                int jpsiIdx,
                                                int wLepIdx) const;
  pat::CompositeCandidate buildGenVisibleCandidate(const GenBcTruthInfo& truth) const;

  int channelIdFromFlavors(int jpsiAbsLepPdgId, int wAbsLepPdgId) const;
  void collectFinalStateLeptons(const reco::Candidate* start, std::vector<const reco::GenParticle*>& out) const;
  std::vector<GenBcTruthInfo> buildGenTruthInfo(const reco::GenParticleCollection& genParticles) const;
  std::array<RecoLeptonInfo, 3> extractRecoLeptons(const pat::CompositeCandidate& cand) const;
  MatchResult matchRecoToGenThreeLeptons(const pat::CompositeCandidate& cand,
                                         const std::vector<GenBcTruthInfo>& genTruths) const;
  void addGenMatchingInfo(pat::CompositeCandidate& cand, const std::vector<GenBcTruthInfo>& genTruths) const;

  // Input tokens
  edm::EDGetTokenT<CCC> jpsiMuMuToken_;
  edm::EDGetTokenT<CCC> jpsiEEToken_;
  edm::EDGetTokenT<pat::MuonCollection> muonToken_;
  edm::EDGetTokenT<pat::ElectronCollection> electronToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> genToken_;

  // Selection parameters
  double minJpsiPt_;
  double maxJpsiAbsY_;
  double jpsiMassMin_;
  double jpsiMassMax_;

  double minMuonPt_;
  double maxMuonAbsEta_;
  bool requireGlobalMuon_;
  bool requirePFMuon_;

  double minElectronPt_;
  double maxElectronAbsEta_;

  double minBcVisiblePt_;
  double maxBcVisibleAbsY_;
  double minBcVisibleMass_;
  double maxBcVisibleMass_;

  double maxOverlapDR_;

  // Channel switches
  bool doMuMuMu_;
  bool doMuMuE_;
  bool doEEMu_;
  bool doEEE_;

  // MC truth matching
  bool doGenMatching_;
  double genMatchMaxDR_;
};

#endif
