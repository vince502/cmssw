#include "HeavyIonsAnalysis/TrackAnalysis/interface/GenTrackNtuplizer.h"

// HepMC headers - try to include what's available
#include "HepMC/GenEvent.h"
#include "HepMC/GenParticle.h"
#include "HepMC/GenVertex.h"

GenTrackNtuplizer::GenTrackNtuplizer(const edm::ParameterSet& iConfig)
    : genParticleSrc_(iConfig.exists("genParticleSrc") ? 
                      consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleSrc")) :
                      edm::EDGetTokenT<reco::GenParticleCollection>()),
      hepmcSrc_(iConfig.exists("hepmcSrc") ?
                consumes<edm::HepMCProduct>(iConfig.getParameter<edm::InputTag>("hepmcSrc")) :
                edm::EDGetTokenT<edm::HepMCProduct>()),
      useHepMC_(iConfig.exists("hepmcSrc")),
      onlyStable_(iConfig.getParameter<bool>("onlyStable")),
      onlyCharged_(iConfig.getParameter<bool>("onlyCharged")) {
}

GenTrackNtuplizer::~GenTrackNtuplizer() {}

void GenTrackNtuplizer::beginJob() {
  fs->file().cd();
  trackTree_ = fs->make<TTree>("trackTree", "Gen track tree");

  trackTree_->Branch("nRun", &nRun);
  trackTree_->Branch("nEv", &nEv);
  trackTree_->Branch("nLumi", &nLumi);
  trackTree_->Branch("nTrk", &nTrk);

  trackTree_->Branch("trkPt", &trkPt);
  trackTree_->Branch("trkEta", &trkEta);
  trackTree_->Branch("trkPhi", &trkPhi);
  trackTree_->Branch("trkP", &trkP);
  trackTree_->Branch("trkEnergy", &trkEnergy);
  trackTree_->Branch("trkCharge", &trkCharge);
  trackTree_->Branch("trkChi2", &trkChi2);
  trackTree_->Branch("trkNdof", &trkNdof);
  trackTree_->Branch("trkNormChi2", &trkNormChi2);
  trackTree_->Branch("trkNHits", &trkNHits);
  trackTree_->Branch("trkNPixelHits", &trkNPixelHits);
  trackTree_->Branch("trkNStripHits", &trkNStripHits);
  trackTree_->Branch("trkHighPurity", &trkHighPurity);
  trackTree_->Branch("trkDxy", &trkDxy);
  trackTree_->Branch("trkDz", &trkDz);
  trackTree_->Branch("trkPtError", &trkPtError);
  trackTree_->Branch("trkEtaError", &trkEtaError);
  trackTree_->Branch("trkPhiError", &trkPhiError);
  
  // Gen-specific branches
  trackTree_->Branch("trkPdgId", &trkPdgId);
  trackTree_->Branch("trkStatus", &trkStatus);
  trackTree_->Branch("trkVx", &trkVx);
  trackTree_->Branch("trkVy", &trkVy);
  trackTree_->Branch("trkVz", &trkVz);
}

void GenTrackNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  nEv = (int)iEvent.id().event();
  nRun = (int)iEvent.id().run();
  nLumi = (int)iEvent.luminosityBlock();

  clearVectors();

  nTrk = 0;

  if (useHepMC_) {
    // Read from HepMCProduct
    edm::Handle<edm::HepMCProduct> hepmcHandle;
    if (!iEvent.getByToken(hepmcSrc_, hepmcHandle)) {
      // HepMC product not found, skip this event
      trackTree_->Fill();
      return;
    }
    
    const HepMC::GenEvent* genEvent = hepmcHandle->GetEvent();
    if (!genEvent) {
      trackTree_->Fill();
      return;
    }
    
    // Loop over all particles in the event
    for (HepMC::GenEvent::particle_const_iterator it = genEvent->particles_begin();
         it != genEvent->particles_end(); ++it) {
      const HepMC::GenParticle* hepmcParticle = *it;
      
      // Apply filters
      if (onlyStable_ && hepmcParticle->status() != 1) continue;
      
      // Get charge from PDG ID
      int pdgId = hepmcParticle->pdg_id();
      int charge = 0;
      
      // Calculate charge from PDG ID
      // Quarks: u(+2/3), d(-1/3), s(-1/3), c(+2/3), b(-1/3), t(+2/3)
      // Leptons: e(-1), mu(-1), tau(-1), nu(0)
      // Hadrons: charge depends on quark content
      int absPdgId = abs(pdgId);
      
      if (absPdgId == 11 || absPdgId == 13 || absPdgId == 15) {
        // Charged leptons
        charge = (pdgId > 0) ? -1 : 1;
      } else if (absPdgId == 12 || absPdgId == 14 || absPdgId == 16) {
        // Neutrinos
        charge = 0;
      } else if (absPdgId == 22) {
        // Photon
        charge = 0;
      } else if (absPdgId == 211 || absPdgId == 321 || absPdgId == 2212) {
        // Charged hadrons: pi+, K+, p
        charge = (pdgId > 0) ? 1 : -1;
      } else if (absPdgId == 111 || absPdgId == 130 || absPdgId == 310 || absPdgId == 2112) {
        // Neutral hadrons: pi0, K0, n
        charge = 0;
      } else {
        // For other particles, try to estimate from PDG ID structure
        // This is a simplified approach - for more accuracy, use PDG charge lookup
        if (absPdgId > 1000000000) {
          // Nucleus - charge is Z
          charge = (pdgId % 10000) / 10;
        } else {
          // Try to get from HepMC if available, otherwise estimate
          // Most particles with |pdgId| < 100 are charged
          charge = (absPdgId < 100 && absPdgId != 22) ? ((pdgId > 0) ? 1 : -1) : 0;
        }
      }
      
      if (onlyCharged_ && charge == 0) continue;
      
      // Get 4-momentum components directly
      double px = hepmcParticle->momentum().px();
      double py = hepmcParticle->momentum().py();
      double pz = hepmcParticle->momentum().pz();
      double energy = hepmcParticle->momentum().e();
      
      double pt = sqrt(px * px + py * py);
      double p = sqrt(px * px + py * py + pz * pz);
      double eta = (p > 0 && pt > 0) ? 0.5 * log((p + pz) / (p - pz)) : 0.0;
      double phi = atan2(py, px);
      
      // Skip particles with zero momentum
      if (p < 1e-6) continue;
      
      nTrk++;
      
      trkPt.push_back(pt);
      trkEta.push_back(eta);
      trkPhi.push_back(phi);
      trkP.push_back(p);
      trkEnergy.push_back(energy);
      trkCharge.push_back(charge);
      
      // For gen particles, these are not applicable but set to defaults for compatibility
      trkChi2.push_back(0.0);
      trkNdof.push_back(0.0);
      trkNormChi2.push_back(0.0);
      trkNHits.push_back(0);
      trkNPixelHits.push_back(0);
      trkNStripHits.push_back(0);
      trkHighPurity.push_back(true);
      
      // Impact parameters
      trkDxy.push_back(0.0);
      trkDz.push_back(0.0);
      
      // Errors
      trkPtError.push_back(0.0);
      trkEtaError.push_back(0.0);
      trkPhiError.push_back(0.0);
      
      // Gen-specific information
      trkPdgId.push_back(pdgId);
      trkStatus.push_back(hepmcParticle->status());
      
      // Production vertex position
      HepMC::GenVertex* prodVertex = hepmcParticle->production_vertex();
      if (prodVertex) {
        trkVx.push_back(prodVertex->position().x());
        trkVy.push_back(prodVertex->position().y());
        trkVz.push_back(prodVertex->position().z());
      } else {
        trkVx.push_back(0.0);
        trkVy.push_back(0.0);
        trkVz.push_back(0.0);
      }
    }
  } else {
    // Read from GenParticleCollection
    const auto& genParticles = iEvent.get(genParticleSrc_);

    for (const auto& genParticle : genParticles) {
      // Apply filters
      if (onlyStable_ && genParticle.status() != 1) continue;
      if (onlyCharged_ && genParticle.charge() == 0) continue;
      
      // Skip particles with zero momentum
      if (genParticle.p4().P() < 1e-6) continue;
      
      nTrk++;
      
      trkPt.push_back(genParticle.pt());
      trkEta.push_back(genParticle.eta());
      trkPhi.push_back(genParticle.phi());
      trkP.push_back(genParticle.p());
      trkEnergy.push_back(genParticle.energy());
      trkCharge.push_back(genParticle.charge());
      
      // For gen particles, these are not applicable but set to defaults for compatibility
      trkChi2.push_back(0.0);
      trkNdof.push_back(0.0);
      trkNormChi2.push_back(0.0);
      trkNHits.push_back(0);
      trkNPixelHits.push_back(0);
      trkNStripHits.push_back(0);
      trkHighPurity.push_back(true);  // Gen particles are always "pure"
      
      // Impact parameters (from production vertex)
      trkDxy.push_back(0.0);  // Could calculate from vertex if needed
      trkDz.push_back(0.0);   // Could calculate from vertex if needed
      
      // Errors (not applicable for gen, set to 0)
      trkPtError.push_back(0.0);
      trkEtaError.push_back(0.0);
      trkPhiError.push_back(0.0);
      
      // Gen-specific information
      trkPdgId.push_back(genParticle.pdgId());
      trkStatus.push_back(genParticle.status());
      
      // Production vertex position
      trkVx.push_back(genParticle.vx());
      trkVy.push_back(genParticle.vy());
      trkVz.push_back(genParticle.vz());
    }
  }

  trackTree_->Fill();
}

void GenTrackNtuplizer::endJob() {}

void GenTrackNtuplizer::clearVectors() {
  nTrk = 0;
  trkPt.clear();
  trkEta.clear();
  trkPhi.clear();
  trkP.clear();
  trkEnergy.clear();
  trkCharge.clear();
  trkChi2.clear();
  trkNdof.clear();
  trkNormChi2.clear();
  trkNHits.clear();
  trkNPixelHits.clear();
  trkNStripHits.clear();
  trkHighPurity.clear();
  trkDxy.clear();
  trkDz.clear();
  trkPtError.clear();
  trkEtaError.clear();
  trkPhiError.clear();
  trkPdgId.clear();
  trkStatus.clear();
  trkVx.clear();
  trkVy.clear();
  trkVz.clear();
}

DEFINE_FWK_MODULE(GenTrackNtuplizer);
