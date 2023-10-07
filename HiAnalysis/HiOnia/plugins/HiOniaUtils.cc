#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"

void
HiOniaAnalyzer::checkTriggers(const pat::CompositeCandidate* aJpsiCand) {

  if(aJpsiCand==NULL){
    std::cout<<"ERROR: 'aJpsiCand' pointer in checkTriggers is NULL ! Return now"<<std::endl; return;
  } else{

    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));

    if (muon1==NULL || muon2==NULL){
      std::cout<<"ERROR: 'muon1' or 'muon2' pointer in checkTriggers is NULL ! Return now"<<std::endl; return;
    } else {

      // Trigger passed
      for (unsigned int iTr = 1; iTr<NTRIGGERS; ++iTr) {
        const auto& lastFilter = filterNameMap.at(theTriggerNames[iTr]);
	    const auto& mu1HLTMatchesFilter = muon1->triggerObjectMatchesByFilter( lastFilter );
	    const auto& mu2HLTMatchesFilter = muon2->triggerObjectMatchesByFilter( lastFilter );
    
	// const pat::TriggerObjectStandAloneCollection mu1HLTMatchesPath = muon1->triggerObjectMatchesByPath( theTriggerNames.at(iTr), true, false );
	// const pat::TriggerObjectStandAloneCollection mu2HLTMatchesPath = muon2->triggerObjectMatchesByPath( theTriggerNames.at(iTr), true, false );
    
	bool pass1 = mu1HLTMatchesFilter.size() > 0;
	bool pass2 = mu2HLTMatchesFilter.size() > 0;
    
	//    pass1 = mu1HLTMatchesPath.size() > 0;
	//    pass2 = mu2HLTMatchesPath.size() > 0;
    
	if (iTr > NTRIGGERS_DBL) {  // single triggers here
	  isTriggerMatched[iTr] = pass1 || pass2;
	} else {        // double triggers here
	  isTriggerMatched[iTr] = pass1 && pass2;
	}
      }

      for (unsigned int iTr=1;iTr<NTRIGGERS;++iTr) {
	if (isTriggerMatched[iTr]) {
	  // since we have bins for event info, let's try to fill here the trigger info for each pair
	  // also if there are several pairs matched to the same kind of trigger
	  hStats->Fill(iTr+NTRIGGERS); // pair info
	}
      }
    }
  }

  return;
}

void
HiOniaAnalyzer::makeCuts(bool keepSameSign) {
  math::XYZPoint RefVtx_tmp = RefVtx;

  if (collJpsi.isValid()) {

    for(std::vector<pat::CompositeCandidate>::const_iterator it=collJpsi->begin();
        it!=collJpsi->end(); ++it) {
      
      const pat::CompositeCandidate* cand = &(*it);     

      if(cand==NULL){
	std::cout<<"ERROR: 'cand' pointer in makeCuts is NULL ! Return now"<<std::endl; return;
      } else{

        const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(cand->daughter("muon1"));
        const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(cand->daughter("muon2"));

        if (muon1==NULL || muon2==NULL){
	  std::cout<<"ERROR: 'muon1' or 'muon2' pointer in makeCuts is NULL ! Return now"<<std::endl; return;
        } else {

          if(!keepSameSign && (muon1->charge() + muon2->charge() != 0)) continue;

	  if (!(_isHI) && _muonLessPrimaryVertex && cand->hasUserData("muonlessPV"))
	    RefVtx = (*cand->userData<reco::Vertex>("muonlessPV")).position();
	  else if (!_muonLessPrimaryVertex && cand->hasUserData("PVwithmuons"))
	    RefVtx = (*cand->userData<reco::Vertex>("PVwithmuons")).position();
	  else {
	    std::cout << "HiOniaAnalyzer::makeCuts: no PV for muon pair stored ! Go to next candidate." << std::endl;
	    continue;
	  }

	  if (fabs(RefVtx.Z()) > _iConfig.getParameter< double > ("maxAbsZ")) continue;
      
	  if (fabs(muon1->eta()) >= etaMax ||
	      fabs(muon2->eta()) >= etaMax) continue;

	  //Pass muon selection? 
	  bool muonSelFound = false;
	  if ( _muonSel==(std::string)("Glb") ) {
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalMuon,&HiOniaAnalyzer::selGlobalMuon)){
	      _thePassedCats.push_back(Glb_Glb);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
	  }
	  if ( _muonSel==(std::string)("TwoGlbAmongThree") ) {
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalMuon,&HiOniaAnalyzer::selTrackerMuon)){
	      _thePassedCats.push_back(TwoGlbAmongThree);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
	  }
	  if ( _muonSel==(std::string)("GlbTrk") ) {
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalMuon,&HiOniaAnalyzer::selGlobalMuon)){
	      _thePassedCats.push_back(GlbTrk_GlbTrk);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
	  }
	  if ( _muonSel==(std::string)("Trk") ) {
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selTrackerMuon,&HiOniaAnalyzer::selTrackerMuon)){
	      _thePassedCats.push_back(Trk_Trk);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
	  }
          if ( _muonSel==(std::string)("GlbOrTrk") ){
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalOrTrackerMuon,&HiOniaAnalyzer::selGlobalOrTrackerMuon)){
	      _thePassedCats.push_back(GlbOrTrk_GlbOrTrk);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
          }
          if ( _muonSel==(std::string)("All") ){
	    if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selAllMuon,&HiOniaAnalyzer::selAllMuon)){
	      _thePassedCats.push_back(All_All);  _thePassedCands.push_back(cand);
	      if(!_fillSingleMuons){
		EtaOfWantedMuons.push_back(muon1->eta()); EtaOfWantedMuons.push_back(muon2->eta());}
	    }
	    muonSelFound = true;
          }
	  if (!muonSelFound) {
	    std::cout << "[HiOniaAnalyzer::makeCuts] --- The muon selection: " << _muonSel << " is invalid. The supported options are: All, Glb, GlbTrk, GlbOrTrk, Trk, and TwoGlbAmongThree" << std::endl;
	  }
	}
      }
    }
  }  
  RefVtx = RefVtx_tmp;
  return;
};


void
HiOniaAnalyzer::hltReport(const edm::Event &iEvent ,const edm::EventSetup& iSetup)
{
  std::map<std::string, bool> mapTriggernameToTriggerFired;
  std::map<std::string, unsigned int> mapTriggernameToHLTbit;

  for(std::vector<std::string>::const_iterator it=theTriggerNames.begin(); it !=theTriggerNames.end(); ++it){
    mapTriggernameToTriggerFired[*it]=false;
    mapTriggernameToHLTbit[*it]=1000;
  }

  // HLTConfigProvider
  if ( hltConfigInit ) {
    //! Use HLTConfigProvider
    const unsigned int n= hltConfig.size();
    for (std::map<std::string, unsigned int>::iterator it = mapTriggernameToHLTbit.begin(); it != mapTriggernameToHLTbit.end(); it++) {
      unsigned int triggerIndex= hltConfig.triggerIndex( triggerNameMap.at(it->first) );
      if (it->first == "NoTrigger") continue;
      if (triggerIndex >= n) {
	if (_checkTrigNames) std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " not available in config!" << std::endl;
      }
      else {
        it->second= triggerIndex;
        //      std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " available in config!" << std::endl;
      }
    }
  }
    
  // Get Trigger Results
  iEvent.getByToken( _tagTriggerResultsToken, collTriggerResults );
  if ( collTriggerResults.isValid() && (collTriggerResults->size()==hltConfig.size()) ){
    //    std::cout << "[HiOniaAnalyzer::hltReport] --- J/psi TriggerResults IS valid in current event" << std::endl;
      
    // loop over Trigger Results to check if paths was fired
    for(std::vector< std::string >::iterator itHLTNames= theTriggerNames.begin(); itHLTNames != theTriggerNames.end(); itHLTNames++){
      const std::string triggerPathName =  *itHLTNames;
      if ( mapTriggernameToHLTbit[triggerPathName] < 1000 ) {
        if (collTriggerResults->accept( mapTriggernameToHLTbit[triggerPathName] ) ){
          mapTriggerNameToIntFired_[triggerPathName] = 3;
        }
        if (_isMC) {
          mapTriggerNameToPrescaleFac_[triggerPathName] = 1;
        } else {
          //-------prescale factor------------
          if ( hltPrescaleInit && hltPrescaleProvider.prescaleSet(iEvent,iSetup)>=0 ) {
            auto const detailedPrescaleInfo = hltPrescaleProvider.prescaleValuesInDetail<double,double>(iEvent, iSetup, triggerNameMap.at(triggerPathName));
            //std::pair<std::vector<std::pair<std::string,int> >,int> detailedPrescaleInfo = hltPrescaleProvider.prescaleValuesInDetail(iEvent, iSetup, triggerPathName);
            //get HLT prescale info from hltPrescaleProvider     
            const int hltPrescale = detailedPrescaleInfo.second;
            //get L1 prescale info from hltPrescaleProvider
            int l1Prescale = -1;
            if (detailedPrescaleInfo.first.size()==1) {
              l1Prescale = detailedPrescaleInfo.first.at(0).second;
            }
            else if (detailedPrescaleInfo.first.size()>1) {
              l1Prescale = 1; // Means it is a complex l1 seed, and for us it is only Mu3 OR Mu5, both of them unprescaled at L1
            }
            else if(_checkTrigNames) {
              std::cout << "[HiOniaAnalyzer::hltReport] --- L1 prescale was NOT found for TriggerName " << triggerPathName  << " , default L1 prescale value set to 1 " <<  std::endl;
            }
            //compute the total prescale = HLT prescale * L1 prescale
            mapTriggerNameToPrescaleFac_[triggerPathName] = hltPrescale * l1Prescale;
          }
        }
      }
    }
  } else std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerResults NOT valid in current event" << std::endl;

  return;
};

std::pair< std::vector<reco::GenParticleRef>, std::pair<float, float> >  
HiOniaAnalyzer::findGenMCInfo(const reco::GenParticle* genJpsi) {

  float trueLife = -99.;
  float trueLife3D = -99.;
  std::vector<reco::GenParticleRef> JpsiBrothers;

  if (genJpsi->numberOfMothers()>0) {
    TVector3 trueVtx(0.0,0.0,0.0);
    TVector3 trueP(0.0,0.0,0.0);
    TVector3 trueVtxMom(0.0,0.0,0.0);

    trueVtx.SetXYZ(genJpsi->vertex().x(),genJpsi->vertex().y(),genJpsi->vertex().z());
    trueP.SetXYZ(genJpsi->momentum().x(),genJpsi->momentum().y(),genJpsi->momentum().z());

    bool aBhadron = false;
    reco::GenParticleRef Jpsimom_final;
    reco::GenParticleRef Jpsimom = findMotherRef(genJpsi->motherRef(), genJpsi->pdgId());      

    if (Jpsimom.isNull()) {
      std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
      std::pair< std::vector<reco::GenParticleRef>, std::pair<float, float> > result = std::make_pair(JpsiBrothers, trueLifePair);
      return result;
    } 
    else if (Jpsimom->numberOfMothers()<=0) {
      if (isAbHadron(Jpsimom->pdgId())) {  
	Jpsimom_final = Jpsimom;
	aBhadron = true;
      }
    } 

    else {
      reco::GenParticleRef Jpsigrandmom = findMotherRef(Jpsimom->motherRef(), Jpsimom->pdgId());   
      if (isAbHadron(Jpsimom->pdgId())) {
        if (Jpsigrandmom.isNonnull() && isAMixedbHadron(Jpsimom->pdgId(),Jpsigrandmom->pdgId())) {       
	  Jpsimom_final = Jpsigrandmom;
        } 
        else {                  
	Jpsimom_final = Jpsimom;
        }
        aBhadron = true;
      } 

      else if (Jpsigrandmom.isNonnull() && isAbHadron(Jpsigrandmom->pdgId()))  {  
        if (Jpsigrandmom->numberOfMothers()<=0) {
	  Jpsimom_final = Jpsigrandmom;
        } 
        else { 
          reco::GenParticleRef JpsiGrandgrandmom = findMotherRef(Jpsigrandmom->motherRef(), Jpsigrandmom->pdgId());
          if (JpsiGrandgrandmom.isNonnull() && isAMixedbHadron(Jpsigrandmom->pdgId(),JpsiGrandgrandmom->pdgId())) {
	    Jpsimom_final = JpsiGrandgrandmom;
          } 
          else {
	    Jpsimom_final = Jpsigrandmom;
          }
        }
        aBhadron = true;
      }

      //This is to forcefully find the b-like mother of Jpsi 
      else if (Jpsigrandmom.isNonnull() && Jpsigrandmom->numberOfMothers()>0){
	reco::GenParticleRef JpsiGrandgrandmom = findMotherRef(Jpsigrandmom->motherRef(), Jpsigrandmom->pdgId());
	if(JpsiGrandgrandmom.isNonnull() && isAbHadron(JpsiGrandgrandmom->pdgId())){
          Jpsimom_final = JpsiGrandgrandmom;
	  aBhadron = true;
	}
      }

    }
    if (!aBhadron) {
      Jpsimom_final = Jpsimom;
    }

    if (Jpsimom_final.isNonnull()){
      trueVtxMom.SetXYZ(Jpsimom_final->vertex().x(),Jpsimom_final->vertex().y(),Jpsimom_final->vertex().z());
      if(_genealogyInfo && Reco_3mu_size>0
	 ){
	JpsiBrothers = GenBrothers(Jpsimom_final, genJpsi->pdgId() );
      }
      JpsiBrothers.insert(JpsiBrothers.begin(), Jpsimom_final);
    }
    
    TVector3 vdiff = trueVtx - trueVtxMom;
    trueLife = vdiff.Perp()*JpsiPDGMass/trueP.Perp();
    trueLife3D = vdiff.Mag()*JpsiPDGMass/trueP.Mag();

  }

  std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
  std::pair<std::vector<reco::GenParticleRef>, std::pair<float, float> > result = std::make_pair(JpsiBrothers, trueLifePair);
  return result;

};


int HiOniaAnalyzer::muonIDmask(const pat::Muon* muon)
{
   int mask = 0;
   int type;
   for (type=muon::All; type<=muon::RPCMuLoose; type++)
      if (muon->hasUserInt(Form("muonID_%d", type)) ? muon->userInt(Form("muonID_%d", type)) : muon::isGoodMuon(*muon, muon::SelectionType(type)))
         mask = mask | (int) pow(2, type);

   return mask;
};

long int HiOniaAnalyzer::FloatToIntkey(float v)
{
  float vres = fabs(v);
  while(vres>0.1) vres = vres/10; //Assume argument v is always above 0.1, true for abs(Pt)
  return (long int) (10000000*vres); // Precision 10^-6 (i.e. 7-1) on the comparison
};