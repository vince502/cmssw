#include "VertexCompositeAnalysis/VertexCompositeAnalyzer/plugins/BDiMuMuNtuplizer.h"

using namespace std;
using namespace reco;
using namespace edm;

BDiMuMuNtuplizer::BDiMuMuNtuplizer(const edm::ParameterSet& iConfig) {
  // Input collections
  bPlusToken_ = consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bPlusCollection"));
  bZeroToken_ = consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bZeroCollection"));
  bcToken_ = consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bcCollection"));
  
  if (iConfig.exists("bPlusMVACollection"))
    bPlusMVAToken_ = consumes<MVACollection>(iConfig.getParameter<edm::InputTag>("bPlusMVACollection"));
  if (iConfig.exists("bZeroMVACollection"))
    bZeroMVAToken_ = consumes<MVACollection>(iConfig.getParameter<edm::InputTag>("bZeroMVACollection"));
  if (iConfig.exists("bcMVACollection"))
    bcMVAToken_ = consumes<MVACollection>(iConfig.getParameter<edm::InputTag>("bcMVACollection"));
    
  vertexToken_ = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexCollection"));
  beamSpotToken_ = consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotCollection"));
  triggerToken_ = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResults"));
  
  if (iConfig.exists("centralityCollection"))
    centralityToken_ = consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralityCollection"));
  if (iConfig.exists("centralityBinCollection"))
    centralityBinToken_ = consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinCollection"));

  // Configuration options
  doGenMatching_ = iConfig.getParameter<bool>("doGenMatching");
  saveBPlus_ = iConfig.getParameter<bool>("saveBPlus");
  saveBZero_ = iConfig.getParameter<bool>("saveBZero");
  saveBc_ = iConfig.getParameter<bool>("saveBc");
  saveFullInfo_ = iConfig.getParameter<bool>("saveFullInfo");
  isMC_ = iConfig.getParameter<bool>("isMC");
  genMatchingDeltaR_ = iConfig.getParameter<double>("genMatchingDeltaR");
  
  if (doGenMatching_ || isMC_) {
    genToken_ = consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleCollection"));
  }
}

BDiMuMuNtuplizer::~BDiMuMuNtuplizer() {}

void BDiMuMuNtuplizer::beginJob() {
  initTree();
}

void BDiMuMuNtuplizer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {}

void BDiMuMuNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  resetBranches();
  fillEventInfo(iEvent, iSetup);
  fillBMesons(iEvent, iSetup);
  
  if (doGenMatching_ || isMC_) {
    fillGenInfo(iEvent, iSetup);
  }
  
  bDiMuMuTree->Fill();
}

void BDiMuMuNtuplizer::endJob() {}

void BDiMuMuNtuplizer::initTree() {
  bDiMuMuTree = fs->make<TTree>("bDiMuMuTree", "B meson analysis tree");
  
  // Event information
  bDiMuMuTree->Branch("runNb", &runNb, "runNb/i");
  bDiMuMuTree->Branch("eventNb", &eventNb, "eventNb/l");
  bDiMuMuTree->Branch("lumiSection", &lumiSection, "lumiSection/i");
  bDiMuMuTree->Branch("centrality", &centrality, "centrality/S");
  bDiMuMuTree->Branch("Ntrkoffline", &Ntrkoffline, "Ntrkoffline/I");
  bDiMuMuTree->Branch("nPV", &nPV, "nPV/S");
  bDiMuMuTree->Branch("bestvx", &bestvx, "bestvx/F");
  bDiMuMuTree->Branch("bestvy", &bestvy, "bestvy/F");
  bDiMuMuTree->Branch("bestvz", &bestvz, "bestvz/F");
  bDiMuMuTree->Branch("bestvxError", &bestvxError, "bestvxError/F");
  bDiMuMuTree->Branch("bestvyError", &bestvyError, "bestvyError/F");
  bDiMuMuTree->Branch("bestvzError", &bestvzError, "bestvzError/F");

  // B meson candidate information
  bDiMuMuTree->Branch("nBMeson", &nBMeson, "nBMeson/s");
  bDiMuMuTree->Branch("bMesonType", bMesonType, "bMesonType[nBMeson]/S");
  bDiMuMuTree->Branch("bMesonPt", bMesonPt, "bMesonPt[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonEta", bMesonEta, "bMesonEta[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonPhi", bMesonPhi, "bMesonPhi[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonY", bMesonY, "bMesonY[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonMass", bMesonMass, "bMesonMass[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonCharge", bMesonCharge, "bMesonCharge[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonMVA", bMesonMVA, "bMesonMVA[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonMatchGEN", bMesonMatchGEN, "bMesonMatchGEN[nBMeson]/O");

  // Vertex quality
  bDiMuMuTree->Branch("bMesonVtxChi2", bMesonVtxChi2, "bMesonVtxChi2[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxNdof", bMesonVtxNdof, "bMesonVtxNdof[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxProb", bMesonVtxProb, "bMesonVtxProb[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxX", bMesonVtxX, "bMesonVtxX[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxY", bMesonVtxY, "bMesonVtxY[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxZ", bMesonVtxZ, "bMesonVtxZ[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxXError", bMesonVtxXError, "bMesonVtxXError[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxYError", bMesonVtxYError, "bMesonVtxYError[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonVtxZError", bMesonVtxZError, "bMesonVtxZError[nBMeson]/F");

  // Topological variables
  bDiMuMuTree->Branch("bMesonPointingAngle2D", bMesonPointingAngle2D, "bMesonPointingAngle2D[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonPointingAngle3D", bMesonPointingAngle3D, "bMesonPointingAngle3D[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonDecayLength2D", bMesonDecayLength2D, "bMesonDecayLength2D[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonDecayLength3D", bMesonDecayLength3D, "bMesonDecayLength3D[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonDecayLengthSig2D", bMesonDecayLengthSig2D, "bMesonDecayLengthSig2D[nBMeson]/F");
  bDiMuMuTree->Branch("bMesonDecayLengthSig3D", bMesonDecayLengthSig3D, "bMesonDecayLengthSig3D[nBMeson]/F");

  // Dimuon information
  bDiMuMuTree->Branch("dimuonPt", dimuonPt, "dimuonPt[nBMeson]/F");
  bDiMuMuTree->Branch("dimuonEta", dimuonEta, "dimuonEta[nBMeson]/F");
  bDiMuMuTree->Branch("dimuonPhi", dimuonPhi, "dimuonPhi[nBMeson]/F");
  bDiMuMuTree->Branch("dimuonY", dimuonY, "dimuonY[nBMeson]/F");
  bDiMuMuTree->Branch("dimuonMass", dimuonMass, "dimuonMass[nBMeson]/F");
  bDiMuMuTree->Branch("dimuonCharge", dimuonCharge, "dimuonCharge[nBMeson]/F");

  // Individual muon information
  bDiMuMuTree->Branch("muonPt", muonPt, "muonPt[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonEta", muonEta, "muonEta[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonPhi", muonPhi, "muonPhi[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonCharge", muonCharge, "muonCharge[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonIsGlobal", muonIsGlobal, "muonIsGlobal[nBMeson][2]/O");
  bDiMuMuTree->Branch("muonIsTracker", muonIsTracker, "muonIsTracker[nBMeson][2]/O");
  bDiMuMuTree->Branch("muonIsPF", muonIsPF, "muonIsPF[nBMeson][2]/O");
  bDiMuMuTree->Branch("muonDxy", muonDxy, "muonDxy[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonDz", muonDz, "muonDz[nBMeson][2]/F");
  bDiMuMuTree->Branch("muonNHits", muonNHits, "muonNHits[nBMeson][2]/S");
  bDiMuMuTree->Branch("muonNPixelHits", muonNPixelHits, "muonNPixelHits[nBMeson][2]/S");
  bDiMuMuTree->Branch("muonNStations", muonNStations, "muonNStations[nBMeson][2]/S");

  // Additional track information
  bDiMuMuTree->Branch("nTracks", nTracks, "nTracks[nBMeson]/s");
  bDiMuMuTree->Branch("trackPt", trackPt, "trackPt[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackEta", trackEta, "trackEta[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackPhi", trackPhi, "trackPhi[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackCharge", trackCharge, "trackCharge[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackDxy", trackDxy, "trackDxy[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackDz", trackDz, "trackDz[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackPtError", trackPtError, "trackPtError[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackNHits", trackNHits, "trackNHits[nBMeson][10]/S");
  bDiMuMuTree->Branch("trackNPixelHits", trackNPixelHits, "trackNPixelHits[nBMeson][10]/S");
  bDiMuMuTree->Branch("trackChi2", trackChi2, "trackChi2[nBMeson][10]/F");
  bDiMuMuTree->Branch("trackHighPurity", trackHighPurity, "trackHighPurity[nBMeson][10]/O");
  bDiMuMuTree->Branch("trackPID", trackPID, "trackPID[nBMeson][10]/S");

  // K*0 information (for B0 only)
  bDiMuMuTree->Branch("kstarPt", kstarPt, "kstarPt[nBMeson]/F");
  bDiMuMuTree->Branch("kstarEta", kstarEta, "kstarEta[nBMeson]/F");
  bDiMuMuTree->Branch("kstarPhi", kstarPhi, "kstarPhi[nBMeson]/F");
  bDiMuMuTree->Branch("kstarMass", kstarMass, "kstarMass[nBMeson]/F");

  // Generator information (MC only)
  if (doGenMatching_ || isMC_) {
    bDiMuMuTree->Branch("nGenBMeson", &nGenBMeson, "nGenBMeson/s");
    bDiMuMuTree->Branch("genBMesonType", genBMesonType, "genBMesonType[nGenBMeson]/S");
    bDiMuMuTree->Branch("genBMesonPt", genBMesonPt, "genBMesonPt[nGenBMeson]/F");
    bDiMuMuTree->Branch("genBMesonEta", genBMesonEta, "genBMesonEta[nGenBMeson]/F");
    bDiMuMuTree->Branch("genBMesonPhi", genBMesonPhi, "genBMesonPhi[nGenBMeson]/F");
    bDiMuMuTree->Branch("genBMesonY", genBMesonY, "genBMesonY[nGenBMeson]/F");
    bDiMuMuTree->Branch("genBMesonMass", genBMesonMass, "genBMesonMass[nGenBMeson]/F");
    bDiMuMuTree->Branch("genBMesonPdgId", genBMesonPdgId, "genBMesonPdgId[nGenBMeson]/S");
  }
}

void BDiMuMuNtuplizer::resetBranches() {
  // Event info
  runNb = 0; eventNb = 0; lumiSection = 0;
  centrality = -1; Ntrkoffline = 0; nPV = 0;
  bestvx = -999; bestvy = -999; bestvz = -999;
  bestvxError = -999; bestvyError = -999; bestvzError = -999;
  
  // B meson info  
  nBMeson = 0;
  for (int i = 0; i < MAXBMESON; i++) {
    bMesonType[i] = -1;
    bMesonPt[i] = -999; bMesonEta[i] = -999; bMesonPhi[i] = -999;
    bMesonY[i] = -999; bMesonMass[i] = -999; bMesonCharge[i] = -999;
    bMesonMVA[i] = -999; bMesonMatchGEN[i] = false;
    
    bMesonVtxChi2[i] = -999; bMesonVtxNdof[i] = -999; bMesonVtxProb[i] = -999;
    bMesonVtxX[i] = -999; bMesonVtxY[i] = -999; bMesonVtxZ[i] = -999;
    bMesonVtxXError[i] = -999; bMesonVtxYError[i] = -999; bMesonVtxZError[i] = -999;
    
    bMesonPointingAngle2D[i] = -999; bMesonPointingAngle3D[i] = -999;
    bMesonDecayLength2D[i] = -999; bMesonDecayLength3D[i] = -999;
    bMesonDecayLengthSig2D[i] = -999; bMesonDecayLengthSig3D[i] = -999;
    
    dimuonPt[i] = -999; dimuonEta[i] = -999; dimuonPhi[i] = -999;
    dimuonY[i] = -999; dimuonMass[i] = -999; dimuonCharge[i] = -999;
    
    nTracks[i] = 0;
    kstarPt[i] = -999; kstarEta[i] = -999; kstarPhi[i] = -999; kstarMass[i] = -999;
    
    for (int j = 0; j < 2; j++) {
      muonPt[i][j] = -999; muonEta[i][j] = -999; muonPhi[i][j] = -999;
      muonCharge[i][j] = -999;
      muonIsGlobal[i][j] = false; muonIsTracker[i][j] = false; muonIsPF[i][j] = false;
      muonDxy[i][j] = -999; muonDz[i][j] = -999;
      muonNHits[i][j] = -999; muonNPixelHits[i][j] = -999; muonNStations[i][j] = -999;
    }
    
    for (int j = 0; j < MAXTRACK; j++) {
      trackPt[i][j] = -999; trackEta[i][j] = -999; trackPhi[i][j] = -999;
      trackCharge[i][j] = -999; trackDxy[i][j] = -999; trackDz[i][j] = -999;
      trackPtError[i][j] = -999; trackNHits[i][j] = -999; trackNPixelHits[i][j] = -999;
      trackChi2[i][j] = -999; trackHighPurity[i][j] = false; trackPID[i][j] = 0;
    }
  }
  
  // Gen info
  nGenBMeson = 0;
  for (int i = 0; i < MAXBMESON; i++) {
    genBMesonType[i] = -1; genBMesonPt[i] = -999; genBMesonEta[i] = -999;
    genBMesonPhi[i] = -999; genBMesonY[i] = -999; genBMesonMass[i] = -999;
    genBMesonPdgId[i] = 0;
  }
}

void BDiMuMuNtuplizer::fillEventInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  runNb = iEvent.id().run();
  eventNb = iEvent.id().event();
  lumiSection = iEvent.luminosityBlock();
  
  // Primary vertices
  Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken_, vertices);
  
  nPV = vertices->size();
  if (vertices->size() > 0 && !vertices->begin()->isFake()) {
    bestvx = vertices->begin()->x();
    bestvy = vertices->begin()->y();
    bestvz = vertices->begin()->z();
    bestvxError = vertices->begin()->xError();
    bestvyError = vertices->begin()->yError();
    bestvzError = vertices->begin()->zError();
  }
  
  // Centrality (for heavy ion events)
  if (!centralityToken_.isUninitialized()) {
    Handle<reco::Centrality> centralityHandle;
    if (iEvent.getByToken(centralityToken_, centralityHandle)) {
      centrality = centralityHandle->NpixelTracks();
    }
  }
  
  if (!centralityBinToken_.isUninitialized()) {
    Handle<int> centralityBinHandle;
    if (iEvent.getByToken(centralityBinToken_, centralityBinHandle)) {
      centrality = *centralityBinHandle;
    }
  }
}

void BDiMuMuNtuplizer::fillBMesons(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  nBMeson = 0;
  
  // Get generator particles for matching if needed
  Handle<reco::GenParticleCollection> genParticles;
  if (doGenMatching_ && isMC_) {
    iEvent.getByToken(genToken_, genParticles);
  }
  
  // Process B+ candidates
  if (saveBPlus_) {
    Handle<pat::CompositeCandidateCollection> bPlusHandle;
    if (iEvent.getByToken(bPlusToken_, bPlusHandle)) {
      
      Handle<MVACollection> bPlusMVAHandle;
      bool hasMVA = false;
      if (!bPlusMVAToken_.isUninitialized()) {
        hasMVA = iEvent.getByToken(bPlusMVAToken_, bPlusMVAHandle);
      }
      
      for (size_t i = 0; i < bPlusHandle->size() && nBMeson < MAXBMESON; ++i) {
        const auto& bPlus = (*bPlusHandle)[i];
        
        bMesonType[nBMeson] = 0; // B+ = 0
        bMesonPt[nBMeson] = bPlus.pt();
        bMesonEta[nBMeson] = bPlus.eta();
        bMesonPhi[nBMeson] = bPlus.phi();
        bMesonY[nBMeson] = bPlus.rapidity();
        bMesonMass[nBMeson] = bPlus.mass();
        bMesonCharge[nBMeson] = bPlus.charge();
        
        if (hasMVA && i < bPlusMVAHandle->size()) {
          bMesonMVA[nBMeson] = (*bPlusMVAHandle)[i];
        }
        
        // Vertex quality
        if (bPlus.hasUserData("vertex")) {
          const reco::Vertex* vtx = bPlus.userData<reco::Vertex>("vertex");
          if (vtx) {
            bMesonVtxX[nBMeson] = vtx->x();
            bMesonVtxY[nBMeson] = vtx->y();
            bMesonVtxZ[nBMeson] = vtx->z();
            bMesonVtxXError[nBMeson] = vtx->xError();
            bMesonVtxYError[nBMeson] = vtx->yError();
            bMesonVtxZError[nBMeson] = vtx->zError();
          }
        }
        
        if (bPlus.hasUserFloat("vtxChi2")) {
          bMesonVtxChi2[nBMeson] = bPlus.userFloat("vtxChi2");
        }
        if (bPlus.hasUserFloat("vtxNdof")) {
          bMesonVtxNdof[nBMeson] = bPlus.userFloat("vtxNdof");
        }
        if (bPlus.hasUserFloat("vtxProb")) {
          bMesonVtxProb[nBMeson] = bPlus.userFloat("vtxProb");
        }
        
        // Topological variables
        if (bPlus.hasUserFloat("pointingAngle2D")) {
          bMesonPointingAngle2D[nBMeson] = bPlus.userFloat("pointingAngle2D");
        }
        if (bPlus.hasUserFloat("pointingAngle3D")) {
          bMesonPointingAngle3D[nBMeson] = bPlus.userFloat("pointingAngle3D");
        }
        if (bPlus.hasUserFloat("decayLength2D")) {
          bMesonDecayLength2D[nBMeson] = bPlus.userFloat("decayLength2D");
        }
        if (bPlus.hasUserFloat("decayLength3D")) {
          bMesonDecayLength3D[nBMeson] = bPlus.userFloat("decayLength3D");
        }
        
        // Fill daughter information
        fillDaughterInfo(bPlus, nBMeson);
        
        // Gen matching
        if (doGenMatching_ && genParticles.isValid()) {
          bMesonMatchGEN[nBMeson] = matchToGen(bPlus, genParticles);
        }
        
        nBMeson++;
      }
    }
  }
  
  // Process B0 candidates
  if (saveBZero_) {
    Handle<pat::CompositeCandidateCollection> bZeroHandle;
    if (iEvent.getByToken(bZeroToken_, bZeroHandle)) {
      
      Handle<MVACollection> bZeroMVAHandle;
      bool hasMVA = false;
      if (!bZeroMVAToken_.isUninitialized()) {
        hasMVA = iEvent.getByToken(bZeroMVAToken_, bZeroMVAHandle);
      }
      
      for (size_t i = 0; i < bZeroHandle->size() && nBMeson < MAXBMESON; ++i) {
        const auto& bZero = (*bZeroHandle)[i];
        
        bMesonType[nBMeson] = 1; // B0 = 1
        bMesonPt[nBMeson] = bZero.pt();
        bMesonEta[nBMeson] = bZero.eta();
        bMesonPhi[nBMeson] = bZero.phi();
        bMesonY[nBMeson] = bZero.rapidity();
        bMesonMass[nBMeson] = bZero.mass();
        bMesonCharge[nBMeson] = bZero.charge();
        
        if (hasMVA && i < bZeroMVAHandle->size()) {
          bMesonMVA[nBMeson] = (*bZeroMVAHandle)[i];
        }
        
        // Vertex quality (similar to B+)
        if (bZero.hasUserData("vertex")) {
          const reco::Vertex* vtx = bZero.userData<reco::Vertex>("vertex");
          if (vtx) {
            bMesonVtxX[nBMeson] = vtx->x();
            bMesonVtxY[nBMeson] = vtx->y();
            bMesonVtxZ[nBMeson] = vtx->z();
            bMesonVtxXError[nBMeson] = vtx->xError();
            bMesonVtxYError[nBMeson] = vtx->yError();
            bMesonVtxZError[nBMeson] = vtx->zError();
          }
        }
        
        if (bZero.hasUserFloat("vtxChi2")) {
          bMesonVtxChi2[nBMeson] = bZero.userFloat("vtxChi2");
        }
        if (bZero.hasUserFloat("vtxNdof")) {
          bMesonVtxNdof[nBMeson] = bZero.userFloat("vtxNdof");
        }
        if (bZero.hasUserFloat("vtxProb")) {
          bMesonVtxProb[nBMeson] = bZero.userFloat("vtxProb");
        }
        
        // Topological variables
        if (bZero.hasUserFloat("pointingAngle2D")) {
          bMesonPointingAngle2D[nBMeson] = bZero.userFloat("pointingAngle2D");
        }
        if (bZero.hasUserFloat("pointingAngle3D")) {
          bMesonPointingAngle3D[nBMeson] = bZero.userFloat("pointingAngle3D");
        }
        if (bZero.hasUserFloat("decayLength2D")) {
          bMesonDecayLength2D[nBMeson] = bZero.userFloat("decayLength2D");
        }
        if (bZero.hasUserFloat("decayLength3D")) {
          bMesonDecayLength3D[nBMeson] = bZero.userFloat("decayLength3D");
        }
        
        // Fill daughter information
        fillDaughterInfo(bZero, nBMeson);
        
        // Fill K*0 information for B0
        if (bZero.numberOfDaughters() >= 2) {
          const auto* kstar = dynamic_cast<const pat::CompositeCandidate*>(bZero.daughter("kstar"));
          if (kstar) {
            kstarPt[nBMeson] = kstar->pt();
            kstarEta[nBMeson] = kstar->eta();
            kstarPhi[nBMeson] = kstar->phi();
            kstarMass[nBMeson] = kstar->mass();
          }
        }
        
        // Gen matching
        if (doGenMatching_ && genParticles.isValid()) {
          bMesonMatchGEN[nBMeson] = matchToGen(bZero, genParticles);
        }
        
        nBMeson++;
      }
    }
  }
  
  // Process Bc candidates
  if (saveBc_) {
    Handle<pat::CompositeCandidateCollection> bcHandle;
    if (iEvent.getByToken(bcToken_, bcHandle)) {
      
      Handle<MVACollection> bcMVAHandle;
      bool hasMVA = false;
      if (!bcMVAToken_.isUninitialized()) {
        hasMVA = iEvent.getByToken(bcMVAToken_, bcMVAHandle);
      }
      
      for (size_t i = 0; i < bcHandle->size() && nBMeson < MAXBMESON; ++i) {
        const auto& bc = (*bcHandle)[i];
        
        bMesonType[nBMeson] = 2; // Bc = 2
        bMesonPt[nBMeson] = bc.pt();
        bMesonEta[nBMeson] = bc.eta();
        bMesonPhi[nBMeson] = bc.phi();
        bMesonY[nBMeson] = bc.rapidity();
        bMesonMass[nBMeson] = bc.mass();
        bMesonCharge[nBMeson] = bc.charge();
        
        if (hasMVA && i < bcMVAHandle->size()) {
          bMesonMVA[nBMeson] = (*bcMVAHandle)[i];
        }
        
        // Vertex quality (similar to B+)
        if (bc.hasUserData("vertex")) {
          const reco::Vertex* vtx = bc.userData<reco::Vertex>("vertex");
          if (vtx) {
            bMesonVtxX[nBMeson] = vtx->x();
            bMesonVtxY[nBMeson] = vtx->y();
            bMesonVtxZ[nBMeson] = vtx->z();
            bMesonVtxXError[nBMeson] = vtx->xError();
            bMesonVtxYError[nBMeson] = vtx->yError();
            bMesonVtxZError[nBMeson] = vtx->zError();
          }
        }
        
        if (bc.hasUserFloat("vtxChi2")) {
          bMesonVtxChi2[nBMeson] = bc.userFloat("vtxChi2");
        }
        if (bc.hasUserFloat("vtxNdof")) {
          bMesonVtxNdof[nBMeson] = bc.userFloat("vtxNdof");
        }
        if (bc.hasUserFloat("vtxProb")) {
          bMesonVtxProb[nBMeson] = bc.userFloat("vtxProb");
        }
        
        // Topological variables
        if (bc.hasUserFloat("pointingAngle2D")) {
          bMesonPointingAngle2D[nBMeson] = bc.userFloat("pointingAngle2D");
        }
        if (bc.hasUserFloat("pointingAngle3D")) {
          bMesonPointingAngle3D[nBMeson] = bc.userFloat("pointingAngle3D");
        }
        if (bc.hasUserFloat("decayLength2D")) {
          bMesonDecayLength2D[nBMeson] = bc.userFloat("decayLength2D");
        }
        if (bc.hasUserFloat("decayLength3D")) {
          bMesonDecayLength3D[nBMeson] = bc.userFloat("decayLength3D");
        }
        
        // Fill daughter information
        fillDaughterInfo(bc, nBMeson);
        
        // Gen matching
        if (doGenMatching_ && genParticles.isValid()) {
          bMesonMatchGEN[nBMeson] = matchToGen(bc, genParticles);
        }
        
        nBMeson++;
      }
    }
  }
}

void BDiMuMuNtuplizer::fillDaughterInfo(const pat::CompositeCandidate& bmeson, int bIndex) {
  // Fill dimuon information
  if (bmeson.numberOfDaughters() >= 1) {
    const auto* dimuon = dynamic_cast<const pat::CompositeCandidate*>(bmeson.daughter(0));
    if (dimuon) {
      dimuonPt[bIndex] = dimuon->pt();
      dimuonEta[bIndex] = dimuon->eta();
      dimuonPhi[bIndex] = dimuon->phi();
      dimuonY[bIndex] = dimuon->rapidity();
      dimuonMass[bIndex] = dimuon->mass();
      dimuonCharge[bIndex] = dimuon->charge();
      
      // Fill individual muon information
      if (dimuon->numberOfDaughters() >= 2) {
        for (size_t i = 0; i < 2 && i < dimuon->numberOfDaughters(); ++i) {
          const auto* muon = dynamic_cast<const pat::Muon*>(dimuon->daughter(i));
          if (muon) {
            muonPt[bIndex][i] = muon->pt();
            muonEta[bIndex][i] = muon->eta();
            muonPhi[bIndex][i] = muon->phi();
            muonCharge[bIndex][i] = muon->charge();
            
            muonIsGlobal[bIndex][i] = muon->isGlobalMuon();
            muonIsTracker[bIndex][i] = muon->isTrackerMuon();
            muonIsPF[bIndex][i] = muon->isPFMuon();
            
            if (muon->track().isNonnull()) {
              muonDxy[bIndex][i] = muon->track()->dxy();
              muonDz[bIndex][i] = muon->track()->dz();
              muonDxyError[bIndex][i] = muon->track()->dxyError();
              muonDzError[bIndex][i] = muon->track()->dzError();
              muonNHits[bIndex][i] = muon->track()->numberOfValidHits();
              muonNPixelHits[bIndex][i] = muon->track()->hitPattern().numberOfValidPixelHits();
            }
            
            if (muon->isGlobalMuon() && muon->globalTrack().isNonnull()) {
              muonNStations[bIndex][i] = muon->numberOfMatchedStations();
            }
          }
        }
      }
    }
  }
  
  // Fill additional track information (depends on B meson type)
  nTracks[bIndex] = 0;
  int trackIndex = 0;
  
  // Process remaining daughters (excluding first daughter which is dimuon)
  for (size_t dauIdx = 1; dauIdx < bmeson.numberOfDaughters() && trackIndex < MAXTRACK; ++dauIdx) {
    const auto* daughter = bmeson.daughter(dauIdx);
    
    if (daughter) {
      // Check if this is a single track (B+ kaon or Bc pion)
      const auto* track_cand = dynamic_cast<const reco::RecoChargedCandidate*>(daughter);
      if (track_cand && track_cand->track().isNonnull()) {
        const auto& track = track_cand->track();
        
        trackPt[bIndex][trackIndex] = track->pt();
        trackEta[bIndex][trackIndex] = track->eta();
        trackPhi[bIndex][trackIndex] = track->phi();
        trackCharge[bIndex][trackIndex] = track->charge();
        trackDxy[bIndex][trackIndex] = track->dxy();
        trackDz[bIndex][trackIndex] = track->dz();
        trackDxyError[bIndex][trackIndex] = track->dxyError();
        trackDzError[bIndex][trackIndex] = track->dzError();
        trackPtError[bIndex][trackIndex] = track->ptError();
        trackNHits[bIndex][trackIndex] = track->numberOfValidHits();
        trackNPixelHits[bIndex][trackIndex] = track->hitPattern().numberOfValidPixelHits();
        trackChi2[bIndex][trackIndex] = track->chi2();
        trackNdof[bIndex][trackIndex] = track->ndof();
        trackHighPurity[bIndex][trackIndex] = track->quality(reco::TrackBase::highPurity);
        
        // Set PID based on B meson type
        if (bMesonType[bIndex] == 0) { // B+
          trackPID[bIndex][trackIndex] = 321; // kaon
        } else if (bMesonType[bIndex] == 2) { // Bc
          trackPID[bIndex][trackIndex] = 211; // pion
        }
        
        trackIndex++;
      }
      // Check if this is K*0 composite (B0 case)
      else {
        const auto* kstar = dynamic_cast<const pat::CompositeCandidate*>(daughter);
        if (kstar) {
          for (size_t gdauIdx = 0; gdauIdx < kstar->numberOfDaughters() && trackIndex < MAXTRACK; ++gdauIdx) {
            const auto* grandDaughter = kstar->daughter(gdauIdx);
            const auto* gtrack_cand = dynamic_cast<const reco::RecoChargedCandidate*>(grandDaughter);
            
            if (gtrack_cand && gtrack_cand->track().isNonnull()) {
              const auto& track = gtrack_cand->track();
              
              trackPt[bIndex][trackIndex] = track->pt();
              trackEta[bIndex][trackIndex] = track->eta();
              trackPhi[bIndex][trackIndex] = track->phi();
              trackCharge[bIndex][trackIndex] = track->charge();
              trackDxy[bIndex][trackIndex] = track->dxy();
              trackDz[bIndex][trackIndex] = track->dz();
              trackDxyError[bIndex][trackIndex] = track->dxyError();
              trackDzError[bIndex][trackIndex] = track->dzError();
              trackPtError[bIndex][trackIndex] = track->ptError();
              trackNHits[bIndex][trackIndex] = track->numberOfValidHits();
              trackNPixelHits[bIndex][trackIndex] = track->hitPattern().numberOfValidPixelHits();
              trackChi2[bIndex][trackIndex] = track->chi2();
              trackNdof[bIndex][trackIndex] = track->ndof();
              trackHighPurity[bIndex][trackIndex] = track->quality(reco::TrackBase::highPurity);
              
              // Set PID for K*0 daughters: positive = kaon, negative = pion
              if (track->charge() > 0) {
                trackPID[bIndex][trackIndex] = 321; // kaon
              } else {
                trackPID[bIndex][trackIndex] = 211; // pion
              }
              
              trackIndex++;
            }
          }
        }
      }
    }
  }
  
  nTracks[bIndex] = trackIndex;
}

void BDiMuMuNtuplizer::fillGenInfo(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (!isMC_) return;
  
  Handle<reco::GenParticleCollection> genParticles;
  if (!iEvent.getByToken(genToken_, genParticles)) return;
  
  nGenBMeson = 0;
  
  for (const auto& genParticle : *genParticles) {
    if (nGenBMeson >= MAXBMESON) break;
    
    int absPdgId = abs(genParticle.pdgId());
    
    // Look for B+, B0, or Bc mesons
    if (absPdgId == 521 || absPdgId == 511 || absPdgId == 541) {
      genBMesonPt[nGenBMeson] = genParticle.pt();
      genBMesonEta[nGenBMeson] = genParticle.eta();
      genBMesonPhi[nGenBMeson] = genParticle.phi();
      genBMesonY[nGenBMeson] = genParticle.rapidity();
      genBMesonMass[nGenBMeson] = genParticle.mass();
      genBMesonPdgId[nGenBMeson] = genParticle.pdgId();
      
      // Set B meson type
      if (absPdgId == 521) genBMesonType[nGenBMeson] = 0; // B+
      else if (absPdgId == 511) genBMesonType[nGenBMeson] = 1; // B0
      else if (absPdgId == 541) genBMesonType[nGenBMeson] = 2; // Bc
      
      nGenBMeson++;
    }
  }
}

bool BDiMuMuNtuplizer::matchToGen(const pat::CompositeCandidate& bmeson, 
                                  const edm::Handle<reco::GenParticleCollection>& genHandle, 
                                  double deltaR) {
  if (!genHandle.isValid()) return false;
  
  for (const auto& genParticle : *genHandle) {
    int absPdgId = abs(genParticle.pdgId());
    
    // Check if this is a B meson
    if (absPdgId == 521 || absPdgId == 511 || absPdgId == 541) {
      double dR = reco::deltaR(bmeson.eta(), bmeson.phi(), genParticle.eta(), genParticle.phi());
      if (dR < deltaR) {
        return true;
      }
    }
  }
  
  return false;
}

reco::GenParticleRef BDiMuMuNtuplizer::findLastParticle(const reco::GenParticleRef& genParticle) {
  reco::GenParticleRef lastParticle = genParticle;
  
  for (size_t i = 0; i < genParticle->numberOfDaughters(); ++i) {
    const auto* daughter = dynamic_cast<const reco::GenParticle*>(genParticle->daughter(i));
    if (daughter && daughter->pdgId() == genParticle->pdgId()) {
      reco::GenParticleRef daughterRef(genParticle.id(), daughter, genParticle.key());
      lastParticle = findLastParticle(daughterRef);
      break;
    }
  }
  
  return lastParticle;
}

reco::GenParticleRef BDiMuMuNtuplizer::findMother(const reco::GenParticleRef& genParticle) {
  if (genParticle->numberOfMothers() > 0) {
    const auto* mother = dynamic_cast<const reco::GenParticle*>(genParticle->mother(0));
    if (mother) {
      reco::GenParticleRef motherRef(genParticle.id(), mother, 0);
      return motherRef;
    }
  }
  return reco::GenParticleRef();
}

//define this as a plug-in
DEFINE_FWK_MODULE(BDiMuMuNtuplizer);