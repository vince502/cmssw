#ifndef VertexCompositeProducer_BcDiMuMuFunctional_h
#define VertexCompositeProducer_BcDiMuMuFunctional_h

// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/Common/interface/RefToBase.h"

#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "TrackingTools/IPTools/interface/IPTools.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "DataFormats/Math/interface/Vector3D.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/Measurement1D.h"

class BcDiMuMuFunctional {
public:
    
    // Constructor
    BcDiMuMuFunctional(const edm::ParameterSet& iConfig, edm::ConsumesCollector && iC);
    
    // Destructor
    ~BcDiMuMuFunctional();

    // Main fitting function
    void fitBc(const edm::Event& iEvent, const edm::EventSetup& iSetup);

    // Physics constants
    static constexpr double bcMass = 6.2756;           // Bc meson mass (GeV/c²) - PDG 2020
    static constexpr double jpsiMass = 3.096916;       // J/ψ mass (GeV/c²) - PDG 2020
    static constexpr double muonMass = 0.1056583745;   // Muon mass (GeV/c²) - PDG 2020
    static constexpr double bcMass_sigma = 1.0e-4;     // Mass constraint uncertainty
    static constexpr double bcLifetime = 0.507e-12;    // Bc lifetime (s) - PDG 2020
    
    // Get results
    const pat::CompositeCandidateCollection& getBcCandidates() const { return theBc; }

private:

    // ===== Core Functional Components =====
    
    struct TrimuonCandidate {
        TransientVertex vertex;
        std::vector<reco::TransientTrack> tracks;
        pat::CompositeCandidate candidate;
        int jpsiMuon1Idx, jpsiMuon2Idx, extraMuonIdx;  // Track indices
        double jpsiMass, trimuonMass;
        double vtxChi2, vtxNdof, vtxProb;
        GlobalVector momentum;
        double charge;
        bool isValid;
        
        TrimuonCandidate() : isValid(false) {}
    };
    
    struct NeutrinoInference {
        double correctedMass;           // Mass corrected for neutrino
        double perpendicularMomentum;   // P_perp component
        double pointingAngle3D;         // 3D pointing angle
        double pointingAngle2D;         // 2D pointing angle
        double cosAlpha3D, cosAlpha2D;  // Cosines of pointing angles
        GlobalVector flightDirection3D;
        GlobalVector flightDirection2D;
        double decayLength3D, decayLength2D;
        double decayLengthSig3D, decayLengthSig2D;
        bool isValid;
        
        NeutrinoInference() : isValid(false) {}
    };
    
    struct BcCandidate {
        pat::CompositeCandidate candidate;
        TrimuonCandidate trimuon;
        NeutrinoInference neutrino;
        double qualityScore;
        bool passesSelection;
        
        BcCandidate() : passesSelection(false) {}
    };
    
    struct QualityMetrics {
        double vertexQuality;      // Combined vertex quality score
        double kinematicQuality;   // Kinematic consistency score
        double pointingQuality;    // Flight direction quality
        double overallScore;       // Combined quality score
        bool passesMinQuality;
    };

    // ===== Core Reconstruction Functions =====
    
    /**
     * @brief Fit trimuon vertex from dimuon and additional muon
     * @param dimuon The J/ψ candidate (dimuon)
     * @param extraMuon The additional muon for Bc reconstruction
     * @param primaryVertex The primary vertex
     * @return TrimuonCandidate with fitted vertex and kinematics
     */
    TrimuonCandidate fitTrimuonVertex(const pat::CompositeCandidate& dimuon, 
                                     const pat::Muon& extraMuon,
                                     const reco::Vertex& primaryVertex);
    
    /**
     * @brief Calculate neutrino kinematics using pointing angle method
     * @param trimuon The fitted trimuon candidate
     * @param primaryVertex The primary vertex
     * @return NeutrinoInference with corrected mass and momentum
     */
    NeutrinoInference calculateNeutrinoKinematics(const TrimuonCandidate& trimuon,
                                                 const reco::Vertex& primaryVertex);
    
    /**
     * @brief Reconstruct complete Bc meson candidate
     * @param trimuon The trimuon candidate
     * @param neutrino The neutrino inference
     * @return Complete BcCandidate
     */
    BcCandidate reconstructBcMeson(const TrimuonCandidate& trimuon,
                                  const NeutrinoInference& neutrino);

    // ===== Prong Rotation and Systematic Functions =====
    
    /**
     * @brief Rotate momentum vector to flight direction frame
     * @param momentum Original momentum vector
     * @param flightDirection Flight direction vector
     * @return Rotated momentum vector
     */
    GlobalVector rotateToFlightDirection(const GlobalVector& momentum, 
                                       const GlobalVector& flightDirection);
    
    /**
     * @brief Calculate prong rotation angle for systematic studies
     * @param vertex The fitted vertex
     * @param primaryVertex The primary vertex
     * @return Rotation angle in radians
     */
    double calculateProngRotationAngle(const TransientVertex& vertex, 
                                     const reco::Vertex& primaryVertex);
    
    /**
     * @brief Apply J/ψ flipping for systematic uncertainty studies
     * @param track Input track
     * @param flipMode Flip mode (0-8 for different rotation patterns)
     * @return Modified track with rotated momentum
     */
    reco::Track applyJpsiFlipping(const reco::Track& track, int flipMode);
    
    /**
     * @brief Rotate track momentum components
     * @param track Input track
     * @param flipJpsi Rotation pattern index
     * @return Rotated momentum vector
     */
    reco::TrackBase::Vector rotateMomentum(const reco::Track& track, int flipJpsi);

    // ===== Selection and Quality Assessment =====
    
    /**
     * @brief Check if trimuon candidate passes selection criteria
     * @param trimuon The trimuon candidate
     * @return True if passes selection
     */
    bool passesTrimuonSelection(const TrimuonCandidate& trimuon);
    
    /**
     * @brief Calculate Bc mass hypothesis with neutrino correction
     * @param trimuon The trimuon candidate
     * @param neutrino The neutrino inference
     * @return Corrected Bc mass
     */
    double calculateBcMassHypothesis(const TrimuonCandidate& trimuon,
                                   const NeutrinoInference& neutrino);
    
    /**
     * @brief Assess overall reconstruction quality
     * @param bcCandidate The Bc candidate
     * @return Quality metrics and scores
     */
    QualityMetrics assessReconstructionQuality(const BcCandidate& bcCandidate);
    
    /**
     * @brief Apply Bc-specific selection cuts
     * @param bcCandidate The Bc candidate
     * @return True if passes all cuts
     */
    bool passesBcSelection(const BcCandidate& bcCandidate);

    // ===== Utility Functions =====
    
    /**
     * @brief Calculate 3D pointing angle
     * @param momentum Momentum vector
     * @param displacement Displacement vector (vertex - PV)
     * @return Cosine of 3D pointing angle
     */
    double calculatePointingAngle3D(const GlobalVector& momentum,
                                   const GlobalVector& displacement);
    
    /**
     * @brief Calculate 2D pointing angle (transverse plane)
     * @param momentum Momentum vector
     * @param displacement Displacement vector
     * @return Cosine of 2D pointing angle
     */
    double calculatePointingAngle2D(const GlobalVector& momentum,
                                   const GlobalVector& displacement);
    
    /**
     * @brief Calculate decay length and significance
     * @param primaryVertex Primary vertex
     * @param secondaryVertex Secondary vertex
     * @param is2D Calculate 2D (transverse) or 3D
     * @return Pair of (decay length, significance)
     */
    std::pair<double, double> calculateDecayLength(const reco::Vertex& primaryVertex,
                                                  const TransientVertex& secondaryVertex,
                                                  bool is2D = false);
    
    /**
     * @brief Get best primary vertex for the event
     * @param vertices Primary vertex collection
     * @return Best primary vertex
     */
    const reco::Vertex& getBestPrimaryVertex(const reco::VertexCollection& vertices);
    
    /**
     * @brief Calculate impact parameter and significance
     * @param track Transient track
     * @param vertex Primary vertex
     * @return Pair of (IP, IP significance)
     */
    std::pair<double, double> calculateImpactParameter(const reco::TransientTrack& track,
                                                      const reco::Vertex& vertex);

    // ===== Configuration and Data Members =====
    
    // Input tokens
    edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
    edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
    edm::EDGetTokenT<pat::CompositeCandidateCollection> dimuonToken_;
    edm::EDGetTokenT<pat::MuonCollection> muonToken_;
    
    // Configuration parameters
    double bcMassCut_;              // Mass window for Bc candidates (GeV)
    double bcPtCut_;                // Minimum Bc pT (GeV/c)
    double bcYCut_;                 // Maximum |y| for Bc
    double dimuonMassMin_;          // J/ψ mass window minimum (GeV/c²)
    double dimuonMassMax_;          // J/ψ mass window maximum (GeV/c²)
    double dimuonPtCut_;            // Minimum J/ψ pT (GeV/c)
    double tkPtCut_;                // Minimum track pT for extra muon (GeV/c)
    double tkChi2Cut_;              // Maximum track χ²/ndf
    double vtxProbCut_;             // Minimum vertex probability
    double rVtxSigCut_;             // Minimum 2D decay length significance
    double rVtx3DSigCut_;           // Minimum 3D decay length significance
    double pointingAngleCut_;       // Maximum pointing angle (cos > cut)
    double impactParameterCut_;     // Maximum impact parameter significance
    
    // Analysis control
    bool doJpsiFlip_;               // Enable J/ψ flipping for systematics
    int jpsiFlipMode_;              // J/ψ flip mode (0-8)
    bool useBeamSpotConstraint_;    // Use beam spot constraint in fitting
    bool saveDetailedInfo_;         // Save detailed analysis variables
    
    // Results storage
    pat::CompositeCandidateCollection theBc;
    
    // Framework handles (CMSSW 15+ style with ESGetToken)
    edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> theBToken_;
    edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bFieldToken_;
    const TransientTrackBuilder* theB;
    const MagneticField* bField;
    
    // Vertex fitter
    KalmanVertexFitter kvf;
    
    // Statistics counters
    mutable unsigned int nBcCandidates_;
    mutable unsigned int nTrimuonCandidates_;
    mutable unsigned int nPassedSelection_;

    // ===== Private Helper Functions =====
    
    /**
     * @brief Initialize event setup handles
     * @param iSetup Event setup
     */
    void initializeEventSetup(const edm::EventSetup& iSetup);
    
    /**
     * @brief Create pat::CompositeCandidate from trimuon
     * @param trimuon Trimuon candidate
     * @return pat::CompositeCandidate
     */
    pat::CompositeCandidate createCompositeBc(const TrimuonCandidate& trimuon);
    
    /**
     * @brief Add analysis variables to candidate using UserData
     * @param candidate The composite candidate
     * @param bcCandidate The Bc candidate with analysis info
     */
    void addAnalysisVariables(pat::CompositeCandidate& candidate, 
                             const BcCandidate& bcCandidate);
    
    /**
     * @brief Validate input collections
     * @param iEvent Event
     * @return True if all required collections are valid
     */
    bool validateInputs(const edm::Event& iEvent);
    
    /**
     * @brief Print debugging information
     * @param bcCandidate The Bc candidate
     */
    void printDebugInfo(const BcCandidate& bcCandidate) const;
};

#endif