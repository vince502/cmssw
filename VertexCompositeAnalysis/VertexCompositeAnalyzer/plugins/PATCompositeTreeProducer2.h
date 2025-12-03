
// system include files
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>

#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TString.h>
#include <TObjString.h>
#include <TCanvas.h>
#include <TVector3.h>
#include <TMatrixD.h>
#include <TRandom.h>
#include <TMath.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"

#include "DataFormats/Candidate/interface/Candidate.h"
// #include "DataFormats/Candidate/interface/PATCompositeCandidate.h"
// #include "DataFormats/Candidate/interface/PATCompositeCandidateFwd.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/TrackReco/interface/DeDxData.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonChamberMatch.h"
#include "DataFormats/MuonReco/interface/MuonSegmentMatch.h"
#include "DataFormats/HeavyIonEvent/interface/CentralityBins.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"

//#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h"
//#include "RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h"
//#include "RecoHI/HiEvtPlaneAlgos/interface/LoadEPDB.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>

//#define DEBUG true


//
// class decleration
//

#define PI 3.1416
#define MAXCAN 100

using namespace std;
using CC = pat::CompositeCandidate;
using CCC = pat::CompositeCandidateCollection;

class PATCompositeTreeProducer2 : public edm::one::EDAnalyzer<> {
public:
  explicit PATCompositeTreeProducer2(const edm::ParameterSet&);
  ~PATCompositeTreeProducer2();

  using MVACollection = std::vector<float>;

private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void fillRECO(const edm::Event&, const edm::EventSetup&) ;
  virtual void fillGEN(const edm::Event&, const edm::EventSetup&) ;
  virtual void endJob() ;
  virtual void initHistogram();
  virtual void initTree();
  void genDecayLength(const uint&, const reco::GenParticle&);

  bool matchHadron(const reco::Candidate* _dmeson_, const reco::GenParticle& _gen_, bool isMatchD0) const;
  bool matchHadron(const reco::Candidate* _dmeson_, const reco::Candidate& _gen_, bool isMatchD0) const;
  bool matchTrackdR(const reco::Candidate* _recoTrk_, const reco::Candidate* _genTrk_, bool chkchrg) const;
  bool checkSwap(const reco::Candidate* _dmeson_, const reco::GenParticle& _gen_) const;
  bool checkSwap(const reco::Candidate* _dmeson_, const reco::Candidate& _gen_) const;

  int muAssocToTrack( const reco::TrackRef& trackref, const edm::Handle<reco::MuonCollection>& muonh) const;

  reco::GenParticleRef findMother(const reco::GenParticleRef&);
  void genDecayLength(const reco::Candidate& gCand, float& gen_decayLength2D_, float& gen_decayLength3D_, float& gen_angle2D_, float& gen_angle3D_);
  void getAncestorId(const reco::Candidate& gCand, int& gen_ancestorId_, int& gen_ancestorFlavor_ );

  // ----------member data ---------------------------
    
    edm::Service<TFileService> fs;

    TTree* PATCompositeNtuple;
    TH2F*  hMassVsMVA[6][10];
    TH2F*  hpTVsMVA[6][10];
    TH2F*  hetaVsMVA[6][10];
    TH2F*  hyVsMVA[6][10];
    TH2F*  hVtxProbVsMVA[6][10];
    TH2F*  h3DCosPointingAngleVsMVA[6][10];
    TH2F*  h3DPointingAngleVsMVA[6][10];
    TH2F*  h2DCosPointingAngleVsMVA[6][10];
    TH2F*  h2DPointingAngleVsMVA[6][10];
    TH2F*  h3DDecayLengthSignificanceVsMVA[6][10];
    TH2F*  h3DDecayLengthVsMVA[6][10];
    TH2F*  h2DDecayLengthSignificanceVsMVA[6][10];
    TH2F*  h2DDecayLengthVsMVA[6][10];
    TH2F*  h3DDCAVsMVA[6][10];
    TH2F*  h2DDCAVsMVA[6][10];
    TH2F*  hzDCASignificanceDaugther1VsMVA[6][10];
    TH2F*  hxyDCASignificanceDaugther1VsMVA[6][10];
    TH2F*  hNHitD1VsMVA[6][10];
    TH2F*  hpTD1VsMVA[6][10];
    TH2F*  hpTerrD1VsMVA[6][10];
    TH2F*  hEtaD1VsMVA[6][10];
    TH2F*  hdedxHarmonic2D1VsMVA[6][10];
    TH2F*  hdedxHarmonic2D1VsP[6][10];
    TH2F*  hzDCASignificanceDaugther2VsMVA[6][10];
    TH2F*  hxyDCASignificanceDaugther2VsMVA[6][10];
    TH2F*  hNHitD2VsMVA[6][10];
    TH2F*  hpTD2VsMVA[6][10];
    TH2F*  hpTerrD2VsMVA[6][10];
    TH2F*  hEtaD2VsMVA[6][10];
    TH2F*  hdedxHarmonic2D2VsMVA[6][10];
    TH2F*  hdedxHarmonic2D2VsP[6][10];
    TH2F*  hzDCASignificanceDaugther3VsMVA[6][10];
    TH2F*  hxyDCASignificanceDaugther3VsMVA[6][10];
    TH2F*  hNHitD3VsMVA[6][10];
    TH2F*  hpTD3VsMVA[6][10];
    TH2F*  hpTerrD3VsMVA[6][10];
    TH2F*  hEtaD3VsMVA[6][10];
    TH2F*  hdedxHarmonic2D3VsMVA[6][10];
    TH2F*  hdedxHarmonic2D3VsP[6][10];
    
    bool   saveTree_;
    bool   saveHistogram_;
    bool   saveAllHistogram_;
    double massHistPeak_;
    double massHistWidth_;
    int    massHistBins_;

    //options
    bool doRecoNtuple_;
    bool doGenNtuple_;   
    bool doGenMatching_;
    bool doGenMatchingTOF_;
    bool hasSwap_;
    bool decayInGen_;
    bool twoLayerDecay_;
    bool threeProngDecay_;
    bool doMuon_;
    bool doMuonFull_;
    int PID_;
    int PID_dau1_;
    int PID_dau2_;
    int PID_dau3_;
    
    //cut variables
    double multMax_;
    double multMin_;
    double deltaR_; //deltaR for Gen matching

    vector<double> pTBins_;
    vector<double> yBins_;

    //tree branches
    //event info
    int centrality;
    int Ntrkoffline;
    int Npixel;
    float HFsumETPlus;
    float HFsumETMinus;
    float ZDCPlus;
    float ZDCMinus;
    float bestvx;
    float bestvy;
    float bestvz;
    int candSize;
    float ephfpAngle[3];
    float ephfmAngle[3];
    float ephfpQ[3];
    float ephfmQ[3];
    float ephfpSumW;
    float ephfmSumW;
    
    //Composite candidate info
    float mva[MAXCAN];
    float pt[MAXCAN];
    float eta[MAXCAN];
    float phi[MAXCAN];
    float flavor[MAXCAN];
    float y[MAXCAN];
    float mass[MAXCAN];
    float VtxProb[MAXCAN];
    float dlos[MAXCAN];
    float dl[MAXCAN];
    float dlerror[MAXCAN];
    float agl[MAXCAN];
    float vtxChi2[MAXCAN];
    float ndf[MAXCAN];
    float agl_abs[MAXCAN];
    float agl2D[MAXCAN];
    float agl2D_abs[MAXCAN];
    float dlos2D[MAXCAN];
    float dl2D[MAXCAN];
    float trk3Ddca[MAXCAN];
    float trk3DdcaErr[MAXCAN];
    float dca3D[MAXCAN];
    float dca3DErr[MAXCAN];
    bool isSwap[MAXCAN];
    bool matchGEN[MAXCAN];
    int idBAnc_reco[MAXCAN];
    int pionFlavor[MAXCAN];
    int idmom_reco[MAXCAN];
    float gen_agl_abs[MAXCAN];
    float gen_agl2D_abs[MAXCAN];
    float gen_dl[MAXCAN];
    float gen_dl2D[MAXCAN];
    
    //dau candidate info
    float grand_mass[MAXCAN];
    float grand_VtxProb[MAXCAN];
    float grand_dlos[MAXCAN];
    float grand_dl[MAXCAN];
    float grand_dlerror[MAXCAN];
    float grand_agl[MAXCAN];
    float grand_vtxChi2[MAXCAN];
    float grand_ndf[MAXCAN];
    float grand_agl_abs[MAXCAN];
    float grand_agl2D[MAXCAN];
    float grand_agl2D_abs[MAXCAN];
    float grand_dlos2D[MAXCAN];

    //dau info
    float dzos1[MAXCAN];
    float dzos2[MAXCAN];
    float dzos3[MAXCAN];
    float dxyos1[MAXCAN];
    float dxyos2[MAXCAN];
    float dxyos3[MAXCAN];
    float dzval1[MAXCAN];
    float dzval2[MAXCAN];
    float dzval3[MAXCAN];
    float dxyval1[MAXCAN];
    float dxyval2[MAXCAN];
    float dxyval3[MAXCAN];
    float nhit1[MAXCAN];
    float nhit2[MAXCAN];
    float nhit3[MAXCAN];
    bool trkquality1[MAXCAN];
    bool trkquality2[MAXCAN];
    bool trkquality3[MAXCAN];
    float pt1[MAXCAN];
    float pt2[MAXCAN];
    float pt3[MAXCAN];
    float ptErr1[MAXCAN];
    float ptErr2[MAXCAN];
    float ptErr3[MAXCAN];
    float p1[MAXCAN];
    float p2[MAXCAN];
    float p3[MAXCAN];
    float eta1[MAXCAN];
    float eta2[MAXCAN];
    float eta3[MAXCAN];
    float phi1[MAXCAN];
    float phi2[MAXCAN];
    float phi3[MAXCAN];
    int charge1[MAXCAN];
    int charge2[MAXCAN];
    int charge3[MAXCAN];
    int pid1[MAXCAN];
    int pid2[MAXCAN];
    int pid3[MAXCAN];
    float tof1[MAXCAN];
    float tof2[MAXCAN];
    float tof3[MAXCAN];
    float H2dedx1[MAXCAN];
    float H2dedx2[MAXCAN];
    float H2dedx3[MAXCAN];
    float T4dedx1[MAXCAN];
    float T4dedx2[MAXCAN];
    float T4dedx3[MAXCAN];
    float trkChi1[MAXCAN];
    float trkChi2[MAXCAN];
    float trkChi3[MAXCAN];
   
    //grand-dau info
    float grand_dzos1[MAXCAN];
    float grand_dzos2[MAXCAN];
    float grand_dxyos1[MAXCAN];
    float grand_dxyos2[MAXCAN];
    float grand_nhit1[MAXCAN];
    float grand_nhit2[MAXCAN];
    bool grand_trkquality1[MAXCAN];
    bool grand_trkquality2[MAXCAN];
    float grand_pt1[MAXCAN];
    float grand_pt2[MAXCAN];
    float grand_ptErr1[MAXCAN];
    float grand_ptErr2[MAXCAN];
    float grand_p1[MAXCAN];
    float grand_p2[MAXCAN];
    float grand_eta1[MAXCAN];
    float grand_eta2[MAXCAN];
    int grand_charge1[MAXCAN];
    int grand_charge2[MAXCAN];
    float grand_H2dedx1[MAXCAN];
    float grand_H2dedx2[MAXCAN];
    float grand_T4dedx1[MAXCAN];
    float grand_T4dedx2[MAXCAN];
    float grand_trkChi1[MAXCAN];
    float grand_trkChi2[MAXCAN];
    
    //dau muon info
    bool  onestmuon1[MAXCAN];
    bool  onestmuon2[MAXCAN];
    bool  pfmuon1[MAXCAN];
    bool  pfmuon2[MAXCAN];
    bool  glbmuon1[MAXCAN];
    bool  glbmuon2[MAXCAN];
    bool  trkmuon1[MAXCAN];
    bool  trkmuon2[MAXCAN];
    bool  calomuon1[MAXCAN];
    bool  calomuon2[MAXCAN];
    bool  softmuon1[MAXCAN];
    bool  softmuon2[MAXCAN];
    float nmatchedst1[MAXCAN];
    float nmatchedch1[MAXCAN];
    float ntrackerlayer1[MAXCAN];
    float npixellayer1[MAXCAN];
    float matchedenergy1[MAXCAN];
    float nmatchedst2[MAXCAN];
    float nmatchedch2[MAXCAN];
    float ntrackerlayer2[MAXCAN];
    float npixellayer2[MAXCAN];
    float matchedenergy2[MAXCAN];
    float dx1_seg_[MAXCAN];
    float dy1_seg_[MAXCAN];
    float dxSig1_seg_[MAXCAN];
    float dySig1_seg_[MAXCAN];
    float ddxdz1_seg_[MAXCAN];
    float ddydz1_seg_[MAXCAN];
    float ddxdzSig1_seg_[MAXCAN];
    float ddydzSig1_seg_[MAXCAN];
    float dx2_seg_[MAXCAN];
    float dy2_seg_[MAXCAN];
    float dxSig2_seg_[MAXCAN];
    float dySig2_seg_[MAXCAN];
    float ddxdz2_seg_[MAXCAN];
    float ddydz2_seg_[MAXCAN];
    float ddxdzSig2_seg_[MAXCAN];
    float ddydzSig2_seg_[MAXCAN];

    // gen info    
    int candSize_gen;
    float mass_gen[MAXCAN];
    float pt_gen[MAXCAN];
    float eta_gen[MAXCAN];
    float phi_gen[MAXCAN];
    int status_gen[MAXCAN];
    int pdgId_gen[MAXCAN];
    int charge_gen[MAXCAN];
    int idmom[MAXCAN];
    float y_gen[MAXCAN];
    int iddau1[MAXCAN];
    int iddau2[MAXCAN];
    int iddau3[MAXCAN];

    float matchGen_DStarpT_[MAXCAN];
		float matchGen_DStareta_[MAXCAN];
		float matchGen_DStarphi_[MAXCAN];
		float matchGen_DStarmass_[MAXCAN];
		float matchGen_DStary_[MAXCAN];
		int matchGen_DStarcharge_[MAXCAN];
		int matchGen_DStarpdgId_[MAXCAN];

		float matchGen_D0pT_[MAXCAN];
		float matchGen_D0eta_[MAXCAN];
		float matchGen_D0phi_[MAXCAN];
		float matchGen_D0mass_[MAXCAN];
		float matchGen_D0y_[MAXCAN];
		int matchGen_D0charge_[MAXCAN];
		int matchGen_D0pdgId_[MAXCAN];

		float matchGen_D0Dau1_pT_[MAXCAN];
		float matchGen_D0Dau1_eta_[MAXCAN];
		float matchGen_D0Dau1_phi_[MAXCAN];
		float matchGen_D0Dau1_mass_[MAXCAN];
		float matchGen_D0Dau1_y_[MAXCAN];
		int matchGen_D0Dau1_charge_[MAXCAN];
		int matchGen_D0Dau1_pdgId_[MAXCAN];

		float matchGen_D0Dau2_pT_[MAXCAN];
		float matchGen_D0Dau2_eta_[MAXCAN];
		float matchGen_D0Dau2_phi_[MAXCAN];
		float matchGen_D0Dau2_mass_[MAXCAN];
		float matchGen_D0Dau2_y_[MAXCAN];
		int matchGen_D0Dau2_charge_[MAXCAN];
		int matchGen_D0Dau2_pdgId_[MAXCAN];

		float matchGen_D1pT_[MAXCAN];
		float matchGen_D1eta_[MAXCAN];
		float matchGen_D1phi_[MAXCAN];
		float matchGen_D1mass_[MAXCAN];
		float matchGen_D1y_[MAXCAN];
		float matchGen_D1decayLength2D_[MAXCAN];
		float matchGen_D1decayLength3D_[MAXCAN];
		float matchGen_D1angle2D_[MAXCAN];
		float matchGen_D1angle3D_[MAXCAN];
		int matchGen_D1ancestorId_[MAXCAN];
		int matchGen_D1ancestorFlavor_[MAXCAN];
		int matchGen_D1charge_[MAXCAN];
		int matchGen_D1pdgId_[MAXCAN];


		float gen_D0pT_[MAXCAN];
		float gen_D0eta_[MAXCAN];
		float gen_D0phi_[MAXCAN];
		float gen_D0mass_[MAXCAN];
		float gen_D0y_[MAXCAN];
		int gen_D0charge_[MAXCAN];
		int gen_D0pdgId_[MAXCAN];
		int gen_D0ancestorId_[MAXCAN];
		int gen_D0ancestorFlavor_[MAXCAN];

		float gen_D0Dau1_pT_[MAXCAN];
		float gen_D0Dau1_eta_[MAXCAN];
		float gen_D0Dau1_phi_[MAXCAN];
		float gen_D0Dau1_mass_[MAXCAN];
		float gen_D0Dau1_y_[MAXCAN];
		int gen_D0Dau1_charge_[MAXCAN];
		int gen_D0Dau1_pdgId_[MAXCAN];

		float gen_D0Dau2_pT_[MAXCAN];
		float gen_D0Dau2_eta_[MAXCAN];
		float gen_D0Dau2_phi_[MAXCAN];
		float gen_D0Dau2_mass_[MAXCAN];
		float gen_D0Dau2_y_[MAXCAN];
		int gen_D0Dau2_charge_[MAXCAN];
		int gen_D0Dau2_pdgId_[MAXCAN];

		float gen_D1pT_[MAXCAN];
		float gen_D1eta_[MAXCAN];
		float gen_D1phi_[MAXCAN];
		float gen_D1mass_[MAXCAN];
		float gen_D1y_[MAXCAN];
		int gen_D1charge_[MAXCAN];
		int gen_D1pdgId_[MAXCAN];

    //vector for gen match
    vector< vector<double> > *pVect;
    vector< vector<double> > *gpVect;
    vector<double> *Dvector1;
    vector<double> *GDvector1;
    vector<double> *Dvector2;
    vector<double> *GDvector2;
    vector<double> *Dvector3;
    vector<int> *pVectIDmom;
    
    bool useAnyMVA_;
    bool isSkimMVA_;
    bool isCentrality_;
    bool isEventPlane_;

    edm::Handle<int> cbin_;

    //tokens
    edm::EDGetTokenT<reco::VertexCollection> tok_offlinePV_;
    edm::EDGetTokenT<reco::TrackCollection> tok_generalTrk_;
    edm::EDGetTokenT<CCC> PATCompositeCandidateCollection_Token_;
    edm::EDGetTokenT<MVACollection> MVAValues_Token_;

    edm::EDGetTokenT<edm::ValueMap<reco::DeDxData> > Dedx_Token1_;
    edm::EDGetTokenT<edm::ValueMap<reco::DeDxData> > Dedx_Token2_;
    edm::EDGetTokenT<reco::GenParticleCollection> tok_genParticle_;
    edm::EDGetTokenT<reco::MuonCollection> tok_muon_;

    edm::EDGetTokenT<int> tok_centBinLabel_;
    edm::EDGetTokenT<reco::Centrality> tok_centSrc_;

    edm::EDGetTokenT<reco::EvtPlaneCollection> tok_eventplaneSrc_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//



bool PATCompositeTreeProducer2::matchHadron(const reco::Candidate* _dmeson_, const reco::GenParticle& _gen_, bool isMatchD0) const {
  bool match = false;
  if(isMatchD0){
    reco::Candidate const* reco_trk1 = _dmeson_->daughter(0);
    reco::Candidate const* reco_trk2 = _dmeson_->daughter(1);

    reco::Candidate const* gen_trk1 = _gen_.daughter(0);
    reco::Candidate const* gen_trk2 = _gen_.daughter(1);

    if( matchTrackdR(reco_trk1, gen_trk1, true)){
        if( matchTrackdR(reco_trk2, gen_trk2,true)) {
            match = true;
            return match;
        }
    }    
    if( matchTrackdR(reco_trk2, gen_trk1, true)){
        if( matchTrackdR(reco_trk1, gen_trk2,true)) {
            match = true;
            return match;
        }
    }    
  }
  if(!isMatchD0){
    if(matchTrackdR(_dmeson_, &_gen_,true)) match = true;
  }
  return match;
};
bool PATCompositeTreeProducer2::matchHadron(const reco::Candidate* _dmeson_, const reco::Candidate& _gen_, bool isMatchD0) const {
  bool match = false;
  if(isMatchD0){
    reco::Candidate const* reco_trk1 = _dmeson_->daughter(0);
    reco::Candidate const* reco_trk2 = _dmeson_->daughter(1);

    reco::Candidate const* gen_trk1 = _gen_.daughter(0);
    reco::Candidate const* gen_trk2 = _gen_.daughter(1);

    if( matchTrackdR(reco_trk1, gen_trk1, true)){
        if( matchTrackdR(reco_trk2, gen_trk2,true)) {
            match = true;
            return match;
        }
    }    
    if( matchTrackdR(reco_trk2, gen_trk1, true)){
        if( matchTrackdR(reco_trk1, gen_trk2,true)) {
            match = true;
            return match;
        }
    }    
  }
  if(!isMatchD0){
    if(matchTrackdR(_dmeson_, &_gen_,true)) match = true;
  }
  return match;
};

bool PATCompositeTreeProducer2::checkSwap(const reco::Candidate* _dmeson_, const reco::GenParticle& _gen_) const {
    return _dmeson_->pdgId() != _gen_.pdgId();
};
bool PATCompositeTreeProducer2::checkSwap(const reco::Candidate* _dmeson_, const reco::Candidate& _gen_) const {
    return _dmeson_->pdgId() != _gen_.pdgId();
};

bool PATCompositeTreeProducer2::matchTrackdR(const reco::Candidate* _recoTrk_, const reco::Candidate* _genTrk_, bool chkchrg=true) const {
    bool pass= false;
    // _deltaR_
    if(chkchrg && (_recoTrk_->charge() != _genTrk_->charge())) return false;
    const double dR = reco::deltaR(*_recoTrk_, *_genTrk_);
    if(dR < deltaR_) pass = true;
    return pass;
};


reco::GenParticleRef PATCompositeTreeProducer2::findMother(const reco::GenParticleRef& genParRef)
{
  if(genParRef.isNull()) return genParRef;
  reco::GenParticleRef genMomRef = genParRef;
  int pdg = genParRef->pdgId(); const int pdg_OLD = pdg;
  while(pdg==pdg_OLD && genMomRef->numberOfMothers()>0)
  {
    genMomRef = genMomRef->motherRef(0);
    pdg = genMomRef->pdgId();
  }
  if(pdg==pdg_OLD) genMomRef = reco::GenParticleRef();
  return genMomRef;
};

void PATCompositeTreeProducer2::genDecayLength(const reco::Candidate& gCand, float& gen_decayLength2D_, float& gen_decayLength3D_, float& gen_angle2D_, float& gen_angle3D_){
  gen_decayLength2D_ = -99.;
  gen_decayLength3D_ = -99.;
  gen_angle2D_ = -99;
  gen_angle3D_ = -99;

  if(gCand.numberOfDaughters()==0 || !gCand.daughter(0)) return;
  const auto& dauVtx = gCand.daughter(0)->vertex();
  const auto& genVertex_ = gCand.vertex();
  TVector3 ptosvec(dauVtx.X() - genVertex_.x(), dauVtx.Y() - genVertex_.y(), dauVtx.Z() - genVertex_.z());
  TVector3 secvec(gCand.px(), gCand.py(), gCand.pz());
  gen_angle3D_ = secvec.Angle(ptosvec);
  gen_decayLength3D_ = ptosvec.Mag();
  TVector3 ptosvec2D(dauVtx.X() - genVertex_.x(), dauVtx.Y() - genVertex_.y(), 0.0);
  TVector3 secvec2D(gCand.px(), gCand.py(), 0.0);
  gen_angle2D_ = secvec2D.Angle(ptosvec2D);
  gen_decayLength2D_ = ptosvec2D.Mag();
};

void PATCompositeTreeProducer2::getAncestorId(const reco::Candidate& gCand, int& gen_ancestorId_, int& gen_ancestorFlavor_ ){
  gen_ancestorId_ = 0;
  gen_ancestorFlavor_ = 0;
//  reco::GenParticle gCand1(gCand.charge(),gCand.p4(),gCand.vertex(),421,2,true);
  //for (auto mothers = gCand.motherRefVector();
  //    !mothers.empty(); ) {
  //  auto mom = mothers.at(0);
  //  mothers = mom->motherRefVector();
  //  gen_ancestorId_ = mom->pdgId();
  //  cout << "gen_ancestorId_ : " << gen_ancestorId_ << endl;
  //  const auto idstr = std::to_string(std::abs(gen_ancestorId_));
  //  gen_ancestorFlavor_ = std::stoi(std::string{idstr.begin(), idstr.begin()+1});
  //  cout << "gen_ancestorFlavor_ : " << gen_ancestorFlavor_ << endl;
  //  if (idstr[0] == '5') {
  //    break;
  //  }
  //  if (std::abs(gen_ancestorId_) <= 40) break;
  //}
  for (auto mom = gCand.mother(); !(mom==nullptr);){
    gen_ancestorId_= mom->pdgId();
    const auto idstr = std::to_string(std::abs(gen_ancestorId_));
    gen_ancestorFlavor_ = std::stoi(std::string{idstr.begin(), idstr.begin()+1});
    if (idstr.find('5') != std::string::npos) {
      break;
    }
    if (std::abs(gen_ancestorId_) <= 40) break;
          mom = mom->mother();
  }

          
};
