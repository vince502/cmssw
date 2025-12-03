#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BcDiMuMuFunctional.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"

#include "TMath.h"
#include <algorithm>
#include <cmath>

using namespace std;
using namespace reco;
using namespace edm;

BcDiMuMuFunctional::BcDiMuMuFunctional(const edm::ParameterSet& iConfig, edm::ConsumesCollector && iC) :
    // Initialize input tokens (must match declaration order in header)
    vertexToken_(iC.consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexCollection"))),
    beamSpotToken_(iC.consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotCollection"))),
    dimuonToken_(iC.consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("dimuonCollection"))),
    muonToken_(iC.consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muonCollection"))),
    
    // Configuration parameters
    bcMassCut_(iConfig.getParameter<double>("bcMassCut")),
    bcPtCut_(iConfig.getParameter<double>("bcPtCut")),
    bcYCut_(iConfig.getParameter<double>("bcYCut")),
    dimuonMassMin_(iConfig.getParameter<double>("dimuonMassMin")),
    dimuonMassMax_(iConfig.getParameter<double>("dimuonMassMax")),
    dimuonPtCut_(iConfig.getParameter<double>("dimuonPtCut")),
    tkPtCut_(iConfig.getParameter<double>("tkPtCut")),
    tkChi2Cut_(iConfig.getParameter<double>("tkChi2Cut")),
    vtxProbCut_(iConfig.getParameter<double>("vtxProbCut")),
    rVtxSigCut_(iConfig.getParameter<double>("rVtxSigCut")),
    rVtx3DSigCut_(iConfig.getParameter<double>("rVtx3DSigCut")),
    pointingAngleCut_(iConfig.getParameter<double>("pointingAngleCut")),
    impactParameterCut_(iConfig.getParameter<double>("impactParameterCut")),
    
    // Analysis control
    doJpsiFlip_(iConfig.getParameter<bool>("doJpsiFlip")),
    jpsiFlipMode_(iConfig.getParameter<int>("jpsiFlipMode")),
    useBeamSpotConstraint_(iConfig.getParameter<bool>("useBeamSpotConstraint")),
    saveDetailedInfo_(iConfig.getParameter<bool>("saveDetailedInfo")),
    
    // ESGetTokens for CMSSW 15+ (after theBc in declaration order)
    theBToken_(iC.esConsumes<TransientTrackBuilder, TransientTrackRecord>()),
    bFieldToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>()),
    theB(nullptr),
    bField(nullptr),
    
    // Statistics counters
    nBcCandidates_(0),
    nTrimuonCandidates_(0),
    nPassedSelection_(0)
{
}

BcDiMuMuFunctional::~BcDiMuMuFunctional() {
}

void BcDiMuMuFunctional::fitBc(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    // Initialize event setup
    initializeEventSetup(iSetup);
    
    // Validate inputs
    if (!validateInputs(iEvent)) {
        return;
    }
    
    // Get input collections
    edm::Handle<reco::VertexCollection> vertexHandle;
    edm::Handle<reco::BeamSpot> beamSpotHandle;
    edm::Handle<pat::CompositeCandidateCollection> dimuonHandle;
    edm::Handle<pat::MuonCollection> muonHandle;
    
    iEvent.getByToken(vertexToken_, vertexHandle);
    iEvent.getByToken(beamSpotToken_, beamSpotHandle);
    iEvent.getByToken(dimuonToken_, dimuonHandle);
    iEvent.getByToken(muonToken_, muonHandle);
    
    // Get best primary vertex
    const reco::Vertex& primaryVertex = getBestPrimaryVertex(*vertexHandle);
    
    // Clear previous results
    theBc.clear();
    
    // Loop over dimuon candidates (J/ψ candidates)
    for (const auto& dimuon : *dimuonHandle) {
        
        // Check if dimuon passes J/ψ selection
        if (dimuon.mass() < dimuonMassMin_ || dimuon.mass() > dimuonMassMax_) continue;
        if (dimuon.pt() < dimuonPtCut_) continue;
        
        // Loop over additional muon candidates
        for (const auto& extraMuon : *muonHandle) {
            
            // Skip if this muon is already part of the dimuon
            bool skipMuon = false;
            for (size_t i = 0; i < dimuon.numberOfDaughters(); ++i) {
                const pat::Muon* dimuonMuon = dynamic_cast<const pat::Muon*>(dimuon.daughter(i));
                if (dimuonMuon && deltaR(*dimuonMuon, extraMuon) < 0.01) {
                    skipMuon = true;
                    break;
                }
            }
            if (skipMuon) continue;
            
            // Apply basic track cuts to extra muon
            if (extraMuon.pt() < tkPtCut_) continue;
            if (extraMuon.innerTrack().isNull()) continue;
            if (extraMuon.innerTrack()->normalizedChi2() > tkChi2Cut_) continue;
            
            // Step 1: Fit trimuon vertex
            TrimuonCandidate trimuon = fitTrimuonVertex(dimuon, extraMuon, primaryVertex);
            if (!trimuon.isValid) continue;
            
            nTrimuonCandidates_++;
            
            // Check trimuon selection
            if (!passesTrimuonSelection(trimuon)) continue;
            
            // Step 2: Calculate neutrino kinematics
            NeutrinoInference neutrino = calculateNeutrinoKinematics(trimuon, primaryVertex);
            if (!neutrino.isValid) continue;
            
            // Step 3: Reconstruct Bc meson
            BcCandidate bcCandidate = reconstructBcMeson(trimuon, neutrino);
            
            // Step 4: Apply Bc selection
            if (!passesBcSelection(bcCandidate)) continue;
            
            // Step 5: Assess quality
            QualityMetrics quality = assessReconstructionQuality(bcCandidate);
            bcCandidate.qualityScore = quality.overallScore;
            
            if (!quality.passesMinQuality) continue;
            
            // Create final composite candidate
            pat::CompositeCandidate finalBc = createCompositeBc(trimuon);
            
            // Add analysis variables
            addAnalysisVariables(finalBc, bcCandidate);
            
            // Store the candidate
            theBc.push_back(finalBc);
            nBcCandidates_++;
            nPassedSelection_++;
            
            // Debug output if requested
            if (saveDetailedInfo_) {
                printDebugInfo(bcCandidate);
            }
        }
    }
}

BcDiMuMuFunctional::TrimuonCandidate BcDiMuMuFunctional::fitTrimuonVertex(
    const pat::CompositeCandidate& dimuon, 
    const pat::Muon& extraMuon,
    const reco::Vertex& primaryVertex) {
    
    TrimuonCandidate result;
    
    try {
        // Get transient tracks
        vector<reco::TransientTrack> tracks;
        
        // Add dimuon tracks
        for (size_t i = 0; i < dimuon.numberOfDaughters(); ++i) {
            const pat::Muon* muon = dynamic_cast<const pat::Muon*>(dimuon.daughter(i));
            if (muon && muon->innerTrack().isNonnull()) {
                reco::Track track = *muon->innerTrack();
                
                // Apply J/ψ flipping if requested
                if (doJpsiFlip_ && jpsiFlipMode_ >= 0) {
                    track = applyJpsiFlipping(track, jpsiFlipMode_);
                }
                
                reco::TransientTrack tt = theB->build(track);
                if (tt.isValid()) {
                    tracks.push_back(tt);
                    if (i == 0) result.jpsiMuon1Idx = tracks.size() - 1;
                    if (i == 1) result.jpsiMuon2Idx = tracks.size() - 1;
                }
            }
        }
        
        // Add extra muon track
        if (extraMuon.innerTrack().isNonnull()) {
            reco::TransientTrack tt = theB->build(*extraMuon.innerTrack());
            if (tt.isValid()) {
                tracks.push_back(tt);
                result.extraMuonIdx = tracks.size() - 1;
            }
        }
        
        if (tracks.size() != 3) {
            return result; // Invalid - need exactly 3 tracks
        }
        
        // Fit vertex
        result.vertex = kvf.vertex(tracks);
        if (!result.vertex.isValid()) {
            return result;
        }
        
        // Store tracks and basic info
        result.tracks = tracks;
        result.vtxChi2 = result.vertex.totalChiSquared();
        result.vtxNdof = result.vertex.degreesOfFreedom();
        result.vtxProb = TMath::Prob(result.vtxChi2, (int)result.vtxNdof);
        
        // Calculate trimuon 4-momentum
        math::PtEtaPhiMLorentzVector p4_trimuon;
        double totalCharge = 0;
        
        for (const auto& track : tracks) {
            math::PtEtaPhiMLorentzVector p4_muon(track.track().pt(), 
                                                track.track().eta(),
                                                track.track().phi(), 
                                                muonMass);
            p4_trimuon += p4_muon;
            totalCharge += track.track().charge();
        }
        
        // Store masses and momentum
        result.jpsiMass = dimuon.mass();
        result.trimuonMass = p4_trimuon.M();
        result.charge = totalCharge;
        result.momentum = GlobalVector(p4_trimuon.px(), p4_trimuon.py(), p4_trimuon.pz());
        
        // Create composite candidate
        result.candidate.setP4(p4_trimuon);
        result.candidate.setCharge(totalCharge);
        result.candidate.setVertex(reco::Candidate::Point(result.vertex.position().x(),
                                                         result.vertex.position().y(),
                                                         result.vertex.position().z()));
        
        result.isValid = true;
        
    } catch (const std::exception& e) {
        // Vertex fitting failed
        result.isValid = false;
    }
    
    return result;
}

BcDiMuMuFunctional::NeutrinoInference BcDiMuMuFunctional::calculateNeutrinoKinematics(
    const TrimuonCandidate& trimuon,
    const reco::Vertex& primaryVertex) {
    
    NeutrinoInference result;
    
    try {
        // Calculate flight direction
        GlobalPoint primaryPos(primaryVertex.position().x(),
                              primaryVertex.position().y(),
                              primaryVertex.position().z());
        GlobalPoint secondaryPos = trimuon.vertex.position();
        
        GlobalVector displacement = secondaryPos - primaryPos;
        result.flightDirection3D = displacement.unit();
        result.flightDirection2D = GlobalVector(displacement.x(), displacement.y(), 0).unit();
        
        // Calculate decay lengths
        auto decayLength3D = calculateDecayLength(primaryVertex, trimuon.vertex, false);
        auto decayLength2D = calculateDecayLength(primaryVertex, trimuon.vertex, true);
        
        result.decayLength3D = decayLength3D.first;
        result.decayLengthSig3D = decayLength3D.second;
        result.decayLength2D = decayLength2D.first;
        result.decayLengthSig2D = decayLength2D.second;
        
        // Calculate pointing angles
        result.cosAlpha3D = calculatePointingAngle3D(trimuon.momentum, displacement);
        result.cosAlpha2D = calculatePointingAngle2D(trimuon.momentum, displacement);
        result.pointingAngle3D = acos(std::max(-1.0, std::min(1.0, result.cosAlpha3D)));
        result.pointingAngle2D = acos(std::max(-1.0, std::min(1.0, result.cosAlpha2D)));
        
        // Core neutrino momentum inference algorithm
        // Based on OniaTreeSubmodule implementation
        double Mtrimu = trimuon.trimuonMass;                    // Visible trimuon mass
        double Ptrimu = trimuon.momentum.mag();                 // Total momentum magnitude
        double sinalpha = sin(result.pointingAngle3D);          // sin(pointing angle)
        double PperpTrimu = sinalpha * Ptrimu;                  // Perpendicular momentum component
        
        // Corrected mass formula: M_corrected = √(M_visible² + P_perp²) + P_perp
        result.perpendicularMomentum = PperpTrimu;
        result.correctedMass = sqrt(Mtrimu * Mtrimu + PperpTrimu * PperpTrimu) + PperpTrimu;
        
        result.isValid = true;
        
    } catch (const std::exception& e) {
        result.isValid = false;
    }
    
    return result;
}

BcDiMuMuFunctional::BcCandidate BcDiMuMuFunctional::reconstructBcMeson(
    const TrimuonCandidate& trimuon,
    const NeutrinoInference& neutrino) {
    
    BcCandidate result;
    result.trimuon = trimuon;
    result.neutrino = neutrino;
    
    // Create the Bc candidate with corrected mass
    math::PtEtaPhiMLorentzVector bcP4(trimuon.candidate.pt(),
                                     trimuon.candidate.eta(),
                                     trimuon.candidate.phi(),
                                     neutrino.correctedMass);
    
    result.candidate = trimuon.candidate;
    result.candidate.setP4(bcP4);
    
    return result;
}

// ===== Prong Rotation and Systematic Functions =====

GlobalVector BcDiMuMuFunctional::rotateToFlightDirection(const GlobalVector& momentum, 
                                                         const GlobalVector& flightDirection) {
    // Rotate momentum vector to align with flight direction
    // This is used for systematic studies and resolution corrections
    
    double cosTheta = momentum.dot(flightDirection) / (momentum.mag() * flightDirection.mag());
    double theta = acos(std::max(-1.0, std::min(1.0, cosTheta)));
    
    // Simple rotation preserving magnitude
    double newPx = momentum.x() * cos(theta) - momentum.z() * sin(theta);
    double newPy = momentum.y();
    double newPz = momentum.x() * sin(theta) + momentum.z() * cos(theta);
    
    return GlobalVector(newPx, newPy, newPz);
}

double BcDiMuMuFunctional::calculateProngRotationAngle(const TransientVertex& vertex, 
                                                      const reco::Vertex& primaryVertex) {
    GlobalPoint primaryPos(primaryVertex.position().x(),
                          primaryVertex.position().y(),
                          primaryVertex.position().z());
    GlobalPoint secondaryPos = vertex.position();
    GlobalVector displacement = secondaryPos - primaryPos;
    
    // Return angle relative to z-axis
    return atan2(sqrt(displacement.x()*displacement.x() + displacement.y()*displacement.y()), 
                displacement.z());
}

reco::Track BcDiMuMuFunctional::applyJpsiFlipping(const reco::Track& track, int flipMode) {
    if (flipMode < 0 || flipMode > 8) {
        return track; // No flipping
    }
    
    // Get rotated momentum
    reco::TrackBase::Vector rotatedP = rotateMomentum(track, flipMode);
    
    // Create new track with rotated momentum
    reco::Track newTrack(track.chi2(), track.ndof(), track.referencePoint(),
                        rotatedP, track.charge(), track.covariance(),
                        track.algo(), track.qualityMask());
    
    return newTrack;
}

reco::TrackBase::Vector BcDiMuMuFunctional::rotateMomentum(const reco::Track& track, int flipJpsi) {
    // Implementation based on OniaTreeSubmodule HiOnia2MuMuPAT.cc
    float px = track.px(), py = track.py(), pz = track.pz();
    
    // Apply z-flip for modes 0-4
    if (flipJpsi <= 4) {
        pz = -track.pz();
    }
    
    // Apply specific rotation patterns
    switch (flipJpsi) {
        case 1: case 6: px = -track.px(); break;                    // Flip x
        case 2: case 7: py = -track.py(); break;                    // Flip y  
        case 3: case 8: px = -track.px(); py = -track.py(); break;  // Flip x,y
        case 4: case 5: break;                                      // Only z or no flip
        default: break;
    }
    
    return reco::TrackBase::Vector(px, py, pz);
}

// ===== Selection and Quality Assessment =====

bool BcDiMuMuFunctional::passesTrimuonSelection(const TrimuonCandidate& trimuon) {
    // Vertex quality cuts
    if (trimuon.vtxProb < vtxProbCut_) return false;
    if (trimuon.vtxChi2 / trimuon.vtxNdof > 10.0) return false;
    
    // Mass cuts
    if (trimuon.jpsiMass < dimuonMassMin_ || trimuon.jpsiMass > dimuonMassMax_) return false;
    
    // Charge requirement (Bc should have charge ±1)
    if (abs(trimuon.charge) != 1) return false;
    
    return true;
}

double BcDiMuMuFunctional::calculateBcMassHypothesis(const TrimuonCandidate& trimuon,
                                                    const NeutrinoInference& neutrino) {
    return neutrino.correctedMass;
}

BcDiMuMuFunctional::QualityMetrics BcDiMuMuFunctional::assessReconstructionQuality(
    const BcCandidate& bcCandidate) {
    
    QualityMetrics metrics;
    
    // Vertex quality (0-1 scale)
    metrics.vertexQuality = std::min(1.0, bcCandidate.trimuon.vtxProb * 10.0);
    
    // Kinematic quality based on mass resolution
    double massDiff = abs(bcCandidate.neutrino.correctedMass - bcMass);
    metrics.kinematicQuality = exp(-massDiff / 0.5); // Gaussian-like
    
    // Pointing quality
    metrics.pointingQuality = std::max(0.0, bcCandidate.neutrino.cosAlpha3D);
    
    // Combined score
    metrics.overallScore = (metrics.vertexQuality + metrics.kinematicQuality + 
                           metrics.pointingQuality) / 3.0;
    
    // Minimum quality threshold
    metrics.passesMinQuality = metrics.overallScore > 0.1;
    
    return metrics;
}

bool BcDiMuMuFunctional::passesBcSelection(const BcCandidate& bcCandidate) {
    const auto& neutrino = bcCandidate.neutrino;
    const auto& trimuon = bcCandidate.trimuon;
    
    // Mass window cut
    double massDiff = abs(neutrino.correctedMass - bcMass);
    if (massDiff > bcMassCut_) return false;
    
    // Kinematic cuts
    if (bcCandidate.candidate.pt() < bcPtCut_) return false;
    if (abs(bcCandidate.candidate.rapidity()) > bcYCut_) return false;
    
    // Decay length significance cuts
    if (neutrino.decayLengthSig2D < rVtxSigCut_) return false;
    if (neutrino.decayLengthSig3D < rVtx3DSigCut_) return false;
    
    // Pointing angle cut
    if (neutrino.cosAlpha3D < pointingAngleCut_) return false;
    
    return true;
}

// ===== Utility Functions =====

double BcDiMuMuFunctional::calculatePointingAngle3D(const GlobalVector& momentum,
                                                    const GlobalVector& displacement) {
    if (momentum.mag() == 0 || displacement.mag() == 0) return -1;
    return momentum.dot(displacement) / (momentum.mag() * displacement.mag());
}

double BcDiMuMuFunctional::calculatePointingAngle2D(const GlobalVector& momentum,
                                                    const GlobalVector& displacement) {
    GlobalVector momentum2D(momentum.x(), momentum.y(), 0);
    GlobalVector displacement2D(displacement.x(), displacement.y(), 0);
    
    if (momentum2D.mag() == 0 || displacement2D.mag() == 0) return -1;
    return momentum2D.dot(displacement2D) / (momentum2D.mag() * displacement2D.mag());
}

std::pair<double, double> BcDiMuMuFunctional::calculateDecayLength(
    const reco::Vertex& primaryVertex,
    const TransientVertex& secondaryVertex,
    bool is2D) {
    
    GlobalPoint primaryPos(primaryVertex.position().x(),
                          primaryVertex.position().y(),
                          primaryVertex.position().z());
    GlobalPoint secondaryPos = secondaryVertex.position();
    
    GlobalVector displacement = secondaryPos - primaryPos;
    
    double length, error;
    if (is2D) {
        length = sqrt(displacement.x()*displacement.x() + displacement.y()*displacement.y());
        // Approximate error calculation for 2D
        error = sqrt(primaryVertex.covariance(0,0) + primaryVertex.covariance(1,1) +
                    secondaryVertex.positionError().cxx() + secondaryVertex.positionError().cyy());
    } else {
        length = displacement.mag();
        // Approximate error calculation for 3D
        error = sqrt(primaryVertex.covariance(0,0) + primaryVertex.covariance(1,1) + 
                    primaryVertex.covariance(2,2) +
                    secondaryVertex.positionError().cxx() + secondaryVertex.positionError().cyy() +
                    secondaryVertex.positionError().czz());
    }
    
    double significance = (error > 0) ? length / error : 0;
    
    return std::make_pair(length, significance);
}

const reco::Vertex& BcDiMuMuFunctional::getBestPrimaryVertex(const reco::VertexCollection& vertices) {
    // Return the first valid vertex (highest sum pT²)
    for (const auto& vertex : vertices) {
        if (!vertex.isFake() && vertex.ndof() > 4) {
            return vertex;
        }
    }
    // If no good vertex found, return the first one
    return vertices[0];
}

std::pair<double, double> BcDiMuMuFunctional::calculateImpactParameter(
    const reco::TransientTrack& track,
    const reco::Vertex& vertex) {
    
    // This is a simplified implementation
    // In production, use IPTools::absoluteImpactParameter3D
    GlobalPoint vertexPos(vertex.position().x(), vertex.position().y(), vertex.position().z());
    
    // Get closest approach point
    auto ca = track.trajectoryStateClosestToPoint(vertexPos);
    if (!ca.isValid()) {
        return std::make_pair(-999, -999);
    }
    
    double ip = (ca.position() - vertexPos).mag();
    double ipError = 0.01; // Simplified error
    double significance = ipError > 0 ? ip / ipError : -999;
    
    return std::make_pair(ip, significance);
}

// ===== Private Helper Functions =====

void BcDiMuMuFunctional::initializeEventSetup(const edm::EventSetup& iSetup) {
    theB = &iSetup.getData(theBToken_);
    bField = &iSetup.getData(bFieldToken_);
}

pat::CompositeCandidate BcDiMuMuFunctional::createCompositeBc(const TrimuonCandidate& trimuon) {
    pat::CompositeCandidate composite = trimuon.candidate;
    
    // The composite candidate is already properly set up in fitTrimuonVertex
    // Additional setup can be done here if needed
    
    return composite;
}

void BcDiMuMuFunctional::addAnalysisVariables(pat::CompositeCandidate& candidate, 
                                              const BcCandidate& bcCandidate) {
    const auto& trimuon = bcCandidate.trimuon;
    const auto& neutrino = bcCandidate.neutrino;
    
    // Vertex information
    candidate.addUserFloat("vtxChi2", trimuon.vtxChi2);
    candidate.addUserFloat("vtxNdof", trimuon.vtxNdof);
    candidate.addUserFloat("vtxProb", trimuon.vtxProb);
    
    // Masses
    candidate.addUserFloat("trimuonMass", trimuon.trimuonMass);
    candidate.addUserFloat("jpsiMass", trimuon.jpsiMass);
    candidate.addUserFloat("correctedMass", neutrino.correctedMass);
    
    // Neutrino inference
    candidate.addUserFloat("perpendicularMomentum", neutrino.perpendicularMomentum);
    candidate.addUserFloat("pointingAngle3D", neutrino.pointingAngle3D);
    candidate.addUserFloat("pointingAngle2D", neutrino.pointingAngle2D);
    candidate.addUserFloat("cosAlpha3D", neutrino.cosAlpha3D);
    candidate.addUserFloat("cosAlpha2D", neutrino.cosAlpha2D);
    
    // Decay lengths
    candidate.addUserFloat("decayLength3D", neutrino.decayLength3D);
    candidate.addUserFloat("decayLength2D", neutrino.decayLength2D);
    candidate.addUserFloat("decayLengthSig3D", neutrino.decayLengthSig3D);
    candidate.addUserFloat("decayLengthSig2D", neutrino.decayLengthSig2D);
    
    // Quality score
    candidate.addUserFloat("qualityScore", bcCandidate.qualityScore);
    
    // Configuration info
    if (doJpsiFlip_) {
        candidate.addUserInt("jpsiFlipMode", jpsiFlipMode_);
    }
}

bool BcDiMuMuFunctional::validateInputs(const edm::Event& iEvent) {
    // Check if all required collections are available
    edm::Handle<reco::VertexCollection> vertexHandle;
    edm::Handle<pat::CompositeCandidateCollection> dimuonHandle;
    edm::Handle<pat::MuonCollection> muonHandle;
    
    bool valid = true;
    valid &= iEvent.getByToken(vertexToken_, vertexHandle);
    valid &= iEvent.getByToken(dimuonToken_, dimuonHandle);
    valid &= iEvent.getByToken(muonToken_, muonHandle);
    
    if (valid) {
        valid &= !vertexHandle->empty();
        valid &= !dimuonHandle->empty();
        valid &= !muonHandle->empty();
    }
    
    return valid;
}

void BcDiMuMuFunctional::printDebugInfo(const BcCandidate& bcCandidate) const {
    const auto& trimuon = bcCandidate.trimuon;
    const auto& neutrino = bcCandidate.neutrino;
    
    std::cout << "=== Bc Candidate Debug Info ===" << std::endl;
    std::cout << "Trimuon mass: " << trimuon.trimuonMass << " GeV" << std::endl;
    std::cout << "J/ψ mass: " << trimuon.jpsiMass << " GeV" << std::endl;
    std::cout << "Corrected Bc mass: " << neutrino.correctedMass << " GeV" << std::endl;
    std::cout << "Vertex prob: " << trimuon.vtxProb << std::endl;
    std::cout << "Pointing angle 3D: " << neutrino.pointingAngle3D << " rad" << std::endl;
    std::cout << "Decay length sig 3D: " << neutrino.decayLengthSig3D << std::endl;
    std::cout << "Quality score: " << bcCandidate.qualityScore << std::endl;
    std::cout << "==============================" << std::endl;
}