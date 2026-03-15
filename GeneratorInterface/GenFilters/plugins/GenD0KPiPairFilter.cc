#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDFilter.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

class GenD0KPiPairFilter : public edm::global::EDFilter<> {
public:
  explicit GenD0KPiPairFilter(const edm::ParameterSet&);
  bool filter(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

private:
  bool passParentKinematics(const HepMC::GenParticle* particle, int status, double minPt, double minEta, double maxEta) const;
  bool passDaughterKinematics(const HepMC::GenParticle* particle, double minPt, double minEta, double maxEta) const;
  bool isAcceptedD0ToKPi(const HepMC::GenParticle* particle, int parentID) const;

  const edm::EDGetTokenT<edm::HepMCProduct> token_;

  const int particleID1_;
  const int particleID2_;
  const int status1_;
  const int status2_;
  const double minPt1_;
  const double minPt2_;
  const double minEta1_;
  const double minEta2_;
  const double maxEta1_;
  const double maxEta2_;

  const int daughterStatus_;
  const double kaonMinPt_;
  const double kaonMinEta_;
  const double kaonMaxEta_;
  const double pionMinPt_;
  const double pionMinEta_;
  const double pionMaxEta_;
  const bool allowAdditionalPhotons_;

  const int minAccepted1_;
  const int maxAccepted1_;
  const int minAccepted2_;
  const int maxAccepted2_;

  const int minPairCount_;
  const int maxPairCount_;
};

GenD0KPiPairFilter::GenD0KPiPairFilter(const edm::ParameterSet& iConfig)
    : token_(consumes<edm::HepMCProduct>(
          iConfig.getUntrackedParameter<edm::InputTag>("src", edm::InputTag("generator", "unsmeared")))),
      particleID1_(iConfig.getParameter<int>("ParticleID1")),
      particleID2_(iConfig.getParameter<int>("ParticleID2")),
      status1_(iConfig.getParameter<int>("Status1")),
      status2_(iConfig.getParameter<int>("Status2")),
      minPt1_(iConfig.getParameter<double>("MinPt1")),
      minPt2_(iConfig.getParameter<double>("MinPt2")),
      minEta1_(iConfig.getParameter<double>("MinEta1")),
      minEta2_(iConfig.getParameter<double>("MinEta2")),
      maxEta1_(iConfig.getParameter<double>("MaxEta1")),
      maxEta2_(iConfig.getParameter<double>("MaxEta2")),
      daughterStatus_(iConfig.getParameter<int>("DaughterStatus")),
      kaonMinPt_(iConfig.getParameter<double>("KaonMinPt")),
      kaonMinEta_(iConfig.getParameter<double>("KaonMinEta")),
      kaonMaxEta_(iConfig.getParameter<double>("KaonMaxEta")),
      pionMinPt_(iConfig.getParameter<double>("PionMinPt")),
      pionMinEta_(iConfig.getParameter<double>("PionMinEta")),
      pionMaxEta_(iConfig.getParameter<double>("PionMaxEta")),
      allowAdditionalPhotons_(iConfig.getParameter<bool>("AllowAdditionalPhotons")),
      minAccepted1_(iConfig.getParameter<int>("MinAccepted1")),
      maxAccepted1_(iConfig.getParameter<int>("MaxAccepted1")),
      minAccepted2_(iConfig.getParameter<int>("MinAccepted2")),
      maxAccepted2_(iConfig.getParameter<int>("MaxAccepted2")),
      minPairCount_(iConfig.getParameter<int>("MinPairCount")),
      maxPairCount_(iConfig.getParameter<int>("MaxPairCount")) {
  if (std::abs(particleID1_) != 421 || std::abs(particleID2_) != 421) {
    throw cms::Exception("BadConfig") << "GenD0KPiPairFilter only supports signed D0 IDs (+/-421)";
  }
  if (minPairCount_ < 1) {
    throw cms::Exception("BadConfig") << "MinPairCount must be >= 1";
  }
  if (maxPairCount_ != -1 && maxPairCount_ < minPairCount_) {
    throw cms::Exception("BadConfig") << "MaxPairCount must be -1 or >= MinPairCount";
  }
  if (maxAccepted1_ != -1 && maxAccepted1_ < minAccepted1_) {
    throw cms::Exception("BadConfig") << "MaxAccepted1 must be -1 or >= MinAccepted1";
  }
  if (maxAccepted2_ != -1 && maxAccepted2_ < minAccepted2_) {
    throw cms::Exception("BadConfig") << "MaxAccepted2 must be -1 or >= MinAccepted2";
  }
}

bool GenD0KPiPairFilter::passParentKinematics(const HepMC::GenParticle* particle,
                                              int status,
                                              double minPt,
                                              double minEta,
                                              double maxEta) const {
  if (status != 0 && particle->status() != status) {
    return false;
  }

  const double pt = particle->momentum().perp();
  const double eta = particle->momentum().eta();
  return pt >= minPt && eta >= minEta && eta <= maxEta;
}

bool GenD0KPiPairFilter::passDaughterKinematics(const HepMC::GenParticle* particle,
                                                double minPt,
                                                double minEta,
                                                double maxEta) const {
  if (daughterStatus_ != 0 && particle->status() != daughterStatus_) {
    return false;
  }

  const double pt = particle->momentum().perp();
  const double eta = particle->momentum().eta();
  return pt >= minPt && eta >= minEta && eta <= maxEta;
}

bool GenD0KPiPairFilter::isAcceptedD0ToKPi(const HepMC::GenParticle* particle, int parentID) const {
  if (particle->pdg_id() != parentID || particle->end_vertex() == nullptr) {
    return false;
  }

  const int kaonID = parentID > 0 ? -321 : 321;
  const int pionID = parentID > 0 ? 211 : -211;

  bool hasKaon = false;
  bool hasPion = false;

  for (HepMC::GenVertex::particles_out_const_iterator child = particle->end_vertex()->particles_out_const_begin();
       child != particle->end_vertex()->particles_out_const_end();
       ++child) {
    const HepMC::GenParticle* daughter = *child;
    const int daughterID = daughter->pdg_id();

    if (!hasKaon && daughterID == kaonID &&
        passDaughterKinematics(daughter, kaonMinPt_, kaonMinEta_, kaonMaxEta_)) {
      hasKaon = true;
      continue;
    }

    if (!hasPion && daughterID == pionID &&
        passDaughterKinematics(daughter, pionMinPt_, pionMinEta_, pionMaxEta_)) {
      hasPion = true;
      continue;
    }

    if (allowAdditionalPhotons_ && daughterID == 22) {
      continue;
    }

    return false;
  }

  return hasKaon && hasPion;
}

bool GenD0KPiPairFilter::filter(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const {
  edm::Handle<edm::HepMCProduct> evt;
  iEvent.getByToken(token_, evt);

  const HepMC::GenEvent* genEvent = evt->GetEvent();
  std::vector<const HepMC::GenParticle*> firstLeg;
  std::vector<const HepMC::GenParticle*> secondLeg;

  for (HepMC::GenEvent::particle_const_iterator particle = genEvent->particles_begin();
       particle != genEvent->particles_end();
       ++particle) {
    const HepMC::GenParticle* genParticle = *particle;

    if (genParticle->pdg_id() == particleID1_ &&
        passParentKinematics(genParticle, status1_, minPt1_, minEta1_, maxEta1_) &&
        isAcceptedD0ToKPi(genParticle, particleID1_)) {
      firstLeg.push_back(genParticle);
    }

    if (genParticle->pdg_id() == particleID2_ &&
        passParentKinematics(genParticle, status2_, minPt2_, minEta2_, maxEta2_) &&
        isAcceptedD0ToKPi(genParticle, particleID2_)) {
      secondLeg.push_back(genParticle);
    }
  }

  const int accepted1 = static_cast<int>(firstLeg.size());
  if (accepted1 < minAccepted1_) {
    return false;
  }
  if (maxAccepted1_ != -1 && accepted1 > maxAccepted1_) {
    return false;
  }

  const int accepted2 = static_cast<int>(secondLeg.size());
  if (particleID1_ != particleID2_) {
    if (accepted2 < minAccepted2_) {
      return false;
    }
    if (maxAccepted2_ != -1 && accepted2 > maxAccepted2_) {
      return false;
    }
  }

  int pairCount = 0;
  if (particleID1_ == particleID2_) {
    pairCount = accepted1 / 2;
  } else {
    pairCount = std::min(accepted1, accepted2);
  }

  if (maxPairCount_ != -1 && pairCount > maxPairCount_) {
    return false;
  }

  return pairCount >= minPairCount_;
}

DEFINE_FWK_MODULE(GenD0KPiPairFilter);
