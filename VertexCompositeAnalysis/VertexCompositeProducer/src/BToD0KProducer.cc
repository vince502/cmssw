// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BToD0KProducer
//

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BToD0KProducer.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"

#include <TMath.h>
#include <TVector3.h>

namespace {
  constexpr double kKaonMass = 0.493677;
  constexpr double kBPlusMass = 5.27934;
  constexpr double kInvalidFloat = -999.f;

  double rapidity(const reco::Candidate::LorentzVector& p4) {
    const double numerator = p4.E() + p4.pz();
    const double denominator = p4.E() - p4.pz();
    if (numerator <= 0. || denominator <= 0.) {
      return kInvalidFloat;
    }
    return 0.5 * std::log(numerator / denominator);
  }

  double clampCos(double value) {
    return std::max(-1., std::min(1., value));
  }

  void copyUserFloat(const pat::CompositeCandidate& src,
                     pat::CompositeCandidate& dst,
                     const std::string& inputName,
                     const std::string& outputName) {
    if (src.hasUserFloat(inputName)) {
      dst.addUserFloat(outputName, src.userFloat(inputName));
    } else {
      dst.addUserFloat(outputName, kInvalidFloat);
    }
  }
}

BToD0KProducer::BToD0KProducer(const edm::ParameterSet& iConfig)
    : d0Token_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("d0Src"))),
      kaonToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("kaonSrc"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("pvSrc"))),
      bsToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotSrc"))),
      ttbToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      ttBuilder_(nullptr),
      minKaonPt_(iConfig.getParameter<double>("minKaonPt")),
      maxKaonEta_(iConfig.getParameter<double>("maxKaonEta")),
      minKaonNhits_(iConfig.getParameter<int>("minKaonNhits")),
      requireHighPurity_(iConfig.getParameter<bool>("requireHighPurity")),
      minD0Pt_(iConfig.getParameter<double>("minD0Pt")),
      maxD0AbsY_(iConfig.getParameter<double>("maxD0AbsY")),
      d0MassMin_(iConfig.getParameter<double>("d0MassMin")),
      d0MassMax_(iConfig.getParameter<double>("d0MassMax")),
      requireD0PdgId_(iConfig.getParameter<bool>("requireD0PdgId")),
      isWrongSign_(iConfig.getParameter<bool>("isWrongSign")),
      minBPt_(iConfig.getParameter<double>("minBPt")),
      maxBAbsY_(iConfig.getParameter<double>("maxBAbsY")),
      bMassMin_(iConfig.getParameter<double>("bMassMin")),
      bMassMax_(iConfig.getParameter<double>("bMassMax")),
      minVtxProb_(iConfig.getParameter<double>("minVtxProb")),
      doVertexFit_(iConfig.getParameter<bool>("doVertexFit")),
      maxDeltaR_overlap_(iConfig.getParameter<double>("maxDeltaR_overlap")),
      useDeDx_(false) {
  const edm::InputTag dedxTag = iConfig.getParameter<edm::InputTag>("dedxSrc");
  useDeDx_ = !dedxTag.label().empty();
  if (useDeDx_) {
    dedxToken_ = consumes<edm::ValueMap<reco::DeDxData>>(dedxTag);
  }
  produces<pat::CompositeCandidateCollection>("BToD0K");
}

void BToD0KProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("d0Src", edm::InputTag("generalD0Candidates", "D0"));
  desc.add<edm::InputTag>("kaonSrc", edm::InputTag("packedPFCandidates"));
  desc.add<edm::InputTag>("pvSrc", edm::InputTag("offlineSlimmedPrimaryVertices"));
  desc.add<edm::InputTag>("beamSpotSrc", edm::InputTag("offlineBeamSpot"));
  desc.add<edm::InputTag>("dedxSrc", edm::InputTag(""));

  desc.add<double>("minKaonPt", 0.8);
  desc.add<double>("maxKaonEta", 2.4);
  desc.add<int>("minKaonNhits", 3);
  desc.add<bool>("requireHighPurity", true);

  desc.add<double>("minD0Pt", 0.0);
  desc.add<double>("maxD0AbsY", 2.4);
  desc.add<double>("d0MassMin", 1.72);
  desc.add<double>("d0MassMax", 2.01);
  desc.add<bool>("requireD0PdgId", true);
  desc.add<bool>("isWrongSign", false);

  desc.add<double>("minBPt", 5.0);
  desc.add<double>("maxBAbsY", 2.4);
  desc.add<double>("bMassMin", kBPlusMass - 0.7);
  desc.add<double>("bMassMax", kBPlusMass + 0.7);
  desc.add<double>("minVtxProb", 0.001);

  desc.add<bool>("doVertexFit", true);
  desc.add<double>("maxDeltaR_overlap", 0.01);

  descriptions.add("bToD0KProducer", desc);
}

void BToD0KProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  auto output = std::make_unique<pat::CompositeCandidateCollection>();

  ttBuilder_ = &iSetup.getData(ttbToken_);

  edm::Handle<pat::CompositeCandidateCollection> d0Handle;
  iEvent.getByToken(d0Token_, d0Handle);
  if (!d0Handle.isValid() || d0Handle->empty()) {
    iEvent.put(std::move(output), "BToD0K");
    return;
  }

  edm::Handle<pat::PackedCandidateCollection> kaonHandle;
  iEvent.getByToken(kaonToken_, kaonHandle);
  if (!kaonHandle.isValid() || kaonHandle->empty()) {
    iEvent.put(std::move(output), "BToD0K");
    return;
  }

  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByToken(pvToken_, pvHandle);

  edm::Handle<reco::BeamSpot> bsHandle;
  iEvent.getByToken(bsToken_, bsHandle);

  edm::Handle<edm::ValueMap<reco::DeDxData>> dEdxHandle;
  if (useDeDx_) {
    iEvent.getByToken(dedxToken_, dEdxHandle);
  }

  reco::Vertex pv;
  if (pvHandle.isValid() && !pvHandle->empty() && !pvHandle->front().isFake()) {
    pv = pvHandle->front();
  } else if (bsHandle.isValid()) {
    pv = reco::Vertex(bsHandle->position(), bsHandle->covariance3D());
  } else {
    iEvent.put(std::move(output), "BToD0K");
    return;
  }

  for (size_t d0Idx = 0; d0Idx < d0Handle->size(); ++d0Idx) {
    const auto& d0 = (*d0Handle)[d0Idx];
    if (!passD0Cuts(d0)) {
      continue;
    }

    const auto kaons = selectKaons(kaonHandle, d0, dEdxHandle);
    for (const auto& kaon : kaons) {
      if (!passChargeSelection(d0, kaon)) {
        continue;
      }

      const auto bP4 = d0.p4() + kaon.p4;
      if (!passBCuts(bP4)) {
        continue;
      }

      auto candidate = buildCandidate(d0, static_cast<int>(d0Idx), kaon, pv);
      if (passBCuts(candidate.p4())) {
        output->push_back(candidate);
      }
    }
  }

  iEvent.put(std::move(output), "BToD0K");
}

bool BToD0KProducer::passD0Cuts(const pat::CompositeCandidate& d0) const {
  if (d0.pt() < minD0Pt_) {
    return false;
  }
  if (std::abs(d0.rapidity()) > maxD0AbsY_) {
    return false;
  }
  if (d0.mass() < d0MassMin_ || d0.mass() > d0MassMax_) {
    return false;
  }
  if (requireD0PdgId_ && std::abs(d0.pdgId()) != 421) {
    return false;
  }
  if (d0.numberOfDaughters() < 2) {
    return false;
  }
  return true;
}

bool BToD0KProducer::passKaonCuts(const pat::PackedCandidate& kaon) const {
  if (kaon.charge() == 0) {
    return false;
  }
  if (kaon.pt() < minKaonPt_) {
    return false;
  }
  if (std::abs(kaon.eta()) > maxKaonEta_) {
    return false;
  }
  if (!kaon.hasTrackDetails()) {
    return false;
  }

  const auto& track = kaon.pseudoTrack();
  if (track.numberOfValidHits() < minKaonNhits_) {
    return false;
  }
  if (requireHighPurity_ && !track.quality(reco::TrackBase::highPurity)) {
    return false;
  }
  return true;
}

bool BToD0KProducer::passBCuts(const reco::Candidate::LorentzVector& p4) const {
  if (p4.pt() < minBPt_) {
    return false;
  }
  const double y = rapidity(p4);
  if (y == kInvalidFloat || std::abs(y) > maxBAbsY_) {
    return false;
  }
  if (p4.mass() < bMassMin_ || p4.mass() > bMassMax_) {
    return false;
  }
  return true;
}

bool BToD0KProducer::passChargeSelection(const pat::CompositeCandidate& d0,
                                         const KaonInfo& kaon) const {
  if (std::abs(d0.pdgId()) != 421) {
    return !requireD0PdgId_;
  }

  const bool rightSign = (d0.pdgId() * kaon.charge) < 0;
  return isWrongSign_ ? !rightSign : rightSign;
}

std::vector<BToD0KProducer::KaonInfo>
BToD0KProducer::selectKaons(const edm::Handle<pat::PackedCandidateCollection>& kaons,
                            const pat::CompositeCandidate& d0,
                            const edm::Handle<edm::ValueMap<reco::DeDxData>>& dEdxHandle) const {
  std::vector<KaonInfo> result;
  result.reserve(kaons->size());

  for (size_t i = 0; i < kaons->size(); ++i) {
    const auto& kaon = (*kaons)[i];
    if (!passKaonCuts(kaon)) {
      continue;
    }

    KaonInfo info;
    info.charge = kaon.charge();
    info.index = i;
    info.track = kaon.pseudoTrack();
    info.dedx = kInvalidFloat;
    info.valid = true;
    if (useDeDx_ && dEdxHandle.isValid()) {
      const edm::Ptr<pat::PackedCandidate> kaonPtr(kaons, i);
      const auto& dEdxMap = *dEdxHandle.product();
      if (kaonPtr.isNonnull() && dEdxMap.contains(kaonPtr.id())) {
        info.dedx = dEdxMap[kaonPtr].dEdx();
      }
    }

    const double px = kaon.px();
    const double py = kaon.py();
    const double pz = kaon.pz();
    const double p2 = px * px + py * py + pz * pz;
    const double energy = std::sqrt(p2 + kKaonMass * kKaonMass);
    info.p4 = reco::Candidate::LorentzVector(px, py, pz, energy);

    try {
      info.transientTrack = ttBuilder_->build(info.track);
    } catch (...) {
      info.valid = false;
    }

    if (!info.valid || isOverlappingWithD0(d0, info)) {
      continue;
    }
    result.push_back(info);
  }

  return result;
}

bool BToD0KProducer::isOverlappingWithD0(const pat::CompositeCandidate& d0,
                                         const KaonInfo& kaon) const {
  for (size_t iDau = 0; iDau < d0.numberOfDaughters(); ++iDau) {
    const auto* dau = d0.daughter(iDau);
    if (!dau) {
      continue;
    }
    const double dR = reco::deltaR(dau->eta(), dau->phi(), kaon.p4.eta(), kaon.p4.phi());
    if (dR < maxDeltaR_overlap_) {
      return true;
    }
  }
  return false;
}

bool BToD0KProducer::getD0DaughterTracks(const pat::CompositeCandidate& d0,
                                         std::vector<reco::TransientTrack>& tracks) const {
  for (size_t iDau = 0; iDau < d0.numberOfDaughters(); ++iDau) {
    const auto* dau = d0.daughter(iDau);
    if (!dau) {
      continue;
    }

    const auto* track = dau->bestTrack();
    if (!track) {
      continue;
    }

    try {
      tracks.push_back(ttBuilder_->build(*track));
    } catch (...) {
      continue;
    }
  }

  return tracks.size() >= 2;
}

TransientVertex BToD0KProducer::fitVertex(const std::vector<reco::TransientTrack>& tracks) const {
  KalmanVertexFitter fitter;
  TransientVertex vertex;

  try {
    vertex = fitter.vertex(tracks);
  } catch (...) {
  }

  return vertex;
}

void BToD0KProducer::computeVertexQuantities(const TransientVertex& vertex,
                                             const reco::Candidate::LorentzVector& p4,
                                             const reco::Vertex& pv,
                                             float& lxy,
                                             float& lxySig,
                                             float& l3D,
                                             float& l3DSig,
                                             float& alpha2D,
                                             float& alpha3D,
                                             float& cosAlpha,
                                             float& dca3D,
                                             float& dca3DErr,
                                             float& track3DDCA,
                                             float& track3DDCAErr) const {
  lxy = lxySig = l3D = l3DSig = alpha2D = alpha3D = cosAlpha = kInvalidFloat;
  dca3D = dca3DErr = track3DDCA = track3DDCAErr = kInvalidFloat;
  if (!vertex.isValid()) {
    return;
  }

  const auto vtxPos = vertex.position();
  const double dx = vtxPos.x() - pv.x();
  const double dy = vtxPos.y() - pv.y();
  const double dz = vtxPos.z() - pv.z();

  const reco::Vertex sv(vertex);
  VertexDistanceXY vdistXY;
  VertexDistance3D vdist3D;
  const Measurement1D distXY = vdistXY.distance(sv, pv);
  const Measurement1D dist3D = vdist3D.distance(sv, pv);

  lxy = distXY.value();
  l3D = dist3D.value();
  lxySig = distXY.significance();
  l3DSig = dist3D.significance();

  TVector3 p3(p4.px(), p4.py(), p4.pz());
  TVector3 d3(dx, dy, dz);
  if (p3.Mag() > 0. && d3.Mag() > 0.) {
    cosAlpha = clampCos(p3.Dot(d3) / (p3.Mag() * d3.Mag()));
    alpha3D = std::acos(cosAlpha);
  }

  const double pT = std::sqrt(p4.px() * p4.px() + p4.py() * p4.py());
  if (pT > 0. && lxy > 0.) {
    const double cos2D = clampCos((p4.px() * dx + p4.py() * dy) / (pT * lxy));
    alpha2D = std::acos(cos2D);
  }

  if (l3D >= 0.f && dist3D.error() >= 0. && alpha3D != kInvalidFloat) {
    const double sinAlpha3D = std::sin(alpha3D);
    dca3D = dist3D.value() * sinAlpha3D;
    dca3DErr = dist3D.error() * std::abs(sinAlpha3D);
    track3DDCA = dca3D;
    track3DDCAErr = dca3DErr;
  }
}

pat::CompositeCandidate BToD0KProducer::buildCandidate(const pat::CompositeCandidate& d0,
                                                       int d0Idx,
                                                       const KaonInfo& kaon,
                                                       const reco::Vertex& pv) const {
  pat::CompositeCandidate candidate;
  const auto bP4 = d0.p4() + kaon.p4;

  float vtxChi2 = kInvalidFloat;
  float vtxNdof = kInvalidFloat;
  float vtxProb = kInvalidFloat;
  float vtxX = kInvalidFloat;
  float vtxY = kInvalidFloat;
  float vtxZ = kInvalidFloat;
  float lxy = kInvalidFloat;
  float lxySig = kInvalidFloat;
  float l3D = kInvalidFloat;
  float l3DSig = kInvalidFloat;
  float alpha2D = kInvalidFloat;
  float alpha3D = kInvalidFloat;
  float cosAlpha = kInvalidFloat;
  float dca3D = kInvalidFloat;
  float dca3DErr = kInvalidFloat;
  float track3DDCA = kInvalidFloat;
  float track3DDCAErr = kInvalidFloat;

  if (doVertexFit_) {
    std::vector<reco::TransientTrack> fitTracks;
    if (!getD0DaughterTracks(d0, fitTracks)) {
      candidate.setP4(reco::Candidate::LorentzVector(0., 0., 0., -1.));
      return candidate;
    }
    fitTracks.push_back(kaon.transientTrack);

    const auto vertex = fitVertex(fitTracks);
    if (!vertex.isValid()) {
      candidate.setP4(reco::Candidate::LorentzVector(0., 0., 0., -1.));
      return candidate;
    }

    vtxChi2 = vertex.totalChiSquared();
    vtxNdof = vertex.degreesOfFreedom();
    vtxProb = TMath::Prob(vtxChi2, static_cast<int>(vtxNdof));
    if (vtxProb < minVtxProb_) {
      candidate.setP4(reco::Candidate::LorentzVector(0., 0., 0., -1.));
      return candidate;
    }

    const auto vtxPos = vertex.position();
    vtxX = vtxPos.x();
    vtxY = vtxPos.y();
    vtxZ = vtxPos.z();
    computeVertexQuantities(vertex,
                            bP4,
                            pv,
                            lxy,
                            lxySig,
                            l3D,
                            l3DSig,
                            alpha2D,
                            alpha3D,
                            cosAlpha,
                            dca3D,
                            dca3DErr,
                            track3DDCA,
                            track3DDCAErr);
  } else {
    vtxX = d0.vx();
    vtxY = d0.vy();
    vtxZ = d0.vz();
  }

  candidate.setP4(bP4);
  candidate.setCharge(kaon.charge);
  candidate.setPdgId(kaon.charge > 0 ? 521 : -521);
  candidate.setVertex(reco::Candidate::Point(vtxX, vtxY, vtxZ));

  candidate.addDaughter(d0, "d0");

  reco::RecoChargedCandidate kaonCandidate(kaon.charge, kaon.p4, candidate.vertex());
  candidate.addDaughter(kaonCandidate, "kaon");

  candidate.addUserInt("decayId", 0);
  candidate.addUserInt("d0Idx", d0Idx);
  candidate.addUserInt("d0PdgId", d0.pdgId());
  candidate.addUserInt("kaonIdx", static_cast<int>(kaon.index));
  candidate.addUserInt("kaonCharge", kaon.charge);
  candidate.addUserInt("isWrongSign", isWrongSign_ ? 1 : 0);
  candidate.addUserInt("nTracks", 1);
  candidate.addUserInt("hasDaughter2", 0);

  candidate.addUserFloat("d0Mass", d0.mass());
  candidate.addUserFloat("d0Pt", d0.pt());
  candidate.addUserFloat("d0Eta", d0.eta());
  candidate.addUserFloat("d0Phi", d0.phi());
  candidate.addUserFloat("d0Y", d0.rapidity());
  copyUserFloat(d0, candidate, "mva", "d0Mva");
  copyUserFloat(d0, candidate, "posDauDeDx", "d0PosDauDeDx");
  copyUserFloat(d0, candidate, "negDauDeDx", "d0NegDauDeDx");
  copyUserFloat(d0, candidate, "mva", "D0mva");
  copyUserFloat(d0, candidate, "posDauDeDx", "D0posDauDeDx");
  copyUserFloat(d0, candidate, "negDauDeDx", "D0negDauDeDx");

  candidate.addUserFloat("kaonMass", kKaonMass);
  candidate.addUserFloat("kaonPt", kaon.p4.pt());
  candidate.addUserFloat("kaonEta", kaon.p4.eta());
  candidate.addUserFloat("kaonPhi", kaon.p4.phi());
  candidate.addUserFloat("kaonNhits", kaon.track.numberOfValidHits());
  candidate.addUserFloat("kaonPtErr", kaon.track.ptError());
  candidate.addUserFloat("kaonChi2", kaon.track.normalizedChi2());
  const math::XYZPoint pvPos(pv.x(), pv.y(), pv.z());
  const double kaonDz = kaon.track.dz(pvPos);
  const double kaonDxy = kaon.track.dxy(pvPos);
  const double kaonDzErr = std::sqrt(kaon.track.dzError() * kaon.track.dzError() + pv.zError() * pv.zError());
  const double kaonDxyErr =
      std::sqrt(kaon.track.d0Error() * kaon.track.d0Error() + pv.xError() * pv.yError());
  candidate.addUserFloat("kaonDz", kaonDz);
  candidate.addUserFloat("kaonDxy", kaonDxy);
  candidate.addUserFloat("kaonDzErr", kaonDzErr);
  candidate.addUserFloat("kaonDxyErr", kaonDxyErr);
  candidate.addUserFloat("kaonDzSig", kaonDzErr > 0. ? kaonDz / kaonDzErr : kInvalidFloat);
  candidate.addUserFloat("kaonDxySig", kaonDxyErr > 0. ? kaonDxy / kaonDxyErr : kInvalidFloat);
  candidate.addUserFloat("kaonDeDx", kaon.dedx);
  candidate.addUserInt("kaonHighPurity", kaon.track.quality(reco::TrackBase::highPurity) ? 1 : 0);
  candidate.addUserFloat("deltaM", candidate.mass() - d0.mass());

  candidate.addUserFloat("VtxChi2", vtxChi2);
  candidate.addUserFloat("VtxNdof", vtxNdof);
  candidate.addUserFloat("VtxProb", vtxProb);
  candidate.addUserFloat("vtxChi2", vtxChi2);
  candidate.addUserFloat("vtxNdof", vtxNdof);
  candidate.addUserFloat("vtxProb", vtxProb);
  candidate.addUserFloat("vtxX", vtxX);
  candidate.addUserFloat("vtxY", vtxY);
  candidate.addUserFloat("vtxZ", vtxZ);
  candidate.addUserFloat("alpha2D", alpha2D);
  candidate.addUserFloat("alpha3D", alpha3D);
  candidate.addUserFloat("decaylength2D", lxy);
  candidate.addUserFloat("decaylength3D", l3D);
  candidate.addUserFloat("decaylengthsignif2D", lxySig);
  candidate.addUserFloat("decaylengthsignif3D", l3DSig);
  candidate.addUserFloat("lxy", lxy);
  candidate.addUserFloat("lxySig", lxySig);
  candidate.addUserFloat("l3D", l3D);
  candidate.addUserFloat("l3DSig", l3DSig);
  candidate.addUserFloat("cosAlpha", cosAlpha);
  candidate.addUserFloat("dca3D", dca3D);
  candidate.addUserFloat("dca3DErr", dca3DErr);
  candidate.addUserFloat("track3DDCA", track3DDCA);
  candidate.addUserFloat("track3DDCAErr", track3DDCAErr);

  return candidate;
}

DEFINE_FWK_MODULE(BToD0KProducer);
