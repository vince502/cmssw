// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BcSemiLeptonicProducer
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BcSemiLeptonicProducer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

namespace {
constexpr double kPi = 3.14159265358979323846;

inline double deltaPhi(double p1, double p2) {
  double dphi = std::fmod(p1 - p2, 2.0 * kPi);
  if (dphi > kPi) {
    dphi -= 2.0 * kPi;
  } else if (dphi <= -kPi) {
    dphi += 2.0 * kPi;
  }
  return dphi;
}

inline double deltaR2(double eta1, double phi1, double eta2, double phi2) {
  const double dEta = eta1 - eta2;
  const double dPhi = deltaPhi(phi1, phi2);
  return dEta * dEta + dPhi * dPhi;
}

inline bool isLeptonAbsPdgId(int absPdgId) { return (absPdgId == 11 || absPdgId == 13); }

inline int leptonChargeFromPdgId(int pdgId) {
  const int absPdgId = std::abs(pdgId);
  if (!isLeptonAbsPdgId(absPdgId)) {
    return 0;
  }
  return (pdgId > 0) ? -1 : 1;
}

inline bool isJpsiLikeAbsPdgId(int absPdgId) { return (absPdgId == 443 || absPdgId == 100443); }

bool isDescendantOf(const reco::Candidate* child, const reco::Candidate* ancestor, int depth = 0) {
  if (!child || !ancestor || depth > 100) {
    return false;
  }
  if (child == ancestor) {
    return true;
  }
  for (size_t i = 0; i < child->numberOfMothers(); ++i) {
    const reco::Candidate* mom = child->mother(i);
    if (!mom) {
      continue;
    }
    if (mom == ancestor || isDescendantOf(mom, ancestor, depth + 1)) {
      return true;
    }
  }
  return false;
}

}  // namespace

BcSemiLeptonicProducer::BcSemiLeptonicProducer(const edm::ParameterSet& iConfig)
    : jpsiMuMuToken_(consumes<CCC>(iConfig.getParameter<edm::InputTag>("jpsiMuMuSrc"))),
      jpsiEEToken_(consumes<CCC>(iConfig.getParameter<edm::InputTag>("jpsiEESrc"))),
      muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muonSrc"))),
      electronToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("electronSrc"))),
      genToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
      minJpsiPt_(iConfig.getParameter<double>("minJpsiPt")),
      maxJpsiAbsY_(iConfig.getParameter<double>("maxJpsiAbsY")),
      jpsiMassMin_(iConfig.getParameter<double>("jpsiMassMin")),
      jpsiMassMax_(iConfig.getParameter<double>("jpsiMassMax")),
      minMuonPt_(iConfig.getParameter<double>("minMuonPt")),
      maxMuonAbsEta_(iConfig.getParameter<double>("maxMuonAbsEta")),
      requireGlobalMuon_(iConfig.getParameter<bool>("requireGlobalMuon")),
      requirePFMuon_(iConfig.getParameter<bool>("requirePFMuon")),
      minElectronPt_(iConfig.getParameter<double>("minElectronPt")),
      maxElectronAbsEta_(iConfig.getParameter<double>("maxElectronAbsEta")),
      minBcVisiblePt_(iConfig.getParameter<double>("minBcVisiblePt")),
      maxBcVisibleAbsY_(iConfig.getParameter<double>("maxBcVisibleAbsY")),
      minBcVisibleMass_(iConfig.getParameter<double>("minBcVisibleMass")),
      maxBcVisibleMass_(iConfig.getParameter<double>("maxBcVisibleMass")),
      maxOverlapDR_(iConfig.getParameter<double>("maxOverlapDR")),
      doMuMuMu_(iConfig.getParameter<bool>("doJpsiMuMuMu")),
      doMuMuE_(iConfig.getParameter<bool>("doJpsiMuMuE")),
      doEEMu_(iConfig.getParameter<bool>("doJpsiEEMu")),
      doEEE_(iConfig.getParameter<bool>("doJpsiEEE")),
      doGenMatching_(iConfig.getParameter<bool>("doGenMatching")),
      genMatchMaxDR_(iConfig.getParameter<double>("genMatchMaxDR")) {
  produces<CCC>("BcToJpsiLepNu");
  produces<CCC>("BcToJpsiCrossLepNu");
  produces<CCC>("BcToJpsiMuMuMuNu");
  produces<CCC>("BcToJpsiMuMuENu");
  produces<CCC>("BcToJpsiEEMuNu");
  produces<CCC>("BcToJpsiEEENu");
  produces<CCC>("GenBcToJpsiLepNu");
  produces<CCC>("GenBcToJpsiCrossLepNu");
  produces<CCC>("GenBcToJpsiMuMuMuNu");
  produces<CCC>("GenBcToJpsiMuMuENu");
  produces<CCC>("GenBcToJpsiEEMuNu");
  produces<CCC>("GenBcToJpsiEEENu");
}

bool BcSemiLeptonicProducer::passJpsi(const pat::CompositeCandidate& jpsi) const {
  if (jpsi.pt() < minJpsiPt_) {
    return false;
  }
  if (std::abs(jpsi.p4().Rapidity()) > maxJpsiAbsY_) {
    return false;
  }
  if (jpsi.mass() < jpsiMassMin_ || jpsi.mass() > jpsiMassMax_) {
    return false;
  }
  return true;
}

bool BcSemiLeptonicProducer::passMuon(const pat::Muon& mu) const {
  if (mu.pt() < minMuonPt_) {
    return false;
  }
  if (std::abs(mu.eta()) > maxMuonAbsEta_) {
    return false;
  }
  if (requireGlobalMuon_ && !mu.isGlobalMuon()) {
    return false;
  }
  if (requirePFMuon_ && !mu.isPFMuon()) {
    return false;
  }
  return true;
}

bool BcSemiLeptonicProducer::passElectron(const pat::Electron& ele) const {
  if (ele.pt() < minElectronPt_) {
    return false;
  }
  if (std::abs(ele.eta()) > maxElectronAbsEta_) {
    return false;
  }
  return true;
}

bool BcSemiLeptonicProducer::passVisible(const pat::CompositeCandidate& cand) const {
  if (cand.pt() < minBcVisiblePt_) {
    return false;
  }
  if (std::abs(cand.p4().Rapidity()) > maxBcVisibleAbsY_) {
    return false;
  }
  if (cand.mass() < minBcVisibleMass_ || cand.mass() > maxBcVisibleMass_) {
    return false;
  }
  return true;
}

bool BcSemiLeptonicProducer::overlapsWithJpsiDaughters(const reco::Candidate& lep,
                                                       const pat::CompositeCandidate& jpsi) const {
  const double maxDR2 = maxOverlapDR_ * maxOverlapDR_;
  for (size_t i = 0; i < jpsi.numberOfDaughters(); ++i) {
    const reco::Candidate* dau = jpsi.daughter(i);
    if (!dau) {
      continue;
    }
    if (deltaR2(lep.eta(), lep.phi(), dau->eta(), dau->phi()) < maxDR2) {
      return true;
    }
  }
  return false;
}

pat::CompositeCandidate BcSemiLeptonicProducer::buildVisibleCandidate(const pat::CompositeCandidate& jpsi,
                                                                      const reco::Candidate& lep,
                                                                      ChannelId channel,
                                                                      int jpsiIdx,
                                                                      int wLepIdx) const {
  pat::CompositeCandidate out;
  out.addDaughter(jpsi, "jpsi");
  out.addDaughter(lep, "lep");

  const auto visP4 = jpsi.p4() + lep.p4();
  out.setP4(visP4);
  out.setCharge(lep.charge());
  out.setVertex(jpsi.vertex());

  const int chId = static_cast<int>(channel);
  const int isJpsiMuMu = (channel == ChannelId::MuMuMu || channel == ChannelId::MuMuE) ? 1 : 0;
  const int lepFlavor = (channel == ChannelId::MuMuMu || channel == ChannelId::EEMu) ? 13 : 11;
  const int isCrossFlavor = (channel == ChannelId::MuMuE || channel == ChannelId::EEMu) ? 1 : 0;

  out.addUserInt("channelId", chId);
  out.addUserInt("isJpsiMuMu", isJpsiMuMu);
  out.addUserInt("leptonFlavor", lepFlavor);
  out.addUserInt("isCrossFlavor", isCrossFlavor);
  out.addUserInt("hasNeutrino", 1);

  out.addUserFloat("jpsiMass", jpsi.mass());
  out.addUserFloat("visibleMass", visP4.mass());
  out.addUserFloat("visiblePt", visP4.pt());
  out.addUserFloat("visibleY", visP4.Rapidity());
  out.addUserFloat("lepPt", lep.pt());
  out.addUserFloat("lepEta", lep.eta());
  out.addUserInt("jpsiIdx", jpsiIdx);
  out.addUserInt("wLepIdx", wLepIdx);

  if (jpsi.hasUserFloat("vProb")) {
    out.addUserFloat("jpsiVProb", jpsi.userFloat("vProb"));
  }

  return out;
}

pat::CompositeCandidate BcSemiLeptonicProducer::buildGenVisibleCandidate(const GenBcTruthInfo& truth) const {
  pat::CompositeCandidate out;
  const auto visP4 = truth.jpsiLep1.p4 + truth.jpsiLep2.p4 + truth.wLep.p4;
  const auto jpsiP4 = truth.jpsiLep1.p4 + truth.jpsiLep2.p4;

  out.setP4(visP4);
  out.setCharge(truth.wLep.charge);

  const int chId = truth.channelId;
  const int isJpsiMuMu =
      (chId == static_cast<int>(ChannelId::MuMuMu) || chId == static_cast<int>(ChannelId::MuMuE)) ? 1 : 0;
  const int lepFlavor =
      (chId == static_cast<int>(ChannelId::MuMuMu) || chId == static_cast<int>(ChannelId::EEMu)) ? 13 : 11;
  const int isCrossFlavor =
      (chId == static_cast<int>(ChannelId::MuMuE) || chId == static_cast<int>(ChannelId::EEMu)) ? 1 : 0;

  out.addUserInt("channelId", chId);
  out.addUserInt("isJpsiMuMu", isJpsiMuMu);
  out.addUserInt("leptonFlavor", lepFlavor);
  out.addUserInt("isCrossFlavor", isCrossFlavor);
  out.addUserInt("hasNeutrino", 1);
  out.addUserInt("isGenOnly", 1);
  out.addUserInt("gen_bcPdgId", truth.bcPdgId);
  out.addUserInt("gen_jpsiPdgId", truth.jpsiPdgId);

  out.addUserFloat("jpsiMass", jpsiP4.mass());
  out.addUserFloat("visibleMass", visP4.mass());
  out.addUserFloat("visiblePt", visP4.pt());
  out.addUserFloat("visibleY", visP4.Rapidity());
  out.addUserFloat("gen_jpsiLep1Pt", truth.jpsiLep1.p4.pt());
  out.addUserFloat("gen_jpsiLep1Eta", truth.jpsiLep1.p4.eta());
  out.addUserFloat("gen_jpsiLep2Pt", truth.jpsiLep2.p4.pt());
  out.addUserFloat("gen_jpsiLep2Eta", truth.jpsiLep2.p4.eta());
  out.addUserFloat("gen_wLepPt", truth.wLep.p4.pt());
  out.addUserFloat("gen_wLepEta", truth.wLep.p4.eta());

  return out;
}

int BcSemiLeptonicProducer::channelIdFromFlavors(int jpsiAbsLepPdgId, int wAbsLepPdgId) const {
  if (jpsiAbsLepPdgId == 13 && wAbsLepPdgId == 13) {
    return static_cast<int>(ChannelId::MuMuMu);
  }
  if (jpsiAbsLepPdgId == 13 && wAbsLepPdgId == 11) {
    return static_cast<int>(ChannelId::MuMuE);
  }
  if (jpsiAbsLepPdgId == 11 && wAbsLepPdgId == 13) {
    return static_cast<int>(ChannelId::EEMu);
  }
  if (jpsiAbsLepPdgId == 11 && wAbsLepPdgId == 11) {
    return static_cast<int>(ChannelId::EEE);
  }
  return -1;
}

void BcSemiLeptonicProducer::collectFinalStateLeptons(const reco::Candidate* start,
                                                      std::vector<const reco::GenParticle*>& out) const {
  if (!start) {
    return;
  }
  const auto* gen = dynamic_cast<const reco::GenParticle*>(start);
  if (!gen) {
    return;
  }

  const int absId = std::abs(gen->pdgId());
  if (isLeptonAbsPdgId(absId)) {
    bool hasSameLeptonDaughter = false;
    for (size_t i = 0; i < gen->numberOfDaughters(); ++i) {
      const auto* dau = dynamic_cast<const reco::GenParticle*>(gen->daughter(i));
      if (!dau) {
        continue;
      }
      if (std::abs(dau->pdgId()) == absId) {
        hasSameLeptonDaughter = true;
        break;
      }
    }
    if (gen->status() == 1 || !hasSameLeptonDaughter) {
      out.push_back(gen);
      return;
    }
  }

  for (size_t i = 0; i < gen->numberOfDaughters(); ++i) {
    collectFinalStateLeptons(gen->daughter(i), out);
  }
}

std::vector<BcSemiLeptonicProducer::GenBcTruthInfo> BcSemiLeptonicProducer::buildGenTruthInfo(
    const reco::GenParticleCollection& genParticles) const {
  std::vector<GenBcTruthInfo> out;

  for (const auto& gp : genParticles) {
    if (std::abs(gp.pdgId()) != 541) {
      continue;
    }

    bool hasBcDaughter = false;
    for (size_t i = 0; i < gp.numberOfDaughters(); ++i) {
      const auto* dau = dynamic_cast<const reco::GenParticle*>(gp.daughter(i));
      if (dau && std::abs(dau->pdgId()) == 541) {
        hasBcDaughter = true;
        break;
      }
    }
    if (hasBcDaughter) {
      continue;
    }

    const reco::Candidate* jpsiCand = nullptr;
    std::vector<const reco::Candidate*> stack;
    stack.push_back(&gp);
    while (!stack.empty()) {
      const reco::Candidate* cur = stack.back();
      stack.pop_back();
      if (!cur) {
        continue;
      }
      for (size_t i = 0; i < cur->numberOfDaughters(); ++i) {
        const reco::Candidate* dau = cur->daughter(i);
        if (!dau) {
          continue;
        }
        if (!jpsiCand && isJpsiLikeAbsPdgId(std::abs(dau->pdgId()))) {
          jpsiCand = dau;
        }
        stack.push_back(dau);
      }
    }
    if (!jpsiCand) {
      continue;
    }

    std::vector<const reco::GenParticle*> jpsiFinalLeptons;
    collectFinalStateLeptons(jpsiCand, jpsiFinalLeptons);
    jpsiFinalLeptons.erase(
        std::remove_if(jpsiFinalLeptons.begin(), jpsiFinalLeptons.end(), [](const reco::GenParticle* p) {
          return (!p || !isLeptonAbsPdgId(std::abs(p->pdgId())));
        }),
        jpsiFinalLeptons.end());

    const reco::GenParticle* jpsiLep1 = nullptr;
    const reco::GenParticle* jpsiLep2 = nullptr;
    double bestJpsiPairPtSum = -1.0;
    for (size_t i = 0; i < jpsiFinalLeptons.size(); ++i) {
      const auto* li = jpsiFinalLeptons[i];
      if (!li) {
        continue;
      }
      for (size_t j = i + 1; j < jpsiFinalLeptons.size(); ++j) {
        const auto* lj = jpsiFinalLeptons[j];
        if (!lj) {
          continue;
        }
        if (std::abs(li->pdgId()) != std::abs(lj->pdgId())) {
          continue;
        }
        if (leptonChargeFromPdgId(li->pdgId()) * leptonChargeFromPdgId(lj->pdgId()) != -1) {
          continue;
        }
        const double ptSum = li->pt() + lj->pt();
        if (ptSum > bestJpsiPairPtSum) {
          bestJpsiPairPtSum = ptSum;
          jpsiLep1 = li;
          jpsiLep2 = lj;
        }
      }
    }
    if (!jpsiLep1 || !jpsiLep2) {
      continue;
    }

    std::vector<const reco::GenParticle*> bcFinalLeptons;
    for (size_t i = 0; i < gp.numberOfDaughters(); ++i) {
      collectFinalStateLeptons(gp.daughter(i), bcFinalLeptons);
    }
    bcFinalLeptons.erase(
        std::remove_if(bcFinalLeptons.begin(), bcFinalLeptons.end(), [](const reco::GenParticle* p) {
          return (!p || !isLeptonAbsPdgId(std::abs(p->pdgId())));
        }),
        bcFinalLeptons.end());

    const reco::GenParticle* wLep = nullptr;
    bool foundBcChargeMatched = false;
    double bestWPt = -1.0;
    const int bcCharge = (gp.pdgId() > 0) ? 1 : -1;
    for (const auto* lep : bcFinalLeptons) {
      if (!lep || lep == jpsiLep1 || lep == jpsiLep2) {
        continue;
      }
      if (isDescendantOf(lep, jpsiCand)) {
        continue;
      }

      const int lepCharge = leptonChargeFromPdgId(lep->pdgId());
      const bool chargeMatched = (lepCharge == bcCharge);
      if (chargeMatched) {
        if (!foundBcChargeMatched || lep->pt() > bestWPt) {
          foundBcChargeMatched = true;
          bestWPt = lep->pt();
          wLep = lep;
        }
      } else if (!foundBcChargeMatched && lep->pt() > bestWPt) {
        bestWPt = lep->pt();
        wLep = lep;
      }
    }
    if (!wLep) {
      continue;
    }

    const int jpsiAbsFlavor = std::abs(jpsiLep1->pdgId());
    const int wAbsFlavor = std::abs(wLep->pdgId());
    const int chId = channelIdFromFlavors(jpsiAbsFlavor, wAbsFlavor);
    if (chId < 0) {
      continue;
    }

    GenBcTruthInfo truth;
    truth.channelId = chId;
    truth.bcPdgId = gp.pdgId();
    truth.jpsiPdgId = jpsiCand->pdgId();
    truth.jpsiLep1.p4 = jpsiLep1->p4();
    truth.jpsiLep1.pdgId = jpsiLep1->pdgId();
    truth.jpsiLep1.charge = leptonChargeFromPdgId(jpsiLep1->pdgId());
    truth.jpsiLep2.p4 = jpsiLep2->p4();
    truth.jpsiLep2.pdgId = jpsiLep2->pdgId();
    truth.jpsiLep2.charge = leptonChargeFromPdgId(jpsiLep2->pdgId());
    truth.wLep.p4 = wLep->p4();
    truth.wLep.pdgId = wLep->pdgId();
    truth.wLep.charge = leptonChargeFromPdgId(wLep->pdgId());
    out.push_back(truth);
  }

  return out;
}

std::array<BcSemiLeptonicProducer::RecoLeptonInfo, 3> BcSemiLeptonicProducer::extractRecoLeptons(
    const pat::CompositeCandidate& cand) const {
  std::array<RecoLeptonInfo, 3> recoLeps;

  const reco::Candidate* jpsi = cand.daughter("jpsi");
  if (!jpsi && cand.numberOfDaughters() > 0) {
    jpsi = cand.daughter(0);
  }
  const reco::Candidate* lep = cand.daughter("lep");
  if (!lep && cand.numberOfDaughters() > 1) {
    lep = cand.daughter(1);
  }

  if (jpsi && jpsi->numberOfDaughters() > 0 && jpsi->daughter(0)) {
    recoLeps[0].p4 = jpsi->daughter(0)->p4();
    recoLeps[0].charge = static_cast<int>(std::lround(jpsi->daughter(0)->charge()));
  }
  if (jpsi && jpsi->numberOfDaughters() > 1 && jpsi->daughter(1)) {
    recoLeps[1].p4 = jpsi->daughter(1)->p4();
    recoLeps[1].charge = static_cast<int>(std::lround(jpsi->daughter(1)->charge()));
  }
  if (lep) {
    recoLeps[2].p4 = lep->p4();
    recoLeps[2].charge = static_cast<int>(std::lround(lep->charge()));
  }

  return recoLeps;
}

BcSemiLeptonicProducer::MatchResult BcSemiLeptonicProducer::matchRecoToGenThreeLeptons(
    const pat::CompositeCandidate& cand, const std::vector<GenBcTruthInfo>& genTruths) const {
  MatchResult best;
  const int recoChannel = cand.hasUserInt("channelId") ? cand.userInt("channelId") : -1;
  if (recoChannel < 0) {
    return best;
  }

  const auto recoLeps = extractRecoLeptons(cand);
  const int perms[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};

  for (size_t igen = 0; igen < genTruths.size(); ++igen) {
    const auto& truth = genTruths[igen];
    if (truth.channelId != recoChannel) {
      continue;
    }

    const GenLeptonInfo genLeps[3] = {truth.jpsiLep1, truth.jpsiLep2, truth.wLep};
    int bestPermMatched = -1;
    int bestPermNMatched = -1;
    double bestPermScore = std::numeric_limits<double>::max();
    std::array<float, 3> bestPermDR{{-1.f, -1.f, -1.f}};
    std::array<int, 3> bestPermRecoToGen{{-1, -1, -1}};

    for (int iperm = 0; iperm < 6; ++iperm) {
      bool chargeCompatible = true;
      int nMatched = 0;
      double score = 0.0;
      std::array<float, 3> thisDR{{-1.f, -1.f, -1.f}};
      std::array<int, 3> thisRecoToGen{{-1, -1, -1}};

      for (int i = 0; i < 3; ++i) {
        const int gidx = perms[iperm][i];
        thisRecoToGen[i] = gidx;

        const int recoQ = recoLeps[i].charge;
        const int genQ = genLeps[gidx].charge;
        if (recoQ != 0 && genQ != 0 && recoQ != genQ) {
          chargeCompatible = false;
          break;
        }

        const double dr = std::sqrt(deltaR2(recoLeps[i].p4.eta(), recoLeps[i].p4.phi(), genLeps[gidx].p4.eta(), genLeps[gidx].p4.phi()));
        thisDR[i] = static_cast<float>(dr);
        score += dr;
        if (dr < genMatchMaxDR_) {
          ++nMatched;
        }
      }

      if (!chargeCompatible) {
        continue;
      }
      if (nMatched > bestPermNMatched || (nMatched == bestPermNMatched && score < bestPermScore)) {
        bestPermMatched = iperm;
        bestPermNMatched = nMatched;
        bestPermScore = score;
        bestPermDR = thisDR;
        bestPermRecoToGen = thisRecoToGen;
      }
    }

    if (bestPermMatched < 0) {
      continue;
    }

    if (bestPermNMatched > best.nMatched ||
        (bestPermNMatched == best.nMatched &&
         (best.genIndex < 0 || (bestPermDR[0] + bestPermDR[1] + bestPermDR[2]) < (best.dR[0] + best.dR[1] + best.dR[2])))) {
      best.genIndex = static_cast<int>(igen);
      best.nMatched = bestPermNMatched;
      best.isMatched = (bestPermNMatched == 3) ? 1 : 0;
      best.dR = bestPermDR;
      best.recoToGen = bestPermRecoToGen;
    }
  }

  return best;
}

void BcSemiLeptonicProducer::addGenMatchingInfo(pat::CompositeCandidate& cand,
                                                const std::vector<GenBcTruthInfo>& genTruths) const {
  const MatchResult match = matchRecoToGenThreeLeptons(cand, genTruths);
  int genChannelId = -1;
  int genBcPdgId = 0;
  int genJpsiPdgId = 0;
  if (match.genIndex >= 0 && static_cast<size_t>(match.genIndex) < genTruths.size()) {
    const auto& truth = genTruths[match.genIndex];
    genChannelId = truth.channelId;
    genBcPdgId = truth.bcPdgId;
    genJpsiPdgId = truth.jpsiPdgId;
  }

  cand.addUserInt("gen_isMatched3L", match.isMatched);
  cand.addUserInt("gen_nMatchedLep", match.nMatched);
  cand.addUserInt("gen_matchedBcIdx", match.genIndex);
  cand.addUserInt("Reco_3lep_isGenMatched", match.isMatched);
  cand.addUserInt("Reco_3lep_nMatched", match.nMatched);
  cand.addUserInt("Reco_3lep_whichGen", match.genIndex);
  cand.addUserInt("gen_channelId", genChannelId);
  cand.addUserInt("gen_bcPdgId", genBcPdgId);
  cand.addUserInt("gen_jpsiPdgId", genJpsiPdgId);
  cand.addUserFloat("gen_match_dR_jpsiLep1", match.dR[0]);
  cand.addUserFloat("gen_match_dR_jpsiLep2", match.dR[1]);
  cand.addUserFloat("gen_match_dR_wLep", match.dR[2]);
  cand.addUserInt("gen_match_recoToGen_jpsiLep1", match.recoToGen[0]);
  cand.addUserInt("gen_match_recoToGen_jpsiLep2", match.recoToGen[1]);
  cand.addUserInt("gen_match_recoToGen_wLep", match.recoToGen[2]);
}

void BcSemiLeptonicProducer::produce(edm::Event& iEvent, const edm::EventSetup&) {
  edm::Handle<CCC> jpsiMuMuHandle;
  edm::Handle<CCC> jpsiEEHandle;
  edm::Handle<pat::MuonCollection> muonHandle;
  edm::Handle<pat::ElectronCollection> electronHandle;
  edm::Handle<reco::GenParticleCollection> genHandle;

  iEvent.getByToken(jpsiMuMuToken_, jpsiMuMuHandle);
  iEvent.getByToken(jpsiEEToken_, jpsiEEHandle);
  iEvent.getByToken(muonToken_, muonHandle);
  iEvent.getByToken(electronToken_, electronHandle);
  if (doGenMatching_) {
    iEvent.getByToken(genToken_, genHandle);
  }

  std::vector<GenBcTruthInfo> genTruths;
  if (doGenMatching_ && genHandle.isValid()) {
    genTruths = buildGenTruthInfo(*genHandle);
  }

  auto allOut = std::make_unique<CCC>();
  auto crossOut = std::make_unique<CCC>();
  auto mumuMuOut = std::make_unique<CCC>();
  auto mumuEOut = std::make_unique<CCC>();
  auto eeMuOut = std::make_unique<CCC>();
  auto eeEOut = std::make_unique<CCC>();
  auto genAllOut = std::make_unique<CCC>();
  auto genCrossOut = std::make_unique<CCC>();
  auto genMumuMuOut = std::make_unique<CCC>();
  auto genMumuEOut = std::make_unique<CCC>();
  auto genEeMuOut = std::make_unique<CCC>();
  auto genEeEOut = std::make_unique<CCC>();

  if (doGenMatching_) {
    for (const auto& truth : genTruths) {
      const int chId = truth.channelId;
      auto genCand = buildGenVisibleCandidate(truth);
      genAllOut->push_back(genCand);

      if (chId == static_cast<int>(ChannelId::MuMuMu)) {
        genMumuMuOut->push_back(genCand);
      } else if (chId == static_cast<int>(ChannelId::MuMuE)) {
        genMumuEOut->push_back(genCand);
        genCrossOut->push_back(genCand);
      } else if (chId == static_cast<int>(ChannelId::EEMu)) {
        genEeMuOut->push_back(genCand);
        genCrossOut->push_back(genCand);
      } else if (chId == static_cast<int>(ChannelId::EEE)) {
        genEeEOut->push_back(genCand);
      }
    }
  }

  if (jpsiMuMuHandle.isValid()) {
    for (size_t ijpsi = 0; ijpsi < jpsiMuMuHandle->size(); ++ijpsi) {
      const auto& jpsi = (*jpsiMuMuHandle)[ijpsi];
      if (!passJpsi(jpsi)) {
        continue;
      }

      if (muonHandle.isValid() && doMuMuMu_) {
        for (size_t imu = 0; imu < muonHandle->size(); ++imu) {
          const auto& mu = (*muonHandle)[imu];
          if (!passMuon(mu) || overlapsWithJpsiDaughters(mu, jpsi)) {
            continue;
          }
          auto cand =
              buildVisibleCandidate(jpsi, mu, ChannelId::MuMuMu, static_cast<int>(ijpsi), static_cast<int>(imu));
          if (!passVisible(cand)) {
            continue;
          }
          if (doGenMatching_) {
            addGenMatchingInfo(cand, genTruths);
          }
          mumuMuOut->push_back(cand);
          allOut->push_back(cand);
        }
      }

      if (electronHandle.isValid() && doMuMuE_) {
        for (size_t iele = 0; iele < electronHandle->size(); ++iele) {
          const auto& ele = (*electronHandle)[iele];
          if (!passElectron(ele) || overlapsWithJpsiDaughters(ele, jpsi)) {
            continue;
          }
          auto cand =
              buildVisibleCandidate(jpsi, ele, ChannelId::MuMuE, static_cast<int>(ijpsi), static_cast<int>(iele));
          if (!passVisible(cand)) {
            continue;
          }
          if (doGenMatching_) {
            addGenMatchingInfo(cand, genTruths);
          }
          mumuEOut->push_back(cand);
          crossOut->push_back(cand);
          allOut->push_back(cand);
        }
      }
    }
  }

  if (jpsiEEHandle.isValid()) {
    for (size_t ijpsi = 0; ijpsi < jpsiEEHandle->size(); ++ijpsi) {
      const auto& jpsi = (*jpsiEEHandle)[ijpsi];
      if (!passJpsi(jpsi)) {
        continue;
      }

      if (muonHandle.isValid() && doEEMu_) {
        for (size_t imu = 0; imu < muonHandle->size(); ++imu) {
          const auto& mu = (*muonHandle)[imu];
          if (!passMuon(mu) || overlapsWithJpsiDaughters(mu, jpsi)) {
            continue;
          }
          auto cand = buildVisibleCandidate(jpsi, mu, ChannelId::EEMu, static_cast<int>(ijpsi), static_cast<int>(imu));
          if (!passVisible(cand)) {
            continue;
          }
          if (doGenMatching_) {
            addGenMatchingInfo(cand, genTruths);
          }
          eeMuOut->push_back(cand);
          crossOut->push_back(cand);
          allOut->push_back(cand);
        }
      }

      if (electronHandle.isValid() && doEEE_) {
        for (size_t iele = 0; iele < electronHandle->size(); ++iele) {
          const auto& ele = (*electronHandle)[iele];
          if (!passElectron(ele) || overlapsWithJpsiDaughters(ele, jpsi)) {
            continue;
          }
          auto cand = buildVisibleCandidate(jpsi, ele, ChannelId::EEE, static_cast<int>(ijpsi), static_cast<int>(iele));
          if (!passVisible(cand)) {
            continue;
          }
          if (doGenMatching_) {
            addGenMatchingInfo(cand, genTruths);
          }
          eeEOut->push_back(cand);
          allOut->push_back(cand);
        }
      }
    }
  }

  iEvent.put(std::move(allOut), "BcToJpsiLepNu");
  iEvent.put(std::move(crossOut), "BcToJpsiCrossLepNu");
  iEvent.put(std::move(mumuMuOut), "BcToJpsiMuMuMuNu");
  iEvent.put(std::move(mumuEOut), "BcToJpsiMuMuENu");
  iEvent.put(std::move(eeMuOut), "BcToJpsiEEMuNu");
  iEvent.put(std::move(eeEOut), "BcToJpsiEEENu");
  iEvent.put(std::move(genAllOut), "GenBcToJpsiLepNu");
  iEvent.put(std::move(genCrossOut), "GenBcToJpsiCrossLepNu");
  iEvent.put(std::move(genMumuMuOut), "GenBcToJpsiMuMuMuNu");
  iEvent.put(std::move(genMumuEOut), "GenBcToJpsiMuMuENu");
  iEvent.put(std::move(genEeMuOut), "GenBcToJpsiEEMuNu");
  iEvent.put(std::move(genEeEOut), "GenBcToJpsiEEENu");
}

void BcSemiLeptonicProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("jpsiMuMuSrc", edm::InputTag("onia2MuMuPatGlbGlb"));
  desc.add<edm::InputTag>("jpsiEESrc", edm::InputTag("onia2ElectronElectronPatGlbGlb"));
  desc.add<edm::InputTag>("muonSrc", edm::InputTag("patMuonsWithTrigger"));
  desc.add<edm::InputTag>("electronSrc", edm::InputTag("patElectrons"));
  desc.add<edm::InputTag>("genParticles", edm::InputTag("prunedGenParticles"));

  desc.add<double>("minJpsiPt", 0.0);
  desc.add<double>("maxJpsiAbsY", 2.4);
  desc.add<double>("jpsiMassMin", 2.5);
  desc.add<double>("jpsiMassMax", 3.5);

  desc.add<double>("minMuonPt", 1.0);
  desc.add<double>("maxMuonAbsEta", 2.4);
  desc.add<bool>("requireGlobalMuon", false);
  desc.add<bool>("requirePFMuon", false);

  desc.add<double>("minElectronPt", 1.0);
  desc.add<double>("maxElectronAbsEta", 2.5);

  desc.add<double>("minBcVisiblePt", 0.0);
  desc.add<double>("maxBcVisibleAbsY", 2.4);
  desc.add<double>("minBcVisibleMass", 2.0);
  desc.add<double>("maxBcVisibleMass", 10.0);

  desc.add<double>("maxOverlapDR", 0.01);
  desc.add<bool>("doJpsiMuMuMu", true);
  desc.add<bool>("doJpsiMuMuE", true);
  desc.add<bool>("doJpsiEEMu", true);
  desc.add<bool>("doJpsiEEE", true);
  desc.add<bool>("doGenMatching", false);
  desc.add<double>("genMatchMaxDR", 0.03);

  descriptions.add("generalBcSemiLeptonicCandidates", desc);
}

DEFINE_FWK_MODULE(BcSemiLeptonicProducer);
