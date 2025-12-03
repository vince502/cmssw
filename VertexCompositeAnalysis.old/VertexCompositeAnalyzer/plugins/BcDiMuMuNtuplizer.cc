#include "VertexCompositeAnalysis/VertexCompositeAnalyzer/plugins/BcDiMuMuNtuplizer.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "TMath.h"
#include "TLorentzVector.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace edm;

BcDiMuMuNtuplizer::BcDiMuMuNtuplizer(const edm::ParameterSet& iConfig) :
    // Input tokens
    bcToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bcCollection"))),
    trimuonToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("trimuonCollection"))),
    vertexToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexCollection"))),
    beamSpotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotCollection"))),
    triggerToken_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResults"))),
    centralityToken_(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralityCollection"))),
    centralityBinToken_(consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinCollection"))),
    genToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleCollection"))),
    
    // Configuration parameters
    doGenMatching_(iConfig.getParameter<bool>("doGenMatching")),
    saveBcCandidates_(iConfig.getParameter<bool>("saveBcCandidates")),
    saveTrimuonInfo_(iConfig.getParameter<bool>("saveTrimuonInfo")),
    saveFullInfo_(iConfig.getParameter<bool>("saveFullInfo")),
    isMC_(iConfig.getParameter<bool>("isMC")),
    genMatchingDeltaR_(iConfig.getParameter<double>("genMatchingDeltaR")),
    
    // Initialize counters
    nEventsTotal(0),
    nEventsWithBc(0),
    nBcCandidatesTotal(0),
    nBcCandidatesSelected(0)
{
    // Get trigger names if provided
    if (iConfig.exists("triggerNames")) {
        triggerNames = iConfig.getParameter<std::vector<std::string>>("triggerNames");
    }
    
    LogInfo("BcDiMuMuNtuplizer") << "BcDiMuMuNtuplizer initialized with:"
                                << "\n  doGenMatching: " << doGenMatching_
                                << "\n  saveBcCandidates: " << saveBcCandidates_
                                << "\n  saveTrimuonInfo: " << saveTrimuonInfo_
                                << "\n  saveFullInfo: " << saveFullInfo_
                                << "\n  isMC: " << isMC_;
}

BcDiMuMuNtuplizer::~BcDiMuMuNtuplizer() {
}

void BcDiMuMuNtuplizer::beginJob() {
    // Initialize the tree
    initTree();
    
    LogInfo("BcDiMuMuNtuplizer") << "Analysis tree initialized";
}

void BcDiMuMuNtuplizer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    // Run-level initialization if needed
}

void BcDiMuMuNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    nEventsTotal++;
    
    // Reset all branches for this event
    resetBranches();
    
    // Fill event information
    fillEventInfo(iEvent, iSetup);
    
    // Fill Bc candidate information
    fillBcCandidates(iEvent, iSetup);
    
    // Fill generator information (MC only)
    if (isMC_ && doGenMatching_) {
        fillGenInfo(iEvent, iSetup);
    }
    
    // Fill the tree
    bcDiMuMuTree->Fill();
    
    // Update statistics
    if (nBc > 0) {
        nEventsWithBc++;
        nBcCandidatesTotal += nBc;
    }
    
    // Log progress
    if (nEventsTotal % 10000 == 0) {
        LogInfo("BcDiMuMuNtuplizer") << "Processed " << nEventsTotal << " events, "
                                    << "found Bc in " << nEventsWithBc << " events ("
                                    << std::fixed << std::setprecision(2)
                                    << 100.0 * double(nEventsWithBc) / double(nEventsTotal) << "%)";
    }
}

void BcDiMuMuNtuplizer::endJob() {
    
    // Print final statistics
    LogInfo("BcDiMuMuNtuplizer") << "\n"
        << "=== BcDiMuMuNtuplizer Final Statistics ===\n"
        << "Total events analyzed: " << nEventsTotal << "\n"
        << "Events with Bc candidates: " << nEventsWithBc << " ("
        << std::fixed << std::setprecision(2) 
        << 100.0 * double(nEventsWithBc) / double(nEventsTotal) << "%)\n"
        << "Total Bc candidates: " << nBcCandidatesTotal << "\n"
        << "Selected Bc candidates: " << nBcCandidatesSelected << "\n"
        << "Average Bc per event: " << std::fixed << std::setprecision(3)
        << (nEventsTotal > 0 ? double(nBcCandidatesTotal) / double(nEventsTotal) : 0.0) << "\n"
        << "Selection efficiency: " << std::fixed << std::setprecision(2)
        << (nBcCandidatesTotal > 0 ? 100.0 * double(nBcCandidatesSelected) / double(nBcCandidatesTotal) : 0.0) << "%\n"
        << "==========================================";
}

void BcDiMuMuNtuplizer::initTree() {
    
    // Get TFileService
    bcDiMuMuTree = fs->make<TTree>("bcDiMuMuTree", "Bc -> J/psi + mu + nu analysis tree");
    
    // Event information branches
    bcDiMuMuTree->Branch("runNb", &runNb, "runNb/i");
    bcDiMuMuTree->Branch("eventNb", &eventNb, "eventNb/l");
    bcDiMuMuTree->Branch("lumiSection", &lumiSection, "lumiSection/i");
    bcDiMuMuTree->Branch("centrality", &centrality, "centrality/S");
    bcDiMuMuTree->Branch("Ntrkoffline", &Ntrkoffline, "Ntrkoffline/I");
    bcDiMuMuTree->Branch("nPV", &nPV, "nPV/S");
    bcDiMuMuTree->Branch("bestvx", &bestvx, "bestvx/F");
    bcDiMuMuTree->Branch("bestvy", &bestvy, "bestvy/F");
    bcDiMuMuTree->Branch("bestvz", &bestvz, "bestvz/F");
    bcDiMuMuTree->Branch("bestvxError", &bestvxError, "bestvxError/F");
    bcDiMuMuTree->Branch("bestvyError", &bestvyError, "bestvyError/F");
    bcDiMuMuTree->Branch("bestvzError", &bestvzError, "bestvzError/F");
    
    // Bc candidate information
    bcDiMuMuTree->Branch("nBc", &nBc, "nBc/s");
    
    // Basic Bc kinematics
    bcDiMuMuTree->Branch("bcPt", bcPt, "bcPt[nBc]/F");
    bcDiMuMuTree->Branch("bcEta", bcEta, "bcEta[nBc]/F");
    bcDiMuMuTree->Branch("bcPhi", bcPhi, "bcPhi[nBc]/F");
    bcDiMuMuTree->Branch("bcY", bcY, "bcY[nBc]/F");
    bcDiMuMuTree->Branch("bcMass", bcMass, "bcMass[nBc]/F");
    bcDiMuMuTree->Branch("bcCharge", bcCharge, "bcCharge[nBc]/F");
    
    // Trimuon system
    bcDiMuMuTree->Branch("trimuonPt", trimuonPt, "trimuonPt[nBc]/F");
    bcDiMuMuTree->Branch("trimuonEta", trimuonEta, "trimuonEta[nBc]/F");
    bcDiMuMuTree->Branch("trimuonPhi", trimuonPhi, "trimuonPhi[nBc]/F");
    bcDiMuMuTree->Branch("trimuonY", trimuonY, "trimuonY[nBc]/F");
    bcDiMuMuTree->Branch("trimuonMass", trimuonMass, "trimuonMass[nBc]/F");
    bcDiMuMuTree->Branch("trimuonCharge", trimuonCharge, "trimuonCharge[nBc]/F");
    
    // J/psi information
    bcDiMuMuTree->Branch("jpsiPt", jpsiPt, "jpsiPt[nBc]/F");
    bcDiMuMuTree->Branch("jpsiEta", jpsiEta, "jpsiEta[nBc]/F");
    bcDiMuMuTree->Branch("jpsiPhi", jpsiPhi, "jpsiPhi[nBc]/F");
    bcDiMuMuTree->Branch("jpsiY", jpsiY, "jpsiY[nBc]/F");
    bcDiMuMuTree->Branch("jpsiMass", jpsiMass, "jpsiMass[nBc]/F");
    bcDiMuMuTree->Branch("jpsiCharge", jpsiCharge, "jpsiCharge[nBc]/F");
    
    // Individual muon information
    bcDiMuMuTree->Branch("muon1Pt", muon1Pt, "muon1Pt[nBc]/F");
    bcDiMuMuTree->Branch("muon1Eta", muon1Eta, "muon1Eta[nBc]/F");
    bcDiMuMuTree->Branch("muon1Phi", muon1Phi, "muon1Phi[nBc]/F");
    bcDiMuMuTree->Branch("muon1Charge", muon1Charge, "muon1Charge[nBc]/F");
    bcDiMuMuTree->Branch("muon1IsGlobal", muon1IsGlobal, "muon1IsGlobal[nBc]/O");
    bcDiMuMuTree->Branch("muon1IsTracker", muon1IsTracker, "muon1IsTracker[nBc]/O");
    bcDiMuMuTree->Branch("muon1IsPF", muon1IsPF, "muon1IsPF[nBc]/O");
    bcDiMuMuTree->Branch("muon1Dxy", muon1Dxy, "muon1Dxy[nBc]/F");
    bcDiMuMuTree->Branch("muon1Dz", muon1Dz, "muon1Dz[nBc]/F");
    bcDiMuMuTree->Branch("muon1NHits", muon1NHits, "muon1NHits[nBc]/S");
    bcDiMuMuTree->Branch("muon1NPixelHits", muon1NPixelHits, "muon1NPixelHits[nBc]/S");
    bcDiMuMuTree->Branch("muon1NStations", muon1NStations, "muon1NStations[nBc]/S");
    bcDiMuMuTree->Branch("muon1Chi2", muon1Chi2, "muon1Chi2[nBc]/F");
    
    bcDiMuMuTree->Branch("muon2Pt", muon2Pt, "muon2Pt[nBc]/F");
    bcDiMuMuTree->Branch("muon2Eta", muon2Eta, "muon2Eta[nBc]/F");
    bcDiMuMuTree->Branch("muon2Phi", muon2Phi, "muon2Phi[nBc]/F");
    bcDiMuMuTree->Branch("muon2Charge", muon2Charge, "muon2Charge[nBc]/F");
    bcDiMuMuTree->Branch("muon2IsGlobal", muon2IsGlobal, "muon2IsGlobal[nBc]/O");
    bcDiMuMuTree->Branch("muon2IsTracker", muon2IsTracker, "muon2IsTracker[nBc]/O");
    bcDiMuMuTree->Branch("muon2IsPF", muon2IsPF, "muon2IsPF[nBc]/O");
    bcDiMuMuTree->Branch("muon2Dxy", muon2Dxy, "muon2Dxy[nBc]/F");
    bcDiMuMuTree->Branch("muon2Dz", muon2Dz, "muon2Dz[nBc]/F");
    bcDiMuMuTree->Branch("muon2NHits", muon2NHits, "muon2NHits[nBc]/S");
    bcDiMuMuTree->Branch("muon2NPixelHits", muon2NPixelHits, "muon2NPixelHits[nBc]/S");
    bcDiMuMuTree->Branch("muon2NStations", muon2NStations, "muon2NStations[nBc]/S");
    bcDiMuMuTree->Branch("muon2Chi2", muon2Chi2, "muon2Chi2[nBc]/F");
    
    bcDiMuMuTree->Branch("muon3Pt", muon3Pt, "muon3Pt[nBc]/F");
    bcDiMuMuTree->Branch("muon3Eta", muon3Eta, "muon3Eta[nBc]/F");
    bcDiMuMuTree->Branch("muon3Phi", muon3Phi, "muon3Phi[nBc]/F");
    bcDiMuMuTree->Branch("muon3Charge", muon3Charge, "muon3Charge[nBc]/F");
    bcDiMuMuTree->Branch("muon3IsGlobal", muon3IsGlobal, "muon3IsGlobal[nBc]/O");
    bcDiMuMuTree->Branch("muon3IsTracker", muon3IsTracker, "muon3IsTracker[nBc]/O");
    bcDiMuMuTree->Branch("muon3IsPF", muon3IsPF, "muon3IsPF[nBc]/O");
    bcDiMuMuTree->Branch("muon3Dxy", muon3Dxy, "muon3Dxy[nBc]/F");
    bcDiMuMuTree->Branch("muon3Dz", muon3Dz, "muon3Dz[nBc]/F");
    bcDiMuMuTree->Branch("muon3NHits", muon3NHits, "muon3NHits[nBc]/S");
    bcDiMuMuTree->Branch("muon3NPixelHits", muon3NPixelHits, "muon3NPixelHits[nBc]/S");
    bcDiMuMuTree->Branch("muon3NStations", muon3NStations, "muon3NStations[nBc]/S");
    bcDiMuMuTree->Branch("muon3Chi2", muon3Chi2, "muon3Chi2[nBc]/F");
    
    // Vertex information
    bcDiMuMuTree->Branch("bcVtxChi2", bcVtxChi2, "bcVtxChi2[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxNdof", bcVtxNdof, "bcVtxNdof[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxProb", bcVtxProb, "bcVtxProb[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxX", bcVtxX, "bcVtxX[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxY", bcVtxY, "bcVtxY[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxZ", bcVtxZ, "bcVtxZ[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxXError", bcVtxXError, "bcVtxXError[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxYError", bcVtxYError, "bcVtxYError[nBc]/F");
    bcDiMuMuTree->Branch("bcVtxZError", bcVtxZError, "bcVtxZError[nBc]/F");
    
    // Neutrino inference variables
    bcDiMuMuTree->Branch("neutrinoPerpMomentum", neutrinoPerpMomentum, "neutrinoPerpMomentum[nBc]/F");
    bcDiMuMuTree->Branch("neutrinoInferredMass", neutrinoInferredMass, "neutrinoInferredMass[nBc]/F");
    bcDiMuMuTree->Branch("massCorrectionFactor", massCorrectionFactor, "massCorrectionFactor[nBc]/F");
    
    // Topological variables
    bcDiMuMuTree->Branch("bcPointingAngle3D", bcPointingAngle3D, "bcPointingAngle3D[nBc]/F");
    bcDiMuMuTree->Branch("bcPointingAngle2D", bcPointingAngle2D, "bcPointingAngle2D[nBc]/F");
    bcDiMuMuTree->Branch("bcCosAlpha3D", bcCosAlpha3D, "bcCosAlpha3D[nBc]/F");
    bcDiMuMuTree->Branch("bcCosAlpha2D", bcCosAlpha2D, "bcCosAlpha2D[nBc]/F");
    bcDiMuMuTree->Branch("bcDecayLength3D", bcDecayLength3D, "bcDecayLength3D[nBc]/F");
    bcDiMuMuTree->Branch("bcDecayLength2D", bcDecayLength2D, "bcDecayLength2D[nBc]/F");
    bcDiMuMuTree->Branch("bcDecayLengthSig3D", bcDecayLengthSig3D, "bcDecayLengthSig3D[nBc]/F");
    bcDiMuMuTree->Branch("bcDecayLengthSig2D", bcDecayLengthSig2D, "bcDecayLengthSig2D[nBc]/F");
    bcDiMuMuTree->Branch("bcFlightDistance3D", bcFlightDistance3D, "bcFlightDistance3D[nBc]/F");
    bcDiMuMuTree->Branch("bcFlightDistance2D", bcFlightDistance2D, "bcFlightDistance2D[nBc]/F");
    
    // Impact parameters
    bcDiMuMuTree->Branch("bcImpactParameter3D", bcImpactParameter3D, "bcImpactParameter3D[nBc]/F");
    bcDiMuMuTree->Branch("bcImpactParameter2D", bcImpactParameter2D, "bcImpactParameter2D[nBc]/F");
    bcDiMuMuTree->Branch("bcImpactParameterSig3D", bcImpactParameterSig3D, "bcImpactParameterSig3D[nBc]/F");
    bcDiMuMuTree->Branch("bcImpactParameterSig2D", bcImpactParameterSig2D, "bcImpactParameterSig2D[nBc]/F");
    
    // Quality and selection variables
    bcDiMuMuTree->Branch("bcQualityScore", bcQualityScore, "bcQualityScore[nBc]/F");
    bcDiMuMuTree->Branch("bcPassSelection", bcPassSelection, "bcPassSelection[nBc]/O");
    bcDiMuMuTree->Branch("bcMVAScore", bcMVAScore, "bcMVAScore[nBc]/F");
    
    // Isolation variables
    if (saveFullInfo_) {
        bcDiMuMuTree->Branch("bcTrackIsolation", bcTrackIsolation, "bcTrackIsolation[nBc]/F");
        bcDiMuMuTree->Branch("bcCaloIsolation", bcCaloIsolation, "bcCaloIsolation[nBc]/F");
    }
    
    // Systematic variations
    bcDiMuMuTree->Branch("bcJpsiFlipped", bcJpsiFlipped, "bcJpsiFlipped[nBc]/O");
    bcDiMuMuTree->Branch("bcJpsiFlipMode", bcJpsiFlipMode, "bcJpsiFlipMode[nBc]/S");
    bcDiMuMuTree->Branch("bcMassOriginal", bcMassOriginal, "bcMassOriginal[nBc]/F");
    bcDiMuMuTree->Branch("bcMassFlipped", bcMassFlipped, "bcMassFlipped[nBc]/F");
    
    // Angular variables
    bcDiMuMuTree->Branch("jpsiMuonDeltaR", jpsiMuonDeltaR, "jpsiMuonDeltaR[nBc]/F");
    bcDiMuMuTree->Branch("jpsiExtraMuonDeltaR", jpsiExtraMuonDeltaR, "jpsiExtraMuonDeltaR[nBc]/F");
    bcDiMuMuTree->Branch("trimuonOpeningAngle", trimuonOpeningAngle, "trimuonOpeningAngle[nBc]/F");
    
    // Mass combinations
    bcDiMuMuTree->Branch("muon1Muon3Mass", muon1Muon3Mass, "muon1Muon3Mass[nBc]/F");
    bcDiMuMuTree->Branch("muon2Muon3Mass", muon2Muon3Mass, "muon2Muon3Mass[nBc]/F");
    bcDiMuMuTree->Branch("minDimuonMass", minDimuonMass, "minDimuonMass[nBc]/F");
    bcDiMuMuTree->Branch("maxDimuonMass", maxDimuonMass, "maxDimuonMass[nBc]/F");
    
    // Generator information (MC only)
    if (isMC_) {
        bcDiMuMuTree->Branch("nGenBc", &nGenBc, "nGenBc/s");
        bcDiMuMuTree->Branch("genBcPt", genBcPt, "genBcPt[nGenBc]/F");
        bcDiMuMuTree->Branch("genBcEta", genBcEta, "genBcEta[nGenBc]/F");
        bcDiMuMuTree->Branch("genBcPhi", genBcPhi, "genBcPhi[nGenBc]/F");
        bcDiMuMuTree->Branch("genBcY", genBcY, "genBcY[nGenBc]/F");
        bcDiMuMuTree->Branch("genBcMass", genBcMass, "genBcMass[nGenBc]/F");
        bcDiMuMuTree->Branch("genBcPdgId", genBcPdgId, "genBcPdgId[nGenBc]/S");
        bcDiMuMuTree->Branch("genBcMotherId", genBcMotherId, "genBcMotherId[nGenBc]/S");
        
        // Generator matching
        bcDiMuMuTree->Branch("bcMatchedGen", bcMatchedGen, "bcMatchedGen[nBc]/O");
        bcDiMuMuTree->Branch("bcGenDeltaR", bcGenDeltaR, "bcGenDeltaR[nBc]/F");
        bcDiMuMuTree->Branch("bcGenIndex", bcGenIndex, "bcGenIndex[nBc]/S");
        
        // Generator-level decay products
        bcDiMuMuTree->Branch("nGenMuons", &nGenMuons, "nGenMuons/s");
        bcDiMuMuTree->Branch("genMuonPt", genMuonPt, "genMuonPt[nGenMuons]/F");
        bcDiMuMuTree->Branch("genMuonEta", genMuonEta, "genMuonEta[nGenMuons]/F");
        bcDiMuMuTree->Branch("genMuonPhi", genMuonPhi, "genMuonPhi[nGenMuons]/F");
        bcDiMuMuTree->Branch("genMuonCharge", genMuonCharge, "genMuonCharge[nGenMuons]/S");
        bcDiMuMuTree->Branch("genMuonMother", genMuonMother, "genMuonMother[nGenMuons]/S");
        
        bcDiMuMuTree->Branch("nGenNeutrinos", &nGenNeutrinos, "nGenNeutrinos/s");
        bcDiMuMuTree->Branch("genNeutrinoPt", genNeutrinoPt, "genNeutrinoPt[nGenNeutrinos]/F");
        bcDiMuMuTree->Branch("genNeutrinoEta", genNeutrinoEta, "genNeutrinoEta[nGenNeutrinos]/F");
        bcDiMuMuTree->Branch("genNeutrinoPhi", genNeutrinoPhi, "genNeutrinoPhi[nGenNeutrinos]/F");
        bcDiMuMuTree->Branch("genNeutrinoE", genNeutrinoE, "genNeutrinoE[nGenNeutrinos]/F");
        bcDiMuMuTree->Branch("genNeutrinoPdgId", genNeutrinoPdgId, "genNeutrinoPdgId[nGenNeutrinos]/S");
        
        // Event-level generator information
        bcDiMuMuTree->Branch("genMissingPt", &genMissingPt, "genMissingPt/F");
        bcDiMuMuTree->Branch("genMissingPhi", &genMissingPhi, "genMissingPhi/F");
        bcDiMuMuTree->Branch("genMissingMass", &genMissingMass, "genMissingMass/F");
    }
    
    // Trigger information
    bcDiMuMuTree->Branch("triggerFired", triggerFired, "triggerFired[32]/O");
    
    // Heavy ion specific
    bcDiMuMuTree->Branch("eventPlane", &eventPlane, "eventPlane/F");
    bcDiMuMuTree->Branch("eventPlaneResolution", &eventPlaneResolution, "eventPlaneResolution/F");
}

void BcDiMuMuNtuplizer::resetBranches() {
    
    // Reset event information
    runNb = 0;
    eventNb = 0;
    lumiSection = 0;
    centrality = -1;
    Ntrkoffline = -1;
    nPV = 0;
    bestvx = -999.;
    bestvy = -999.;
    bestvz = -999.;
    bestvxError = -999.;
    bestvyError = -999.;
    bestvzError = -999.;
    
    // Reset Bc candidate information
    nBc = 0;
    
    // Reset all arrays
    for (int i = 0; i < MAXBC; i++) {
        bcPt[i] = -999.;
        bcEta[i] = -999.;
        bcPhi[i] = -999.;
        bcY[i] = -999.;
        bcMass[i] = -999.;
        bcCharge[i] = -999.;
        
        trimuonPt[i] = -999.;
        trimuonEta[i] = -999.;
        trimuonPhi[i] = -999.;
        trimuonY[i] = -999.;
        trimuonMass[i] = -999.;
        trimuonCharge[i] = -999.;
        
        jpsiPt[i] = -999.;
        jpsiEta[i] = -999.;
        jpsiPhi[i] = -999.;
        jpsiY[i] = -999.;
        jpsiMass[i] = -999.;
        jpsiCharge[i] = -999.;
        
        // Muon arrays
        muon1Pt[i] = -999.;
        muon1Eta[i] = -999.;
        muon1Phi[i] = -999.;
        muon1Charge[i] = -999.;
        muon1IsGlobal[i] = false;
        muon1IsTracker[i] = false;
        muon1IsPF[i] = false;
        muon1Dxy[i] = -999.;
        muon1Dz[i] = -999.;
        muon1NHits[i] = -1;
        muon1NPixelHits[i] = -1;
        muon1NStations[i] = -1;
        muon1Chi2[i] = -999.;
        
        muon2Pt[i] = -999.;
        muon2Eta[i] = -999.;
        muon2Phi[i] = -999.;
        muon2Charge[i] = -999.;
        muon2IsGlobal[i] = false;
        muon2IsTracker[i] = false;
        muon2IsPF[i] = false;
        muon2Dxy[i] = -999.;
        muon2Dz[i] = -999.;
        muon2NHits[i] = -1;
        muon2NPixelHits[i] = -1;
        muon2NStations[i] = -1;
        muon2Chi2[i] = -999.;
        
        muon3Pt[i] = -999.;
        muon3Eta[i] = -999.;
        muon3Phi[i] = -999.;
        muon3Charge[i] = -999.;
        muon3IsGlobal[i] = false;
        muon3IsTracker[i] = false;
        muon3IsPF[i] = false;
        muon3Dxy[i] = -999.;
        muon3Dz[i] = -999.;
        muon3NHits[i] = -1;
        muon3NPixelHits[i] = -1;
        muon3NStations[i] = -1;
        muon3Chi2[i] = -999.;
        
        // Vertex information
        bcVtxChi2[i] = -999.;
        bcVtxNdof[i] = -999.;
        bcVtxProb[i] = -999.;
        bcVtxX[i] = -999.;
        bcVtxY[i] = -999.;
        bcVtxZ[i] = -999.;
        bcVtxXError[i] = -999.;
        bcVtxYError[i] = -999.;
        bcVtxZError[i] = -999.;
        
        // Neutrino inference
        neutrinoPerpMomentum[i] = -999.;
        neutrinoInferredMass[i] = -999.;
        massCorrectionFactor[i] = -999.;
        
        // Topological variables
        bcPointingAngle3D[i] = -999.;
        bcPointingAngle2D[i] = -999.;
        bcCosAlpha3D[i] = -999.;
        bcCosAlpha2D[i] = -999.;
        bcDecayLength3D[i] = -999.;
        bcDecayLength2D[i] = -999.;
        bcDecayLengthSig3D[i] = -999.;
        bcDecayLengthSig2D[i] = -999.;
        bcFlightDistance3D[i] = -999.;
        bcFlightDistance2D[i] = -999.;
        
        // Impact parameters
        bcImpactParameter3D[i] = -999.;
        bcImpactParameter2D[i] = -999.;
        bcImpactParameterSig3D[i] = -999.;
        bcImpactParameterSig2D[i] = -999.;
        
        // Quality variables
        bcQualityScore[i] = -999.;
        bcPassSelection[i] = false;
        bcMVAScore[i] = -999.;
        
        // Isolation
        if (saveFullInfo_) {
            bcTrackIsolation[i] = -999.;  
            bcCaloIsolation[i] = -999.;
        }
        
        // Systematic variations
        bcJpsiFlipped[i] = false;
        bcJpsiFlipMode[i] = -1;
        bcMassOriginal[i] = -999.;
        bcMassFlipped[i] = -999.;
        
        // Angular variables
        jpsiMuonDeltaR[i] = -999.;
        jpsiExtraMuonDeltaR[i] = -999.;
        trimuonOpeningAngle[i] = -999.;
        
        // Mass combinations
        muon1Muon3Mass[i] = -999.;
        muon2Muon3Mass[i] = -999.;
        minDimuonMass[i] = -999.;
        maxDimuonMass[i] = -999.;
        
        // Generator matching
        if (isMC_) {
            bcMatchedGen[i] = false;
            bcGenDeltaR[i] = -999.;
            bcGenIndex[i] = -1;
        }
    }
    
    // Reset generator arrays
    if (isMC_) {
        nGenBc = 0;
        nGenMuons = 0;
        nGenNeutrinos = 0;
        genMissingPt = -999.;
        genMissingPhi = -999.;
        genMissingMass = -999.;
        
        for (int i = 0; i < MAXGEN; i++) {
            genBcPt[i] = -999.;
            genBcEta[i] = -999.;
            genBcPhi[i] = -999.;
            genBcY[i] = -999.;
            genBcMass[i] = -999.;
            genBcPdgId[i] = 0;
            genBcMotherId[i] = 0;
            
            genMuonPt[i] = -999.;
            genMuonEta[i] = -999.;
            genMuonPhi[i] = -999.;
            genMuonCharge[i] = 0;
            genMuonMother[i] = 0;
            
            genNeutrinoPt[i] = -999.;
            genNeutrinoEta[i] = -999.;
            genNeutrinoPhi[i] = -999.;
            genNeutrinoE[i] = -999.;
            genNeutrinoPdgId[i] = 0;
        }
    }
    
    // Reset trigger array
    for (int i = 0; i < 32; i++) {
        triggerFired[i] = false;
    }
    
    // Reset heavy ion variables
    eventPlane = -999.;
    eventPlaneResolution = -999.;
}

void BcDiMuMuNtuplizer::fillEventInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    // Basic event information
    runNb = iEvent.id().run();
    eventNb = iEvent.id().event();
    lumiSection = iEvent.id().luminosityBlock();
    
    // Primary vertex information
    edm::Handle<reco::VertexCollection> vertices;
    iEvent.getByToken(vertexToken_, vertices);
    
    if (vertices.isValid() && !vertices->empty()) {
        const reco::Vertex& primaryVertex = vertices->front();
        nPV = vertices->size();
        bestvx = primaryVertex.x();
        bestvy = primaryVertex.y();
        bestvz = primaryVertex.z();
        bestvxError = primaryVertex.xError();
        bestvyError = primaryVertex.yError();
        bestvzError = primaryVertex.zError();
        
        // Count good vertices
        nPV = 0;
        for (const auto& vtx : *vertices) {
            if (!vtx.isFake() && vtx.ndof() > 4 && fabs(vtx.z()) < 24. && vtx.position().rho() < 2.) {
                nPV++;
            }
        }
    }
    
    // Centrality information (heavy ion events)
    edm::Handle<reco::Centrality> centralityHandle;
    if (iEvent.getByToken(centralityToken_, centralityHandle) && centralityHandle.isValid()) {
        // Fill centrality info - use raw HF sum as proxy for Ntrkoffline
        Ntrkoffline = static_cast<int>(centralityHandle->EtHFtowerSum());
        
        edm::Handle<int> centralityBinHandle;
        if (iEvent.getByToken(centralityBinToken_, centralityBinHandle) && centralityBinHandle.isValid()) {
            centrality = *centralityBinHandle;
        }
    }
    
    // Trigger information
    edm::Handle<edm::TriggerResults> triggerResults;
    if (iEvent.getByToken(triggerToken_, triggerResults) && triggerResults.isValid()) {
        const edm::TriggerNames& triggerNames = iEvent.triggerNames(*triggerResults);
        
        // Fill trigger bits (up to 32 triggers)
        for (size_t i = 0; i < std::min<size_t>(triggerResults->size(), 32); i++) {
            triggerFired[i] = triggerResults->accept(i);
        }
    }
}

void BcDiMuMuNtuplizer::fillBcCandidates(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    // Get Bc candidate collection
    edm::Handle<pat::CompositeCandidateCollection> bcCandidates;
    iEvent.getByToken(bcToken_, bcCandidates);
    
    if (!bcCandidates.isValid()) {
        return;
    }
    
    nBc = 0;
    
    for (const auto& bc : *bcCandidates) {
        
        if (nBc >= MAXBC) {
            LogWarning("BcDiMuMuNtuplizer") << "Too many Bc candidates (" << bcCandidates->size() 
                                           << "), truncating to " << MAXBC;
            break;
        }
        
        // Apply selection if needed
        if (!isGoodBcCandidate(bc)) {
            continue;
        }
        
        nBcCandidatesSelected++;
        
        // Basic kinematics  
        bcPt[nBc] = bc.pt();
        bcEta[nBc] = bc.eta();
        bcPhi[nBc] = bc.phi();
        bcY[nBc] = bc.rapidity();
        bcMass[nBc] = bc.mass();
        bcCharge[nBc] = bc.charge();
        
        // Extract user float information (from BcDiMuMuFunctional)
        if (bc.hasUserFloat("trimuonMass")) {
            trimuonMass[nBc] = bc.userFloat("trimuonMass");
            trimuonPt[nBc] = bc.hasUserFloat("trimuonPt") ? bc.userFloat("trimuonPt") : bc.pt();
            trimuonEta[nBc] = bc.hasUserFloat("trimuonEta") ? bc.userFloat("trimuonEta") : bc.eta();
            trimuonPhi[nBc] = bc.hasUserFloat("trimuonPhi") ? bc.userFloat("trimuonPhi") : bc.phi();
            trimuonY[nBc] = bc.hasUserFloat("trimuonY") ? bc.userFloat("trimuonY") : bc.rapidity();
            trimuonCharge[nBc] = bc.hasUserFloat("trimuonCharge") ? bc.userFloat("trimuonCharge") : bc.charge();
        }
        
        // J/psi information
        if (bc.hasUserFloat("jpsiMass")) {
            jpsiMass[nBc] = bc.userFloat("jpsiMass");
            jpsiPt[nBc] = bc.hasUserFloat("jpsiPt") ? bc.userFloat("jpsiPt") : -999.;
            jpsiEta[nBc] = bc.hasUserFloat("jpsiEta") ? bc.userFloat("jpsiEta") : -999.;
            jpsiPhi[nBc] = bc.hasUserFloat("jpsiPhi") ? bc.userFloat("jpsiPhi") : -999.;
            jpsiY[nBc] = bc.hasUserFloat("jpsiY") ? bc.userFloat("jpsiY") : -999.;
            jpsiCharge[nBc] = bc.hasUserFloat("jpsiCharge") ? bc.userFloat("jpsiCharge") : -999.;
        }
        
        // Individual muon information (if available)
        if (bc.hasUserFloat("muon1Pt")) {
            muon1Pt[nBc] = bc.userFloat("muon1Pt");
            muon1Eta[nBc] = bc.hasUserFloat("muon1Eta") ? bc.userFloat("muon1Eta") : -999.;
            muon1Phi[nBc] = bc.hasUserFloat("muon1Phi") ? bc.userFloat("muon1Phi") : -999.;
            muon1Charge[nBc] = bc.hasUserFloat("muon1Charge") ? bc.userFloat("muon1Charge") : -999.;
        }
        
        if (bc.hasUserFloat("muon2Pt")) {
            muon2Pt[nBc] = bc.userFloat("muon2Pt");
            muon2Eta[nBc] = bc.hasUserFloat("muon2Eta") ? bc.userFloat("muon2Eta") : -999.;
            muon2Phi[nBc] = bc.hasUserFloat("muon2Phi") ? bc.userFloat("muon2Phi") : -999.;
            muon2Charge[nBc] = bc.hasUserFloat("muon2Charge") ? bc.userFloat("muon2Charge") : -999.;
        }
        
        if (bc.hasUserFloat("muon3Pt")) {
            muon3Pt[nBc] = bc.userFloat("muon3Pt");
            muon3Eta[nBc] = bc.hasUserFloat("muon3Eta") ? bc.userFloat("muon3Eta") : -999.;
            muon3Phi[nBc] = bc.hasUserFloat("muon3Phi") ? bc.userFloat("muon3Phi") : -999.;
            muon3Charge[nBc] = bc.hasUserFloat("muon3Charge") ? bc.userFloat("muon3Charge") : -999.;
        }
        
        // Vertex information
        if (bc.hasUserFloat("vtxChi2")) {
            bcVtxChi2[nBc] = bc.userFloat("vtxChi2");
            bcVtxNdof[nBc] = bc.hasUserFloat("vtxNdof") ? bc.userFloat("vtxNdof") : -999.;
            bcVtxProb[nBc] = bc.hasUserFloat("vtxProb") ? bc.userFloat("vtxProb") : -999.;
        }
        
        bcVtxX[nBc] = bc.vx();
        bcVtxY[nBc] = bc.vy();
        bcVtxZ[nBc] = bc.vz();
        bcVtxXError[nBc] = bc.vertexCovariance(0,0) > 0 ? sqrt(bc.vertexCovariance(0,0)) : -999.;
        bcVtxYError[nBc] = bc.vertexCovariance(1,1) > 0 ? sqrt(bc.vertexCovariance(1,1)) : -999.;
        bcVtxZError[nBc] = bc.vertexCovariance(2,2) > 0 ? sqrt(bc.vertexCovariance(2,2)) : -999.;
        
        // Neutrino inference variables (key for Bc reconstruction)
        if (bc.hasUserFloat("neutrinoPerpMomentum")) {
            neutrinoPerpMomentum[nBc] = bc.userFloat("neutrinoPerpMomentum");
            neutrinoInferredMass[nBc] = bc.hasUserFloat("neutrinoInferredMass") ? 
                                       bc.userFloat("neutrinoInferredMass") : -999.;
            massCorrectionFactor[nBc] = bc.hasUserFloat("massCorrectionFactor") ? 
                                       bc.userFloat("massCorrectionFactor") : -999.;
        }
        
        // Topological variables
        if (bc.hasUserFloat("pointingAngle3D")) {
            bcPointingAngle3D[nBc] = bc.userFloat("pointingAngle3D");
            bcCosAlpha3D[nBc] = cos(bcPointingAngle3D[nBc]);
        }
        if (bc.hasUserFloat("pointingAngle2D")) {
            bcPointingAngle2D[nBc] = bc.userFloat("pointingAngle2D");
            bcCosAlpha2D[nBc] = cos(bcPointingAngle2D[nBc]);
        }
        
        if (bc.hasUserFloat("decayLength3D")) {
            bcDecayLength3D[nBc] = bc.userFloat("decayLength3D");
            bcDecayLengthSig3D[nBc] = bc.hasUserFloat("decayLengthSig3D") ? 
                                     bc.userFloat("decayLengthSig3D") : -999.;
        }
        if (bc.hasUserFloat("decayLength2D")) {
            bcDecayLength2D[nBc] = bc.userFloat("decayLength2D");
            bcDecayLengthSig2D[nBc] = bc.hasUserFloat("decayLengthSig2D") ? 
                                     bc.userFloat("decayLengthSig2D") : -999.;
        }
        
        if (bc.hasUserFloat("flightDistance3D")) {
            bcFlightDistance3D[nBc] = bc.userFloat("flightDistance3D");
        }
        if (bc.hasUserFloat("flightDistance2D")) {
            bcFlightDistance2D[nBc] = bc.userFloat("flightDistance2D");
        }
        
        // Impact parameters
        if (bc.hasUserFloat("impactParameter3D")) {
            bcImpactParameter3D[nBc] = bc.userFloat("impactParameter3D");
            bcImpactParameterSig3D[nBc] = bc.hasUserFloat("impactParameterSig3D") ? 
                                         bc.userFloat("impactParameterSig3D") : -999.;
        }
        if (bc.hasUserFloat("impactParameter2D")) {
            bcImpactParameter2D[nBc] = bc.userFloat("impactParameter2D");
            bcImpactParameterSig2D[nBc] = bc.hasUserFloat("impactParameterSig2D") ? 
                                         bc.userFloat("impactParameterSig2D") : -999.;
        }
        
        // Quality variables
        if (bc.hasUserFloat("qualityScore")) {
            bcQualityScore[nBc] = bc.userFloat("qualityScore");
        }
        if (bc.hasUserInt("passSelection")) {
            bcPassSelection[nBc] = bc.userInt("passSelection");
        }
        if (bc.hasUserFloat("mvaScore")) {
            bcMVAScore[nBc] = bc.userFloat("mvaScore");
        }
        
        // Isolation (if requested and available)
        if (saveFullInfo_) {
            auto isolation = calculateIsolation(bc, iEvent);
            bcTrackIsolation[nBc] = isolation.first;
            bcCaloIsolation[nBc] = isolation.second;
        }
        
        // Systematic variations
        if (bc.hasUserInt("jpsiFlipped")) {
            bcJpsiFlipped[nBc] = bc.userInt("jpsiFlipped");
            bcJpsiFlipMode[nBc] = bc.hasUserInt("jpsiFlipMode") ? bc.userInt("jpsiFlipMode") : -1;
        }
        if (bc.hasUserFloat("massOriginal")) {
            bcMassOriginal[nBc] = bc.userFloat("massOriginal");
        }
        if (bc.hasUserFloat("massFlipped")) {
            bcMassFlipped[nBc] = bc.userFloat("massFlipped");
        }
        
        // Angular variables
        if (bc.hasUserFloat("jpsiMuonDeltaR")) {
            jpsiMuonDeltaR[nBc] = bc.userFloat("jpsiMuonDeltaR");
        }
        if (bc.hasUserFloat("jpsiExtraMuonDeltaR")) {
            jpsiExtraMuonDeltaR[nBc] = bc.userFloat("jpsiExtraMuonDeltaR");
        }
        if (bc.hasUserFloat("trimuonOpeningAngle")) {
            trimuonOpeningAngle[nBc] = bc.userFloat("trimuonOpeningAngle");
        }
        
        // Mass combinations
        if (bc.hasUserFloat("muon1Muon3Mass")) {
            muon1Muon3Mass[nBc] = bc.userFloat("muon1Muon3Mass");
        }
        if (bc.hasUserFloat("muon2Muon3Mass")) {
            muon2Muon3Mass[nBc] = bc.userFloat("muon2Muon3Mass");
        }
        if (bc.hasUserFloat("minDimuonMass")) {
            minDimuonMass[nBc] = bc.userFloat("minDimuonMass");
            maxDimuonMass[nBc] = bc.hasUserFloat("maxDimuonMass") ? 
                                 bc.userFloat("maxDimuonMass") : -999.;
        }
        
        nBc++;
    }
}

void BcDiMuMuNtuplizer::fillGenInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    
    if (!isMC_) return;
    
    edm::Handle<reco::GenParticleCollection> genParticles;
    iEvent.getByToken(genToken_, genParticles);
    
    if (!genParticles.isValid()) {
        return;
    }
    
    nGenBc = 0;
    nGenMuons = 0;
    nGenNeutrinos = 0;
    
    TLorentzVector totalNeutrino(0, 0, 0, 0);
    
    // Loop through generator particles
    for (const auto& genPart : *genParticles) {
        
        // Look for Bc mesons (PDG ID = ±541)
        if (abs(genPart.pdgId()) == 541 && nGenBc < MAXGEN) {
            genBcPt[nGenBc] = genPart.pt();
            genBcEta[nGenBc] = genPart.eta();
            genBcPhi[nGenBc] = genPart.phi();
            genBcY[nGenBc] = genPart.rapidity();
            genBcMass[nGenBc] = genPart.mass();
            genBcPdgId[nGenBc] = genPart.pdgId();
            genBcMotherId[nGenBc] = genPart.numberOfMothers() > 0 ? genPart.mother(0)->pdgId() : 0;
            nGenBc++;
        }
        
        // Look for muons from Bc decay
        if (abs(genPart.pdgId()) == 13 && nGenMuons < MAXGEN) {
            // Check if it comes from Bc decay chain
            bool fromBc = false;
            const reco::GenParticle* mother = &genPart;
            while (mother && mother->numberOfMothers() > 0) {
                mother = dynamic_cast<const reco::GenParticle*>(mother->mother(0));
                if (mother && abs(mother->pdgId()) == 541) {
                    fromBc = true;
                    break;
                }
                if (mother && abs(mother->pdgId()) == 443) { // J/psi
                    // Check if J/psi comes from Bc
                    const reco::GenParticle* jpsiMother = mother;
                    while (jpsiMother && jpsiMother->numberOfMothers() > 0) {
                        jpsiMother = dynamic_cast<const reco::GenParticle*>(jpsiMother->mother(0));
                        if (jpsiMother && abs(jpsiMother->pdgId()) == 541) {
                            fromBc = true;
                            break;
                        }
                    }
                    break;
                }
            }
            
            if (fromBc) {
                genMuonPt[nGenMuons] = genPart.pt();
                genMuonEta[nGenMuons] = genPart.eta();
                genMuonPhi[nGenMuons] = genPart.phi();
                genMuonCharge[nGenMuons] = genPart.charge();
                genMuonMother[nGenMuons] = genPart.numberOfMothers() > 0 ? genPart.mother(0)->pdgId() : 0;
                nGenMuons++;
            }
        }
        
        // Look for neutrinos from Bc decay
        if ((abs(genPart.pdgId()) == 14 || abs(genPart.pdgId()) == 16) && nGenNeutrinos < MAXGEN) {
            // Check if it comes from Bc decay
            bool fromBc = false;
            const reco::GenParticle* mother = &genPart;
            while (mother && mother->numberOfMothers() > 0) {
                mother = dynamic_cast<const reco::GenParticle*>(mother->mother(0));
                if (mother && abs(mother->pdgId()) == 541) {
                    fromBc = true;
                    break;
                }
            }
            
            if (fromBc) {
                genNeutrinoPt[nGenNeutrinos] = genPart.pt();
                genNeutrinoEta[nGenNeutrinos] = genPart.eta();
                genNeutrinoPhi[nGenNeutrinos] = genPart.phi();
                genNeutrinoE[nGenNeutrinos] = genPart.energy();
                genNeutrinoPdgId[nGenNeutrinos] = genPart.pdgId();
                
                // Add to total missing energy
                TLorentzVector neutrinoP4;
                neutrinoP4.SetPtEtaPhiE(genPart.pt(), genPart.eta(), genPart.phi(), genPart.energy());
                totalNeutrino += neutrinoP4;
                
                nGenNeutrinos++;
            }
        }
    }
    
    // Calculate event-level missing quantities
    genMissingPt = totalNeutrino.Pt();
    genMissingPhi = totalNeutrino.Phi();
    genMissingMass = totalNeutrino.M();
    
    // Perform generator matching for reconstructed Bc candidates
    if (doGenMatching_ && nBc > 0 && nGenBc > 0) {
        for (int i = 0; i < nBc; i++) {
            double minDeltaR = 999.;
            int bestMatch = -1;
            
            for (int j = 0; j < nGenBc; j++) {
                double deltaR = reco::deltaR(bcEta[i], bcPhi[i], genBcEta[j], genBcPhi[j]);
                if (deltaR < minDeltaR) {
                    minDeltaR = deltaR;
                    bestMatch = j;
                }
            }
            
            if (minDeltaR < genMatchingDeltaR_) {
                bcMatchedGen[i] = true;
                bcGenDeltaR[i] = minDeltaR;
                bcGenIndex[i] = bestMatch;
            }
        }
    }
}

bool BcDiMuMuNtuplizer::isGoodBcCandidate(const pat::CompositeCandidate& bc) {
    
    // Basic quality checks
    if (bc.pt() < 5.0) return false;  // Minimum pT cut
    if (std::abs(bc.eta()) > 2.4) return false;  // Acceptance cut
    
    // Vertex quality
    if (bc.hasUserFloat("vtxProb")) {
        if (bc.userFloat("vtxProb") < 0.01) return false;  // Minimum vertex probability
    }
    
    // Mass window (if available)
    if (bc.hasUserFloat("trimuonMass")) {
        double trimuonMass = bc.userFloat("trimuonMass");
        if (trimuonMass < 1.0 || trimuonMass > 15.0) return false;  // Reasonable mass range
    }
    
    return true;
}

double BcDiMuMuNtuplizer::calculateInvariantMass(const std::vector<TLorentzVector>& particles) {
    
    if (particles.empty()) return -999.;
    
    TLorentzVector total;
    for (const auto& p : particles) {
        total += p;
    }
    return total.M();
}

std::pair<double, double> BcDiMuMuNtuplizer::calculateIsolation(const pat::CompositeCandidate& bc, 
                                                                const edm::Event& iEvent) {
    
    // Placeholder implementation - would need access to tracks and calo towers
    // This is a complex calculation that would require additional input collections
    
    double trackIso = -999.0;
    double caloIso = -999.0;
    
    // TODO: Implement actual isolation calculation
    // This would typically involve:
    // 1. Loop over tracks in a cone around the Bc candidate
    // 2. Sum track pT (excluding Bc decay products)
    // 3. Loop over calorimeter towers in a cone
    // 4. Sum calorimeter energy (excluding Bc-related deposits)
    
    return std::make_pair(trackIso, caloIso);
}

// Define this as a plug-in
DEFINE_FWK_MODULE(BcDiMuMuNtuplizer);