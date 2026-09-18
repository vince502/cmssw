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
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

class MCParticlePairDecayFilter : public edm::global::EDFilter<> {
public:
  explicit MCParticlePairDecayFilter(const edm::ParameterSet&);
  bool filter(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

private:
  struct DecayNode {
    std::vector<int> particleIDs;
    int status;
    double minPt;
    double maxPt;
    double minY;
    double maxY;
    std::vector<DecayNode> daughters;
    bool allowAdditionalDaughters;
    std::vector<int> allowedAdditionalDaughterIDs;
  };

  static DecayNode makeDecayNode(const edm::ParameterSet&);
  static bool sameDecayNode(const DecayNode&, const DecayNode&);
  static bool matchesKinematics(const HepMC::GenParticle*, const DecayNode&);
  static bool isAllowedAdditionalDaughter(const HepMC::GenParticle*, const DecayNode&);
  bool matchesDecayNode(const HepMC::GenParticle*, const DecayNode&) const;
  bool matchRequiredDaughters(const std::vector<const HepMC::GenParticle*>&,
                              const std::vector<DecayNode>&,
                              std::vector<bool>&,
                              std::size_t) const;

  const edm::EDGetTokenT<edm::HepMCProduct> token_;
  const DecayNode leg1_;
  const DecayNode leg2_;
  const int minAccepted1_;
  const int maxAccepted1_;
  const int minAccepted2_;
  const int maxAccepted2_;
  const int minPairCount_;
  const int maxPairCount_;
  const bool equivalentLegs_;
};

MCParticlePairDecayFilter::DecayNode MCParticlePairDecayFilter::makeDecayNode(const edm::ParameterSet& config) {
  DecayNode node{config.getParameter<std::vector<int>>("ParticleIDs"),
                 config.getParameter<int>("Status"),
                 config.getParameter<double>("MinPt"),
                 config.getParameter<double>("MaxPt"),
                 config.getParameter<double>("MinY"),
                 config.getParameter<double>("MaxY"),
                 {},
                 config.getParameter<bool>("AllowAdditionalDaughters"),
                 config.getParameter<std::vector<int>>("AllowedAdditionalDaughterIDs")};

  for (const auto& daughterConfig : config.getParameter<std::vector<edm::ParameterSet>>("Daughters")) {
    node.daughters.push_back(makeDecayNode(daughterConfig));
  }
  return node;
}

MCParticlePairDecayFilter::MCParticlePairDecayFilter(const edm::ParameterSet& config)
    : token_(consumes<edm::HepMCProduct>(config.getParameter<edm::InputTag>("src"))),
      leg1_(makeDecayNode(config.getParameter<edm::ParameterSet>("Leg1"))),
      leg2_(makeDecayNode(config.getParameter<edm::ParameterSet>("Leg2"))),
      minAccepted1_(config.getParameter<int>("MinAccepted1")),
      maxAccepted1_(config.getParameter<int>("MaxAccepted1")),
      minAccepted2_(config.getParameter<int>("MinAccepted2")),
      maxAccepted2_(config.getParameter<int>("MaxAccepted2")),
      minPairCount_(config.getParameter<int>("MinPairCount")),
      maxPairCount_(config.getParameter<int>("MaxPairCount")),
      equivalentLegs_(sameDecayNode(leg1_, leg2_)) {
  if (minAccepted1_ < 0 || minAccepted2_ < 0) {
    throw cms::Exception("BadConfig") << "MinAccepted1 and MinAccepted2 must be non-negative";
  }
  if ((maxAccepted1_ != -1 && maxAccepted1_ < minAccepted1_) ||
      (maxAccepted2_ != -1 && maxAccepted2_ < minAccepted2_)) {
    throw cms::Exception("BadConfig") << "Each maximum accepted-candidate count must be -1 or at least its minimum";
  }
  if (minPairCount_ < 1) {
    throw cms::Exception("BadConfig") << "MinPairCount must be at least one";
  }
  if (maxPairCount_ != -1 && maxPairCount_ < minPairCount_) {
    throw cms::Exception("BadConfig") << "MaxPairCount must be -1 or at least MinPairCount";
  }
}

bool MCParticlePairDecayFilter::sameDecayNode(const DecayNode& first, const DecayNode& second) {
  if (first.particleIDs != second.particleIDs || first.status != second.status || first.minPt != second.minPt ||
      first.maxPt != second.maxPt || first.minY != second.minY || first.maxY != second.maxY ||
      first.allowAdditionalDaughters != second.allowAdditionalDaughters ||
      first.allowedAdditionalDaughterIDs != second.allowedAdditionalDaughterIDs ||
      first.daughters.size() != second.daughters.size()) {
    return false;
  }
  for (std::size_t index = 0; index < first.daughters.size(); ++index) {
    if (!sameDecayNode(first.daughters[index], second.daughters[index])) {
      return false;
    }
  }
  return true;
}

bool MCParticlePairDecayFilter::matchesKinematics(const HepMC::GenParticle* particle, const DecayNode& node) {
  if (!node.particleIDs.empty() &&
      std::find(node.particleIDs.begin(), node.particleIDs.end(), particle->pdg_id()) == node.particleIDs.end()) {
    return false;
  }
  if (node.status != 0 && particle->status() != node.status) {
    return false;
  }

  const HepMC::FourVector& momentum = particle->momentum();
  const double energyMinusPz = momentum.e() - momentum.pz();
  const double energyPlusPz = momentum.e() + momentum.pz();
  double rapidity = 0.;
  if (energyMinusPz <= 0.) {
    rapidity = std::numeric_limits<double>::infinity();
  } else if (energyPlusPz <= 0.) {
    rapidity = -std::numeric_limits<double>::infinity();
  } else {
    rapidity = 0.5 * std::log(energyPlusPz / energyMinusPz);
  }
  return momentum.perp() >= node.minPt && momentum.perp() <= node.maxPt && rapidity >= node.minY &&
         rapidity <= node.maxY;
}

bool MCParticlePairDecayFilter::isAllowedAdditionalDaughter(const HepMC::GenParticle* particle, const DecayNode& node) {
  return node.allowAdditionalDaughters || std::find(node.allowedAdditionalDaughterIDs.begin(),
                                                    node.allowedAdditionalDaughterIDs.end(),
                                                    particle->pdg_id()) != node.allowedAdditionalDaughterIDs.end();
}

bool MCParticlePairDecayFilter::matchRequiredDaughters(const std::vector<const HepMC::GenParticle*>& children,
                                                       const std::vector<DecayNode>& requirements,
                                                       std::vector<bool>& used,
                                                       std::size_t requirementIndex) const {
  if (requirementIndex == requirements.size()) {
    return true;
  }

  for (std::size_t childIndex = 0; childIndex < children.size(); ++childIndex) {
    if (used[childIndex] || !matchesDecayNode(children[childIndex], requirements[requirementIndex])) {
      continue;
    }
    used[childIndex] = true;
    if (matchRequiredDaughters(children, requirements, used, requirementIndex + 1)) {
      return true;
    }
    used[childIndex] = false;
  }
  return false;
}

bool MCParticlePairDecayFilter::matchesDecayNode(const HepMC::GenParticle* particle, const DecayNode& node) const {
  if (!matchesKinematics(particle, node)) {
    return false;
  }

  std::vector<const HepMC::GenParticle*> children;
  if (particle->end_vertex() != nullptr) {
    for (HepMC::GenVertex::particles_out_const_iterator child = particle->end_vertex()->particles_out_const_begin();
         child != particle->end_vertex()->particles_out_const_end();
         ++child) {
      children.push_back(*child);
    }
  }

  if (children.size() < node.daughters.size()) {
    return false;
  }

  std::vector<bool> used(children.size(), false);
  if (!matchRequiredDaughters(children, node.daughters, used, 0)) {
    return false;
  }

  for (std::size_t childIndex = 0; childIndex < children.size(); ++childIndex) {
    if (!used[childIndex] && !isAllowedAdditionalDaughter(children[childIndex], node)) {
      return false;
    }
  }
  return true;
}

bool MCParticlePairDecayFilter::filter(edm::StreamID, edm::Event& event, const edm::EventSetup&) const {
  edm::Handle<edm::HepMCProduct> eventHandle;
  event.getByToken(token_, eventHandle);

  std::vector<const HepMC::GenParticle*> accepted1;
  std::vector<const HepMC::GenParticle*> accepted2;
  const HepMC::GenEvent* genEvent = eventHandle->GetEvent();
  for (HepMC::GenEvent::particle_const_iterator particle = genEvent->particles_begin();
       particle != genEvent->particles_end();
       ++particle) {
    if (matchesDecayNode(*particle, leg1_)) {
      accepted1.push_back(*particle);
    }
    if (matchesDecayNode(*particle, leg2_)) {
      accepted2.push_back(*particle);
    }
  }

  const auto countIsAccepted = [](int count, int minimum, int maximum) {
    return count >= minimum && (maximum == -1 || count <= maximum);
  };
  if (!countIsAccepted(static_cast<int>(accepted1.size()), minAccepted1_, maxAccepted1_) ||
      !countIsAccepted(static_cast<int>(accepted2.size()), minAccepted2_, maxAccepted2_)) {
    return false;
  }

  int pairCount = 0;
  if (equivalentLegs_) {
    for (std::size_t firstIndex = 0; firstIndex < accepted1.size(); ++firstIndex) {
      for (std::size_t secondIndex = firstIndex + 1; secondIndex < accepted1.size(); ++secondIndex) {
        ++pairCount;
      }
    }
  } else {
    for (const HepMC::GenParticle* first : accepted1) {
      for (const HepMC::GenParticle* second : accepted2) {
        if (first != second) {
          ++pairCount;
        }
      }
    }
  }

  return pairCount >= minPairCount_ && (maxPairCount_ == -1 || pairCount <= maxPairCount_);
}

DEFINE_FWK_MODULE(MCParticlePairDecayFilter);
