//////////////////////////////////////////////////////////
// BDiMuMuFlatProducer
// Simple flat tree producer for B meson analysis
// Converts CMSSW collections into flat ROOT trees
//////////////////////////////////////////////////////////

#include "VertexCompositeAnalysis/VertexCompositeAnalyzer/interface/BDiMuMuFlatProducer.h"

BDiMuMuFlatProducer::BDiMuMuFlatProducer(const edm::ParameterSet& iConfig) :
    // Input tokens
    bPlusToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bPlusCollection"))),
    bZeroToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bZeroCollection"))),
    bcToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("bcCollection"))),
    vertexToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexCollection"))),
    beamSpotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotCollection"))),
    genToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleCollection"))),
    centralityToken_(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralityCollection"))),
    centralityBinToken_(consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinCollection"))),
    
    // Configuration
    isMC_(iConfig.getParameter<bool>("isMC")),
    saveBPlus_(iConfig.getParameter<bool>("saveBPlus")),
    saveBZero_(iConfig.getParameter<bool>("saveBZero")),
    saveBc_(iConfig.getParameter<bool>("saveBc"))
{
}

BDiMuMuFlatProducer::~BDiMuMuFlatProducer()
{
}

void BDiMuMuFlatProducer::beginJob()
{
    initTree();
}

void BDiMuMuFlatProducer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
}

void BDiMuMuFlatProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    resetBranches();
    fillEventInfo(iEvent);
    
    // Fill B meson collections
    if (saveBPlus_) {
        edm::Handle<pat::CompositeCandidateCollection> bPlusCollection;
        if (iEvent.getByToken(bPlusToken_, bPlusCollection)) {
            fillBCandidates(bPlusCollection, 0); // B+ type = 0
        }
    }
    
    if (saveBZero_) {
        edm::Handle<pat::CompositeCandidateCollection> bZeroCollection;
        if (iEvent.getByToken(bZeroToken_, bZeroCollection)) {
            fillBCandidates(bZeroCollection, 1); // B0 type = 1
        }
    }
    
    if (saveBc_) {
        edm::Handle<pat::CompositeCandidateCollection> bcCollection;
        if (iEvent.getByToken(bcToken_, bcCollection)) {
            fillBCandidates(bcCollection, 2); // Bc type = 2
        }
    }
    
    // Fill generator information for MC
    if (isMC_) {
        edm::Handle<reco::GenParticleCollection> genCollection;
        if (iEvent.getByToken(genToken_, genCollection)) {
            fillGenInfo(genCollection);
        }
    }
    
    flatTree->Fill();
}

void BDiMuMuFlatProducer::endJob()
{
}

void BDiMuMuFlatProducer::initTree()
{
    flatTree = fs->make<TTree>("bDiMuMuFlatTree", "B meson flat tree for analysis");
    
    // Event info branches
    flatTree->Branch("runNb", &runNb, "runNb/i");
    flatTree->Branch("eventNb", &eventNb, "eventNb/l");
    flatTree->Branch("lumiSection", &lumiSection, "lumiSection/i");
    flatTree->Branch("centrality", &centrality, "centrality/S");
    flatTree->Branch("nPV", &nPV, "nPV/S");
    flatTree->Branch("bestvx", &bestvx, "bestvx/F");
    flatTree->Branch("bestvy", &bestvy, "bestvy/F");
    flatTree->Branch("bestvz", &bestvz, "bestvz/F");
    
    // B meson branches
    flatTree->Branch("nBMeson", &nBMeson, "nBMeson/s");
    flatTree->Branch("bMesonType", bMesonType, "bMesonType[nBMeson]/S");
    flatTree->Branch("bMesonPt", bMesonPt, "bMesonPt[nBMeson]/F");
    flatTree->Branch("bMesonEta", bMesonEta, "bMesonEta[nBMeson]/F");
    flatTree->Branch("bMesonPhi", bMesonPhi, "bMesonPhi[nBMeson]/F");
    flatTree->Branch("bMesonY", bMesonY, "bMesonY[nBMeson]/F");
    flatTree->Branch("bMesonMass", bMesonMass, "bMesonMass[nBMeson]/F");
    flatTree->Branch("bMesonCharge", bMesonCharge, "bMesonCharge[nBMeson]/F");
    
    // Vertex quality branches
    flatTree->Branch("bMesonVtxChi2", bMesonVtxChi2, "bMesonVtxChi2[nBMeson]/F");
    flatTree->Branch("bMesonVtxNdof", bMesonVtxNdof, "bMesonVtxNdof[nBMeson]/F");
    flatTree->Branch("bMesonVtxProb", bMesonVtxProb, "bMesonVtxProb[nBMeson]/F");
    
    // Topological variables
    flatTree->Branch("bMesonPointingAngle2D", bMesonPointingAngle2D, "bMesonPointingAngle2D[nBMeson]/F");
    flatTree->Branch("bMesonPointingAngle3D", bMesonPointingAngle3D, "bMesonPointingAngle3D[nBMeson]/F");
    flatTree->Branch("bMesonDecayLength2D", bMesonDecayLength2D, "bMesonDecayLength2D[nBMeson]/F");
    flatTree->Branch("bMesonDecayLength3D", bMesonDecayLength3D, "bMesonDecayLength3D[nBMeson]/F");
    
    // Dimuon branches
    flatTree->Branch("dimuonPt", dimuonPt, "dimuonPt[nBMeson]/F");
    flatTree->Branch("dimuonEta", dimuonEta, "dimuonEta[nBMeson]/F");
    flatTree->Branch("dimuonPhi", dimuonPhi, "dimuonPhi[nBMeson]/F");
    flatTree->Branch("dimuonMass", dimuonMass, "dimuonMass[nBMeson]/F");
    flatTree->Branch("dimuonCharge", dimuonCharge, "dimuonCharge[nBMeson]/F");
    
    // Muon branches
    flatTree->Branch("muonPt", muonPt, "muonPt[nBMeson][2]/F");
    flatTree->Branch("muonEta", muonEta, "muonEta[nBMeson][2]/F");
    flatTree->Branch("muonPhi", muonPhi, "muonPhi[nBMeson][2]/F");
    flatTree->Branch("muonCharge", muonCharge, "muonCharge[nBMeson][2]/F");
    flatTree->Branch("muonIsGlobal", muonIsGlobal, "muonIsGlobal[nBMeson][2]/O");
    flatTree->Branch("muonIsTracker", muonIsTracker, "muonIsTracker[nBMeson][2]/O");
    flatTree->Branch("muonIsPF", muonIsPF, "muonIsPF[nBMeson][2]/O");
    
    // Track branches
    flatTree->Branch("nTracks", nTracks, "nTracks[nBMeson]/s");
    flatTree->Branch("trackPt", trackPt, "trackPt[nBMeson][10]/F");
    flatTree->Branch("trackEta", trackEta, "trackEta[nBMeson][10]/F");
    flatTree->Branch("trackPhi", trackPhi, "trackPhi[nBMeson][10]/F");
    flatTree->Branch("trackCharge", trackCharge, "trackCharge[nBMeson][10]/F");
    flatTree->Branch("trackPID", trackPID, "trackPID[nBMeson][10]/S");
    
    // K*0 branches (for B0 only)
    flatTree->Branch("kstarPt", kstarPt, "kstarPt[nBMeson]/F");
    flatTree->Branch("kstarEta", kstarEta, "kstarEta[nBMeson]/F");
    flatTree->Branch("kstarPhi", kstarPhi, "kstarPhi[nBMeson]/F");
    flatTree->Branch("kstarMass", kstarMass, "kstarMass[nBMeson]/F");
    
    // Generator branches (MC only)
    if (isMC_) {
        flatTree->Branch("nGenBMeson", &nGenBMeson, "nGenBMeson/s");
        flatTree->Branch("genBMesonType", genBMesonType, "genBMesonType[nGenBMeson]/S");
        flatTree->Branch("genBMesonPt", genBMesonPt, "genBMesonPt[nGenBMeson]/F");
        flatTree->Branch("genBMesonEta", genBMesonEta, "genBMesonEta[nGenBMeson]/F");
        flatTree->Branch("genBMesonPhi", genBMesonPhi, "genBMesonPhi[nGenBMeson]/F");
        flatTree->Branch("genBMesonMass", genBMesonMass, "genBMesonMass[nGenBMeson]/F");
        flatTree->Branch("genBMesonPdgId", genBMesonPdgId, "genBMesonPdgId[nGenBMeson]/S");
    }
}

void BDiMuMuFlatProducer::resetBranches()
{
    // Reset event info
    runNb = 0;
    eventNb = 0;
    lumiSection = 0;
    centrality = -1;
    nPV = 0;
    bestvx = bestvyError = bestvz = -999.;
    
    // Reset B meson info
    nBMeson = 0;
    for (int i = 0; i < MAXBMESON_FLAT; i++) {
        bMesonType[i] = -1;
        bMesonPt[i] = -999.;
        bMesonEta[i] = -999.;
        bMesonPhi[i] = -999.;
        bMesonY[i] = -999.;
        bMesonMass[i] = -999.;
        bMesonCharge[i] = -999.;
        bMesonVtxChi2[i] = -999.;
        bMesonVtxNdof[i] = -999.;
        bMesonVtxProb[i] = -999.;
        bMesonPointingAngle2D[i] = -999.;
        bMesonPointingAngle3D[i] = -999.;
        bMesonDecayLength2D[i] = -999.;
        bMesonDecayLength3D[i] = -999.;
        
        dimuonPt[i] = -999.;
        dimuonEta[i] = -999.;
        dimuonPhi[i] = -999.;
        dimuonMass[i] = -999.;
        dimuonCharge[i] = -999.;
        
        kstarPt[i] = -999.;
        kstarEta[i] = -999.;
        kstarPhi[i] = -999.;
        kstarMass[i] = -999.;
        
        nTracks[i] = 0;
        
        for (int j = 0; j < 2; j++) {
            muonPt[i][j] = -999.;
            muonEta[i][j] = -999.;
            muonPhi[i][j] = -999.;
            muonCharge[i][j] = -999.;
            muonIsGlobal[i][j] = false;
            muonIsTracker[i][j] = false;
            muonIsPF[i][j] = false;
        }
        
        for (int j = 0; j < MAXTRACK_FLAT; j++) {
            trackPt[i][j] = -999.;
            trackEta[i][j] = -999.;
            trackPhi[i][j] = -999.;
            trackCharge[i][j] = -999.;
            trackPID[i][j] = 0;
        }
    }
    
    // Reset generator info
    if (isMC_) {
        nGenBMeson = 0;
        for (int i = 0; i < MAXBMESON_FLAT; i++) {
            genBMesonType[i] = -1;
            genBMesonPt[i] = -999.;
            genBMesonEta[i] = -999.;
            genBMesonPhi[i] = -999.;
            genBMesonMass[i] = -999.;
            genBMesonPdgId[i] = 0;
        }
    }
}

void BDiMuMuFlatProducer::fillEventInfo(const edm::Event& iEvent)
{
    runNb = iEvent.id().run();
    eventNb = iEvent.id().event();
    lumiSection = iEvent.luminosityBlock();
    
    // Get centrality information
    edm::Handle<reco::Centrality> centralityHandle;
    if (iEvent.getByToken(centralityToken_, centralityHandle)) {
        centrality = (Short_t)(centralityHandle->EtHFtowerSum());
    }
    
    // Get primary vertex information
    edm::Handle<reco::VertexCollection> vertices;
    if (iEvent.getByToken(vertexToken_, vertices) && !vertices->empty()) {
        const reco::Vertex& primaryVertex = vertices->front();
        nPV = (Short_t)vertices->size();
        bestvx = primaryVertex.x();
        bestvy = primaryVertex.y();
        bestvz = primaryVertex.z();
    }
}

void BDiMuMuFlatProducer::fillBCandidates(const edm::Handle<pat::CompositeCandidateCollection>& candidates, int type)
{
    for (const auto& candidate : *candidates) {
        if (nBMeson >= MAXBMESON_FLAT) break;
        
        bMesonType[nBMeson] = type;
        bMesonPt[nBMeson] = candidate.pt();
        bMesonEta[nBMeson] = candidate.eta();
        bMesonPhi[nBMeson] = candidate.phi();
        bMesonY[nBMeson] = candidate.rapidity();
        bMesonMass[nBMeson] = candidate.mass();
        bMesonCharge[nBMeson] = candidate.charge();
        
        // Get vertex quality from userData
        if (candidate.hasUserFloat("vtxChi2")) {
            bMesonVtxChi2[nBMeson] = candidate.userFloat("vtxChi2");
        }
        if (candidate.hasUserFloat("vtxNdof")) {
            bMesonVtxNdof[nBMeson] = candidate.userFloat("vtxNdof");
        }
        if (candidate.hasUserFloat("vtxProb")) {
            bMesonVtxProb[nBMeson] = candidate.userFloat("vtxProb");
        }
        
        // Get topological variables
        if (candidate.hasUserFloat("pointingAngle2D")) {
            bMesonPointingAngle2D[nBMeson] = candidate.userFloat("pointingAngle2D");
        }
        if (candidate.hasUserFloat("pointingAngle3D")) {
            bMesonPointingAngle3D[nBMeson] = candidate.userFloat("pointingAngle3D");
        }
        if (candidate.hasUserFloat("decayLength2D")) {
            bMesonDecayLength2D[nBMeson] = candidate.userFloat("decayLength2D");
        }
        if (candidate.hasUserFloat("decayLength3D")) {
            bMesonDecayLength3D[nBMeson] = candidate.userFloat("decayLength3D");
        }
        
        // Fill daughter information
        if (candidate.numberOfDaughters() >= 2) {
            // First daughter should be dimuon
            const reco::Candidate* dimuon = candidate.daughter(0);
            if (dimuon) {
                dimuonPt[nBMeson] = dimuon->pt();
                dimuonEta[nBMeson] = dimuon->eta();
                dimuonPhi[nBMeson] = dimuon->phi();
                dimuonMass[nBMeson] = dimuon->mass();
                dimuonCharge[nBMeson] = dimuon->charge();
                
                // Fill muon information
                if (dimuon->numberOfDaughters() >= 2) {
                    for (int iMu = 0; iMu < 2 && iMu < (int)dimuon->numberOfDaughters(); iMu++) {
                        const reco::Candidate* muon = dimuon->daughter(iMu);
                        if (muon) {
                            muonPt[nBMeson][iMu] = muon->pt();
                            muonEta[nBMeson][iMu] = muon->eta();
                            muonPhi[nBMeson][iMu] = muon->phi();
                            muonCharge[nBMeson][iMu] = muon->charge();
                            
                            // Try to cast to pat::Muon for additional info
                            const pat::Muon* patMuon = dynamic_cast<const pat::Muon*>(muon);
                            if (patMuon) {
                                muonIsGlobal[nBMeson][iMu] = patMuon->isGlobalMuon();
                                muonIsTracker[nBMeson][iMu] = patMuon->isTrackerMuon();
                                muonIsPF[nBMeson][iMu] = patMuon->isPFMuon();
                            }
                        }
                    }
                }
            }
            
            // Additional tracks (kaon/pion for B+/Bc, K*0 daughters for B0)
            int trackIdx = 0;
            nTracks[nBMeson] = candidate.numberOfDaughters() - 1; // Exclude dimuon
            
            for (size_t iDau = 1; iDau < candidate.numberOfDaughters() && trackIdx < MAXTRACK_FLAT; iDau++) {
                const reco::Candidate* track = candidate.daughter(iDau);
                if (track) {
                    trackPt[nBMeson][trackIdx] = track->pt();
                    trackEta[nBMeson][trackIdx] = track->eta();
                    trackPhi[nBMeson][trackIdx] = track->phi();
                    trackCharge[nBMeson][trackIdx] = track->charge();
                    trackPID[nBMeson][trackIdx] = track->pdgId();
                    
                    // For B0, check if this is K*0
                    if (type == 1 && trackIdx == 0 && track->numberOfDaughters() == 2) {
                        kstarPt[nBMeson] = track->pt();
                        kstarEta[nBMeson] = track->eta();
                        kstarPhi[nBMeson] = track->phi();
                        kstarMass[nBMeson] = track->mass();
                    }
                    
                    trackIdx++;
                }
            }
        }
        
        nBMeson++;
    }
}

void BDiMuMuFlatProducer::fillGenInfo(const edm::Handle<reco::GenParticleCollection>& genParticles)
{
    nGenBMeson = 0;
    
    for (const auto& genParticle : *genParticles) {
        if (nGenBMeson >= MAXBMESON_FLAT) break;
        
        int absPdgId = abs(genParticle.pdgId());
        
        // Check for B mesons (B+: 521, B0: 511, Bc: 541)
        int genType = -1;
        if (absPdgId == 521) genType = 0; // B+
        else if (absPdgId == 511) genType = 1; // B0
        else if (absPdgId == 541) genType = 2; // Bc
        
        if (genType >= 0) {
            genBMesonType[nGenBMeson] = genType;
            genBMesonPt[nGenBMeson] = genParticle.pt();
            genBMesonEta[nGenBMeson] = genParticle.eta();
            genBMesonPhi[nGenBMeson] = genParticle.phi();
            genBMesonMass[nGenBMeson] = genParticle.mass();
            genBMesonPdgId[nGenBMeson] = genParticle.pdgId();
            
            nGenBMeson++;
        }
    }
}

// Define this as a plug-in
DEFINE_FWK_MODULE(BDiMuMuFlatProducer);