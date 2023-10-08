// -*- C++ -*-
//
// Package:    HiOniaAnalyzer
// Class:      HiOniaAnalyzer
// 
/**\class HiOniaAnalyzer HiOniaAnalyzer.cc UserCode/tdahms/HiAnalysis/HiOnia/plugins/HiOniaAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/

// system include files
#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"


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


//
// member functions
//

// ------------ method called to for each event  ------------
void
HiOniaAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //   using namespace edm;
  InitEvent();
  nEvents++;
  hStats->Fill(BIN_nEvents);
   
  runNb = iEvent.id().run();
  eventNb = iEvent.id().event();
  lumiSection = iEvent.luminosityBlock();

  edm::Handle<reco::VertexCollection> privtxs;
  iEvent.getByToken(_thePVsToken, privtxs); 
  reco::VertexCollection::const_iterator privtx;

  if (privtxs.isValid()){
    nPV = privtxs->size();
  
    if ( privtxs->begin() != privtxs->end() ) {
      privtx=privtxs->begin();
      RefVtx = privtx->position();
      RefVtx_xError = privtx->xError();
      RefVtx_yError = privtx->yError();
      RefVtx_zError = privtx->zError();
    } else {
      RefVtx.SetXYZ(0.,0.,0.);
      RefVtx_xError = 0.0;
      RefVtx_yError = 0.0;
      RefVtx_zError = 0.0;
    }

    zVtx = RefVtx.Z();

    hZVtx->Fill(zVtx);
    hPileUp->Fill(nPV);
  }
  else {
    std::cout<<"ERROR: privtxs is NULL or not isValid ! Return now"<<std::endl; return;
  }

  this->hltReport(iEvent, iSetup);

  for (unsigned int iTr = 1 ; iTr < theTriggerNames.size() ; iTr++) {
    if (mapTriggerNameToIntFired_[theTriggerNames.at(iTr)] == 3) {
      HLTriggers += pow(2,iTr-1);
      hStats->Fill(iTr); // event info
    }
    trigPrescale[iTr-1] = mapTriggerNameToPrescaleFac_[theTriggerNames.at(iTr)];
  }

  edm::Handle<reco::Centrality> centrality;
  edm::Handle<int> cbin_;
  if (_isHI || _isPA)  {
    iEvent.getByToken(_centralityTagToken, centrality); 
    iEvent.getByToken(_centralityBinTagToken, cbin_);
  }
  if (centrality.isValid() && cbin_.isValid()) {
    centBin = *cbin_;
    hCent->Fill(centBin);
    
    for (unsigned int iCent=0; iCent<_centralityranges.size(); ++iCent) {
      if ( (_isHI && centBin<_centralityranges.at(iCent)/0.5) ||
           (_isPA && centBin<_centralityranges.at(iCent)) ) {
        theCentralityBin=iCent;
        break;
      }
    }

    Npix            = (Short_t)centrality->multiplicityPixel();
    NpixelTracks    = (Short_t)centrality->NpixelTracks();
    Ntracks         = (Short_t)centrality->Ntracks();
    NtracksPtCut    = centrality->NtracksPtCut();
    NtracksEtaCut   = centrality->NtracksEtaCut();
    NtracksEtaPtCut = centrality->NtracksEtaPtCut();

    if(_SumETvariables){
      SumET_HF          = centrality->EtHFtowerSum();
      SumET_HFplus      = centrality->EtHFtowerSumPlus();
      SumET_HFminus     = centrality->EtHFtowerSumMinus();
      SumET_HFplusEta4  = centrality->EtHFtruncatedPlus();
      SumET_HFminusEta4 = centrality->EtHFtruncatedMinus();

      SumET_HFhit       = centrality->EtHFhitSum(); 
      SumET_HFhitPlus   = centrality->EtHFhitSumPlus();
      SumET_HFhitMinus  = centrality->EtHFhitSumMinus();

      SumET_ZDC         = centrality->zdcSum();
      SumET_ZDCplus     = centrality->zdcSumPlus();
      SumET_ZDCminus    = centrality->zdcSumMinus();

      SumET_EEplus      = centrality->EtEESumPlus();
      SumET_EEminus     = centrality->EtEESumMinus();
      SumET_EE          = centrality->EtEESum();
      SumET_EB          = centrality->EtEBSum();
      SumET_ET          = centrality->EtMidRapiditySum();
    }
  }
  else {
    centBin = 0;
    theCentralityBin=0;

    Npix = 0;
    NpixelTracks = 0;
    Ntracks = 0;
    NtracksPtCut    = 0;
    NtracksEtaCut   = 0;
    NtracksEtaPtCut = 0;

    SumET_HF = 0;
    SumET_HFplus = 0;
    SumET_HFminus = 0;
    SumET_HFplusEta4 = 0;
    SumET_HFminusEta4 = 0;

    SumET_HFhit       = 0;
    SumET_HFhitPlus   = 0; 
    SumET_HFhitMinus  = 0;

    SumET_ZDC = 0;
    SumET_ZDCplus = 0;
    SumET_ZDCminus = 0;
    SumET_EEplus = 0;
    SumET_EEminus = 0;
    SumET_EE = 0;
    SumET_EB = 0;
    SumET_ET = 0;
  }

  if ((_isHI || _isPA) && _useEvtPlane) {
    nEP = 0; 
    edm::Handle<reco::EvtPlaneCollection> flatEvtPlanes;
    iEvent.getByToken(_evtPlaneTagToken,flatEvtPlanes);
    if(flatEvtPlanes.isValid()) {
      for (reco::EvtPlaneCollection::const_iterator rp = flatEvtPlanes->begin(); rp!=flatEvtPlanes->end(); rp++) {
        rpAng[nEP] = rp->angle(2);   // Using Event Plane Level 2 -> Includes recentering and flattening. 
        rpSin[nEP] = rp->sumSin(2);  // Using Event Plane Level 2 -> Includes recentering and flattening. 
        rpCos[nEP] = rp->sumCos(2);  // Using Event Plane Level 2 -> Includes recentering and flattening. 
        nEP++;
      }
    } else if (!_isMC) {
      std::cout << "Warning! Can't get flattened hiEvtPlane product!" << std::endl;
    }
  }

  iEvent.getByToken(_patJpsiToken,collJpsi); 
  iEvent.getByToken(_patDiOniaToken,collDiOnia); 
  if(_doTrimuons)
    iEvent.getByToken(_patTrimuonToken,collTrimuon);
  if(_doDimuTrk)
    iEvent.getByToken(_patDimuTrkToken,collDimutrk);
  iEvent.getByToken(_patMuonToken,collMuon);
  iEvent.getByToken(_patMuonNoTrigToken,collMuonNoTrig);

  if (_useSVfinder)
    iEvent.getByToken(_SVToken,SVs);

  if(!_onlySingleMuons){
    // APPLY CUTS
    this->makeCuts(_storeSs);

    // APPLY CUTS for Bc (trimuon)
    if(_doTrimuons)
      this->makeBcCuts(_storeSs);

    // APPLY CUTS for Bc (dimuon+track)
    if(_doDimuTrk)
      this->makeDimutrkCuts(_storeSs);
  }

  if(_fillSingleMuons || !_AtLeastOneCand || !_doTrimuons || !_isMC || _thePassedBcCands.size()>0){ //not storing the mu reconstructed info if we do a trimuon MC and there is no reco trimuon
    //_fillSingleMuons is checked within the fillRecoMuons function: the info on the wanted muons was stored in the makeCuts function
    this->fillRecoMuons(theCentralityBin);

    if (_useGeTracks){
      iEvent.getByToken(_recoTracksToken,collTracks);
      if (_fillRecoTracks){
        if(!collTracks.isValid()){
          cout<<" collTrack is not valid !!!! Abandoning fillRecoTracks()"<<endl;}
        else this->fillRecoTracks();}
    }
  }

  if(!_onlySingleMuons) this->fillRecoHistos();

  //for pp, record Ntracks as well
  if(!(_isHI) && !(_isPA)){
    iEvent.getByToken(_recoTracksToken,collTracks);
    if(collTracks.isValid()){
      for(unsigned int tidx=0; tidx<collTracks->size();tidx++) {
  	const reco::TrackRef track(collTracks, tidx);
  	if ( track->qualityByName("highPurity") && track->eta()<2.4 && fabs(track->dxy(RefVtx)/track->dxyError())<3 && fabs(track->dz(RefVtx)/track->dzError())<3 && track->dz(RefVtx)<0.5 && fabs(track->ptError()/track->pt())<0.1) {
  	  Ntracks++;
  	}
      }
    }
  }

  if (_isMC) {
    //GEN info
    iEvent.getByToken(_genParticleToken,collGenParticles);
    iEvent.getByToken(_genInfoToken,genInfo);
    this->fillGenInfo();

    //MC MATCHING info
    this->fillMuMatchingInfo(); //Needs to be done after fillGenInfo, and the filling of reco muons collections
    if(!_onlySingleMuons) this->fillQQMatchingInfo(); //Needs to be done after fillMuMatchingInfo
    if(_doTrimuons || _doDimuTrk){
      if(!_onlySingleMuons) this->fillBcMatchingInfo(); //Needs to be done after fillQQMatchingInfo
    }
  }

  //keeping events with at least ONE CANDIDATE when asked
  bool oneGoodCand = !_AtLeastOneCand; //if !_AtLeastOneCand, pass in all cases
  if(_AtLeastOneCand){
    if (_doTrimuons || _doDimuTrk){
      if(Reco_3mu_size>0) oneGoodCand = true;}
    else if (Reco_QQ_size>0) oneGoodCand = true;
  }
  
  // ---- Fill the tree with this event only if AtLeastOneCand=false OR if there is at least one dimuon candidate in the event (or at least one trimuon cand if doTrimuons=true) ---- 
  if (_fillTree && oneGoodCand )
    myTree->Fill();
  
  return;
}


void
HiOniaAnalyzer::fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits) {
  if (Reco_mu_size >= Max_mu_size) {
    std::cout << "Too many muons: " << Reco_mu_size << std::endl;
    std::cout << "Maximum allowed: " << Max_mu_size << std::endl;
    return;
  }

  if (muon!=NULL){
    Reco_mu_charge[Reco_mu_size] = muon->charge();
    Reco_mu_type[Reco_mu_size] = iType;
  
    TLorentzVector vMuon = lorentzMomentum(muon->p4());
    new((*Reco_mu_4mom)[Reco_mu_size])TLorentzVector(vMuon);
    Reco_mu_4mom_pt.push_back(vMuon.Pt());
    Reco_mu_4mom_eta.push_back(vMuon.Eta());
    Reco_mu_4mom_phi.push_back(vMuon.Phi());
    Reco_mu_4mom_m.push_back(vMuon.M());


    TLorentzVector vMuonL1;
    if(muon->hasUserFloat("l1Eta") && muon->hasUserFloat("l1Phi")){
      vMuonL1.SetPtEtaPhiM(vMuon.Pt(), muon->userFloat("l1Eta"), muon->userFloat("l1Phi"), vMuon.M());
    }
    else{
      vMuonL1.SetPtEtaPhiM(0,0,0,0);
    }
    new((*Reco_mu_L1_4mom)[Reco_mu_size])TLorentzVector(vMuonL1);
    Reco_mu_L1_4mom_pt.push_back(vMuonL1.Pt());
    Reco_mu_L1_4mom_eta.push_back(vMuonL1.Eta());
    Reco_mu_L1_4mom_phi.push_back(vMuonL1.Phi());
    Reco_mu_L1_4mom_m.push_back(vMuonL1.M());

    //Fill map of the muon indices. Use long int keys, to avoid rounding errors on a float key. Implies a precision of 10^-6
    mapMuonMomToIndex_[ FloatToIntkey(vMuon.Pt()) ] = Reco_mu_size;

    Reco_mu_trig[Reco_mu_size] = trigBits;

    reco::TrackRef iTrack = muon->innerTrack();
    reco::TrackRef bestTrack = muon->muonBestTrack();
  
    if (!_theMinimumFlag) {
      Reco_mu_InTightAcc[Reco_mu_size] = isMuonInAccept(muon,"GLB");
      Reco_mu_InLooseAcc[Reco_mu_size] = isMuonInAccept(muon,"GLBSOFT");
      Reco_mu_SelectionType[Reco_mu_size] = muonIDmask(muon);
      Reco_mu_StationsMatched[Reco_mu_size] = muon->numberOfMatchedStations();
      Reco_mu_isPF[Reco_mu_size] = muon->isPFMuon();
      Reco_mu_isTracker[Reco_mu_size] = muon->isTrackerMuon();
      Reco_mu_isGlobal[Reco_mu_size] = muon->isGlobalMuon();
      Reco_mu_isSoftCutBased[Reco_mu_size] = muon->passed(reco::Muon::SoftCutBasedId);
      Reco_mu_isHybridSoft[Reco_mu_size] = isHybridSoftMuon(muon);
      Reco_mu_isMedium[Reco_mu_size] = muon->passed(reco::Muon::CutBasedIdMedium);
      Reco_mu_isTightCutBased[Reco_mu_size] = muon->passed(reco::Muon::CutBasedIdTight);
      Reco_mu_candType[Reco_mu_size] = (Short_t)(muon->hasUserInt("candType"))?(muon->userInt("candType")):(-1);
      
      Reco_mu_TMOneStaTight[Reco_mu_size] = muon::isGoodMuon(*muon, muon::TMOneStationTight);

      Reco_mu_localChi2[Reco_mu_size] = muon->combinedQuality().chi2LocalPosition;
      Reco_mu_kink[Reco_mu_size] = muon->combinedQuality().trkKink;
      Reco_mu_segmentComp[Reco_mu_size] = muon->segmentCompatibility(reco::Muon::SegmentAndTrackArbitration);

      Reco_mu_normChi2_bestTracker[Reco_mu_size] = bestTrack->normalizedChi2();

      if (!iTrack.isNull()){
        Reco_mu_highPurity[Reco_mu_size] = iTrack->quality(reco::TrackBase::highPurity);
        Reco_mu_nTrkHits[Reco_mu_size] = iTrack->found();
        Reco_mu_normChi2_inner[Reco_mu_size] = (muon->hasUserFloat("trackChi2") ? muon->userFloat("trackChi2") : iTrack->normalizedChi2());
        Reco_mu_nPixValHits[Reco_mu_size] = iTrack->hitPattern().numberOfValidPixelHits();
        Reco_mu_nPixWMea[Reco_mu_size] = iTrack->hitPattern().pixelLayersWithMeasurement();
        Reco_mu_nTrkWMea[Reco_mu_size] = iTrack->hitPattern().trackerLayersWithMeasurement();
        Reco_mu_dxy[Reco_mu_size] = iTrack->dxy(RefVtx);
        Reco_mu_dxyErr[Reco_mu_size] = iTrack->dxyError();
        Reco_mu_dz[Reco_mu_size] = iTrack->dz(RefVtx);
        Reco_mu_dzErr[Reco_mu_size] = iTrack->dzError();
        //Reco_mu_pt_inner[Reco_mu_size] = iTrack->pt();
        Reco_mu_ptErr_inner[Reco_mu_size] = iTrack->ptError();
        Reco_mu_validFraction[Reco_mu_size] = iTrack->validFraction();
      }
      else if(_muonSel!=(std::string)("All")){
	std::cout<<"ERROR: 'iTrack' pointer in fillTreeMuon is NULL ! Return now"<<std::endl; return;
      }

      if (muon->isGlobalMuon()) {
        reco::TrackRef gTrack = muon->globalTrack();
        Reco_mu_nMuValHits[Reco_mu_size] = gTrack->hitPattern().numberOfValidMuonHits();
        Reco_mu_normChi2_global[Reco_mu_size] = gTrack->normalizedChi2();
        //Reco_mu_pt_global[Reco_mu_size] = gTrack->pt();
        //Reco_mu_ptErr_global[Reco_mu_size] = gTrack->ptError();
      }
      else {
        Reco_mu_nMuValHits[Reco_mu_size] = -1;
        Reco_mu_normChi2_global[Reco_mu_size] = 999;
        //Reco_mu_pt_global[Reco_mu_size] = -1;
        //Reco_mu_ptErr_global[Reco_mu_size] = -1;
      }
    }

    if(_isMC){
      Reco_mu_pTrue[Reco_mu_size] = ( (muon->genParticleRef()).isNonnull() )?
	((float)(muon->genParticleRef())->p()):
	(-1);
      if(_genealogyInfo){
	Reco_mu_simExtType[Reco_mu_size] = muon->simExtType();
      }
    }

  }
  else {
    std::cout<<"ERROR: 'muon' pointer in fillTreeMuon is NULL ! Return now"<<std::endl; return;
  }

  Reco_mu_size++;
  return;
}

void
HiOniaAnalyzer::fillTreeJpsi(int count) {
  if (Reco_QQ_size >= Max_QQ_size) {
    std::cout << "Too many dimuons: " << Reco_QQ_size << std::endl;
    std::cout << "Maximum allowed: " << Max_QQ_size << std::endl;
    return;
  }

  const pat::CompositeCandidate* aJpsiCand = _thePassedCands.at(count);

  if (aJpsiCand!=NULL){
    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));

    ULong64_t trigBits=0;
    for (unsigned int iTr=1; iTr<NTRIGGERS; ++iTr) {
      if (isTriggerMatched[iTr]) {trigBits += pow(2,iTr-1);}
    }

    if (muon1==NULL || muon2==NULL){
      std::cout<<"ERROR: 'muon1' or 'muon2' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;
    } else {

      Reco_QQ_sign[Reco_QQ_size] = muon1->charge() + muon2->charge();
      Reco_QQ_type[Reco_QQ_size] = _thePassedCats.at(count);

      Reco_QQ_trig[Reco_QQ_size] = trigBits;

      if (!(_isHI) && _muonLessPrimaryVertex && aJpsiCand->hasUserData("muonlessPV")) {
	RefVtx = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).position();
	RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).xError();
	RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).yError();
	RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).zError();
      }
      else if (!_muonLessPrimaryVertex && aJpsiCand->hasUserData("PVwithmuons")) {
	RefVtx = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).position();
	RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).xError();
	RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).yError();
	RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).zError();
      }
      else {
	cout << "HiOniaAnalyzer::fillTreeJpsi: no PVfor muon pair stored" << endl;
	return;
      }

      new((*Reco_QQ_vtx)[Reco_QQ_size])TVector3(RefVtx.X(),RefVtx.Y(),RefVtx.Z());

      
      TLorentzVector vMuon1 = lorentzMomentum(muon1->p4());
      TLorentzVector vMuon2 = lorentzMomentum(muon2->p4());
      
      reco::Track iTrack_mupl, iTrack_mumi, mu1Trk, mu2Trk;
      if(_flipJpsiDirection>0 && aJpsiCand->hasUserData("muon1Track") && aJpsiCand->hasUserData("muon2Track")){
      	mu1Trk = *(aJpsiCand->userData<reco::Track>("muon1Track"));
      	mu2Trk = *(aJpsiCand->userData<reco::Track>("muon2Track"));
      }

      Reco_QQ_flipJpsi[Reco_QQ_size] = _flipJpsiDirection;
      if(aJpsiCand->hasUserInt("flipJpsi")) Reco_QQ_flipJpsi[Reco_QQ_size] = aJpsiCand->userInt("flipJpsi");

      if((muon1->innerTrack()).isNull() || (muon2->innerTrack()).isNull()){
	std::cout<<"ERROR: 'iTrack_mupl' or 'iTrack_mumi' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;}

      if (muon1->charge() > muon2->charge()) {

	Reco_QQ_mupl_idx[Reco_QQ_size] = IndexOfThisMuon(&vMuon1); //needs the non-flipped muon momentum
	Reco_QQ_mumi_idx[Reco_QQ_size] = IndexOfThisMuon(&vMuon2);

	if(_flipJpsiDirection>0){
	  iTrack_mupl = mu1Trk;
	  iTrack_mumi = mu2Trk;
	  new((*Reco_QQ_mupl_4mom)[Reco_QQ_size])TLorentzVector(mu1Trk.px(),mu1Trk.py(),mu1Trk.pz(),vMuon1.E());  //only the direction of the 3-momentum changes
    Reco_QQ_mupl_4mom_pt.push_back(mu1Trk.pt());
    Reco_QQ_mupl_4mom_eta.push_back(mu1Trk.eta());
    Reco_QQ_mupl_4mom_phi.push_back(mu1Trk.phi());
    Reco_QQ_mupl_4mom_m.push_back(vMuon1.M());

	  new((*Reco_QQ_mumi_4mom)[Reco_QQ_size])TLorentzVector(mu2Trk.px(),mu2Trk.py(),mu2Trk.pz(),vMuon2.E());
    Reco_QQ_mumi_4mom_pt.push_back(mu2Trk.pt());
    Reco_QQ_mumi_4mom_eta.push_back(mu2Trk.eta());
    Reco_QQ_mumi_4mom_phi.push_back(mu2Trk.phi());
    Reco_QQ_mumi_4mom_m.push_back(vMuon2.M());

	} else if(_muonLessPrimaryVertex || _useGeTracks){
	  iTrack_mupl = *(muon1->innerTrack());
	  iTrack_mumi = *(muon2->innerTrack());
	}
	
      }
      else {

	Reco_QQ_mupl_idx[Reco_QQ_size] = IndexOfThisMuon(&vMuon2); //needs the non-flipped muon momentum
	Reco_QQ_mumi_idx[Reco_QQ_size] = IndexOfThisMuon(&vMuon1);

	if(_flipJpsiDirection>0){
	  iTrack_mupl = mu2Trk;
	  iTrack_mumi = mu1Trk;
	  new((*Reco_QQ_mumi_4mom)[Reco_QQ_size])TLorentzVector(mu1Trk.px(),mu1Trk.py(),mu1Trk.pz(),vMuon1.E());  //only the direction of the 3-momentum changes
    Reco_QQ_mumi_4mom_pt.push_back(mu1Trk.pt());
    Reco_QQ_mumi_4mom_eta.push_back(mu1Trk.eta());
    Reco_QQ_mumi_4mom_phi.push_back(mu1Trk.phi());
    Reco_QQ_mumi_4mom_m.push_back(vMuon1.M());
	  new((*Reco_QQ_mupl_4mom)[Reco_QQ_size])TLorentzVector(mu2Trk.px(),mu2Trk.py(),mu2Trk.pz(),vMuon2.E());
    Reco_QQ_mupl_4mom_pt.push_back(mu2Trk.pt());
    Reco_QQ_mupl_4mom_eta.push_back(mu2Trk.eta());
    Reco_QQ_mupl_4mom_phi.push_back(mu2Trk.phi());
    Reco_QQ_mupl_4mom_m.push_back(vMuon2.M());
	} else if(_muonLessPrimaryVertex || _useGeTracks){
	  iTrack_mupl = *(muon2->innerTrack());
	  iTrack_mumi = *(muon1->innerTrack());
	}

      }

      if ((!_theMinimumFlag && _muonLessPrimaryVertex) || (_flipJpsiDirection>0) ) {
	Reco_QQ_mupl_dxy[Reco_QQ_size] = iTrack_mupl.dxy(RefVtx);
	Reco_QQ_mumi_dxy[Reco_QQ_size] = iTrack_mumi.dxy(RefVtx);
	Reco_QQ_mupl_dz[Reco_QQ_size] = iTrack_mupl.dz(RefVtx);
	Reco_QQ_mumi_dz[Reco_QQ_size] = iTrack_mumi.dz(RefVtx);
      }

      TLorentzVector vJpsi = lorentzMomentum(aJpsiCand->p4());
      new((*Reco_QQ_4mom)[Reco_QQ_size])TLorentzVector(vJpsi);
      Reco_QQ_4mom_pt.push_back(vJpsi.Pt());
      Reco_QQ_4mom_eta.push_back(vJpsi.Eta());
      Reco_QQ_4mom_phi.push_back(vJpsi.Phi());
      Reco_QQ_4mom_m.push_back(vJpsi.M());


      if (_useBS) {
	if (aJpsiCand->hasUserFloat("ppdlBS")) {
	  Reco_QQ_ctau[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS");
	} else {  
	  Reco_QQ_ctau[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlBS was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrBS")) {
	  Reco_QQ_ctauErr[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS");
	} else {
	  Reco_QQ_ctauErr[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrBS was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlBS3D")) {
	  Reco_QQ_ctau3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS3D");
	} else {
	  Reco_QQ_ctau3D[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlBS3D was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrBS3D")) {
	  Reco_QQ_ctauErr3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS3D");
	} else {
	  Reco_QQ_ctauErr3D[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrBS3D was not found" << std::endl;
	}
      }
      else {
	if (aJpsiCand->hasUserFloat("ppdlPV")) {
	  Reco_QQ_ctau[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV");
	} else {
	  Reco_QQ_ctau[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlPV was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrPV")) {
	  Reco_QQ_ctauErr[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV");
	} else {
	  Reco_QQ_ctauErr[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrPV was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlPV3D")) {
	  Reco_QQ_ctau3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV3D");
	} else {
	  Reco_QQ_ctau3D[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlPV3D was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrPV3D")) {
	  Reco_QQ_ctauErr3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV3D");
	} else {
	  Reco_QQ_ctau3D[Reco_QQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrPV3D was not found" << std::endl;
	}
        if (aJpsiCand->hasUserFloat("cosAlpha")) {
          Reco_QQ_cosAlpha[Reco_QQ_size] = aJpsiCand->userFloat("cosAlpha");
        } else {
          Reco_QQ_cosAlpha[Reco_QQ_size] = -10;
	  std::cout << "Warning: User Float cosAlpha was not found" << std::endl;
        }
        if (aJpsiCand->hasUserFloat("cosAlpha3D")) {
          Reco_QQ_cosAlpha3D[Reco_QQ_size] = aJpsiCand->userFloat("cosAlpha3D");
        } else {
          Reco_QQ_cosAlpha3D[Reco_QQ_size] = -10;
	  std::cout << "Warning: User Float cosAlpha3D was not found" << std::endl;
        }
      }
      if (aJpsiCand->hasUserFloat("vProb")) {
	Reco_QQ_VtxProb[Reco_QQ_size] = aJpsiCand->userFloat("vProb");
      } else {
	Reco_QQ_VtxProb[Reco_QQ_size] = -1;
	std::cout << "Warning: User Float vProb was not found" << std::endl;
      }
      if (aJpsiCand->hasUserFloat("DCA")) {
	Reco_QQ_dca[Reco_QQ_size] = aJpsiCand->userFloat("DCA");
      } else {
	Reco_QQ_dca[Reco_QQ_size] = -10;
	std::cout << "Warning: User Float DCA was not found" << std::endl;
      }
      if (aJpsiCand->hasUserFloat("MassErr")) {
	Reco_QQ_MassErr[Reco_QQ_size] = aJpsiCand->userFloat("MassErr");
      } else {
	Reco_QQ_MassErr[Reco_QQ_size] = -10;
	std::cout << "Warning: User Float MassErr was not found" << std::endl;
      }

      Reco_QQ_NtrkDeltaR03[Reco_QQ_size]=0;
      Reco_QQ_NtrkDeltaR04[Reco_QQ_size]=0;
      Reco_QQ_NtrkDeltaR05[Reco_QQ_size]=0;

      Reco_QQ_NtrkPt02[Reco_QQ_size]=0;
      Reco_QQ_NtrkPt03[Reco_QQ_size]=0;
      Reco_QQ_NtrkPt04[Reco_QQ_size]=0;

      //--- counting tracks around Jpsi direction ---
      if (_useGeTracks && !_doDimuTrk && collTracks.isValid()) {
	for(std::vector<reco::Track>::const_iterator it=collTracks->begin();
	    it!=collTracks->end(); ++it) {
	  const reco::Track* track = &(*it);
	  
          if (track==NULL){
	    std::cout<<"ERROR: 'track' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;
          } else {

	    double dz = track->dz(RefVtx);
	    double dzsigma = sqrt(track->dzError()*track->dzError()+RefVtx_zError*RefVtx_zError);    
	    double dxy = track->dxy(RefVtx);
	    double dxysigma = sqrt(track->dxyError()*track->dxyError() + RefVtx_xError*RefVtx_yError);

	    if (track->qualityByName("highPurity") &&
		track->pt()>0.2 && fabs(track->eta())<2.4 &&
		track->ptError()/track->pt()<0.1 && 
		fabs(dz/dzsigma)<3.0 && fabs(dxy/dxysigma)<3.0)  {
         
	      Reco_QQ_NtrkPt02[Reco_QQ_size]++;
	      if (track->pt()>0.3) Reco_QQ_NtrkPt03[Reco_QQ_size]++;
	      if (track->pt()>0.4) {
		Reco_QQ_NtrkPt04[Reco_QQ_size]++;

		if (iTrack_mupl.charge()==track->charge()) {
		  double Reco_QQ_mupl_NtrkDeltaR = deltaR(iTrack_mupl.eta(), iTrack_mupl.phi(), track->eta(), track->phi());
		  double Reco_QQ_mupl_RelDelPt = abs(1.0 - iTrack_mupl.pt()/track->pt());

		  if ( Reco_QQ_mupl_NtrkDeltaR<0.001 &&
		       Reco_QQ_mupl_RelDelPt<0.001 )
		    continue;
		}
		else {
		  double Reco_QQ_mumi_NtrkDeltaR = deltaR(iTrack_mumi.eta(), iTrack_mumi.phi(), track->eta(), track->phi());
		  double Reco_QQ_mumi_RelDelPt = abs(1.0 - iTrack_mumi.pt()/track->pt());
		  if ( Reco_QQ_mumi_NtrkDeltaR<0.001 &&
		       Reco_QQ_mumi_RelDelPt<0.001 ) 
		    continue;
		}

		double Reco_QQ_NtrkDeltaR = deltaR(aJpsiCand->eta(), aJpsiCand->phi(), track->eta(), track->phi());
		if (Reco_QQ_NtrkDeltaR<0.3)
		  Reco_QQ_NtrkDeltaR03[Reco_QQ_size]++;
		if (Reco_QQ_NtrkDeltaR<0.4)
		  Reco_QQ_NtrkDeltaR04[Reco_QQ_size]++;
		if (Reco_QQ_NtrkDeltaR<0.5)
		  Reco_QQ_NtrkDeltaR05[Reco_QQ_size]++;
	      }
	    }
	  }	
	}
      }
    }
  }
  else {
    std::cout<<"ERROR: 'aJpsiCand' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;
  }

  Reco_QQ_size++;
  return;
}

void
HiOniaAnalyzer::fillTreeDiOnia(int count) {
  if (Reco_QQQQ_size >= Max_QQQQ_size) {
    std::cout << "Too many dimuons: " << Reco_QQQQ_size << std::endl;
    std::cout << "Maximum allowed: " << Max_QQQQ_size << std::endl;
    return;
  }

  const pat::CompositeCandidate* aJpsiCand = &(*collDiOnia)[count];

  if (aJpsiCand!=NULL){
    const pat::CompositeCandidate* muon1 = dynamic_cast<const pat::CompositeCandidate*>(aJpsiCand->daughter("muon1"));
    const pat::CompositeCandidate* muon2 = dynamic_cast<const pat::CompositeCandidate*>(aJpsiCand->daughter("muon2"));

    ULong64_t trigBits=0;
    for (unsigned int iTr=1; iTr<NTRIGGERS; ++iTr) {
      if (isTriggerMatched[iTr]) {trigBits += pow(2,iTr-1);}
    }

    if (muon1==NULL || muon2==NULL){
      std::cout<<"ERROR: 'muon1' or 'muon2' pointer in fillTreeDiOnia is NULL ! Return now"<<std::endl; return;
    } else {

      //Reco_QQ_sign[Reco_QQ_size] = muon1->charge() + muon2->charge();
      //Reco_QQ_type[Reco_QQ_size] = _thePassedCats.at(count);

      //Reco_QQ_trig[Reco_QQ_size] = trigBits;

//      if (!(_isHI) && _muonLessPrimaryVertex && aJpsiCand->hasUserData("muonlessPV")) {
//	RefVtx = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).position();
//	RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).xError();
//	RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).yError();
//	RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).zError();
//      }
//      else if (!_muonLessPrimaryVertex && aJpsiCand->hasUserData("PVwithmuons")) {
//	RefVtx = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).position();
//	RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).xError();
//	RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).yError();
//	RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).zError();
//      }
//      else {
//	cout << "HiOniaAnalyzer::fillTreeJpsi: no PVfor muon pair stored" << endl;
//	return;
//      }
//
//      new((*Reco_QQ_vtx)[Reco_QQ_size])TVector3(RefVtx.X(),RefVtx.Y(),RefVtx.Z());

      
      TLorentzVector vMuon1 = lorentzMomentum(muon1->p4());
      TLorentzVector vMuon2 = lorentzMomentum(muon2->p4());
      
      reco::Track iTrack_mupl, iTrack_mumi, mu1Trk, mu2Trk;
      if(_flipJpsiDirection>0 && aJpsiCand->hasUserData("muon1Track") && aJpsiCand->hasUserData("muon2Track")){
      	mu1Trk = *(aJpsiCand->userData<reco::Track>("muon1Track"));
      	mu2Trk = *(aJpsiCand->userData<reco::Track>("muon2Track"));
      }

//      Reco_QQ_flipJpsi[Reco_QQ_size] = _flipJpsiDirection;
//      if(aJpsiCand->hasUserInt("flipJpsi")) Reco_QQ_flipJpsi[Reco_QQ_size] = aJpsiCand->userInt("flipJpsi");

//      if((muon1->innerTrack()).isNull() || (muon2->innerTrack()).isNull()){
//	std::cout<<"ERROR: 'iTrack_mupl' or 'iTrack_mumi' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;}
//
	Reco_QQQQ_mupl_idx[Reco_QQQQ_size] = IndexOfThisJpsi(&vMuon1); //needs the non-flipped muon momentum
	Reco_QQQQ_mumi_idx[Reco_QQQQ_size] = IndexOfThisJpsi(&vMuon2);

	  iTrack_mupl = mu1Trk;
	  iTrack_mumi = mu2Trk;
//	  new((*Reco_QQ_mupl_4mom)[Reco_QQ_size])TLorentzVector(mu1Trk.px(),mu1Trk.py(),mu1Trk.pz(),vMuon1.E());  //only the direction of the 3-momentum changes
//    Reco_QQ_mupl_4mom_pt.push_back(mu1Trk.pt());
//    Reco_QQ_mupl_4mom_eta.push_back(mu1Trk.eta());
//    Reco_QQ_mupl_4mom_phi.push_back(mu1Trk.phi());
//    Reco_QQ_mupl_4mom_m.push_back(vMuon1.M());
//
//	  new((*Reco_QQ_mumi_4mom)[Reco_QQ_size])TLorentzVector(mu2Trk.px(),mu2Trk.py(),mu2Trk.pz(),vMuon2.E());
//    Reco_QQ_mumi_4mom_pt.push_back(mu2Trk.pt());
//    Reco_QQ_mumi_4mom_eta.push_back(mu2Trk.eta());
//    Reco_QQ_mumi_4mom_phi.push_back(mu2Trk.phi());
//    Reco_QQ_mumi_4mom_m.push_back(vMuon2.M());

      TLorentzVector vJpsi = lorentzMomentum(aJpsiCand->p4());
      Reco_QQQQ_4mom_pt.push_back(vJpsi.Pt());
      Reco_QQQQ_4mom_eta.push_back(vJpsi.Eta());
      Reco_QQQQ_4mom_phi.push_back(vJpsi.Phi());
      Reco_QQQQ_4mom_m.push_back(vJpsi.M());


      if (_useBS) {
	if (aJpsiCand->hasUserFloat("ppdlBS")) {
	  Reco_QQQQ_ctau[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS");
	} else {  
	  Reco_QQQQ_ctau[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlBS was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrBS")) {
	  Reco_QQQQ_ctauErr[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS");
	} else {
	  Reco_QQQQ_ctauErr[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrBS was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlBS3D")) {
	  Reco_QQQQ_ctau3D[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS3D");
	} else {
	  Reco_QQQQ_ctau3D[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlBS3D was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrBS3D")) {
	  Reco_QQQQ_ctauErr3D[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS3D");
	} else {
	  Reco_QQQQ_ctauErr3D[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrBS3D was not found" << std::endl;
	}
      }
      else {
	if (aJpsiCand->hasUserFloat("ppdlPV")) {
	  Reco_QQQQ_ctau[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV");
	} else {
	  Reco_QQQQ_ctau[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlPV was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrPV")) {
	  Reco_QQQQ_ctauErr[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV");
	} else {
	  Reco_QQQQ_ctauErr[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrPV was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlPV3D")) {
	  Reco_QQQQ_ctau3D[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV3D");
	} else {
	  Reco_QQQQ_ctau3D[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlPV3D was not found" << std::endl;
	}
	if (aJpsiCand->hasUserFloat("ppdlErrPV3D")) {
	  Reco_QQQQ_ctauErr3D[Reco_QQQQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV3D");
	} else {
	  Reco_QQQQ_ctau3D[Reco_QQQQ_size] = -100;
	  std::cout << "Warning: User Float ppdlErrPV3D was not found" << std::endl;
	}
        if (aJpsiCand->hasUserFloat("cosAlpha")) {
          Reco_QQQQ_cosAlpha[Reco_QQQQ_size] = aJpsiCand->userFloat("cosAlpha");
        } else {
          Reco_QQQQ_cosAlpha[Reco_QQQQ_size] = -10;
	  std::cout << "Warning: User Float cosAlpha was not found" << std::endl;
        }
        if (aJpsiCand->hasUserFloat("cosAlpha3D")) {
          Reco_QQQQ_cosAlpha3D[Reco_QQQQ_size] = aJpsiCand->userFloat("cosAlpha3D");
        } else {
          Reco_QQQQ_cosAlpha3D[Reco_QQQQ_size] = -10;
	  std::cout << "Warning: User Float cosAlpha3D was not found" << std::endl;
        }
      }
      if (aJpsiCand->hasUserFloat("vProb")) {
	Reco_QQQQ_VtxProb[Reco_QQQQ_size] = aJpsiCand->userFloat("vProb");
      } else {
	Reco_QQQQ_VtxProb[Reco_QQQQ_size] = -1;
	std::cout << "Warning: User Float vProb was not found" << std::endl;
      }
      if (aJpsiCand->hasUserFloat("DCA")) {
	Reco_QQQQ_dca[Reco_QQQQ_size] = aJpsiCand->userFloat("DCA");
      } else {
	Reco_QQQQ_dca[Reco_QQQQ_size] = -10;
	std::cout << "Warning: User Float DCA was not found" << std::endl;
      }
      if (aJpsiCand->hasUserFloat("MassErr")) {
	Reco_QQQQ_MassErr[Reco_QQQQ_size] = aJpsiCand->userFloat("MassErr");
      } else {
	Reco_QQQQ_MassErr[Reco_QQQQ_size] = -10;
	std::cout << "Warning: User Float MassErr was not found" << std::endl;
      }

    }
  }
  else {
    std::cout<<"ERROR: 'aJpsiCand' pointer in fillTreeDiQuark is NULL ! Return now"<<std::endl; return;
  }

  Reco_QQQQ_size++;
  return;
}


bool
HiOniaAnalyzer::checkCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1,  const pat::Muon* muon2, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*)) {
  std::string lastFilter = _OneMatchedHLTMu>=0 ? filterNameMap.at(theTriggerNames[_OneMatchedHLTMu]) : "";
  if ( (((this->*callFunc1)(muon1) && (this->*callFunc2)(muon2)) || ((this->*callFunc1)(muon2) && (this->*callFunc2)(muon1))) &&
       (!_applycuts || true) && //Add hard-coded cuts here if desired
       (    (_OneMatchedHLTMu==-1) || 
	    (muon1->triggerObjectMatchesByFilter(lastFilter)).size()>0 || (muon2->triggerObjectMatchesByFilter(lastFilter)).size()>0
	    ) ) 
    return true;
  else
    return false;
}



pair< unsigned int, const pat::CompositeCandidate* > 
HiOniaAnalyzer::theBestQQ() {

  unsigned int theBestCat = 99;
  const pat::CompositeCandidate* theBestCand = new pat::CompositeCandidate();

  for( unsigned int i = 0; i < _thePassedCands.size(); i++) { 
    if (_thePassedCats.at(i) < theBestCat) {
      theBestCat = _thePassedCats.at(i);
      theBestCand = _thePassedCands.at(i);
    }
  }

  pair< unsigned int, const pat::CompositeCandidate* > result = make_pair(theBestCat, theBestCand );
  return result;
}

bool 
HiOniaAnalyzer::isTrkInMuonAccept(TLorentzVector trk4mom, std::string muonType){
  if (muonType == (std::string)("GLB")) {
    return (fabs(trk4mom.Eta()) < 2.4 &&
            ((fabs(trk4mom.Eta()) < 1.2 && trk4mom.Pt() >= 3.5) ||
             (1.2 <= fabs(trk4mom.Eta()) && fabs(trk4mom.Eta()) < 2.1 && trk4mom.Pt() >= 5.47-1.89*fabs(trk4mom.Eta())) ||
             (2.1 <= fabs(trk4mom.Eta()) && trk4mom.Pt() >= 1.5)));
  }
  else if (muonType == (std::string)("TRK") || muonType == (std::string)("TRKSOFT")) {
    return (fabs(trk4mom.Eta()) < 2.4 &&
            ((fabs(trk4mom.Eta()) < 1.1 && trk4mom.Pt() >= 3.3) ||
             (1.1 <= fabs(trk4mom.Eta()) && fabs(trk4mom.Eta()) < 1.3 && trk4mom.Pt() >= 13.2-9.0*fabs(trk4mom.Eta()) ) ||
             (1.3 <= fabs(trk4mom.Eta()) && trk4mom.Pt() >= 0.8 && trk4mom.Pt() >= 3.02-1.17*fabs(trk4mom.Eta()) )));
  }
  else if (muonType == (std::string)("GLBSOFT")) {
    return (fabs(trk4mom.Eta()) < 2.4 &&
            ((fabs(trk4mom.Eta()) < 0.3 && trk4mom.Pt() >= 3.4) ||
             (fabs(trk4mom.Eta()) > 0.3 && fabs(trk4mom.Eta()) < 1.1 && trk4mom.Pt() >= 3.3) ||
             (fabs(trk4mom.Eta()) > 1.1 && fabs(trk4mom.Eta()) < 1.4 && trk4mom.Pt() >= 7.7-4.0*fabs(trk4mom.Eta()) ) ||
             (fabs(trk4mom.Eta()) > 1.4 && fabs(trk4mom.Eta()) < 1.55 && trk4mom.Pt() >= 2.1) ||
             (fabs(trk4mom.Eta()) > 1.55 && fabs(trk4mom.Eta()) < 2.2 && trk4mom.Pt() >= 4.25-1.39*fabs(trk4mom.Eta()) ) ||
             (fabs(trk4mom.Eta()) > 2.2 && trk4mom.Pt() >= 1.2) ));
  }  
  else  std::cout << "ERROR: Incorrect Muon Type" << std::endl;

  return false;
}


bool
HiOniaAnalyzer::isMuonInAccept(const pat::Muon* aMuon, const std::string muonType) {
  if (muonType == (std::string)("GLB")) {
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 1.2 && aMuon->pt() >= 3.5) ||
             (1.2 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 2.1 && aMuon->pt() >= 5.47-1.89*fabs(aMuon->eta())) ||
             (2.1 <= fabs(aMuon->eta()) && aMuon->pt() >= 1.5)));
  }
  else if (muonType == (std::string)("Acceptance2015")) {
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 1.2 && aMuon->pt() >= 3.5) ||
             (1.2 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 2.1 && aMuon->pt() >= 5.77-1.89*fabs(aMuon->eta())) ||
             (2.1 <= fabs(aMuon->eta()) && aMuon->pt() >= 1.8)));
  }
  else if (muonType == (std::string)("TRK")) { //This is actually softer than the "TRKSOFT" acceptance
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 0.8 && aMuon->pt() >= 3.3) ||
             (0.8 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 2. && aMuon->p() >= 2.9) ||
             (2. <= fabs(aMuon->eta()) && aMuon->pt() >= 0.8)));
  }
  else if (muonType == (std::string)("GLBSOFT")) {
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 0.3 && aMuon->pt() >= 3.4) ||
             (fabs(aMuon->eta()) > 0.3 && fabs(aMuon->eta()) < 1.1 && aMuon->pt() >= 3.3) ||
             (fabs(aMuon->eta()) > 1.1 && fabs(aMuon->eta()) < 1.4 && aMuon->pt() >= 7.7-4.0*fabs(aMuon->eta()) ) ||
             (fabs(aMuon->eta()) > 1.4 && fabs(aMuon->eta()) < 1.55 && aMuon->pt() >= 2.1) ||
             (fabs(aMuon->eta()) > 1.55 && fabs(aMuon->eta()) < 2.2 && aMuon->pt() >= 4.25-1.39*fabs(aMuon->eta()) ) ||
             (fabs(aMuon->eta()) > 2.2 && aMuon->pt() >= 1.2) ));
  }
  else if (muonType == (std::string)("TRKSOFT")) {
    return (fabs(aMuon->eta()) < 2.4 &&
	    ((fabs(aMuon->eta()) < 1.1 && aMuon->pt() >= 3.3) ||
	     (1.1 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 1.3 && aMuon->pt() >= 13.2-9.0*fabs(aMuon->eta()) ) ||
	     (1.3 <= fabs(aMuon->eta()) && aMuon->pt() >= 0.8 && aMuon->pt() >= 3.02-1.17*fabs(aMuon->eta()) )));
  }
  else  std::cout << "ERROR: Incorrect Muon Type" << std::endl;
  
  return false;
}

bool
HiOniaAnalyzer::isSoftMuonBase(const pat::Muon* aMuon) {
  return (aMuon->isTrackerMuon() &&
          aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5   &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement()   > 0   &&
          fabs(aMuon->innerTrack()->dxy(RefVtx)) < 0.3 &&
          fabs(aMuon->innerTrack()->dz(RefVtx)) < 20.
          );
}

bool
HiOniaAnalyzer::isHybridSoftMuon(const pat::Muon* aMuon) {
  return (isSoftMuonBase(aMuon) &&
          aMuon->isGlobalMuon()
          );
}

bool
HiOniaAnalyzer::selGlobalMuon(const pat::Muon* aMuon) {
  
  if(!aMuon->isGlobalMuon())
    return false;

  if( _muonSel==(std::string)("GlbTrk") && !aMuon->isTrackerMuon() )
    return false;
  
  if(!_applycuts)
    return true;
  
  bool isInAcc = isMuonInAccept(aMuon, (std::string)(_SofterSgMuAcceptance?"GLBSOFT":"GLB"));
  bool isGood = (_selTightGlobalMuon ? aMuon->passed(reco::Muon::CutBasedIdTight) : isSoftMuonBase(aMuon) );

  return ( isInAcc && isGood && (!_miniAODcut || PassMiniAODcut(aMuon)) );
}

bool 
HiOniaAnalyzer::selTrackerMuon(const pat::Muon* aMuon) {
  
  if(!aMuon->isTrackerMuon())
    return false;

  if(!_applycuts)
    return true;

  bool isInAcc = isMuonInAccept(aMuon, (std::string)(_SofterSgMuAcceptance?"TRKSOFT":"TRK"));
  bool isGood = isSoftMuonBase(aMuon);

  return ( isInAcc && isGood && (!_miniAODcut || PassMiniAODcut(aMuon)));
}

bool
HiOniaAnalyzer::selGlobalOrTrackerMuon(const pat::Muon* aMuon) {

  if(!aMuon->isGlobalMuon() && !aMuon->isTrackerMuon())
    return false;

  if(!_applycuts)
    return true;

  bool isInAcc = isMuonInAccept(aMuon, (std::string)(_SofterSgMuAcceptance?"TRKSOFT":"TRK"));
  bool isGood = isSoftMuonBase(aMuon);

  return ( isInAcc && isGood && (!_miniAODcut || PassMiniAODcut(aMuon)) );
}

bool
HiOniaAnalyzer::selAllMuon(const pat::Muon* aMuon) {
  return !_miniAODcut || PassMiniAODcut(aMuon);
}

bool
HiOniaAnalyzer::PassMiniAODcut(const pat::Muon* aMuon) {
  return aMuon->pt() > 5 || aMuon->isPFMuon() || (aMuon->pt()>1.2 && (aMuon->isGlobalMuon() || aMuon->isStandAloneMuon())) || (aMuon->isTrackerMuon() && aMuon->innerTrack()->quality(reco::TrackBase::highPurity));
}

bool
HiOniaAnalyzer::selTrk(const reco::TrackRef aTrk) {

  if(!(aTrk->qualityByName("highPurity") && aTrk->ptError()/aTrk->pt()<0.1))
    return false;

  if(!_applycuts)
    return true;

  bool isInAcc = aTrk->pt()>1.2 && fabs(aTrk->eta())<2.4;//(aTrk->pt())>0.2 && fabs(aTrk->eta())<2.4 && aTrk->ptError()/aTrk->pt()<0.1 && fabs(aTrk->dxy(RefVtx))<0.35 && fabs(aTrk->dz(RefVtx))<20; //keep margin in dxy and dz, if the RefVtx is not the good one due to muonlessPV

  return ( isInAcc );
}

void
HiOniaAnalyzer::InitEvent()
{
  for (unsigned int iTr=1;iTr<NTRIGGERS;++iTr) {
    alreadyFilled[iTr]=false;
  }
  HLTriggers = 0;
  nEP = 0;

  _thePassedCats.clear();      _thePassedCands.clear();

  Reco_QQ_size = 0;
  Reco_mu_size = 0;
  Reco_trk_size = 0;

  Reco_QQ_4mom->Clear();
  Reco_QQ_4mom_pt.clear();
  Reco_QQ_4mom_eta.clear();
  Reco_QQ_4mom_phi.clear();
  Reco_QQ_4mom_m.clear();
  Reco_QQ_mupl_4mom->Clear();
  Reco_QQ_mupl_4mom_pt.clear();
  Reco_QQ_mupl_4mom_eta.clear();
  Reco_QQ_mupl_4mom_phi.clear();
  Reco_QQ_mupl_4mom_m.clear();
  Reco_QQ_mumi_4mom->Clear();
  Reco_QQ_mumi_4mom_pt.clear();
  Reco_QQ_mumi_4mom_eta.clear();
  Reco_QQ_mumi_4mom_phi.clear();
  Reco_QQ_mumi_4mom_m.clear();
  Reco_QQ_vtx->Clear();
  Reco_mu_4mom->Clear();
  Reco_mu_4mom_pt.clear();
  Reco_mu_4mom_eta.clear();
  Reco_mu_4mom_phi.clear();
  Reco_mu_4mom_m.clear();
  Reco_mu_L1_4mom->Clear();
  Reco_mu_L1_4mom_pt.clear();
  Reco_mu_L1_4mom_eta.clear();
  Reco_mu_L1_4mom_phi.clear();
  Reco_mu_L1_4mom_m.clear();
  Reco_QQQQ_4mom_pt.clear();
  Reco_QQQQ_4mom_eta.clear();
  Reco_QQQQ_4mom_phi.clear();
  Reco_QQQQ_4mom_m.clear();

  if (_useGeTracks && _fillRecoTracks) {
    Reco_trk_4mom->Clear();
    Reco_trk_4mom_pt.clear();
    Reco_trk_4mom_eta.clear();
    Reco_trk_4mom_phi.clear();
    Reco_trk_4mom_m.clear();
    Reco_trk_vtx->Clear();
  }

  if (_isMC) {
    Gen_QQ_4mom->Clear();
    Gen_QQ_4mom_pt.clear();
    Gen_QQ_4mom_eta.clear();
    Gen_QQ_4mom_phi.clear();
    Gen_QQ_4mom_m.clear();
    Gen_mu_4mom->Clear();

    Gen_QQ_size = 0;
    Gen_mu_size = 0;

    Gen_weight = -1.;
    Gen_pthat = -1.;

    mapGenMuonMomToIndex_.clear();
  }

  if(_doTrimuons || _doDimuTrk){
    _thePassedBcCats.clear();      _thePassedBcCands.clear();

    Reco_3mu_size = 0;
    Reco_3mu_vtx->Clear();
    Reco_3mu_4mom->Clear();
    Reco_3mu_4mom_pt.clear();
    Reco_3mu_4mom_eta.clear();
    Reco_3mu_4mom_phi.clear();
    Reco_3mu_4mom_m.clear();

    if (_isMC) {
      Gen_Bc_size = 0;
      Gen_Bc_4mom->Clear();
      Gen_Bc_4mom_pt.clear();
      Gen_Bc_4mom_eta.clear();
      Gen_Bc_4mom_phi.clear();
      Gen_Bc_4mom_m.clear();
      Gen_Bc_nuW_4mom->Clear();
      Gen_Bc_nuW_4mom_pt.clear();
      Gen_Bc_nuW_4mom_eta.clear();
      Gen_Bc_nuW_4mom_phi.clear();
      Gen_Bc_nuW_4mom_m.clear();
      Gen_3mu_4mom->Clear();
      Gen_3mu_4mom_pt.clear();
      Gen_3mu_4mom_eta.clear();
      Gen_3mu_4mom_phi.clear();
      Gen_3mu_4mom_m.clear();
    }
  }

  mapMuonMomToIndex_.clear();
  mapTrkMomToIndex_.clear();
  for(std::map< std::string, int >::iterator clearIt= mapTriggerNameToIntFired_.begin(); clearIt != mapTriggerNameToIntFired_.end(); clearIt++){
    clearIt->second=0;
  }
  for(std::map< std::string, int >::iterator clearIt= mapTriggerNameToPrescaleFac_.begin(); clearIt != mapTriggerNameToPrescaleFac_.end(); clearIt++){
    clearIt->second=-1;
  }

  return;
}

int
HiOniaAnalyzer::IndexOfThisMuon(TLorentzVector* v1, bool isGen){
  const auto& mapMuIdx =  (isGen ?  mapGenMuonMomToIndex_ : mapMuonMomToIndex_);
  const long int& muPt = FloatToIntkey(v1->Pt());

  if (mapMuIdx.count(muPt)==0) return -1;
  else return mapMuIdx.at(muPt);
}


int
HiOniaAnalyzer::IndexOfThisTrack(TLorentzVector* v1, bool isGen){
  const auto& mapTrkIdx =  (isGen ?  mapTrkMomToIndex_ : mapTrkMomToIndex_);
  const long int& trkPt = FloatToIntkey(v1->Pt());

  if (mapTrkIdx.count(trkPt)==0) return -1;
  else return mapTrkIdx.at(trkPt);
}

int
HiOniaAnalyzer::IndexOfThisJpsi(int mu1_idx, int mu2_idx, int flipJpsi){
  int GoodIndex = -1;
  for(int iJpsi=0; iJpsi<Reco_QQ_size; iJpsi++){
    if(((Reco_QQ_mumi_idx[iJpsi] == mu1_idx && Reco_QQ_mupl_idx[iJpsi] == mu2_idx) ||
	(Reco_QQ_mumi_idx[iJpsi] == mu2_idx && Reco_QQ_mupl_idx[iJpsi] == mu1_idx))
       && flipJpsi == Reco_QQ_flipJpsi[iJpsi]
       ){
      GoodIndex = iJpsi;
      break;
    }
  }
  return GoodIndex;
}
int
HiOniaAnalyzer::IndexOfThisJpsi(TLorentzVector* v1){
  int GoodIndex = -1;
  double maxCompat = -9999.;
  auto compat = [](double a, double b){
    return 1-std::abs(a-b);
  };
  for(int iJpsi=0; iJpsi<Reco_QQ_size; iJpsi++){
    auto oldCompat=  maxCompat;
    maxCompat = std::max(compat(((TLorentzVector*) Reco_QQ_4mom->At(iJpsi))->Pt(), v1->Pt()), maxCompat);
    if( oldCompat != maxCompat) GoodIndex = iJpsi;
  }
  return GoodIndex;
}

int
HiOniaAnalyzer::IndexOfThisDiOnia(int mu1_idx, int mu2_idx){
  int GoodIndex = -1;
  for(int iJpsi=0; iJpsi<Reco_QQQQ_size; iJpsi++){
    if(((Reco_QQ_mumi_idx[iJpsi] == mu1_idx && Reco_QQ_mupl_idx[iJpsi] == mu2_idx) ||
	(Reco_QQ_mumi_idx[iJpsi] == mu2_idx && Reco_QQ_mupl_idx[iJpsi] == mu1_idx))
       ){
      GoodIndex = iJpsi;
      break;
    }
  }
  return GoodIndex;
}

Short_t 
HiOniaAnalyzer::MuInSV(TLorentzVector v1, TLorentzVector v2, TLorentzVector v3) {

  //	cout<<"SV collection size : "<<SVs->size()<<endl;
  int nMuInSV = 0;
  for(std::vector<reco::Vertex>::const_iterator vt=SVs->begin(); vt!=SVs->end(); ++vt){
    const reco::Vertex* vtx = &(*vt);
    int nTrksInSV = 0;
    for(reco::Vertex::trackRef_iterator it=vtx->tracks_begin();it!=vtx->tracks_end(); ++it) {
      if((fabs((*it)->pt() - v1.Pt())<1e-3 && fabs((*it)->eta() - v1.Eta())<1e-4) || (fabs((*it)->pt() - v2.Pt())<1e-3 && fabs((*it)->eta() - v2.Eta())<1e-4) || (fabs((*it)->pt() - v3.Pt())<1e-3 && fabs((*it)->eta() - v3.Eta())<1e-4)){
	nTrksInSV+=1;
      }
    }
    if (nTrksInSV>nMuInSV) nMuInSV=nTrksInSV;
  }
  //cout<<"Number of muons from Bc that are in a given SV = "<<nMuInSV<<endl;

  return nMuInSV;
}

reco::GenParticleRef  
HiOniaAnalyzer::findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG) {

  reco::GenParticleRef GenParticleTmp = GenParticleDaughter;
  bool foundFirstDaughter = false;

  for(int j=0; j<1000; ++j) {
    //cout<<""; // trick to prevent rare segfault errors with the GenParticleTmp
    
    if ( GenParticleTmp.isNonnull() && GenParticleTmp->status()>0 && GenParticleTmp->status()<1000 && GenParticleTmp->numberOfDaughters()>0 ) 
      {
	if ( GenParticleTmp->pdgId()==GenParticlePDG || GenParticleTmp->daughterRef(0)->pdgId()==GenParticlePDG ) //if oscillating B, can take two decays to return to pdgID(B parent)
	  {
	    GenParticleTmp = GenParticleTmp->daughterRef(0); 
	  }
	else if ( !foundFirstDaughter ) //if Tmp is not a Bc, it means Tmp is a true daughter
	  {
	    foundFirstDaughter = true;
	    GenParticlePDG = GenParticleTmp->pdgId();
	  } 
      }
    else break;
  }
  if (GenParticleTmp.isNonnull() && GenParticleTmp->status()>0 && GenParticleTmp->status()<1000 && foundFirstDaughter){ //(GenParticleTmp->pdgId()==GenParticlePDG)) {
    GenParticleDaughter = GenParticleTmp;
  }

  return GenParticleDaughter;

}


void
HiOniaAnalyzer::fillGenInfo()
{
  if (Gen_QQ_size >= Max_QQ_size) {
    std::cout << "Too many dimuons: " << Gen_QQ_size << std::endl;
    std::cout << "Maximum allowed: " << Max_QQ_size << std::endl;
    return;
  }

  if (Gen_Bc_size >= Max_Bc_size) {
    std::cout << "Too many Bc's: " << Gen_Bc_size << std::endl;
    std::cout << "Maximum allowed: " << Max_Bc_size << std::endl;
    return;
  }

  if (Gen_mu_size >= Max_mu_size) {
    std::cout << "Too many muons: " << Gen_mu_size << std::endl;
    std::cout << "Maximum allowed: " << Max_mu_size << std::endl;
    return;
  }

  if (genInfo.isValid()) {
    if (genInfo->hasBinningValues()) Gen_pthat = genInfo->binningValues()[0];
    Gen_weight = genInfo->weight();
  }

  if (collGenParticles.isValid()) {
    //Fill the single muons, before the dimuons (important)
    for(std::vector<reco::GenParticle>::const_iterator it=collGenParticles->begin();
        it!=collGenParticles->end();++it) {
      const reco::GenParticle* gen = &(*it);
      
      if (abs(gen->pdgId()) == 13 && (gen->status() == 1)) {
        Gen_mu_type[Gen_mu_size] = _isPromptMC ? 0 : 1; // prompt: 0, non-prompt: 1
        Gen_mu_charge[Gen_mu_size] = gen->charge();

        TLorentzVector vMuon = lorentzMomentum(gen->p4());
        new((*Gen_mu_4mom)[Gen_mu_size])TLorentzVector(vMuon);
        Gen_mu_4mom_pt.push_back(vMuon.Pt());
        Gen_mu_4mom_eta.push_back(vMuon.Eta());
        Gen_mu_4mom_phi.push_back(vMuon.Phi());
        Gen_mu_4mom_m.push_back(vMuon.M());

	//Fill map of the muon indices. Use long int keys, to avoid rounding errors on a float key. Implies a precision of 10^-6     
	mapGenMuonMomToIndex_[ FloatToIntkey(vMuon.Pt()) ] = Gen_mu_size;

        Gen_mu_size++;
      }
    }

    for(std::vector<reco::GenParticle>::const_iterator it=collGenParticles->begin();
        it!=collGenParticles->end();++it) {
      const reco::GenParticle* gen = &(*it);

      if (abs(gen->pdgId()) == _oniaPDG  && (gen->status() == 2 || (abs(gen->pdgId())==23 && gen->status() == 62))  &&
          gen->numberOfDaughters() >= 2) {

        reco::GenParticleRef genMuon1 = findDaughterRef(gen->daughterRef(0), gen->pdgId());
        reco::GenParticleRef genMuon2 = findDaughterRef(gen->daughterRef(1), gen->pdgId());

        if ( abs(genMuon1->pdgId()) == 13 &&
             abs(genMuon2->pdgId()) == 13 &&
             ( genMuon1->status() == 1 ) &&
             ( genMuon2->status() == 1 )
             ) {

	  Gen_QQ_Bc_idx[Gen_QQ_size] = -1;          
          Gen_QQ_type[Gen_QQ_size] = _isPromptMC ? 0 : 1; // prompt: 0, non-prompt: 1
          std::pair< std::vector<reco::GenParticleRef> , std::pair<float, float> > MCinfo = findGenMCInfo(gen);
          Gen_QQ_ctau[Gen_QQ_size] = 10.0*MCinfo.second.first;
          Gen_QQ_ctau3D[Gen_QQ_size] = 10.0*MCinfo.second.second;

	  if(_genealogyInfo){
	    _Gen_QQ_MomAndTrkBro[Gen_QQ_size] = MCinfo.first;
	    Gen_QQ_momId[Gen_QQ_size] = _Gen_QQ_MomAndTrkBro[Gen_QQ_size][0]->pdgId();
	  }
          
          TLorentzVector vJpsi = lorentzMomentum(gen->p4());
          new((*Gen_QQ_4mom)[Gen_QQ_size])TLorentzVector(vJpsi);
          Gen_QQ_4mom_pt.push_back(vJpsi.Pt());
          Gen_QQ_4mom_eta.push_back(vJpsi.Eta());
          Gen_QQ_4mom_phi.push_back(vJpsi.Phi());
          Gen_QQ_4mom_m.push_back(vJpsi.M());

          TLorentzVector vMuon1 = lorentzMomentum(genMuon1->p4());
          TLorentzVector vMuon2 = lorentzMomentum(genMuon2->p4());
            
          if (genMuon1->charge() > genMuon2->charge()) {
            Gen_QQ_mupl_idx[Gen_QQ_size] = IndexOfThisMuon(&vMuon1 , true);
            Gen_QQ_mumi_idx[Gen_QQ_size] = IndexOfThisMuon(&vMuon2 , true);
          }
          else {
            Gen_QQ_mupl_idx[Gen_QQ_size] = IndexOfThisMuon(&vMuon2 , true);
            Gen_QQ_mumi_idx[Gen_QQ_size] = IndexOfThisMuon(&vMuon1 , true);
          }

	  if(_doTrimuons){
	    //GenInfo for the Bc and the daughter muon from the W daughter of the Bc. Beware, this is designed for generated Bc's having QQ as a daughter!!
	    std::pair<bool, reco::GenParticleRef> findBcMom = findBcMotherRef( findMotherRef(gen->motherRef(),gen->pdgId()) , _BcPDG); //the boolean says if the Bc mother was found
	  
	    if (findBcMom.first) {
	  
	      if (Gen_QQ_Bc_idx[Gen_QQ_size] >-1) {std::cout<<"WARNING : Jpsi seems to have more than one Bc mother"<<std::endl;}

	      reco::GenParticleRef genBc = findBcMom.second;
	      if(genBc->numberOfDaughters() >= 3){
		reco::GenParticleRef genDau1 = findDaughterRef(genBc->daughterRef(0), genBc->pdgId());
		reco::GenParticleRef genDau2 = findDaughterRef(genBc->daughterRef(1), genBc->pdgId());
		reco::GenParticleRef genDau3 = findDaughterRef(genBc->daughterRef(2), genBc->pdgId());

		//Which daughter is the mu or nu from the W?
		bool goodDaughters = true;
		reco::GenParticleRef gennuW = genDau1;
		reco::GenParticleRef genmuW = genDau2;
	    
		if ( isNeutrino(genDau1->pdgId()) && (abs(genDau2->pdgId()) == 13) ){
		}
		else if ( isNeutrino(genDau2->pdgId()) && (abs(genDau1->pdgId()) == 13) ){
		  reco::GenParticleRef gennuW = genDau2;
		  reco::GenParticleRef genmuW = genDau1;
		}
		else if ( isNeutrino(genDau1->pdgId()) && (abs(genDau3->pdgId()) == 13) ){
		  reco::GenParticleRef gennuW = genDau1;
		  reco::GenParticleRef genmuW = genDau3;
		}
		else if ( isNeutrino(genDau3->pdgId()) && (abs(genDau1->pdgId()) == 13) ){
		  reco::GenParticleRef gennuW = genDau3;
		  reco::GenParticleRef genmuW = genDau1;
		}
		else if ( isNeutrino(genDau2->pdgId()) && (abs(genDau3->pdgId()) == 13) ){
		  reco::GenParticleRef gennuW = genDau2;
		  reco::GenParticleRef genmuW = genDau3;
		}
		else if ( isNeutrino(genDau3->pdgId()) && (abs(genDau2->pdgId()) == 13) ){
		  reco::GenParticleRef gennuW = genDau3;
		  reco::GenParticleRef genmuW = genDau2;
		}
		else {
		  goodDaughters = false;
		}
	    
		//Fill info for Bc and its mu,nu daughters
		if(goodDaughters && (genmuW->charge() == genBc->charge()) 
		   && ( genmuW->status() == 1 ) ){
	
		  Gen_QQ_Bc_idx[Gen_QQ_size] = Gen_Bc_size;
		  Gen_Bc_QQ_idx[Gen_Bc_size] = Gen_QQ_size;

		  Gen_Bc_pdgId[Gen_Bc_size] = genBc->pdgId();
		  std::pair<int, std::pair<float, float> > MCinfo = findGenBcInfo(genBc, gen);
		  Gen_Bc_ctau[Gen_Bc_size] = 10.0*MCinfo.second.first;

		  TLorentzVector vBc = lorentzMomentum(genBc->p4());
		  new((*Gen_Bc_4mom)[Gen_Bc_size])TLorentzVector(vBc);
      Gen_Bc_4mom_pt.push_back(vBc.Pt());
      Gen_Bc_4mom_eta.push_back(vBc.Eta());
      Gen_Bc_4mom_phi.push_back(vBc.Phi());
      Gen_Bc_4mom_m.push_back(vBc.M());

		  TLorentzVector vmuW = lorentzMomentum(genmuW->p4());
		  Gen_Bc_muW_idx[Gen_Bc_size] = IndexOfThisMuon(&vmuW, true);
	      
		  TLorentzVector vnuW = lorentzMomentum(gennuW->p4());
		  new((*Gen_Bc_nuW_4mom)[Gen_Bc_size])TLorentzVector(vnuW);
      Gen_Bc_nuW_4mom_pt.push_back(vnuW.Pt());
      Gen_Bc_nuW_4mom_eta.push_back(vnuW.Eta());
      Gen_Bc_nuW_4mom_phi.push_back(vnuW.Phi());
      Gen_Bc_nuW_4mom_m.push_back(vnuW.M());
	    
		  Gen_Bc_size++;
		}
		else {std::cout<<"WARNING : Problem with daughters of the gen Bc, hence Bc and its daughters are not written out"<<std::endl;}
	      }
	    }
	  }
          Gen_QQ_size++;
        }
      }
    }
  }  
  return;
}

//Find the indices of the reconstructed muon matching each generated muon, and vice versa
void
HiOniaAnalyzer::fillMuMatchingInfo()
{

  //initialize Gen_mu_whichRec
  for (int igen=0;igen<Gen_mu_size;igen++){
    Gen_mu_whichRec[igen] = -1;
  }
 
  //Find the index of generated muon associated to a reco muon, txs to Reco_mu_pTrue
  for (int irec=0;irec<Reco_mu_size;irec++){
    int foundGen = -1;
    if(Reco_mu_pTrue[irec]>=0){ //if pTrue=-1, then the reco muon is a fake                                                                                                                                                                  

      for (int igen=0;igen<Gen_mu_size;igen++){
        TLorentzVector *genmuMom = (TLorentzVector*)Gen_mu_4mom->ConstructedAt(igen);
        if(fabs(genmuMom->P() - Reco_mu_pTrue[irec])/Reco_mu_pTrue[irec] < 1e-6 && Gen_mu_charge[igen]==Reco_mu_charge[irec]){
          foundGen = igen; 
	  break;
        }
      }
    }

    Reco_mu_whichGen[irec] = foundGen;
    if(foundGen>-1) Gen_mu_whichRec[foundGen] = irec;
  }
  
}

//Find the indices of the reconstructed J/psi matching each generated J/psi (when the two daughter muons are reconstructed), and vice versa
void
HiOniaAnalyzer::fillQQMatchingInfo(){
  for (int igen=0;igen<Gen_QQ_size;igen++){
    Gen_QQ_whichRec[igen] = -1;
    int Reco_mupl_idx = Gen_mu_whichRec[Gen_QQ_mupl_idx[igen]]; //index of the reconstructed mupl associated to the generated mupl of Jpsi
    int Reco_mumi_idx = Gen_mu_whichRec[Gen_QQ_mumi_idx[igen]]; //index of the reconstructed mumi associated to the generated mumi of Jpsi
    
    if((Reco_mupl_idx>=0) && (Reco_mumi_idx>=0)){   //Search for Reco_QQ only if both muons are reco
      for (int irec=0;irec<Reco_QQ_size;irec++){
	if(((Reco_mupl_idx == Reco_QQ_mupl_idx[irec]) && (Reco_mumi_idx == Reco_QQ_mumi_idx[irec])) ||    //the charges might be wrong in reco
	   ((Reco_mupl_idx == Reco_QQ_mumi_idx[irec]) && (Reco_mumi_idx == Reco_QQ_mupl_idx[irec]))	   ){
	  Gen_QQ_whichRec[igen] = irec;
	  break;
	}
      }
      
      if (Gen_QQ_whichRec[igen]==-1) Gen_QQ_whichRec[igen] = -2; //Means the two muons were reconstructed, but the dimuon was not selected
    }
  }

  //Find the index of generated J/psi associated to a reco QQ
  for (int irec=0;irec<Reco_QQ_size;irec++){
    Reco_QQ_whichGen[irec] = -1;
    
    for (int igen=0;igen<Gen_QQ_size;igen++){
      if((Gen_QQ_whichRec[igen] == irec)){
	Reco_QQ_whichGen[irec] = igen;
	break;
      }
    }      
  }
  
}

//Build the visible Bc when the three daugther muons are reconstructed. Record indices between Gen and Rec 

void
HiOniaAnalyzer::fillRecoTracks()
{
  if (collTracks.isValid()) {
    for(unsigned int tidx=0; tidx<collTracks->size();tidx++) {
      const reco::TrackRef track(collTracks, tidx);

      if (!track.isNonnull()){
	std::cout<<"ERROR: 'track' pointer in fillRecoTracks is NULL ! Go to next track."<<endl;
      } else {

	bool WantedTrack = false;
	for (int k=0;k<(int)EtaOfWantedTracks.size();k++){
	  if (fabs(track->eta() - EtaOfWantedTracks[k]) < 1e-5) {
	    WantedTrack = true; break;}
	}
	if (_doDimuTrk && !WantedTrack) continue;

	if (selTrk(track))  {
	  if (Reco_trk_size >= Max_trk_size) {
	    std::cout << "Too many tracks: " << Reco_trk_size << std::endl;
	    std::cout << "Maximum allowed: " << Max_trk_size << std::endl;
	    break;
	  }

	  TLorentzVector vTrack;
	  vTrack.SetPtEtaPhiM(track->pt(), track->eta(), track->phi(), 0.10566); //0.13957018 for the pion

	  Reco_trk_whichGenmu[Reco_trk_size] = -1;
	  if(_isMC){
	    float dRmax = 0.05; //dR max of the matching to gen muons//same than for reco-gen muon matching
	    float dR;
	    float dPtmax = 0.5;
	    for(int igen=0;igen<Gen_mu_size;igen++){
	      TLorentzVector* genmu = (TLorentzVector*)Gen_mu_4mom->ConstructedAt(igen);
	      dR = genmu->DeltaR(vTrack);
	      if(dR<=dRmax && track->charge()==Gen_mu_charge[igen] && fabs(genmu->Pt()-vTrack.Pt())/genmu->Pt() < dPtmax) {
		dRmax = dR;
		Reco_trk_whichGenmu[Reco_trk_size] = igen;
	      }
	    }
	  }

	  if(!_doDimuTrk && Reco_trk_whichGenmu[Reco_trk_size]==-1) continue;
        
	  Reco_trk_charge[Reco_trk_size] = track->charge();

	  //new((*Reco_trk_vtx)[Reco_trk_size])TVector3(track->vx(),track->vy(),track->vz());
	  //cout<<"Just found and filled Reco_trk_vtx"<<endl;
	  
	  Reco_trk_originalAlgo[Reco_mu_size] = track->originalAlgo();
	  Reco_trk_nPixWMea[Reco_mu_size] = track->hitPattern().pixelLayersWithMeasurement();
	  Reco_trk_nTrkWMea[Reco_mu_size] = track->hitPattern().trackerLayersWithMeasurement();
	  Reco_trk_dxyError[Reco_trk_size] = track->dxyError();
	  Reco_trk_dzError[Reco_trk_size] = track->dzError();
	  Reco_trk_dxy[Reco_trk_size] = track->dxy(RefVtx);
	  Reco_trk_dz[Reco_trk_size] = track->dz(RefVtx);
	  Reco_trk_ptErr[Reco_trk_size] = track->ptError();

	  mapTrkMomToIndex_[ FloatToIntkey(vTrack.Pt()) ] = Reco_trk_size;

	  Reco_trk_InLooseAcc[Reco_trk_size] = isTrkInMuonAccept(vTrack,"GLBSOFT");
	  Reco_trk_InTightAcc[Reco_trk_size] = isTrkInMuonAccept(vTrack,"GLB");

	  new((*Reco_trk_4mom)[Reco_trk_size])TLorentzVector(vTrack);
    Reco_trk_4mom_pt.push_back(vTrack.Pt());
    Reco_trk_4mom_eta.push_back(vTrack.Eta());
    Reco_trk_4mom_phi.push_back(vTrack.Phi());
    Reco_trk_4mom_m.push_back(vTrack.M());
	  Reco_trk_size++;
	}
      }
    }
  }
  
  return;
}



// ------------ method called once each job just before starting event loop  ------------
void 
HiOniaAnalyzer::beginJob()
{
  //fOut = new TFile(_histfilename.c_str(), "RECREATE");
  InitTree();

  // book histos
  //hGoodMuonsNoTrig = new TH1F("hGoodMuonsNoTrig","hGoodMuonsNoTrig",10,0,10);
  hGoodMuonsNoTrig = fs->make<TH1F>("hGoodMuonsNoTrig","hGoodMuonsNoTrig",10,0,10);
  //hGoodMuons = new TH1F("hGoodMuons","hGoodMuons",10,0,10);
  hGoodMuons = fs->make<TH1F>("hGoodMuons","hGoodMuons",10,0,10);
  //hL1DoubleMu0 = new TH1F("hL1DoubleMu0","hL1DoubleMu0",10,0,10);
  hL1DoubleMu0 = fs->make<TH1F>("hL1DoubleMu0","hL1DoubleMu0",10,0,10);
  
  hGoodMuonsNoTrig->Sumw2();
  hGoodMuons->Sumw2();
  hL1DoubleMu0->Sumw2();

  // muons
  if (_combineCategories) 
    myRecoMuonHistos = new MyCommonHistoManager("RecoMuon");
  else {
    myRecoGlbMuonHistos = new MyCommonHistoManager("GlobalMuon");
    myRecoTrkMuonHistos = new MyCommonHistoManager("TrackerMuon");
  }

  // J/psi
  if (_combineCategories)
    myRecoJpsiHistos = new MyCommonHistoManager("RecoJpsi");
  else {
    myRecoJpsiGlbGlbHistos = new MyCommonHistoManager("GlbGlbJpsi");
    myRecoJpsiGlbTrkHistos = new MyCommonHistoManager("GlbTrkJpsi");
    myRecoJpsiTrkTrkHistos = new MyCommonHistoManager("TrkTrkJpsi");
  }
  
  for (unsigned int i=0; i<theRegions.size(); ++i) {
    for (unsigned int j=0; j<NTRIGGERS; ++j) {
      for (unsigned int k=0; k<theCentralities.size(); ++k) {

        std::string theAppendix = theRegions.at(i) ;
        theAppendix += "_" + theTriggerNames.at(j);
        theAppendix += "_" + theCentralities.at(k);

        // muons
        if (_combineCategories) {
          myRecoMuonHistos->Add(theAppendix);
          myRecoMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
        }
        else {
          myRecoGlbMuonHistos->Add(theAppendix);
          myRecoTrkMuonHistos->Add(theAppendix);
          
          myRecoGlbMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoGlbMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
          
          myRecoTrkMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoTrkMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
        }

        for (unsigned int l=0; l<theSign.size(); ++l) {
          // J/psi
          if (_combineCategories)
            myRecoJpsiHistos->Add(theAppendix + "_" + theSign.at(l));
          else {
            myRecoJpsiGlbGlbHistos->Add(theAppendix + "_" + theSign.at(l));
            myRecoJpsiGlbTrkHistos->Add(theAppendix + "_" + theSign.at(l));
            myRecoJpsiTrkTrkHistos->Add(theAppendix + "_" + theSign.at(l));
          }
        }
      }
    }
  }
  /*
  if (_combineCategories)
    myRecoMuonHistos->Print();
  else
    myRecoGlbMuonHistos->Print();
  */
  //hStats = new TH1F("hStats","hStats;;Number of Events",2*NTRIGGERS+1,0,2*NTRIGGERS+1);
  hStats = fs->make<TH1F>("hStats","hStats;;Number of Events",2*NTRIGGERS+1,0,2*NTRIGGERS+1);
  hStats->GetXaxis()->SetBinLabel(1,"All");
  for (int i=2; i< (int) theTriggerNames.size()+1; ++i) {
    hStats->GetXaxis()->SetBinLabel(i,theTriggerNames.at(i-1).c_str()); // event info
    hStats->GetXaxis()->SetBinLabel(i+NTRIGGERS,theTriggerNames.at(i-1).c_str()); // muon pair info
  }
  hStats->Sumw2();

  //hCent = new TH1F("hCent","hCent;centrality bin;Number of Events",200,0,200);
  hCent = fs->make<TH1F>("hCent","hCent;centrality bin;Number of Events",200,0,200);
  hCent->Sumw2();

  //hPileUp = new TH1F("hPileUp","Number of Primary Vertices;n_{PV};counts", 50, 0, 50);
  hPileUp = fs->make<TH1F>("hPileUp","Number of Primary Vertices;n_{PV};counts", 50, 0, 50);
  hPileUp->Sumw2();

  //hZVtx = new TH1F("hZVtx","Primary z-vertex distribution;z_{vtx} [cm];counts", 120, -30, 30);
  hZVtx = fs->make<TH1F>("hZVtx","Primary z-vertex distribution;z_{vtx} [cm];counts", 120, -30, 30);
  hZVtx->Sumw2();

  return;
}

void 
HiOniaAnalyzer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  //init HLTConfigProvider
  
  EDConsumerBase::Labels labelTriggerResults;
  EDConsumerBase::labelsForToken(_tagTriggerResultsToken, labelTriggerResults);	
  const std::string pro = labelTriggerResults.process;

  //bool init(const edm::Run& iRun, const edm::EventSetup& iSetup, const std::string& processName, bool& changed);
  bool changed = true;
  hltConfigInit = hltConfig.init(iRun, iSetup, pro, changed);

  changed = true;
  hltPrescaleInit = hltPrescaleProvider.init(iRun, iSetup, pro, changed);

  //extract trigger path names
  for (const auto& pathLabel : theTriggerNames)
    triggerNameMap[pathLabel] = "";

  for (const auto& hltPath : hltConfig.triggerNames())
    if (hltPath.rfind("HLT_", 0) == 0)
      for (const auto& pathLabel : theTriggerNames)
        if (hltPath!="NoString" && TString(hltPath).Contains(TRegexp(TString(pathLabel)))) {
          triggerNameMap.at(pathLabel) = hltPath;
          break;
        }

  //extract last filter names
  for (const auto& p : triggerNameMap) {
    filterNameMap[p.first] = "";
    if (p.second == "") continue;
    const auto& m = hltConfig.moduleLabels(hltConfig.triggerIndex(p.second));
    for (int j = m.size()-1; j >= 0; j--)
      if (m[j].rfind("hltL", 0) == 0 && m[j].rfind("Filtered") != std::string::npos) {
        filterNameMap.at(p.first) = m[j];
        break;
      }
  }

  return;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HiOniaAnalyzer::endJob() {
  std::cout << "Total number of events = " << nEvents << std::endl;
  std::cout << "Total number of passed candidates = " << passedCandidates << std::endl;
  return;
}

TLorentzVector
HiOniaAnalyzer::lorentzMomentum(const reco::Candidate::LorentzVector& p) {
  TLorentzVector res;
  res.SetPtEtaPhiM(p.pt(), p.eta(), p.phi(), p.mass());

  return res;
}


bool 
HiOniaAnalyzer::isAbHadron(int pdgID) {
  return (abs(pdgID) == 511 || abs(pdgID) == 521 || abs(pdgID) == 531 || abs(pdgID) == 5122 || abs(pdgID) == 541);

}

bool
HiOniaAnalyzer::isNeutrino(int pdgID) {
  return (abs(pdgID) == 14 || abs(pdgID) == 16 || abs(pdgID) == 18);
}

bool 
HiOniaAnalyzer::isAMixedbHadron(int pdgID, int momPdgID) {

  if ((abs(pdgID) == 511 && abs(momPdgID) == 511 && pdgID*momPdgID < 0) || 
      (abs(pdgID) == 531 && abs(momPdgID) == 531 && pdgID*momPdgID < 0)) 
      return true;
  return false;

}

reco::GenParticleRef   
HiOniaAnalyzer::findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG) {

  for(int i=0; i<1000; ++i) {
    if (GenParticleMother.isNonnull() && (GenParticleMother->pdgId()==GenParticlePDG) && GenParticleMother->numberOfMothers()>0) {        
      GenParticleMother = GenParticleMother->motherRef();
    } else break;
  }
  return GenParticleMother;

}

bool HiOniaAnalyzer::isChargedTrack(int pdgId){
  return ((fabs(pdgId) == 211) || (fabs(pdgId) == 321) || (fabs(pdgId) == 2212) || (fabs(pdgId) == 11) || (fabs(pdgId) == 13));
}

std::vector<reco::GenParticleRef> HiOniaAnalyzer::GenBrothers(reco::GenParticleRef GenParticleMother, int GenJpsiPDG){
  bool foundJpsi = false;
  std::vector<reco::GenParticleRef> res;

  if(!GenParticleMother.isNonnull()) return res;
  //if(Reco_3mu_size>0) cout<<"\nScanning daughters of Jpsi "<<GenJpsiPDG<<" mother, pdg = "<<GenParticleMother->pdgId()<<endl; 
  for(int i=0;i<(int)GenParticleMother->numberOfDaughters();i++){    
    reco::GenParticleRef dau = findDaughterRef(GenParticleMother->daughterRef(i), GenParticleMother->pdgId());
    for(int l=0;l<100;l++){ //avoid having a daughter of same pdgId
      if(!(dau.isNonnull() && dau->status()>0 && dau->status()<1000)) break;
      if(dau->pdgId()==GenParticleMother->pdgId() && dau->numberOfDaughters()==1)
	dau = findDaughterRef(dau->daughterRef(0), dau->pdgId());
      else break;
    }

    if(!(dau.isNonnull() && dau->status()>0 && dau->status()<1000) ) continue;
    //if(Reco_3mu_size>0) cout<<"Daughter #"<<i<<" pdg = "<< dau->pdgId()<<" pt,eta = "<<dau->pt()<<" "<<dau->eta()<<endl;
    if(isChargedTrack(dau->pdgId())){
      res.push_back(dau);}
    if(dau->pdgId()==GenJpsiPDG) {
      foundJpsi = true; //continue;
    }

    for(int j=0;j<(int)dau->numberOfDaughters();j++){
      reco::GenParticleRef grandDau = findDaughterRef(dau->daughterRef(j), dau->pdgId());
      for(int l=0;l<100;l++){ //avoid having a daughter of same pdgId
	if(!(grandDau.isNonnull() && grandDau->status()>0 && grandDau->status()<1000)) break;
	if(grandDau->pdgId()==dau->pdgId() && grandDau->numberOfDaughters()==1)
	  grandDau = findDaughterRef(grandDau->daughterRef(0), grandDau->pdgId());
	else break;
      }

      if(!(grandDau.isNonnull() && grandDau->status()>0 && grandDau->status()<1000)) continue;
      //if(Reco_3mu_size>0) cout<<"    grand-daughter #"<<j<<" pdg = "<< grandDau->pdgId()<<" pt,eta = "<<grandDau->pt()<<" "<<grandDau->eta()<<endl;
      if(isChargedTrack(grandDau->pdgId())){
	res.push_back(grandDau);}
      if(grandDau->pdgId()==GenJpsiPDG) {
	foundJpsi = true; //continue;
      }

      for(int k=0;k<(int)grandDau->numberOfDaughters();k++){
	reco::GenParticleRef ggrandDau = findDaughterRef(grandDau->daughterRef(k), grandDau->pdgId());
	for(int l=0;l<100;l++){ //avoid having a daughter of same pdgId
	  if(!(ggrandDau.isNonnull() && ggrandDau->status()>0 && ggrandDau->status()<1000)) break;
	  if(ggrandDau->pdgId()==grandDau->pdgId() && ggrandDau->numberOfDaughters()==1)
	    ggrandDau = findDaughterRef(ggrandDau->daughterRef(0), ggrandDau->pdgId());
	  else break;
	}

	if(!(ggrandDau.isNonnull() && ggrandDau->status()>0 && ggrandDau->status()<1000)) continue;
	//if(Reco_3mu_size>0) cout<<"        grand-grand-daughter #"<<k<<" pdg = "<< ggrandDau->pdgId()<<" pt,eta = "<<ggrandDau->pt()<<" "<<ggrandDau->eta()<<endl;
	if(isChargedTrack(ggrandDau->pdgId())){
	  res.push_back(ggrandDau);}
	if(ggrandDau->pdgId()==GenJpsiPDG) {
	  foundJpsi = true; //continue;
	}
      }
    }
  }

  if(!foundJpsi){
    cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!! Incoherence in genealogy: Jpsi not found in the daughters!\n"<<endl;
  }
  // if(!isAbHadron(GenParticleMother->pdgId())){
  //   cout<<"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!! Jpsi ancestor is not a b-hadron! pdgID(mother of this ancestor) = "<<findMotherRef(GenParticleMother->motherRef() , GenParticleMother->pdgId())->pdgId()<<endl;
  // }  

  return res;
}


//define this as a plug-in
DEFINE_FWK_MODULE(HiOniaAnalyzer);
