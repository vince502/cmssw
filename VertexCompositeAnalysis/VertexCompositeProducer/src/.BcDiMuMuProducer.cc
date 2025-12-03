#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BcDiMuMuProducer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace edm;

BcDiMuMuProducer::BcDiMuMuProducer(const edm::ParameterSet& iConfig) :
    // Configuration
    saveDetailedInfo_(iConfig.getParameter<bool>("saveDetailedInfo")),
    saveTrimuonInfo_(iConfig.getParameter<bool>("saveTrimuonInfo")),
    bcLabel_(iConfig.getParameter<std::string>("bcLabel")),
    trimuonLabel_(iConfig.getParameter<std::string>("trimuonLabel")),
    
    // Initialize counters
    nEvents_(0),
    nBcCandidates_(0),
    nTrimuonCandidates_(0)
{
    // Initialize the algorithm
    initializeAlgorithm(iConfig);
    
    // Declare what we produce
    produces<pat::CompositeCandidateCollection>(bcLabel_);
    
    if (saveTrimuonInfo_) {
        produces<pat::CompositeCandidateCollection>(trimuonLabel_);
    }
    
    // Print configuration
    printConfiguration();
}

BcDiMuMuProducer::~BcDiMuMuProducer() {
}

void BcDiMuMuProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    nEvents_++;
    
    // Run the Bc reconstruction algorithm
    bcFitter_->fitBc(iEvent, iSetup);
    
    // Get results
    const pat::CompositeCandidateCollection& bcCandidates = bcFitter_->getBcCandidates();
    
    // Update statistics
    nBcCandidates_ += bcCandidates.size();
    
    // Create output collections
    std::unique_ptr<pat::CompositeCandidateCollection> bcCollection = 
        std::make_unique<pat::CompositeCandidateCollection>();
    
    std::unique_ptr<pat::CompositeCandidateCollection> trimuonCollection = 
        std::make_unique<pat::CompositeCandidateCollection>();
    
    // Fill Bc collection
    for (const auto& bc : bcCandidates) {
        bcCollection->push_back(bc);
        
        // If requested, save trimuon information as well
        if (saveTrimuonInfo_) {
            // Create a trimuon candidate from the Bc candidate
            pat::CompositeCandidate trimuon;
            
            // Copy basic kinematics (this will be the visible 3-muon system)
            double trimuonMass = bc.hasUserFloat("trimuonMass") ? 
                                bc.userFloat("trimuonMass") : bc.mass();
            
            math::PtEtaPhiMLorentzVector trimuonP4(bc.pt(), bc.eta(), bc.phi(), trimuonMass);
            trimuon.setP4(trimuonP4);
            trimuon.setCharge(bc.charge());
            trimuon.setVertex(bc.vertex());
            
            // Copy relevant user data
            if (bc.hasUserFloat("vtxChi2")) trimuon.addUserFloat("vtxChi2", bc.userFloat("vtxChi2"));
            if (bc.hasUserFloat("vtxNdof")) trimuon.addUserFloat("vtxNdof", bc.userFloat("vtxNdof"));
            if (bc.hasUserFloat("vtxProb")) trimuon.addUserFloat("vtxProb", bc.userFloat("vtxProb"));
            if (bc.hasUserFloat("jpsiMass")) trimuon.addUserFloat("jpsiMass", bc.userFloat("jpsiMass"));
            if (bc.hasUserFloat("trimuonMass")) trimuon.addUserFloat("trimuonMass", bc.userFloat("trimuonMass"));
            
            trimuonCollection->push_back(trimuon);
        }
    }
    
    // Put collections into event
    iEvent.put(std::move(bcCollection), bcLabel_);
    
    if (saveTrimuonInfo_) {
        iEvent.put(std::move(trimuonCollection), trimuonLabel_);
    }
    
    // Log progress periodically
    if (nEvents_ % 1000 == 0) {
        LogInfo("BcDiMuMuProducer") << "Processed " << nEvents_ << " events, "
                                   << "found " << nBcCandidates_ << " Bc candidates";
    }
}

void BcDiMuMuProducer::initializeAlgorithm(const edm::ParameterSet& iConfig) {
    
    // Create the functional algorithm with consume tokens
    bcFitter_ = std::make_unique<BcDiMuMuFunctional>(iConfig, consumesCollector());
    
    LogInfo("BcDiMuMuProducer") << "BcDiMuMuFunctional algorithm initialized";
}

void BcDiMuMuProducer::endJob() {
    
    // Print final statistics
    LogInfo("BcDiMuMuProducer") << "\n"
        << "=== BcDiMuMuProducer Final Statistics ===\n"
        << "Total events processed: " << nEvents_ << "\n"
        << "Total Bc candidates: " << nBcCandidates_ << "\n"
        << "Average Bc per event: " << std::fixed << std::setprecision(3) 
        << (nEvents_ > 0 ? double(nBcCandidates_) / double(nEvents_) : 0.0) << "\n"
        << "========================================";
        
    if (nEvents_ > 0) {
        double bcRate = double(nBcCandidates_) / double(nEvents_);
        
        if (bcRate < 0.001) {
            LogWarning("BcDiMuMuProducer") << "Very low Bc candidate rate (" 
                                          << bcRate << " per event). "
                                          << "Check selection criteria and input collections.";
        } else if (bcRate > 10.0) {
            LogWarning("BcDiMuMuProducer") << "Very high Bc candidate rate (" 
                                          << bcRate << " per event). "
                                          << "Consider tightening selection criteria.";
        }
    }
}

void BcDiMuMuProducer::printConfiguration() const {
    
    LogInfo("BcDiMuMuProducer") << "\n"
        << "=== BcDiMuMuProducer Configuration ===\n"
        << "Bc collection label: " << bcLabel_ << "\n"
        << "Save detailed info: " << (saveDetailedInfo_ ? "Yes" : "No") << "\n"
        << "Save trimuon info: " << (saveTrimuonInfo_ ? "Yes" : "No") << "\n";
        
    if (saveTrimuonInfo_) {
        LogInfo("BcDiMuMuProducer") << "Trimuon collection label: " << trimuonLabel_ << "\n";
    }
    
    LogInfo("BcDiMuMuProducer") << "=====================================";
}

// Define this as a plug-in
DEFINE_FWK_MODULE(BcDiMuMuProducer);