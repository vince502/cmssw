#ifndef VertexCompositeProducer_BcDiMuMuProducer_h
#define VertexCompositeProducer_BcDiMuMuProducer_h

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BcDiMuMuFunctional.h"

/**
 * @class BcDiMuMuProducer
 * @brief EDProducer for Bc meson reconstruction from J/ψ + muon + neutrino
 * 
 * This producer implements a functional approach to Bc meson reconstruction
 * for the semi-leptonic decay channel Bc → J/ψ + μ + ν. It uses the 
 * BcDiMuMuFunctional class to perform the actual reconstruction with
 * sophisticated neutrino momentum inference and prong rotation techniques.
 * 
 * Key features:
 * - Modular functional design for easy workflow understanding
 * - Neutrino kinematic inference using pointing angle method
 * - J/ψ flipping for systematic uncertainty studies
 * - Comprehensive quality assessment and selection
 * - Integration with existing dimuon-based framework
 * 
 * Physics process: Bc+ → J/ψ(→ μ+μ-) + μ+ + ν_μ
 * 
 * @author Your Name
 * @date 2025
 */
class BcDiMuMuProducer : public edm::one::EDProducer<> {
public:
    
    /**
     * @brief Constructor
     * @param iConfig Parameter set with configuration
     */
    explicit BcDiMuMuProducer(const edm::ParameterSet& iConfig);
    
    /**
     * @brief Destructor
     */
    ~BcDiMuMuProducer();
    
    /**
     * @brief Produce Bc candidates for one event
     * @param iEvent The event
     * @param iSetup The event setup
     */
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;

private:
    
    // Configuration parameters
    bool saveDetailedInfo_;          ///< Save detailed analysis variables
    bool saveTrimuonInfo_;          ///< Save intermediate trimuon info
    std::string bcLabel_;           ///< Label for Bc collection
    std::string trimuonLabel_;      ///< Label for trimuon collection (if saved)
    
    // Algorithm instance
    std::unique_ptr<BcDiMuMuFunctional> bcFitter_;
    
    // Statistics counters
    mutable unsigned long nEvents_;
    mutable unsigned long nBcCandidates_;
    mutable unsigned long nTrimuonCandidates_;
    
    /**
     * @brief Initialize the algorithm
     * @param iConfig Parameter set
     */
    void initializeAlgorithm(const edm::ParameterSet& iConfig);
    
    /**
     * @brief Print statistics at end of job
     */
    virtual void endJob() override;
    
    /**
     * @brief Print configuration summary
     */
    void printConfiguration() const;
};

#endif