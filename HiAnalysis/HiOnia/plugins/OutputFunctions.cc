#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"

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

    myTree->Branch("Reco_QQQQ_size", &Reco_QQQQ_size,  "Reco_QQQQ_size/S");
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