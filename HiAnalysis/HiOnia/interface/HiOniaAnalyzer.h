#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

#include <TTree.h>
#include <TLorentzVector.h>
#include <TVector2.h>
#include <TClonesArray.h>
#include <TRegexp.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include <DataFormats/RecoCandidate/interface/RecoChargedCandidate.h>
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "HiAnalysis/HiOnia/interface/MyCommonHistoManager.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//

class HiOniaAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> {
public:
  explicit HiOniaAnalyzer(const edm::ParameterSet&);
  ~HiOniaAnalyzer();
  
  
private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  
  
  void InitEvent();
  void InitTree();

  void makeCuts(bool keepSameSign) ;
  void makeBcCuts(bool keepWrongSign) ;
  void makeDimutrkCuts(bool keepWrongSign) ;
  bool checkCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1,  const pat::Muon* muon2, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*)); 
  bool checkBcCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1,  const pat::Muon* muon2, const pat::Muon* muon3, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc3)(const pat::Muon*));
  bool checkDimuTrkCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1, const pat::Muon* muon2, const reco::RecoChargedCandidate* trk, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc3)(const reco::TrackRef));

  reco::GenParticleRef findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG);
  int IndexOfThisMuon(TLorentzVector* v1, bool isGen=false);
  int IndexOfThisTrack(TLorentzVector* v1, bool isGen=false);
  int IndexOfThisJpsi(int mu1_idx, int mu2_idx, int flipJpsi=0);
  int IndexOfThisJpsi(TLorentzVector* v1);
  int IndexOfThisDiOnia(int mu1_idx, int mu2_idx);
  void fillGenInfo();
  void fillMuMatchingInfo();
  void fillQQMatchingInfo();
  void fillBcMatchingInfo();
  bool isAbHadron(int pdgID);
  bool isNeutrino(int pdgID);
  bool isAMixedbHadron(int pdgID, int momPdgID);
  std::pair<bool, reco::GenParticleRef> findBcMotherRef(reco::GenParticleRef GenParticleMother,int GenParticlePDG);
  bool isChargedTrack(int pdgId);
  std::vector<reco::GenParticleRef> GenBrothers(reco::GenParticleRef GenParticleMother, int GenJpsiPDG);
  reco::GenParticleRef findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG);
  std::pair<std::vector<reco::GenParticleRef> , std::pair<float, float> >  findGenMCInfo(const reco::GenParticle *genJpsi);
  std::pair<int, std::pair<float, float> >  findGenBcInfo(reco::GenParticleRef genBc, const reco::GenParticle *genJpsi);

  void fillRecoMuons(int theCentralityBin);
  bool isMuonInAccept(const pat::Muon* aMuon, std::string muonType);
  bool isTrkInMuonAccept(TLorentzVector trk4mom, std::string muonType);

  bool isSoftMuonBase(const pat::Muon* aMuon);
  bool isHybridSoftMuon(const pat::Muon* aMuon);
  Short_t MuInSV(TLorentzVector v1, TLorentzVector v2, TLorentzVector v3);

  void fillRecoTracks();

  pair< unsigned int, const pat::CompositeCandidate* > theBestQQ();
  double CorrectMass(const reco::Muon& mu1,const reco::Muon& mu2, int mode);

  bool selGlobalMuon(const pat::Muon* aMuon);
  bool selTrackerMuon(const pat::Muon* aMuon);
  bool selGlobalOrTrackerMuon(const pat::Muon* aMuon);
  bool selAllMuon(const pat::Muon* aMuon);
  bool selTrk(const reco::TrackRef aTrk);
  bool PassMiniAODcut(const pat::Muon* aMuon);

  void fillRecoHistos();
  void fillRecoJpsi(int count, std::string trigName, std::string centName);
  void fillHistosAndDS(unsigned int theCat, const pat::CompositeCandidate* aJpsiCand);

  void fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits);
  void fillTreeJpsi(int count);
  void fillTreeDiOnia(int count);
  void fillTreeBc(int count);
  void fillTreeDimuTrk(int count);

  void checkTriggers(const pat::CompositeCandidate* aJpsiCand);
  void hltReport(const edm::Event &iEvent ,const edm::EventSetup& iSetup);

  long int FloatToIntkey(float v);
  void beginRun(const edm::Run &, const edm::EventSetup &); 
  void endRun(const edm::Run &, const edm::EventSetup &) {};

  TLorentzVector lorentzMomentum(const reco::Candidate::LorentzVector& p);
  int muonIDmask(const pat::Muon* muon);

  // ----------member data ---------------------------
  enum StatBins {
    BIN_nEvents = 0
  };

  enum dimuonCategories {
    GlbTrk_GlbTrk     = 0,
    Glb_Glb           = 1,
    Trk_Trk           = 2,
    GlbOrTrk_GlbOrTrk = 3,
    TwoGlbAmongThree  = 4,
    All_All           = 5
  };

  enum muonCategories {
    GlbTrk        = 0,
    Trk           = 1,
    Glb           = 2,
    GlbOrTrk      = 3,
    All           = 4
  };

  std::vector<std::string> theRegions;
  std::vector<std::string> theCentralities;
  std::vector<std::string> theTriggerNames;
  std::vector<std::string> theSign;
  std::map<std::string, std::string> triggerNameMap;
  std::map<std::string, std::string> filterNameMap;

  HLTConfigProvider hltConfig;
  bool hltConfigInit;

  float etaMin;
  float etaMax;

  // TFileService
  edm::Service<TFileService> fs;

  // // TFile
  // TFile* fOut;

  // TTree
  TTree* myTree;

  TClonesArray* Reco_mu_4mom;
  TClonesArray* Reco_mu_L1_4mom;
  TClonesArray* Reco_QQ_4mom;
  TClonesArray* Reco_QQ_mumi_4mom;
  TClonesArray* Reco_QQ_mupl_4mom;
  TClonesArray* Reco_3mu_4mom;
  TClonesArray* Reco_QQ_vtx;
  TClonesArray* Reco_3mu_vtx;
  TClonesArray* Reco_trk_4mom;
  TClonesArray* Reco_trk_vtx;
  TClonesArray* Gen_Bc_4mom;
  TClonesArray* Gen_Bc_nuW_4mom;
  TClonesArray* Gen_3mu_4mom;

  TClonesArray* Gen_mu_4mom;
  TClonesArray* Gen_QQ_4mom;

  std::vector<float> Reco_mu_4mom_pt;
  std::vector<float> Reco_mu_L1_4mom_pt;
  std::vector<float> Reco_QQ_4mom_pt;
  std::vector<float> Reco_QQ_mumi_4mom_pt;
  std::vector<float> Reco_QQ_mupl_4mom_pt;
  std::vector<float> Reco_3mu_4mom_pt;
  std::vector<float> Reco_trk_4mom_pt;
  std::vector<float> Gen_Bc_4mom_pt;
  std::vector<float> Gen_Bc_nuW_4mom_pt;
  std::vector<float> Gen_3mu_4mom_pt;
  std::vector<float> Reco_QQQQ_4mom_pt;

  std::vector<float> Gen_mu_4mom_pt;
  std::vector<float> Gen_QQ_4mom_pt;

  std::vector<float> Reco_mu_4mom_eta;
  std::vector<float> Reco_mu_L1_4mom_eta;
  std::vector<float> Reco_QQ_4mom_eta;
  std::vector<float> Reco_QQ_mumi_4mom_eta;
  std::vector<float> Reco_QQ_mupl_4mom_eta;
  std::vector<float> Reco_3mu_4mom_eta;
  std::vector<float> Reco_trk_4mom_eta;
  std::vector<float> Gen_Bc_4mom_eta;
  std::vector<float> Gen_Bc_nuW_4mom_eta;
  std::vector<float> Gen_3mu_4mom_eta;
  std::vector<float> Reco_QQQQ_4mom_eta;

  std::vector<float> Gen_mu_4mom_eta;
  std::vector<float> Gen_QQ_4mom_eta;

  std::vector<float> Reco_mu_4mom_phi;
  std::vector<float> Reco_mu_L1_4mom_phi;
  std::vector<float> Reco_QQ_4mom_phi;
  std::vector<float> Reco_QQ_mumi_4mom_phi;
  std::vector<float> Reco_QQ_mupl_4mom_phi;
  std::vector<float> Reco_3mu_4mom_phi;
  std::vector<float> Reco_trk_4mom_phi;
  std::vector<float> Gen_Bc_4mom_phi;
  std::vector<float> Gen_Bc_nuW_4mom_phi;
  std::vector<float> Gen_3mu_4mom_phi;
  std::vector<float> Reco_QQQQ_4mom_phi;

  std::vector<float> Gen_mu_4mom_phi;
  std::vector<float> Gen_QQ_4mom_phi;

  std::vector<float> Reco_mu_4mom_m;
  std::vector<float> Reco_mu_L1_4mom_m;
  std::vector<float> Reco_QQ_4mom_m;
  std::vector<float> Reco_QQ_mumi_4mom_m;
  std::vector<float> Reco_QQ_mupl_4mom_m;
  std::vector<float> Reco_3mu_4mom_m;
  std::vector<float> Reco_trk_4mom_m;
  std::vector<float> Gen_Bc_4mom_m;
  std::vector<float> Gen_Bc_nuW_4mom_m;
  std::vector<float> Gen_3mu_4mom_m;
  std::vector<float> Reco_QQQQ_4mom_m;

  std::vector<float> Gen_mu_4mom_m;
  std::vector<float> Gen_QQ_4mom_m;

  static const int Max_QQ_size = 10000;
  static const int Max_QQQQ_size = 100000;
  static const int Max_Bc_size = 10000;
  static const int Max_mu_size = 1000;
  static const int Max_trk_size = 10000;

  float Gen_weight; // generator weight
  float Gen_pthat; // ptHat scale of generated hard scattering

  Short_t Gen_QQ_size; // number of generated Onia
  Short_t Gen_QQ_type[Max_QQ_size]; // Onia type: prompt, non-prompt, unmatched
  float Gen_QQ_ctau[Max_QQ_size];    // ctau: flight time
  float Gen_QQ_ctau3D[Max_QQ_size];    // ctau3D: 3D flight time
  int Gen_QQ_momId[Max_QQ_size]; // PDG ID of the generated mother of the Gen QQ, going back far enough in the geneaology to find a potential B mother
  float Gen_QQ_momPt[Max_QQ_size];    // Pt of mother particle of 2 muons
  Short_t Gen_QQ_mupl_idx[Max_QQ_size];    // index of the muon plus from Jpsi, in the full list of muons
  Short_t Gen_QQ_mumi_idx[Max_QQ_size];    // index of the muon minus from Jpsi, in the full list of muons
  Short_t Gen_QQ_whichRec[Max_QQ_size]; // index of the reconstructed Jpsi that was matched with this gen Jpsi. Is -1 if one of the 2 muons from Jpsi was not reconstructed. Is -2 if the two muons were reconstructed, but the dimuon was not selected
  Short_t Gen_QQ_Bc_idx[Max_QQ_size]; //Index of the Bc gen mother. -1 if there is no Bc mother

  Short_t Gen_Bc_size; // number of generated Bc
  float Gen_Bc_ctau[Max_Bc_size];    // ctau: flight time
  int Gen_Bc_pdgId[Max_Bc_size];    // pdgId of Bc
  Short_t Gen_Bc_QQ_idx[Max_Bc_size]; //Points to the number of the associated Jpsi daughter 
  Short_t Gen_Bc_muW_idx[Max_Bc_size];    // index of the muon from W from Bc, in the full list of muons
  Short_t Gen_3mu_whichRec[Max_Bc_size]; // index of the reconstructed trimuon that was matched with this gen visible Bc. Is -1 if one muon of the Bc was not reconstructed
 
  Short_t Gen_mu_size; // number of generated muons
  Short_t Gen_mu_charge[Max_mu_size]; // muon charge
  Short_t Gen_mu_type[Max_mu_size]; // muon type: prompt, non-prompt, unmatched
  Short_t Gen_mu_whichRec[Max_mu_size]; // index of the reconstructed muon that was matched with this gen muon. Is -1 if the muon was not reconstructed
  float Gen_mu_MatchDeltaR[Max_mu_size]; // deltaR between reco and gen matched muons

  Short_t Reco_3mu_size;       // Number of reconstructed trimuons
  Short_t Reco_3mu_charge[Max_Bc_size];
  Short_t Reco_3mu_mupl_idx[Max_Bc_size];    // index of the muon plus from Jpsi, in the full list of muons
  Short_t Reco_3mu_mumi_idx[Max_Bc_size];    // index of the muon minus from Jpsi, in the full list of muons
  Short_t Reco_3mu_muW_idx[Max_Bc_size];    // index of the muon from W, in the full list of muons
  Short_t Reco_3mu_muW2_idx[Max_Bc_size];    // index of the muon from W, for the second possible OS dimuon
  //  Short_t Reco_3mu_muW_trkIdx[Max_Bc_size];    // index of the generated track closest to the reco muon from W, when the Jpsi is true and the muon is fake
  Short_t Reco_3mu_QQ1_idx[Max_Bc_size];    // index of a possible Jpsi from Bc, 1st of the two OS muon pairs
  Short_t Reco_3mu_QQ2_idx[Max_Bc_size];    // index of a possible Jpsi from Bc, 2nd of the two OS muon pairs
  Short_t Reco_3mu_QQss_idx[Max_Bc_size];    // index of a the same-sign muon pair from Bc
  Short_t Reco_3mu_whichGen[Max_Bc_size]; // index of the generated Bc that was matched with this rec Bc. Is -1 if one or more of the 3 muons from Bc was not reconstructed
  bool Reco_3mu_muW_isGenJpsiBro[Max_Bc_size]; // is this true or fake muon matched to a generated particle that is brother/nephew to the gen Jpsi (daughter of the gen B)
  int Reco_3mu_muW_trueId[Max_Bc_size]; //pdgId of the generated particle that the reco_muW is matched to
  float Reco_3mu_VtxProb[Max_Bc_size]; // chi2 probability of vertex fitting 
  float Reco_3mu_KCVtxProb[Max_Bc_size]; // chi2 probability of kinematic constrained vertex fitting 
  float Reco_3mu_ctau[Max_Bc_size];    // ctau: flight time
  float Reco_3mu_ctauErr[Max_Bc_size]; // error on ctau
  float Reco_3mu_cosAlpha[Max_QQ_size];    // cosine of angle between momentum of Bc and direction of PV--displaced vertex segment (in XY plane)
  float Reco_3mu_ctau3D[Max_Bc_size];    // ctau: flight time in 3D
  float Reco_3mu_ctauErr3D[Max_Bc_size]; // error on ctau in 3D
  float Reco_3mu_cosAlpha3D[Max_QQ_size];    // cosine of angle between momentum of Bc and direction of PV--displaced vertex segment (3D)
  float Reco_3mu_KCctau[Max_Bc_size];    // ctau: flight time
  float Reco_3mu_KCctauErr[Max_Bc_size]; // error on ctau
  float Reco_3mu_KCcosAlpha[Max_QQ_size];    // cosine of angle between momentum of Bc and direction of PV--displaced vertex segment (in XY plane)
  float Reco_3mu_KCctau3D[Max_Bc_size];    // ctau: flight time in 3D
  float Reco_3mu_KCctauErr3D[Max_Bc_size]; // error on ctau in 3D
  float Reco_3mu_KCcosAlpha3D[Max_QQ_size];    // cosine of angle between momentum of Bc and direction of PV--displaced vertex segment (3D)
  float Reco_3mu_MassErr[Max_Bc_size];
  float Reco_3mu_CorrM[Max_Bc_size];
  Short_t Reco_3mu_NbMuInSameSV[Max_Bc_size];
  float Reco_3mu_muW_dxy[Max_Bc_size];
  float Reco_3mu_muW_dz[Max_Bc_size];
  float Reco_3mu_mumi_dxy[Max_Bc_size];
  float Reco_3mu_mumi_dz[Max_Bc_size];
  float Reco_3mu_mupl_dxy[Max_Bc_size];
  float Reco_3mu_mupl_dz[Max_Bc_size];

  Short_t Reco_QQ_size;       // Number of reconstructed Onia 
  Short_t Reco_QQQQ_size;       // Number of reconstructed di-Onia 
  Short_t Reco_QQ_type[Max_QQ_size];   // Onia category: GG, GT, TT
  Short_t Reco_QQ_sign[Max_QQ_size];   /* Mu Mu combinations sign:
                             0 = +/- (signal)
                             1 = +/+
                             2 = -/- 
                          */
  Short_t Reco_QQ_mupl_idx[Max_QQ_size];    // index of the muon plus from Jpsi, in the full list of muons
  Short_t Reco_QQ_mumi_idx[Max_QQ_size];    // index of the muon minus from Jpsi, in the full list of muons
  Short_t Reco_QQ_whichGen[Max_QQ_size]; // index of the generated Jpsi that was matched with this rec Jpsi. Is -1 if one of the 2 muons from Jpsi was not reconstructed
  ULong64_t Reco_QQ_trig[Max_QQ_size];      // Vector of trigger bits matched to the Onia
  float Reco_QQ_VtxProb[Max_QQ_size]; // chi2 probability of vertex fitting 
  float Reco_QQ_ctau[Max_QQ_size];    // ctau: flight time
  float Reco_QQ_ctauErr[Max_QQ_size]; // error on ctau
  float Reco_QQ_cosAlpha[Max_QQ_size];    // cosine of angle between momentum of Jpsi and direction of PV--displaced vertex segment (in XY plane)
  float Reco_QQ_ctau3D[Max_QQ_size];    // ctau: flight time in 3D
  float Reco_QQ_ctauErr3D[Max_QQ_size]; // error on ctau in 3D
  float Reco_QQ_cosAlpha3D[Max_QQ_size];    // cosine of angle between momentum of Jpsi and direction of PV--displaced vertex segment (3D)
  float Reco_QQ_dca[Max_QQ_size];
  float Reco_QQ_MassErr[Max_QQ_size];

  int  Reco_QQ_NtrkPt02[Max_QQ_size];
  int  Reco_QQ_NtrkPt03[Max_QQ_size];
  int  Reco_QQ_NtrkPt04[Max_QQ_size];

  int  Reco_QQ_NtrkDeltaR03[Max_QQ_size];
  int  Reco_QQ_NtrkDeltaR04[Max_QQ_size];
  int  Reco_QQ_NtrkDeltaR05[Max_QQ_size];

  float Reco_QQ_mupl_dxy[Max_QQ_size];  // dxy for plus inner track muons
  float Reco_QQ_mumi_dxy[Max_QQ_size];  // dxy for minus inner track muons
  float Reco_QQ_mupl_dz[Max_QQ_size];  // dz for plus inner track muons
  float Reco_QQ_mumi_dz[Max_QQ_size];  // dz for minus inner track muons
  Short_t Reco_QQ_flipJpsi[Max_QQ_size];
  //Di quarkonia branch
  Short_t Reco_QQQQ_mupl_idx[Max_QQQQ_size];    // index of the muon plus from Jpsi, in the full list of muons
  Short_t Reco_QQQQ_mumi_idx[Max_QQQQ_size];    // index of the muon minus from Jpsi, in the full list of muons
  Short_t Reco_QQQQ_whichGen[Max_QQQQ_size]; // index of the generated Jpsi that was matched with this rec Jpsi. Is -1 if one of the 2 muons from Jpsi was not reconstructed
  ULong64_t Reco_QQQQ_trig[Max_QQQQ_size];      // Vector of trigger bits matched to the Onia
  float Reco_QQQQ_VtxProb[Max_QQQQ_size]; // chi2 probability of vertex fitting 
  float Reco_QQQQ_ctau[Max_QQQQ_size];    // ctau: flight time
  float Reco_QQQQ_ctauErr[Max_QQQQ_size]; // error on ctau
  float Reco_QQQQ_cosAlpha[Max_QQQQ_size];    // cosine of angle between momentum of Jpsi and direction of PV--displaced vertex segment (in XY plane)
  float Reco_QQQQ_ctau3D[Max_QQQQ_size];    // ctau: flight time in 3D
  float Reco_QQQQ_ctauErr3D[Max_QQQQ_size]; // error on ctau in 3D
  float Reco_QQQQ_cosAlpha3D[Max_QQQQ_size];    // cosine of angle between momentum of Jpsi and direction of PV--displaced vertex segment (3D)
  float Reco_QQQQ_dca[Max_QQQQ_size];
  float Reco_QQQQ_MassErr[Max_QQQQ_size];

  int  Reco_QQQQ_NtrkPt02[Max_QQQQ_size];
  int  Reco_QQQQ_NtrkPt03[Max_QQQQ_size];
  int  Reco_QQQQ_NtrkPt04[Max_QQQQ_size];

  int  Reco_QQQQ_NtrkDeltaR03[Max_QQQQ_size];
  int  Reco_QQQQ_NtrkDeltaR04[Max_QQQQ_size];
  int  Reco_QQQQ_NtrkDeltaR05[Max_QQQQ_size];

  float Reco_QQQQ_mupl_dxy[Max_QQQQ_size];  // dxy for plus inner track muons
  float Reco_QQQQ_mumi_dxy[Max_QQQQ_size];  // dxy for minus inner track muons
  float Reco_QQQQ_mupl_dz[Max_QQQQ_size];  // dz for plus inner track muons
  float Reco_QQQQ_mumi_dz[Max_QQQQ_size];  // dz for minus inner track muons
  Short_t Reco_QQQQ_flipJpsi[Max_QQQQ_size];

  Short_t Reco_mu_size;           // Number of reconstructed muons
  int Reco_mu_SelectionType[Max_mu_size];           
  ULong64_t Reco_mu_trig[Max_mu_size];      // Vector of trigger bits matched to the muon
  Short_t Reco_mu_charge[Max_mu_size];  // Vector of charge of muons
  Short_t Reco_mu_type[Max_mu_size];  // Vector of type of muon (global=0, tracker=1, calo=2)  
  Short_t Reco_mu_whichGen[Max_mu_size]; // index of the generated muon that was matched with this reco muon. Is -1 if the muon is not associated with a generated muon (fake, or very bad resolution)

  bool Reco_mu_highPurity[Max_mu_size];    // Vector of high purity flag  
  bool Reco_mu_TrkMuArb[Max_mu_size];      // Vector of TrackerMuonArbitrated
  bool Reco_mu_TMOneStaTight[Max_mu_size]; // Vector of TMOneStationTight
  Short_t Reco_mu_candType[Max_mu_size]; // candidate type of muon. 0 (or not present): muon collection, 1: packedPFCandidate, 2: lostTrack collection
  bool Reco_mu_isPF[Max_mu_size];           // Vector of isParticleFlow muon
  bool Reco_mu_isTracker[Max_mu_size];
  bool Reco_mu_isGlobal[Max_mu_size];
  bool Reco_mu_isSoftCutBased[Max_mu_size];
  bool Reco_mu_isHybridSoft[Max_mu_size];
  bool Reco_mu_isMedium[Max_mu_size];
  bool Reco_mu_isTightCutBased[Max_mu_size];
  bool Reco_mu_InTightAcc[Max_mu_size];  // Is in the tight acceptance for global muons
  bool Reco_mu_InLooseAcc[Max_mu_size];  // Is in the loose acceptance for global muons

  int Reco_mu_nPixValHits[Max_mu_size];  // Number of valid pixel hits in sta muons
  int Reco_mu_nMuValHits[Max_mu_size];  // Number of valid muon hits in sta muons
  int Reco_mu_nTrkHits[Max_mu_size];  // track hits global muons
  int Reco_mu_nPixWMea[Max_mu_size];  // pixel layers with measurement for inner track muons
  int Reco_mu_nTrkWMea[Max_mu_size];  // track layers with measurement for inner track muons
  int Reco_mu_StationsMatched[Max_mu_size];  // number of stations matched for inner track muons
  float Reco_mu_segmentComp[Max_mu_size];
  float Reco_mu_kink[Max_mu_size];
  float Reco_mu_localChi2[Max_mu_size];
  float Reco_mu_normChi2_bestTracker[Max_mu_size];
  float Reco_mu_normChi2_inner[Max_mu_size];  // chi2/ndof for inner track muons
  float Reco_mu_normChi2_global[Max_mu_size];  // chi2/ndof for global muons
  float Reco_mu_dxy[Max_mu_size];  // dxy for inner track muons
  float Reco_mu_dxyErr[Max_mu_size];  // dxy error for inner track muons
  float Reco_mu_dz[Max_mu_size];  // dz for inner track muons
  float Reco_mu_dzErr[Max_mu_size];  // dz error for inner track muons
  float Reco_mu_pt_inner[Max_mu_size];  // pT for inner track muons
  float Reco_mu_pt_global[Max_mu_size];  // pT for global muons
  float Reco_mu_ptErr_inner[Max_mu_size];  // pT error for inner track muons
  float Reco_mu_ptErr_global[Max_mu_size];  // pT error for global muons
  float Reco_mu_pTrue[Max_mu_size];  // P of the associated generated muon, used to match the Reco_mu with the Gen_mu
  float Reco_mu_validFraction[Max_mu_size];
  int Reco_mu_simExtType[Max_Bc_size]; //

  Short_t muType; // type of muon (GlbTrk=0, Trk=1, Glb=2, none=-1) 
  std::vector<float> EtaOfWantedMuons; //To know which single muons to fill, when fillSingleMuons=false and we want only the muons from selected dimuons
  std::vector<float> EtaOfWantedTracks; //To know which tracks to fill (only tracks that are part of a selected dimuon-track candidate)

  Short_t Reco_trk_size;           // Number of reconstructed tracks
  Short_t Reco_trk_charge[Max_trk_size];  // Vector of charge of tracks
  Short_t Reco_trk_whichGenmu[Max_trk_size];
  bool Reco_trk_InLooseAcc[Max_trk_size];
  bool Reco_trk_InTightAcc[Max_trk_size];
  float Reco_trk_dxyError[Max_trk_size];
  float Reco_trk_dzError[Max_trk_size];
  float Reco_trk_dxy[Max_trk_size];
  float Reco_trk_dz[Max_trk_size];
  float Reco_trk_ptErr[Max_trk_size];
  int Reco_trk_originalAlgo[Max_trk_size];
  int Reco_trk_nPixWMea[Max_trk_size];
  int Reco_trk_nTrkWMea[Max_trk_size];

  // histos
  TH1F* hGoodMuonsNoTrig = NULL;
  TH1F* hGoodMuons = NULL;
  TH1F* hL1DoubleMu0 = NULL;

  MyCommonHistoManager* myRecoMuonHistos = NULL;
  MyCommonHistoManager* myRecoGlbMuonHistos = NULL;
  MyCommonHistoManager* myRecoTrkMuonHistos = NULL;

  MyCommonHistoManager* myRecoJpsiHistos = NULL;
  MyCommonHistoManager* myRecoJpsiGlbGlbHistos = NULL;
  MyCommonHistoManager* myRecoJpsiGlbTrkHistos = NULL;
  MyCommonHistoManager* myRecoJpsiTrkTrkHistos = NULL;

  // event counters
  TH1F* hStats = NULL;

  // centrality
  TH1F *hCent = NULL;

  // number of primary vertices
  TH1F* hPileUp = NULL;

  // z vertex distribution
  TH1F* hZVtx = NULL;

  // centrality
  int centBin;
  int theCentralityBin;

  Short_t Npix, NpixelTracks, Ntracks;
  int NtracksPtCut, NtracksEtaCut, NtracksEtaPtCut;
  float SumET_HF, SumET_HFplus, SumET_HFminus, SumET_HFplusEta4, SumET_HFminusEta4,SumET_HFhit, SumET_HFhitPlus, SumET_HFhitMinus, SumET_EB, SumET_ET, SumET_EE, SumET_EEplus, SumET_EEminus, SumET_ZDC, SumET_ZDCplus, SumET_ZDCminus;

  // Event Plane variables
  int nEP;   // number of event planes
  //float *hiEvtPlane;
  float rpAng[50];
  float rpCos[50];
  float rpSin[50];

  // handles
  edm::Handle<pat::CompositeCandidateCollection> collJpsi;
  edm::Handle<pat::CompositeCandidateCollection> collDiOnia;
  edm::Handle<pat::CompositeCandidateCollection> collTrimuon;
  edm::Handle<pat::CompositeCandidateCollection> collDimutrk;
  edm::Handle<pat::MuonCollection> collMuon;
  edm::Handle<pat::MuonCollection> collMuonNoTrig;
  edm::Handle<reco::TrackCollection> collTracks;
  edm::Handle<reco::VertexCollection> SVs;

  edm::Handle<reco::GenParticleCollection> collGenParticles;
  edm::Handle<GenEventInfoProduct> genInfo;

  edm::Handle<edm::TriggerResults> collTriggerResults;

  // data members
  edm::EDGetTokenT<pat::MuonCollection>               _patMuonToken;
  edm::EDGetTokenT<pat::MuonCollection>               _patMuonNoTrigToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patJpsiToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patDiOniaToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patTrimuonToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patDimuTrkToken;
  edm::EDGetTokenT<reco::TrackCollection>             _recoTracksToken;
  edm::EDGetTokenT<reco::GenParticleCollection>       _genParticleToken;
  edm::EDGetTokenT<GenEventInfoProduct>               _genInfoToken;
  edm::EDGetTokenT<reco::VertexCollection>            _thePVsToken;
  edm::EDGetTokenT<reco::VertexCollection>            _SVToken;
  edm::EDGetTokenT<edm::TriggerResults>               _tagTriggerResultsToken;
  edm::EDGetTokenT<reco::Centrality>                  _centralityTagToken;
  edm::EDGetTokenT<int>                               _centralityBinTagToken;
  edm::EDGetTokenT<reco::EvtPlaneCollection>          _evtPlaneTagToken;
  std::string         _histfilename;
  std::string         _datasetname;
  std::string         _mom4format;
  std::string         _muonSel;
 
  std::vector<double> _centralityranges;
  std::vector<double> _ptbinranges;
  std::vector<double> _etabinranges;
  std::vector<string> _dblTriggerPathNames;
  std::vector<string> _sglTriggerPathNames;

  bool           _onlythebest;
  bool           _applycuts;
  bool           _SofterSgMuAcceptance;
  bool           _SumETvariables;
  bool           _selTightGlobalMuon;
  bool           _storeefficiency;
  bool           _muonLessPrimaryVertex;
  bool           _useSVfinder;
  bool           _useBS;
  bool           _useRapidity;
  bool           _removeSignal;
  bool           _removeMuons;
  bool           _storeSs;
  bool           _AtLeastOneCand;
  bool           _combineCategories;
  bool           _fillRooDataSet;
  bool           _fillTree;
  bool           _fillHistos;
  bool           _theMinimumFlag;
  bool           _fillSingleMuons;
  bool           _onlySingleMuons;
  bool           _fillRecoTracks;
  bool           _isHI;
  bool           _isPA;
  bool           _isMC;
  bool           _isPromptMC;
  bool           _useEvtPlane;
  bool           _useGeTracks;
  bool           _doTrimuons;
  bool           _doDimuTrk;
  int            _flipJpsiDirection;
  bool           _genealogyInfo;
  bool           _miniAODcut;

  int _oniaPDG;
  int _BcPDG;
  int _OneMatchedHLTMu;
  bool           _checkTrigNames;

  std::vector<unsigned int>                     _thePassedCats;
  std::vector<const pat::CompositeCandidate*>   _thePassedCands;
  std::vector<unsigned int>                     _thePassed2Cats;
  std::vector<const pat::CompositeCandidate*>   _thePassed2Cands;
  std::vector<unsigned int>                     _thePassedBcCats;
  std::vector<const pat::CompositeCandidate*>   _thePassedBcCands;

  std::vector<reco::GenParticleRef> _Gen_QQ_MomAndTrkBro[Max_QQ_size];

  // number of events
  unsigned int nEvents;
  unsigned int passedCandidates;

  unsigned int runNb;
  unsigned int eventNb;
  unsigned int lumiSection;

  // limits 
  float JpsiMassMin;
  float JpsiMassMax;
  float JpsiPtMin;           // SET BY 
  float JpsiPtMax;           // DEFINITION
  float JpsiRapMin;          // OF BIN
  float JpsiRapMax;          // LIMITS 
  float JpsiPDGMass = 3.096916;
  float BcPDGMass = 6.276;

  math::XYZPoint RefVtx;
  float RefVtx_xError;
  float RefVtx_yError;
  float RefVtx_zError;
  float zVtx;
  Short_t nPV;

  // Trigger stuff
  // PUT HERE THE *LAST FILTERS* OF THE BITS YOU LIKE
  static const unsigned int sNTRIGGERS = 65;
  unsigned int NTRIGGERS;
  unsigned int NTRIGGERS_DBL;
  unsigned int nTrig;

  // MC 8E29
  bool isTriggerMatched[sNTRIGGERS];
  bool alreadyFilled[sNTRIGGERS];
  ULong64_t HLTriggers;
  int trigPrescale[sNTRIGGERS];

  std::map<std::string, int> mapTriggerNameToIntFired_;
  std::map<std::string, int> mapTriggerNameToPrescaleFac_;
  std::map<long int, int> mapMuonMomToIndex_;
  std::map<long int, int> mapGenMuonMomToIndex_;
  std::map<long int, int> mapTrkMomToIndex_;

  HLTPrescaleProvider hltPrescaleProvider;
  bool hltPrescaleInit;

  const edm::ParameterSet _iConfig;
};

HiOniaAnalyzer::HiOniaAnalyzer(const edm::ParameterSet& iConfig):
  _patMuonToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuon"))),
  _patMuonNoTrigToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuonNoTrig"))),
  _patJpsiToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDimuon"))),
  _patDiOniaToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDiquarkonia"))),
  _patTrimuonToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcTrimuon"))), //the names of userData are the same as for dimuons, but with 'trimuon' product instance name. Ignored if the collection does not exist
  _patDimuTrkToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDimuTrk"))), //the names of userData are the same as for dimuons, but with 'dimutrk' product instance name. Ignored if the collection does not exist
  _recoTracksToken(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
  _genParticleToken(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
  _genInfoToken(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
  _thePVsToken(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
  _SVToken(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("srcSV"))),//consumes<edm::View<VTX>>
  _tagTriggerResultsToken(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResultsLabel"))),
  _centralityTagToken(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag> ("CentralitySrc"))),
  _centralityBinTagToken(consumes<int>(iConfig.getParameter<edm::InputTag> ("CentralityBinSrc"))),
  _evtPlaneTagToken(consumes<reco::EvtPlaneCollection>(iConfig.getParameter<edm::InputTag> ("EvtPlane"))),
  _histfilename(iConfig.getParameter<std::string>("histFileName")),             
  _datasetname(iConfig.getParameter<std::string>("dataSetName")),         
  _mom4format(iConfig.getParameter<std::string>("mom4format")),
  _muonSel(iConfig.getParameter<std::string>("muonSel")),
  _centralityranges(iConfig.getParameter< std::vector<double> >("centralityRanges")),           
  _ptbinranges(iConfig.getParameter< std::vector<double> >("pTBinRanges")),     
  _etabinranges(iConfig.getParameter< std::vector<double> >("etaBinRanges")),   
  _dblTriggerPathNames(iConfig.getParameter< std::vector<string> >("dblTriggerPathNames")),
  _sglTriggerPathNames(iConfig.getParameter< std::vector<string> >("sglTriggerPathNames")),
  _onlythebest(iConfig.getParameter<bool>("onlyTheBest")),              
  _applycuts(iConfig.getParameter<bool>("applyCuts")),
  _SofterSgMuAcceptance(iConfig.getParameter<bool>("SofterSgMuAcceptance")),
  _SumETvariables(iConfig.getParameter<bool>("SumETvariables")),
  _selTightGlobalMuon(iConfig.getParameter<bool>("selTightGlobalMuon")),                      
  _storeefficiency(iConfig.getParameter<bool>("storeEfficiency")),      
  _muonLessPrimaryVertex(iConfig.getParameter<bool>("muonLessPV")),
  _useSVfinder(iConfig.getParameter<bool>("useSVfinder")),
  _useBS(iConfig.getParameter<bool>("useBeamSpot")),
  _useRapidity(iConfig.getParameter<bool>("useRapidity")),
  _removeSignal(iConfig.getUntrackedParameter<bool>("removeSignalEvents",false)),
  _removeMuons(iConfig.getUntrackedParameter<bool>("removeTrueMuons",false)),
  _storeSs(iConfig.getParameter<bool>("storeSameSign")),
  _AtLeastOneCand(iConfig.getParameter<bool>("AtLeastOneCand")),
  _combineCategories(iConfig.getParameter<bool>("combineCategories")),
  _fillRooDataSet(iConfig.getParameter<bool>("fillRooDataSet")),  
  _fillTree(iConfig.getParameter<bool>("fillTree")),  
  _fillHistos(iConfig.getParameter<bool>("fillHistos")),
  _theMinimumFlag(iConfig.getParameter<bool>("minimumFlag")),  
  _fillSingleMuons(iConfig.getParameter<bool>("fillSingleMuons")),
  _onlySingleMuons(iConfig.getParameter<bool>("onlySingleMuons")),
  _fillRecoTracks(iConfig.getParameter<bool>("fillRecoTracks")),
  _isHI(iConfig.getUntrackedParameter<bool>("isHI",false) ),
  _isPA(iConfig.getUntrackedParameter<bool>("isPA",true) ),
  _isMC(iConfig.getUntrackedParameter<bool>("isMC",false) ),
  _isPromptMC(iConfig.getUntrackedParameter<bool>("isPromptMC",true) ),
  _useEvtPlane(iConfig.getUntrackedParameter<bool>("useEvtPlane",false) ),
  _useGeTracks(iConfig.getUntrackedParameter<bool>("useGeTracks",false) ),
  _doTrimuons(iConfig.getParameter<bool>("doTrimuons")),
  _doDimuTrk(iConfig.getParameter<bool>("DimuonTrk")),
  _flipJpsiDirection(iConfig.getParameter<int>("flipJpsiDirection")),
  _genealogyInfo(iConfig.getParameter<bool>("genealogyInfo")),
  _miniAODcut(iConfig.getParameter<bool>("miniAODcut")),
  _oniaPDG(iConfig.getParameter<int>("oniaPDG")),
  _BcPDG(iConfig.getParameter<int>("BcPDG")),
  _OneMatchedHLTMu(iConfig.getParameter<int>("OneMatchedHLTMu")),
  _checkTrigNames(iConfig.getParameter<bool>("checkTrigNames")),
  hltPrescaleProvider(iConfig, consumesCollector(), *this),
  _iConfig(iConfig)
{
  usesResource(TFileService::kSharedResource);

  if(_doTrimuons && _doDimuTrk){
    cout<<"FATAL ERROR: _doTrimuons and _doDimuTrk cannot be both true! Code not designed to do both at a time; Return now."<<endl;
    return;}
  if(_doDimuTrk){
    if(!_useGeTracks){
      cout<<"Have to use generalTracks if doDimuonTrk==true. _useGeTracks = true is forced."<<endl; _useGeTracks=true;}
    if(!_fillRecoTracks){
      cout<<"Have to use generalTracks if doDimuonTrk==true. _fillRecoTracks = true is forced."<<endl; _fillRecoTracks=true;}
  }

  //now do whatever initialization is needed
  nEvents = 0;
  passedCandidates = 0;

  theRegions.push_back("All");
  theRegions.push_back("Barrel");
  theRegions.push_back("EndCap");

  std::stringstream centLabel;
  for (unsigned int iCent=0; iCent<_centralityranges.size(); ++iCent) {
    if (iCent==0)
      centLabel << "00" << _centralityranges.at(iCent);
    else
      centLabel << _centralityranges.at(iCent-1) << _centralityranges.at(iCent);

    theCentralities.push_back(centLabel.str());
    centLabel.str("");
  }
  theCentralities.push_back("MinBias");

  theSign.push_back("pm");
  if (_storeSs) {
    theSign.push_back("pp");
    theSign.push_back("mm");
  }

  NTRIGGERS_DBL = _dblTriggerPathNames.size();
  NTRIGGERS = NTRIGGERS_DBL + _sglTriggerPathNames.size() + 1; // + 1 for "NoTrigger"
  std::cout << "NTRIGGERS_DBL = " << NTRIGGERS_DBL << "\t NTRIGGERS_SGL = " << _sglTriggerPathNames.size() << "\t NTRIGGERS = " << NTRIGGERS << std::endl;
  nTrig = NTRIGGERS - 1;

  isTriggerMatched[0]=true; // first entry 'hardcoded' true to accept "all" events
  theTriggerNames.push_back("NoTrigger");

  for (unsigned int iTr = 1; iTr<NTRIGGERS; ++iTr) {
    isTriggerMatched[iTr] = false;

    if (iTr<=NTRIGGERS_DBL) {
      theTriggerNames.push_back(_dblTriggerPathNames.at(iTr-1));
    }
    else {
      theTriggerNames.push_back(_sglTriggerPathNames.at(iTr-NTRIGGERS_DBL-1));
    }
    std::cout<<" Trigger "<<iTr<<"\t"<<theTriggerNames[iTr]<<std::endl;
  }

  if(_OneMatchedHLTMu>=(int)NTRIGGERS){
    std::cout<<"WARNING: the _OneMatchedHLTMu parameter is asking for a wrong trigger number. No matching will be done."<<std::endl;
    _OneMatchedHLTMu=-1;}
  if(_OneMatchedHLTMu>-1)
    std::cout<<" Will keep only dimuons (trimuons) that have one (two) daughters matched to "<<theTriggerNames[_OneMatchedHLTMu]<<" filter."<<std::endl;

  etaMax = 2.5;

  JpsiMassMin = 2.6;
  JpsiMassMax = 3.5;

  JpsiPtMin = _ptbinranges[0];
  //std::cout << "Pt min = " << JpsiPtMin << std::endl;
  JpsiPtMax = _ptbinranges[_ptbinranges.size()-1];
  //std::cout << "Pt max = " << JpsiPtMax << std::endl;

     
  JpsiRapMin = _etabinranges[0];
  //std::cout << "Rap min = " << JpsiRapMin << std::endl;
  JpsiRapMax = _etabinranges[_etabinranges.size()-1];
  //std::cout << "Rap max = " << JpsiRapMax << std::endl;
  

  for(std::vector<std::string>::iterator it = theTriggerNames.begin(); it != theTriggerNames.end(); ++it){
      mapTriggerNameToIntFired_[*it] = -9999;
      mapTriggerNameToPrescaleFac_[*it] = -1;
  }
};



HiOniaAnalyzer::~HiOniaAnalyzer()
{
 
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  Reco_mu_4mom->Delete();
  Reco_mu_L1_4mom->Delete();
  Reco_QQ_4mom->Delete();
  Reco_QQ_mumi_4mom->Delete();
  Reco_QQ_mupl_4mom->Delete();
  Reco_QQ_vtx->Delete();
  if (_useGeTracks && _fillRecoTracks) {
    Reco_trk_4mom->Delete();
    Reco_trk_vtx->Delete();
  }
  if(_doTrimuons || _doDimuTrk){
    Reco_3mu_4mom->Delete();
    Reco_3mu_vtx->Delete();
    if(_isMC){
      Gen_Bc_4mom->Delete();
      Gen_Bc_nuW_4mom->Delete();
      Gen_3mu_4mom->Delete();
    }
  }
  if(_isMC){
    Gen_mu_4mom->Delete();
    Gen_QQ_4mom->Delete();
  }
};


void
HiOniaAnalyzer::InitTree()
{

  Reco_mu_4mom = new TClonesArray("TLorentzVector", Max_mu_size);
  Reco_mu_L1_4mom = new TClonesArray("TLorentzVector", Max_mu_size);
  Reco_QQ_4mom = new TClonesArray("TLorentzVector", Max_QQ_size);
  Reco_QQ_mumi_4mom = new TClonesArray("TLorentzVector", Max_QQ_size);
  Reco_QQ_mupl_4mom = new TClonesArray("TLorentzVector", Max_QQ_size);
  Reco_QQ_vtx = new TClonesArray("TVector3", Max_QQ_size);

  if (_useGeTracks && _fillRecoTracks) {
    Reco_trk_4mom = new TClonesArray("TLorentzVector", Max_trk_size);
    Reco_trk_vtx = new TClonesArray("TVector3", Max_trk_size);
  }

  if (_isMC) {
    Gen_mu_4mom = new TClonesArray("TLorentzVector", 10);
    Gen_QQ_4mom = new TClonesArray("TLorentzVector", 10);
  }

  if(_doTrimuons || _doDimuTrk){
    Reco_3mu_4mom = new TClonesArray("TLorentzVector", Max_Bc_size);
    Reco_3mu_vtx = new TClonesArray("TVector3", Max_Bc_size);

    if (_isMC) {
      Gen_Bc_4mom = new TClonesArray("TLorentzVector", 10);
      Gen_Bc_nuW_4mom = new TClonesArray("TLorentzVector", 10);
      Gen_3mu_4mom = new TClonesArray("TLorentzVector", 10);
    }
  }

  //myTree = new TTree("myTree","My TTree of dimuons");
  myTree = fs->make<TTree>("myTree","My TTree of dimuons");
  
  myTree->Branch("eventNb", &eventNb,   "eventNb/i");
  if(!_isMC){
    myTree->Branch("runNb",   &runNb,     "runNb/i");
    myTree->Branch("LS",      &lumiSection, "LS/i");}
  myTree->Branch("zVtx",    &zVtx,        "zVtx/F"); 
  myTree->Branch("nPV",    &nPV,        "nPV/S"); 
  if (_isHI || _isPA){
    myTree->Branch("Centrality", &centBin, "Centrality/I");
    myTree->Branch("Npix",&Npix,"Npix/S");
    myTree->Branch("NpixelTracks",&NpixelTracks,"NpixelTracks/S");
  }
  myTree->Branch("Ntracks", &Ntracks, "Ntracks/S");

  //myTree->Branch("nTrig", &nTrig, "nTrig/I");
  myTree->Branch("trigPrescale", trigPrescale, Form("trigPrescale[%d]/I",nTrig));
  myTree->Branch("HLTriggers", &HLTriggers, "HLTriggers/l");

  if ((_isHI || _isPA) && _SumETvariables){
    myTree->Branch("SumET_HF",&SumET_HF,"SumET_HF/F");
    myTree->Branch("SumET_HFplus",&SumET_HFplus,"SumET_HFplus/F");
    myTree->Branch("SumET_HFminus",&SumET_HFminus,"SumET_HFminus/F");
    myTree->Branch("SumET_HFplusEta4",&SumET_HFplusEta4,"SumET_HFplusEta4/F");
    myTree->Branch("SumET_HFminusEta4",&SumET_HFminusEta4,"SumET_HFminusEta4/F");
    myTree->Branch("SumET_ET",&SumET_ET,"SumET_ET/F");
    myTree->Branch("SumET_EE",&SumET_EE,"SumET_EE/F");
    myTree->Branch("SumET_EB",&SumET_EB,"SumET_EB/F");
    myTree->Branch("SumET_EEplus",&SumET_EEplus,"SumET_EEplus/F");
    myTree->Branch("SumET_EEminus",&SumET_EEminus,"SumET_EEminus/F");
    myTree->Branch("SumET_ZDC",&SumET_ZDC,"SumET_ZDC/F");
    myTree->Branch("SumET_ZDCplus",&SumET_ZDCplus,"SumET_ZDCplus/F");
    myTree->Branch("SumET_ZDCminus",&SumET_ZDCminus,"SumET_ZDCminus/F");
  }

  if ((_isHI || _isPA) && _useEvtPlane) {
    myTree->Branch("nEP", &nEP, "nEP/I");
    myTree->Branch("rpAng", &rpAng, "rpAng[nEP]/F");
    myTree->Branch("rpSin", &rpSin, "rpSin[nEP]/F");
    myTree->Branch("rpCos", &rpCos, "rpCos[nEP]/F");
  }

  if(!_onlySingleMuons){
    if(_doTrimuons || _doDimuTrk){
      myTree->Branch("Reco_3mu_size", &Reco_3mu_size,  "Reco_3mu_size/S");
      myTree->Branch("Reco_3mu_charge", Reco_3mu_charge,   "Reco_3mu_charge[Reco_3mu_size]/S");
      myTree->Branch("Reco_3mu_4mom", "TClonesArray", &Reco_3mu_4mom, 32000, 0);
      myTree->Branch("Reco_3mu_4mom_pt" , &Reco_3mu_4mom_pt , 32000, 0);
      myTree->Branch("Reco_3mu_4mom_eta", &Reco_3mu_4mom_eta, 32000, 0);
      myTree->Branch("Reco_3mu_4mom_phi", &Reco_3mu_4mom_phi, 32000, 0);
      myTree->Branch("Reco_3mu_4mom_m"  , &Reco_3mu_4mom_m  , 32000, 0);
      myTree->Branch("Reco_3mu_mupl_idx",      Reco_3mu_mupl_idx,    "Reco_3mu_mupl_idx[Reco_3mu_size]/S");
      myTree->Branch("Reco_3mu_mumi_idx",      Reco_3mu_mumi_idx,    "Reco_3mu_mumi_idx[Reco_3mu_size]/S");
      myTree->Branch("Reco_3mu_muW_idx",      Reco_3mu_muW_idx,    "Reco_3mu_muW_idx[Reco_3mu_size]/S");
      if(!_doDimuTrk)
	myTree->Branch("Reco_3mu_muW2_idx",      Reco_3mu_muW2_idx,    "Reco_3mu_muW2_idx[Reco_3mu_size]/S");
      myTree->Branch("Reco_3mu_QQ1_idx",      Reco_3mu_QQ1_idx,    "Reco_3mu_QQ1_idx[Reco_3mu_size]/S");
      if(!_doDimuTrk){
	myTree->Branch("Reco_3mu_QQ2_idx",      Reco_3mu_QQ2_idx,    "Reco_3mu_QQ2_idx[Reco_3mu_size]/S");
	myTree->Branch("Reco_3mu_QQss_idx",      Reco_3mu_QQss_idx,    "Reco_3mu_QQss_idx[Reco_3mu_size]/S");}
      if(_isMC && _genealogyInfo){      
	myTree->Branch("Reco_3mu_muW_isGenJpsiBro",      Reco_3mu_muW_isGenJpsiBro,    "Reco_3mu_muW_isGenJpsiBro[Reco_3mu_size]/O");
	myTree->Branch("Reco_3mu_muW_trueId",      Reco_3mu_muW_trueId,    "Reco_3mu_muW_trueId[Reco_3mu_size]/I");
      }

      myTree->Branch("Reco_3mu_ctau", Reco_3mu_ctau,   "Reco_3mu_ctau[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_ctauErr", Reco_3mu_ctauErr,   "Reco_3mu_ctauErr[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_cosAlpha", Reco_3mu_cosAlpha,   "Reco_3mu_cosAlpha[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_ctau3D", Reco_3mu_ctau3D,   "Reco_3mu_ctau3D[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_ctauErr3D", Reco_3mu_ctauErr3D,   "Reco_3mu_ctauErr3D[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_cosAlpha3D", Reco_3mu_cosAlpha3D,   "Reco_3mu_cosAlpha3D[Reco_3mu_size]/F");

      if (_isMC){
	myTree->Branch("Reco_3mu_whichGen", Reco_3mu_whichGen,   "Reco_3mu_whichGen[Reco_3mu_size]/S");
      }
      myTree->Branch("Reco_3mu_VtxProb", Reco_3mu_VtxProb,   "Reco_3mu_VtxProb[Reco_3mu_size]/F");

      if(_doDimuTrk) {
	myTree->Branch("Reco_3mu_KCVtxProb", Reco_3mu_KCVtxProb,   "Reco_3mu_KCVtxProb[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCctau", Reco_3mu_KCctau,   "Reco_3mu_KCctau[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCctauErr", Reco_3mu_KCctauErr,   "Reco_3mu_KCctauErr[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCcosAlpha", Reco_3mu_KCcosAlpha,   "Reco_3mu_KCcosAlpha[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCctau3D", Reco_3mu_KCctau3D,   "Reco_3mu_KCctau3D[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCctauErr3D", Reco_3mu_KCctauErr3D,   "Reco_3mu_KCctauErr3D[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_KCcosAlpha3D", Reco_3mu_KCcosAlpha3D,   "Reco_3mu_KCcosAlpha3D[Reco_3mu_size]/F");
      }
      if ((!_theMinimumFlag && _muonLessPrimaryVertex) || (_flipJpsiDirection>0)) {
	myTree->Branch("Reco_3mu_muW_dxy_muonlessVtx",      Reco_3mu_muW_dxy,    "Reco_3mu_muW_dxy_muonlessVtx[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_muW_dz_muonlessVtx",      Reco_3mu_muW_dz,    "Reco_3mu_muW_dz_muonlessVtx[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_mumi_dxy_muonlessVtx",      Reco_3mu_mumi_dxy,    "Reco_3mu_mumi_dxy_muonlessVtx[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_mumi_dz_muonlessVtx",      Reco_3mu_mumi_dz,    "Reco_3mu_mumi_dz_muonlessVtx[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_mupl_dxy_muonlessVtx",      Reco_3mu_mupl_dxy,    "Reco_3mu_mupl_dxy_muonlessVtx[Reco_3mu_size]/F");
	myTree->Branch("Reco_3mu_mupl_dz_muonlessVtx",      Reco_3mu_mupl_dz,    "Reco_3mu_mupl_dz_muonlessVtx[Reco_3mu_size]/F");
      }

      myTree->Branch("Reco_3mu_MassErr", Reco_3mu_MassErr,   "Reco_3mu_MassErr[Reco_3mu_size]/F");
      myTree->Branch("Reco_3mu_CorrM", Reco_3mu_CorrM,   "Reco_3mu_CorrM[Reco_3mu_size]/F");
      if(_useSVfinder && SVs.isValid() && SVs->size()>0){
	myTree->Branch("Reco_3mu_NbMuInSameSV", Reco_3mu_NbMuInSameSV,   "Reco_3mu_NbMuInSameSV[Reco_3mu_size]/S");}
      myTree->Branch("Reco_3mu_vtx", "TClonesArray", &Reco_3mu_vtx, 32000, 0);
    }

    myTree->Branch("Reco_QQ_size", &Reco_QQ_size,  "Reco_QQ_size/S");
    myTree->Branch("Reco_QQ_type", Reco_QQ_type,   "Reco_QQ_type[Reco_QQ_size]/S");
    myTree->Branch("Reco_QQ_sign", Reco_QQ_sign,   "Reco_QQ_sign[Reco_QQ_size]/S");
    if(std::strcmp("array", _mom4format.c_str()) == 0) myTree->Branch("Reco_QQ_4mom", "TClonesArray", &Reco_QQ_4mom, 32000, 0);
    if(std::strcmp("vector", _mom4format.c_str()) == 0){
    myTree->Branch("Reco_QQ_4mom_pt" , &Reco_QQ_4mom_pt , 32000, 0);
    myTree->Branch("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta, 32000, 0);
    myTree->Branch("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi, 32000, 0);
    myTree->Branch("Reco_QQ_4mom_m"  , &Reco_QQ_4mom_m  , 32000, 0);

    myTree->Branch("Reco_QQQQ_4mom_pt" , &Reco_QQQQ_4mom_pt , 32000, 0);
    myTree->Branch("Reco_QQQQ_4mom_eta", &Reco_QQQQ_4mom_eta, 32000, 0);
    myTree->Branch("Reco_QQQQ_4mom_phi", &Reco_QQQQ_4mom_phi, 32000, 0);
    myTree->Branch("Reco_QQQQ_4mom_m"  , &Reco_QQQQ_4mom_m  , 32000, 0);
    }
    myTree->Branch("Reco_QQ_mupl_idx",      Reco_QQ_mupl_idx,    "Reco_QQ_mupl_idx[Reco_QQ_size]/S");
    myTree->Branch("Reco_QQ_mumi_idx",      Reco_QQ_mumi_idx,    "Reco_QQ_mumi_idx[Reco_QQ_size]/S");

    myTree->Branch("Reco_QQ_trig", Reco_QQ_trig,   "Reco_QQ_trig[Reco_QQ_size]/l");
    myTree->Branch("Reco_QQ_ctau", Reco_QQ_ctau,   "Reco_QQ_ctau[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_ctauErr", Reco_QQ_ctauErr,   "Reco_QQ_ctauErr[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha,   "Reco_QQ_cosAlpha[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_ctau3D", Reco_QQ_ctau3D,   "Reco_QQ_ctau3D[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_ctauErr3D", Reco_QQ_ctauErr3D,   "Reco_QQ_ctauErr3D[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_cosAlpha3D", Reco_QQ_cosAlpha3D,   "Reco_QQ_cosAlpha3D[Reco_QQ_size]/F");

    if (_isMC){
      myTree->Branch("Reco_QQ_whichGen", Reco_QQ_whichGen,   "Reco_QQ_whichGen[Reco_QQ_size]/S");
    }
    myTree->Branch("Reco_QQQQ_VtxProb", Reco_QQQQ_VtxProb,   "Reco_QQQQ_VtxProb[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_dca", Reco_QQQQ_dca,   "Reco_QQQQ_dca[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_MassErr", Reco_QQQQ_MassErr,   "Reco_QQQQ_MassErr[Reco_QQQQ_size]/F");

    myTree->Branch("Reco_QQQQ_mupl_idx",      Reco_QQQQ_mupl_idx,    "Reco_QQQQ_mupl_idx[Reco_QQQQ_size]/S");
    myTree->Branch("Reco_QQQQ_mumi_idx",      Reco_QQQQ_mumi_idx,    "Reco_QQQQ_mumi_idx[Reco_QQQQ_size]/S");

    myTree->Branch("Reco_QQQQ_trig", Reco_QQQQ_trig,   "Reco_QQQQ_trig[Reco_QQQQ_size]/l");
    myTree->Branch("Reco_QQQQ_ctau", Reco_QQQQ_ctau,   "Reco_QQQQ_ctau[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_ctauErr", Reco_QQQQ_ctauErr,   "Reco_QQQQ_ctauErr[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_cosAlpha", Reco_QQQQ_cosAlpha,   "Reco_QQQQ_cosAlpha[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_ctau3D", Reco_QQQQ_ctau3D,   "Reco_QQQQ_ctau3D[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_ctauErr3D", Reco_QQQQ_ctauErr3D,   "Reco_QQQQ_ctauErr3D[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_cosAlpha3D", Reco_QQQQ_cosAlpha3D,   "Reco_QQQQ_cosAlpha3D[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_VtxProb", Reco_QQQQ_VtxProb,   "Reco_QQQQ_VtxProb[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_dca", Reco_QQQQ_dca,   "Reco_QQQQ_dca[Reco_QQQQ_size]/F");
    myTree->Branch("Reco_QQQQ_MassErr", Reco_QQQQ_MassErr,   "Reco_QQQQ_MassErr[Reco_QQQQ_size]/F");

    if ((!_theMinimumFlag && _muonLessPrimaryVertex) || (_flipJpsiDirection>0)) {
      myTree->Branch("Reco_QQ_mupl_dxy_muonlessVtx",Reco_QQ_mupl_dxy, "Reco_QQ_mupl_dxy_muonlessVtx[Reco_QQ_size]/F");
      myTree->Branch("Reco_QQ_mumi_dxy_muonlessVtx",Reco_QQ_mumi_dxy, "Reco_QQ_mumi_dxy_muonlessVtx[Reco_QQ_size]/F");
      myTree->Branch("Reco_QQ_mupl_dz_muonlessVtx",Reco_QQ_mupl_dz, "Reco_QQ_mupl_dz_muonlessVtx[Reco_QQ_size]/F");
      myTree->Branch("Reco_QQ_mumi_dz_muonlessVtx",Reco_QQ_mumi_dz, "Reco_QQ_mumi_dz_muonlessVtx[Reco_QQ_size]/F");
    }
    if(_flipJpsiDirection>0){
      myTree->Branch("Reco_QQ_flipJpsi",Reco_QQ_flipJpsi, "Reco_QQ_flipJpsi[Reco_QQ_size]/S");    
      if(std::strcmp("array", _mom4format.c_str()) == 0) myTree->Branch("Reco_QQ_mumi_4mom", "TClonesArray", &Reco_QQ_mumi_4mom, 32000, 0);
      if(std::strcmp("vector", _mom4format.c_str()) == 0){
        myTree->Branch("Reco_QQ_mumi_4mom_pt" , &Reco_QQ_mumi_4mom_pt , 32000, 0);
        myTree->Branch("Reco_QQ_mumi_4mom_eta", &Reco_QQ_mumi_4mom_eta, 32000, 0);
        myTree->Branch("Reco_QQ_mumi_4mom_phi", &Reco_QQ_mumi_4mom_phi, 32000, 0);
        myTree->Branch("Reco_QQ_mumi_4mom_m"  , &Reco_QQ_mumi_4mom_m  , 32000, 0);
      }
      if(std::strcmp("array", _mom4format.c_str()) == 0) myTree->Branch("Reco_QQ_mupl_4mom", "TClonesArray", &Reco_QQ_mupl_4mom, 32000, 0);
      if(std::strcmp("vector", _mom4format.c_str()) == 0){
        myTree->Branch("Reco_QQ_mupl_4mom_pt" , &Reco_QQ_mupl_4mom_pt , 32000, 0);
        myTree->Branch("Reco_QQ_mupl_4mom_eta", &Reco_QQ_mupl_4mom_eta, 32000, 0);
        myTree->Branch("Reco_QQ_mupl_4mom_phi", &Reco_QQ_mupl_4mom_phi, 32000, 0);
        myTree->Branch("Reco_QQ_mupl_4mom_m"  , &Reco_QQ_mupl_4mom_m  , 32000, 0);
      }
    }
  }

  myTree->Branch("Reco_mu_size", &Reco_mu_size,  "Reco_mu_size/S");
  myTree->Branch("Reco_mu_type", Reco_mu_type,   "Reco_mu_type[Reco_mu_size]/S");
  if (_isMC){
    myTree->Branch("Reco_mu_whichGen", Reco_mu_whichGen,   "Reco_mu_whichGen[Reco_mu_size]/S");
  }
  myTree->Branch("Reco_mu_SelectionType", Reco_mu_SelectionType,   "Reco_mu_SelectionType[Reco_mu_size]/I");
  myTree->Branch("Reco_mu_charge", Reco_mu_charge,   "Reco_mu_charge[Reco_mu_size]/S");
  if(std::strcmp("array", _mom4format.c_str()) == 0){
    myTree->Branch("Reco_mu_4mom", "TClonesArray", &Reco_mu_4mom, 32000, 0);
    myTree->Branch("Reco_mu_L1_4mom", "TClonesArray", &Reco_mu_L1_4mom, 32000, 0);
  }
  if(std::strcmp("vector", _mom4format.c_str()) == 0){
    myTree->Branch("Reco_mu_4mom_pt" , &Reco_mu_4mom_pt , 32000, 0);
    myTree->Branch("Reco_mu_4mom_eta", &Reco_mu_4mom_eta, 32000, 0);
    myTree->Branch("Reco_mu_4mom_phi", &Reco_mu_4mom_phi, 32000, 0);
    myTree->Branch("Reco_mu_4mom_m"  , &Reco_mu_4mom_m  , 32000, 0);
    myTree->Branch("Reco_mu_L1_4mom_pt" , &Reco_mu_L1_4mom_pt , 32000, 0);
    myTree->Branch("Reco_mu_L1_4mom_eta", &Reco_mu_L1_4mom_eta, 32000, 0);
    myTree->Branch("Reco_mu_L1_4mom_phi", &Reco_mu_L1_4mom_phi, 32000, 0);
    myTree->Branch("Reco_mu_L1_4mom_m"  , &Reco_mu_L1_4mom_m  , 32000, 0);
  }
  myTree->Branch("Reco_mu_trig", Reco_mu_trig,   "Reco_mu_trig[Reco_mu_size]/l");

  if (!_theMinimumFlag) {
    myTree->Branch("Reco_mu_InTightAcc",Reco_mu_InTightAcc, "Reco_mu_InTightAcc[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_InLooseAcc",Reco_mu_InLooseAcc, "Reco_mu_InLooseAcc[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_highPurity", Reco_mu_highPurity,   "Reco_mu_highPurity[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_TMOneStaTight", Reco_mu_TMOneStaTight,   "Reco_mu_TMOneStaTight[Reco_mu_size]/O");
    // myTree->Branch("Reco_mu_TrkMuArb", Reco_mu_TrkMuArb,   "Reco_mu_TrkMuArb[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isPF", Reco_mu_isPF, "Reco_mu_isPF[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isTracker", Reco_mu_isTracker, "Reco_mu_isTracker[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isGlobal", Reco_mu_isGlobal, "Reco_mu_isGlobal[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isSoftCutBased", Reco_mu_isSoftCutBased, "Reco_mu_isSoftCutBased[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isHybridSoft", Reco_mu_isHybridSoft, "Reco_mu_isHybridSoft[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isMedium", Reco_mu_isMedium, "Reco_mu_isMedium[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_isTightCutBased", Reco_mu_isTightCutBased, "Reco_mu_isTightCutBased[Reco_mu_size]/O");

    myTree->Branch("Reco_mu_candType", Reco_mu_candType, "Reco_mu_candType[Reco_mu_size]/S");
    myTree->Branch("Reco_mu_nPixValHits", Reco_mu_nPixValHits,   "Reco_mu_nPixValHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nMuValHits", Reco_mu_nMuValHits,   "Reco_mu_nMuValHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nTrkHits",Reco_mu_nTrkHits, "Reco_mu_nTrkHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_segmentComp", Reco_mu_segmentComp, "Reco_mu_segmentComp[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_kink", Reco_mu_kink, "Reco_mu_kink[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_localChi2", Reco_mu_localChi2, "Reco_mu_localChi2[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_validFraction", Reco_mu_validFraction, "Reco_mu_validFraction[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_normChi2_bestTracker", Reco_mu_normChi2_bestTracker, "Reco_mu_normChi2_bestTracker[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_normChi2_inner",Reco_mu_normChi2_inner, "Reco_mu_normChi2_inner[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_normChi2_global",Reco_mu_normChi2_global, "Reco_mu_normChi2_global[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_nPixWMea",Reco_mu_nPixWMea, "Reco_mu_nPixWMea[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nTrkWMea",Reco_mu_nTrkWMea, "Reco_mu_nTrkWMea[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_StationsMatched",Reco_mu_StationsMatched, "Reco_mu_StationsMatched[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_dxy",Reco_mu_dxy, "Reco_mu_dxy[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dxyErr",Reco_mu_dxyErr, "Reco_mu_dxyErr[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dz",Reco_mu_dz, "Reco_mu_dz[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dzErr",Reco_mu_dzErr, "Reco_mu_dzErr[Reco_mu_size]/F");
    // myTree->Branch("Reco_mu_pt_inner",Reco_mu_pt_inner, "Reco_mu_pt_inner[Reco_mu_size]/F");
    // myTree->Branch("Reco_mu_pt_global",Reco_mu_pt_global, "Reco_mu_pt_global[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_ptErr_inner",Reco_mu_ptErr_inner, "Reco_mu_ptErr_inner[Reco_mu_size]/F");
    // myTree->Branch("Reco_mu_ptErr_global",Reco_mu_ptErr_global, "Reco_mu_ptErr_global[Reco_mu_size]/F");
  }

  if (_useGeTracks && _fillRecoTracks) {
    if(!_doDimuTrk){
      myTree->Branch("Reco_QQ_NtrkPt02", Reco_QQ_NtrkPt02, "Reco_QQ_NtrkPt02[Reco_QQ_size]/I");
      myTree->Branch("Reco_QQ_NtrkPt03", Reco_QQ_NtrkPt03, "Reco_QQ_NtrkPt03[Reco_QQ_size]/I");
      myTree->Branch("Reco_QQ_NtrkPt04", Reco_QQ_NtrkPt04, "Reco_QQ_NtrkPt04[Reco_QQ_size]/I");
      myTree->Branch("Reco_QQ_NtrkDeltaR03", Reco_QQ_NtrkDeltaR03, "Reco_QQ_NtrkDeltaR03[Reco_QQ_size]/I");
      myTree->Branch("Reco_QQ_NtrkDeltaR04", Reco_QQ_NtrkDeltaR04, "Reco_QQ_NtrkDeltaR04[Reco_QQ_size]/I");
      myTree->Branch("Reco_QQ_NtrkDeltaR05", Reco_QQ_NtrkDeltaR05, "Reco_QQ_NtrkDeltaR05[Reco_QQ_size]/I");
    }

    myTree->Branch("Reco_trk_size", &Reco_trk_size,  "Reco_trk_size/S");
    myTree->Branch("Reco_trk_charge", Reco_trk_charge,   "Reco_trk_charge[Reco_trk_size]/S");
    myTree->Branch("Reco_trk_InLooseAcc", Reco_trk_InLooseAcc,   "Reco_trk_InLooseAcc[Reco_trk_size]/O");
    myTree->Branch("Reco_trk_InTightAcc", Reco_trk_InTightAcc,   "Reco_trk_InTightAcc[Reco_trk_size]/O");
    if(std::strcmp("array", _mom4format.c_str()) == 0){
      myTree->Branch("Reco_trk_4mom", "TClonesArray", &Reco_trk_4mom, 32000, 0);
    }
    if(std::strcmp("vector", _mom4format.c_str()) == 0){
      myTree->Branch("Reco_trk_4mom_pt" , &Reco_trk_4mom_pt , 32000, 0);
      myTree->Branch("Reco_trk_4mom_eta", &Reco_trk_4mom_eta, 32000, 0);
      myTree->Branch("Reco_trk_4mom_phi", &Reco_trk_4mom_phi, 32000, 0);
      myTree->Branch("Reco_trk_4mom_m"  , &Reco_trk_4mom_m  , 32000, 0);
      myTree->Branch("Reco_trk_dxyError", Reco_trk_dxyError, "Reco_trk_dxyError[Reco_trk_size]/F");
      myTree->Branch("Reco_trk_dzError", Reco_trk_dzError, "Reco_trk_dzError[Reco_trk_size]/F");
      myTree->Branch("Reco_trk_dxy", Reco_trk_dxy, "Reco_trk_dxy[Reco_trk_size]/F");
      myTree->Branch("Reco_trk_dz", Reco_trk_dz, "Reco_trk_dz[Reco_trk_size]/F");
      myTree->Branch("Reco_trk_ptErr", Reco_trk_ptErr, "Reco_trk_ptErr[Reco_trk_size]/F");
      myTree->Branch("Reco_trk_originalAlgo", Reco_trk_originalAlgo, "Reco_trk_originalAlgo[Reco_trk_size]/I");
      myTree->Branch("Reco_trk_nPixWMea", Reco_trk_nPixWMea, "Reco_trk_nPixWMea[Reco_trk_size]/I");
      myTree->Branch("Reco_trk_nTrkWMea", Reco_trk_nTrkWMea, "Reco_trk_nTrkWMea[Reco_trk_size]/I");
    }
    if (_isMC) {
      myTree->Branch("Reco_trk_whichGenmu", Reco_trk_whichGenmu, "Reco_trk_whichGenmu[Reco_trk_size]/S");
    }
  }

  if (_isMC) {
    if(_genealogyInfo){
      myTree->Branch("Reco_mu_simExtType",Reco_mu_simExtType, "Reco_mu_simExtType[Reco_mu_size]/I");
    }
    myTree->Branch("Gen_weight",       &Gen_weight,    "Gen_weight/F");
    myTree->Branch("Gen_pthat",        &Gen_pthat,     "Gen_pthat/F");

    if(!_onlySingleMuons){
      myTree->Branch("Gen_QQ_size",      &Gen_QQ_size,    "Gen_QQ_size/S");
      //myTree->Branch("Gen_QQ_type",      Gen_QQ_type,    "Gen_QQ_type[Gen_QQ_size]/S");
      myTree->Branch("Gen_QQ_4mom",      "TClonesArray", &Gen_QQ_4mom, 32000, 0);
      myTree->Branch("Gen_QQ_4mom_pt" , &Gen_QQ_4mom_pt , 32000, 0);
      myTree->Branch("Gen_QQ_4mom_eta", &Gen_QQ_4mom_eta, 32000, 0);
      myTree->Branch("Gen_QQ_4mom_phi", &Gen_QQ_4mom_phi, 32000, 0);
      myTree->Branch("Gen_QQ_4mom_m"  , &Gen_QQ_4mom_m  , 32000, 0);
      myTree->Branch("Gen_QQ_ctau",      Gen_QQ_ctau,    "Gen_QQ_ctau[Gen_QQ_size]/F");
      myTree->Branch("Gen_QQ_ctau3D",      Gen_QQ_ctau3D,    "Gen_QQ_ctau3D[Gen_QQ_size]/F");  
      myTree->Branch("Gen_QQ_mupl_idx",      Gen_QQ_mupl_idx,    "Gen_QQ_mupl_idx[Gen_QQ_size]/S");
      myTree->Branch("Gen_QQ_mumi_idx",      Gen_QQ_mumi_idx,    "Gen_QQ_mumi_idx[Gen_QQ_size]/S");
      myTree->Branch("Gen_QQ_whichRec", Gen_QQ_whichRec,   "Gen_QQ_whichRec[Gen_QQ_size]/S"); 
      if(_genealogyInfo){
	myTree->Branch("Gen_QQ_momId", Gen_QQ_momId,   "Gen_QQ_momId[Gen_QQ_size]/I");
      }

      if(_doTrimuons || _doDimuTrk){
	myTree->Branch("Gen_QQ_Bc_idx",      Gen_QQ_Bc_idx,    "Gen_QQ_Bc_idx[Gen_QQ_size]/S");
	myTree->Branch("Gen_Bc_size",      &Gen_Bc_size,    "Gen_Bc_size/S");
  if(std::strcmp("array", _mom4format.c_str()) == 0){
	  myTree->Branch("Gen_Bc_4mom",      "TClonesArray", &Gen_Bc_4mom, 32000, 0);
	  myTree->Branch("Gen_Bc_nuW_4mom", "TClonesArray", &Gen_Bc_nuW_4mom, 32000, 0);
	  myTree->Branch("Gen_3mu_4mom",      "TClonesArray", &Gen_3mu_4mom, 32000, 0);
  }
  if(std::strcmp("vector", _mom4format.c_str()) == 0){
    myTree->Branch("Gen_Bc_4mom_pt" , &Gen_Bc_4mom_pt , 32000, 0);
    myTree->Branch("Gen_Bc_4mom_eta", &Gen_Bc_4mom_eta, 32000, 0);
    myTree->Branch("Gen_Bc_4mom_phi", &Gen_Bc_4mom_phi, 32000, 0);
    myTree->Branch("Gen_Bc_4mom_m"  , &Gen_Bc_4mom_m  , 32000, 0);
    myTree->Branch("Gen_Bc_nuW_4mom_pt" , &Gen_Bc_nuW_4mom_pt , 32000, 0);
    myTree->Branch("Gen_Bc_nuW_4mom_eta", &Gen_Bc_nuW_4mom_eta, 32000, 0);
    myTree->Branch("Gen_Bc_nuW_4mom_phi", &Gen_Bc_nuW_4mom_phi, 32000, 0);
    myTree->Branch("Gen_Bc_nuW_4mom_m"  , &Gen_Bc_nuW_4mom_m  , 32000, 0);
    myTree->Branch("Gen_3mu_4mom_pt" , &Gen_3mu_4mom_pt , 32000, 0);
    myTree->Branch("Gen_3mu_4mom_eta", &Gen_3mu_4mom_eta, 32000, 0);
    myTree->Branch("Gen_3mu_4mom_phi", &Gen_3mu_4mom_phi, 32000, 0);
    myTree->Branch("Gen_3mu_4mom_m"  , &Gen_3mu_4mom_m  , 32000, 0);
  }
	myTree->Branch("Gen_Bc_QQ_idx",      Gen_Bc_QQ_idx,    "Gen_Bc_QQ_idx[Gen_Bc_size]/S");
	myTree->Branch("Gen_Bc_muW_idx",      Gen_Bc_muW_idx,    "Gen_Bc_muW_idx[Gen_Bc_size]/S");
	myTree->Branch("Gen_Bc_pdgId",      Gen_Bc_pdgId,    "Gen_Bc_pdgId[Gen_Bc_size]/I");
	myTree->Branch("Gen_Bc_ctau",      Gen_Bc_ctau,    "Gen_Bc_ctau[Gen_Bc_size]/F");

	myTree->Branch("Gen_3mu_whichRec", Gen_3mu_whichRec,   "Gen_3mu_whichRec[Gen_Bc_size]/S");
      }
    }

    myTree->Branch("Gen_mu_size",   &Gen_mu_size,  "Gen_mu_size/S");
    //myTree->Branch("Gen_mu_type",   Gen_mu_type,   "Gen_mu_type[Gen_mu_size]/S");
    myTree->Branch("Gen_mu_charge", Gen_mu_charge, "Gen_mu_charge[Gen_mu_size]/S");
    if(std::strcmp("array", _mom4format.c_str()) == 0){
      myTree->Branch("Gen_mu_4mom",   "TClonesArray", &Gen_mu_4mom, 32000, 0);
    }
    if(std::strcmp("vector", _mom4format.c_str()) == 0){
      myTree->Branch("Gen_mu_4mom_pt" , &Gen_mu_4mom_pt , 32000, 0);
      myTree->Branch("Gen_mu_4mom_eta", &Gen_mu_4mom_eta, 32000, 0);
      myTree->Branch("Gen_mu_4mom_phi", &Gen_mu_4mom_phi, 32000, 0);
      myTree->Branch("Gen_mu_4mom_m"  , &Gen_mu_4mom_m  , 32000, 0);
    }
    myTree->Branch("Gen_mu_whichRec", Gen_mu_whichRec,   "Gen_mu_whichRec[Gen_mu_size]/S");
  }

  return;
};