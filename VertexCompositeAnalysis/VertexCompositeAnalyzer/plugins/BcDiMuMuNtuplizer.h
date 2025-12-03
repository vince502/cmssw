#ifndef VertexCompositeAnalyzer_BcDiMuMuNtuplizer_h
#define VertexCompositeAnalyzer_BcDiMuMuNtuplizer_h

// system include files
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "TTree.h"
#include "TLorentzVector.h"

/**
 * @class BcDiMuMuNtuplizer
 * @brief EDAnalyzer for creating analysis ntuples from Bc meson candidates
 * 
 * This analyzer creates comprehensive ROOT ntuples from Bc meson candidates
 * reconstructed via the semi-leptonic decay Bc → J/ψ + μ + ν. It handles
 * the complex structure of the reconstruction including:
 * - Neutrino momentum inference variables
 * - Trimuon system kinematics
 * - Prong rotation systematic variations
 * - Generator-level matching for MC studies
 * 
 * The ntuple structure is optimized for physics analysis including ML training.
 * 
 * @author Your Name
 * @date 2025
 */
class BcDiMuMuNtuplizer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
    
    explicit BcDiMuMuNtuplizer(const edm::ParameterSet&);
    ~BcDiMuMuNtuplizer();

private:
    
    virtual void beginJob() override;
    virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
    virtual void endRun(const edm::Run&, const edm::EventSetup&) override {}; 
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    // Tree management
    void initTree();
    void resetBranches();
    
    // Analysis functions
    void fillEventInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup);
    void fillBcCandidates(const edm::Event& iEvent, const edm::EventSetup& iSetup);
    void fillGenInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup);
    
    // Helper functions
    bool isGoodBcCandidate(const pat::CompositeCandidate& bc);
    double calculateInvariantMass(const std::vector<TLorentzVector>& particles);
    std::pair<double, double> calculateIsolation(const pat::CompositeCandidate& bc, 
                                                const edm::Event& iEvent);

    // Input tokens
    edm::EDGetTokenT<pat::CompositeCandidateCollection> bcToken_;
    edm::EDGetTokenT<pat::CompositeCandidateCollection> trimuonToken_;
    edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
    edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
    edm::EDGetTokenT<edm::TriggerResults> triggerToken_;
    edm::EDGetTokenT<reco::Centrality> centralityToken_;
    edm::EDGetTokenT<int> centralityBinToken_;
    edm::EDGetTokenT<reco::GenParticleCollection> genToken_;
    
    // Configuration parameters
    bool doGenMatching_;
    bool saveBcCandidates_;
    bool saveTrimuonInfo_;
    bool saveFullInfo_;
    bool isMC_;
    double genMatchingDeltaR_;
    
    // Tree and branches
    edm::Service<TFileService> fs;
    TTree* bcDiMuMuTree;
    
    // Constants for array sizes
    static const int MAXBC = 1000;
    static const int MAXGEN = 1000;

    // Event information branches
    UInt_t   runNb;
    ULong64_t eventNb;
    UInt_t   lumiSection;
    Short_t  centrality;
    Int_t    Ntrkoffline;
    Short_t  nPV;
    Float_t  bestvx;
    Float_t  bestvy;
    Float_t  bestvz;
    Float_t  bestvxError;
    Float_t  bestvyError;
    Float_t  bestvzError;

    // Bc candidate information
    UShort_t nBc;
    
    // Basic Bc kinematics
    Float_t  bcPt[MAXBC];
    Float_t  bcEta[MAXBC];
    Float_t  bcPhi[MAXBC];
    Float_t  bcY[MAXBC];
    Float_t  bcMass[MAXBC];              // Neutrino-corrected mass
    Float_t  bcCharge[MAXBC];
    
    // Trimuon (visible) system
    Float_t  trimuonPt[MAXBC];
    Float_t  trimuonEta[MAXBC]; 
    Float_t  trimuonPhi[MAXBC];
    Float_t  trimuonY[MAXBC];
    Float_t  trimuonMass[MAXBC];         // Visible 3-muon mass
    Float_t  trimuonCharge[MAXBC];
    
    // J/ψ (dimuon) information
    Float_t  jpsiPt[MAXBC];
    Float_t  jpsiEta[MAXBC];
    Float_t  jpsiPhi[MAXBC];
    Float_t  jpsiY[MAXBC];
    Float_t  jpsiMass[MAXBC];
    Float_t  jpsiCharge[MAXBC];
    
    // Individual muon information (J/ψ muons + extra muon)
    Float_t  muon1Pt[MAXBC];             // Leading J/ψ muon
    Float_t  muon1Eta[MAXBC];
    Float_t  muon1Phi[MAXBC];
    Float_t  muon1Charge[MAXBC];
    Bool_t   muon1IsGlobal[MAXBC];
    Bool_t   muon1IsTracker[MAXBC];
    Bool_t   muon1IsPF[MAXBC];
    Float_t  muon1Dxy[MAXBC];
    Float_t  muon1Dz[MAXBC];
    Short_t  muon1NHits[MAXBC];
    Short_t  muon1NPixelHits[MAXBC];
    Short_t  muon1NStations[MAXBC];
    Float_t  muon1Chi2[MAXBC];
    
    Float_t  muon2Pt[MAXBC];             // Subleading J/ψ muon
    Float_t  muon2Eta[MAXBC];
    Float_t  muon2Phi[MAXBC];
    Float_t  muon2Charge[MAXBC];
    Bool_t   muon2IsGlobal[MAXBC];
    Bool_t   muon2IsTracker[MAXBC];
    Bool_t   muon2IsPF[MAXBC];
    Float_t  muon2Dxy[MAXBC];
    Float_t  muon2Dz[MAXBC];
    Short_t  muon2NHits[MAXBC];
    Short_t  muon2NPixelHits[MAXBC];
    Short_t  muon2NStations[MAXBC];
    Float_t  muon2Chi2[MAXBC];
    
    Float_t  muon3Pt[MAXBC];             // Extra muon (from Bc decay)
    Float_t  muon3Eta[MAXBC];
    Float_t  muon3Phi[MAXBC];
    Float_t  muon3Charge[MAXBC];
    Bool_t   muon3IsGlobal[MAXBC];
    Bool_t   muon3IsTracker[MAXBC];
    Bool_t   muon3IsPF[MAXBC];
    Float_t  muon3Dxy[MAXBC];
    Float_t  muon3Dz[MAXBC];
    Short_t  muon3NHits[MAXBC];
    Short_t  muon3NPixelHits[MAXBC];
    Short_t  muon3NStations[MAXBC];
    Float_t  muon3Chi2[MAXBC];

    // Vertex information
    Float_t  bcVtxChi2[MAXBC];
    Float_t  bcVtxNdof[MAXBC];
    Float_t  bcVtxProb[MAXBC];
    Float_t  bcVtxX[MAXBC];
    Float_t  bcVtxY[MAXBC];
    Float_t  bcVtxZ[MAXBC];
    Float_t  bcVtxXError[MAXBC];
    Float_t  bcVtxYError[MAXBC];
    Float_t  bcVtxZError[MAXBC];

    // Neutrino inference variables (key for Bc reconstruction)
    Float_t  neutrinoPerpMomentum[MAXBC]; // Perpendicular momentum component
    Float_t  neutrinoInferredMass[MAXBC]; // Mass with neutrino correction
    Float_t  massCorrectionFactor[MAXBC]; // Correction factor applied
    
    // Topological variables
    Float_t  bcPointingAngle3D[MAXBC];
    Float_t  bcPointingAngle2D[MAXBC];
    Float_t  bcCosAlpha3D[MAXBC];        // Cosine of 3D pointing angle  
    Float_t  bcCosAlpha2D[MAXBC];        // Cosine of 2D pointing angle
    Float_t  bcDecayLength3D[MAXBC];
    Float_t  bcDecayLength2D[MAXBC];
    Float_t  bcDecayLengthSig3D[MAXBC];
    Float_t  bcDecayLengthSig2D[MAXBC];
    Float_t  bcFlightDistance3D[MAXBC];
    Float_t  bcFlightDistance2D[MAXBC];
    
    // Impact parameters
    Float_t  bcImpactParameter3D[MAXBC];
    Float_t  bcImpactParameter2D[MAXBC];
    Float_t  bcImpactParameterSig3D[MAXBC];
    Float_t  bcImpactParameterSig2D[MAXBC];
    
    // Quality and selection variables
    Float_t  bcQualityScore[MAXBC];      // Overall reconstruction quality
    Bool_t   bcPassSelection[MAXBC];     // Passes standard selection
    Float_t  bcMVAScore[MAXBC];          // MVA discriminator (if available)
    
    // Isolation variables
    Float_t  bcTrackIsolation[MAXBC];    // Track-based isolation
    Float_t  bcCaloIsolation[MAXBC];     // Calorimeter-based isolation
    
    // Systematic variations (prong rotation)
    Bool_t   bcJpsiFlipped[MAXBC];       // J/ψ was flipped for systematics
    Short_t  bcJpsiFlipMode[MAXBC];      // Flip mode used (0-8)
    Float_t  bcMassOriginal[MAXBC];      // Mass before any corrections
    Float_t  bcMassFlipped[MAXBC];       // Mass with prong flipping
    
    // Angular variables and correlations
    Float_t  jpsiMuonDeltaR[MAXBC];      // ΔR between J/ψ muons
    Float_t  jpsiExtraMuonDeltaR[MAXBC]; // ΔR between J/ψ and extra muon
    Float_t  trimuonOpeningAngle[MAXBC]; // Opening angle of trimuon system
    
    // Mass combinations and intermediate states
    Float_t  muon1Muon3Mass[MAXBC];      // All possible dimuon combinations
    Float_t  muon2Muon3Mass[MAXBC];
    Float_t  minDimuonMass[MAXBC];       // Minimum dimuon mass
    Float_t  maxDimuonMass[MAXBC];       // Maximum dimuon mass
    
    // Generator-level information (MC only)
    UShort_t nGenBc;
    Float_t  genBcPt[MAXGEN];
    Float_t  genBcEta[MAXGEN];
    Float_t  genBcPhi[MAXGEN];
    Float_t  genBcY[MAXGEN];
    Float_t  genBcMass[MAXGEN];
    Short_t  genBcPdgId[MAXGEN];
    Short_t  genBcMotherId[MAXGEN];
    
    // Generator matching
    Bool_t   bcMatchedGen[MAXBC];        // Matched to generator Bc
    Float_t  bcGenDeltaR[MAXBC];         // ΔR to closest gen Bc
    Short_t  bcGenIndex[MAXBC];          // Index of matched gen Bc
    
    // Generator-level decay products
    UShort_t nGenMuons;
    Float_t  genMuonPt[MAXGEN];
    Float_t  genMuonEta[MAXGEN];
    Float_t  genMuonPhi[MAXGEN];
    Short_t  genMuonCharge[MAXGEN];
    Short_t  genMuonMother[MAXGEN];      // PDG ID of mother particle
    
    UShort_t nGenNeutrinos;
    Float_t  genNeutrinoPt[MAXGEN];
    Float_t  genNeutrinoEta[MAXGEN];
    Float_t  genNeutrinoPhi[MAXGEN];
    Float_t  genNeutrinoE[MAXGEN];
    Short_t  genNeutrinoPdgId[MAXGEN];
    
    // Event-level generator information
    Float_t  genMissingPt;               // True missing pT from neutrinos
    Float_t  genMissingPhi;              // True missing phi
    Float_t  genMissingMass;             // True missing mass
    
    // Trigger information
    Bool_t   triggerFired[32];           // Up to 32 trigger bits
    std::vector<std::string> triggerNames;
    
    // Additional event characterization for heavy ion events
    Float_t  eventPlane;                 // Event plane angle
    Float_t  eventPlaneResolution;       // Event plane resolution
    
    // Counters for efficiency calculations
    mutable unsigned long nEventsTotal;
    mutable unsigned long nEventsWithBc;
    mutable unsigned long nBcCandidatesTotal;
    mutable unsigned long nBcCandidatesSelected;
};

#endif