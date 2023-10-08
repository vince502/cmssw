#include "HiAnalysis/HiOnia/interface/HiOniaAnalyzer.h"


void
HiOniaAnalyzer::fillRecoHistos() {

  if(!_doTrimuons || !_isMC || _thePassedBcCands.size()>0){ //not storing the mu and QQ reconstructed info if we do a trimuon MC and there is no reco trimuon
    // BEST J/PSI? 
    if (_onlythebest) {  // yes, fill simply the best (possibly same-sign)

      pair< unsigned int, const pat::CompositeCandidate* > theBest = theBestQQ();
      if (theBest.first < 10) this->fillHistosAndDS(theBest.first, theBest.second);
    
    } else {   // no, fill all candidates passing cuts (possibly same-sign)
   
      for( unsigned int count = 0; count < _thePassedCands.size(); count++) { 
	const pat::CompositeCandidate* aJpsiCand = _thePassedCands.at(count); 
      
	this->checkTriggers(aJpsiCand);
	if (_fillTree){
	  this->fillTreeJpsi(count);
	  this->fillTreeDiOnia(count);
    }
      
	for (unsigned int iTr=0; iTr<NTRIGGERS; ++iTr) {
	  if (isTriggerMatched[iTr]) {
	    this->fillRecoJpsi(count,theTriggerNames.at(iTr), theCentralities.at(theCentralityBin));
	  }
	}
      }
    }
  }

  //Fill Bc (trimuon) 
  if (_fillTree && _doTrimuons){
    for( unsigned int count = 0; count < _thePassedBcCands.size(); count++) {
      this->fillTreeBc(count);
    }
  }
  //Fill Bc (dimuon+track) 
  if (_fillTree && _doDimuTrk){
    for( unsigned int count = 0; count < _thePassedBcCands.size(); count++) {
      this->fillTreeDimuTrk(count);
    }
  }

  return;
};

void
HiOniaAnalyzer::fillRecoJpsi(int count, std::string trigName, std::string centName) {
  pat::CompositeCandidate* aJpsiCand = _thePassedCands.at(count)->clone();

  if(aJpsiCand==NULL){
    std::cout<<"ERROR: 'aJpsiCand' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;
  }
  else{

    aJpsiCand->addUserInt("centBin",centBin);
    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));

    if (muon1==NULL || muon2==NULL){
      std::cout<<"ERROR: 'muon1' or 'muon2' pointer in fillTreeJpsi is NULL ! Return now"<<std::endl; return;
    }
    else {
      int iSign = muon1->charge() + muon2->charge();
      if(iSign!=0){ (iSign==2)?(iSign=1):(iSign=2) ; }

      std::string theLabel =  trigName + "_" + centName + "_" + theSign.at(iSign);

      bool isBarrel = (fabs(aJpsiCand->rapidity()) < 1.2);

      if (iSign==0 &&
	  aJpsiCand->mass() >= JpsiMassMin && aJpsiCand->mass() < JpsiMassMax &&  
	  aJpsiCand->pt() >= JpsiPtMin && aJpsiCand->pt() < JpsiPtMax && 
	  fabs(aJpsiCand->rapidity()) >= JpsiRapMin && fabs(aJpsiCand->rapidity()) < JpsiRapMax) {
	passedCandidates++;
      }

      if (_fillHistos) {
	if (_combineCategories && _thePassedCats.at(count)<=Trk_Trk) { // for the moment consider Glb+Glb, GlbTrk+GlbTrk, Trk+Trk
	  myRecoJpsiHistos->Fill(aJpsiCand, "All_"+ theLabel);
	  if (isBarrel)
	    myRecoJpsiHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
	  else
	    myRecoJpsiHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
	}
	else {
	  switch (_thePassedCats.at(count)) {
	  case Glb_Glb:
	    myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "All_"+ theLabel);
	    if (isBarrel)
	      myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
	    else
	      myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
	    break;
	  case GlbTrk_GlbTrk:
	    myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "All_"+ theLabel);
	    if (isBarrel)
	      myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
	    else
	      myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
	    break;
	  case Trk_Trk:
	    myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "All_"+ theLabel);
	    if (isBarrel)
	      myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
	    else
	      myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
	    break;
	  default:
	    break;
	  }
	}
      }
    }
  }
  this->fillHistosAndDS(_thePassedCats.at(count), aJpsiCand); 

  delete aJpsiCand;
  return;
};

void
HiOniaAnalyzer::fillHistosAndDS(unsigned int theCat, const pat::CompositeCandidate* aJpsiCand) {

  return;
};

void
HiOniaAnalyzer::fillRecoMuons(int iCent)
{
  int nL1DoubleMu0Muons=0;
  int nGoodMuons=0;
  int nGoodMuonsNoTrig=0;

  if (collMuonNoTrig.isValid()) {
    for(std::vector<pat::Muon>::const_iterator it=collMuonNoTrig->begin();
        it!=collMuonNoTrig->end();++it) {
      const pat::Muon* muon = &(*it);

      if (muon==NULL){
	std::cout<<"ERROR: 'muon' pointer in fillRecoMuons is NULL ! Return now"<<std::endl; return;
      } else {
	if (selGlobalMuon(muon))
	  nGoodMuonsNoTrig++;
      }
    }
  }

  if (collMuon.isValid()) {
    for(vector<pat::Muon>::const_iterator it=collMuon->begin();
        it!=collMuon->end();++it) {
      const pat::Muon* muon = &(*it);

      if (muon==NULL){
	std::cout<<"ERROR: 'muon' pointer in fillRecoMuons is NULL ! Return now"<<std::endl; return;
      } else {

	//Trick to recover feature of filling only muons from selected dimuons
	if(!_fillSingleMuons){
	  bool WantedMuon = false;
	  for (int k=0;k<(int)EtaOfWantedMuons.size();k++){
	    if (fabs(muon->eta() - EtaOfWantedMuons[k]) < 1e-5) {
	      WantedMuon = true; break;}
	  }
	  if (!WantedMuon) continue;
	}
      
	bool isBarrel = (fabs(muon->eta() < 1.2));
	std::string theLabel = theTriggerNames.at(0) + "_" + theCentralities.at(iCent);

	if (_fillHistos) {
	  if (_combineCategories) {
	    if ( selGlobalMuon(muon) || selTrackerMuon(muon) ) {
	      myRecoMuonHistos->Fill(muon, "All_"+theLabel);
	      if (isBarrel)
		myRecoMuonHistos->Fill(muon, "Barrel_"+theLabel);
	      else
		myRecoMuonHistos->Fill(muon, "EndCap_"+theLabel);
	    }
	  }
	  else {
	    if (selGlobalMuon(muon)) {
          
	      myRecoGlbMuonHistos->Fill(muon, "All_"+theLabel);
	      if (isBarrel)
		myRecoGlbMuonHistos->Fill(muon, "Barrel_"+theLabel);
	      else
		myRecoGlbMuonHistos->Fill(muon, "EndCap_"+theLabel);
	    }
	    else if (selTrackerMuon(muon)) {
	      myRecoTrkMuonHistos->Fill(muon, "All_"+theLabel);
	      if (isBarrel)
		myRecoTrkMuonHistos->Fill(muon, "Barrel_"+theLabel);
	      else
		myRecoTrkMuonHistos->Fill(muon, "EndCap_"+theLabel);
	    }
	  }
	}
      
	muType = -99;
        if ( _muonSel==(std::string)("Glb")      && selGlobalMuon(muon)  ) muType = Glb;
        if ( _muonSel==(std::string)("GlbTrk")   && selGlobalMuon(muon)  ) muType = GlbTrk;
        if ( _muonSel==(std::string)("Trk")      && selTrackerMuon(muon) ) muType = Trk;
        if ( _muonSel==(std::string)("TwoGlbAmongThree") && selGlobalOrTrackerMuon(muon) ) muType = GlbOrTrk;
	if ( _muonSel==(std::string)("GlbOrTrk") && selGlobalOrTrackerMuon(muon) ) muType = GlbOrTrk;
	if ( _muonSel==(std::string)("All") && selAllMuon(muon) ) muType = All;
      
	if ( muType==GlbOrTrk || muType==GlbTrk || muType==Trk || muType==Glb || muType==All ) {
	  nGoodMuons++;

	  ULong64_t trigBits=0;
	  for (unsigned int iTr=1; iTr<NTRIGGERS; ++iTr) {
	    const pat::TriggerObjectStandAloneCollection muHLTMatchesFilter = muon->triggerObjectMatchesByFilter( filterNameMap.at(theTriggerNames[iTr]) );
	    
	    // apparently matching by path gives false positives so we use matching by filter for all triggers for which we know the filter name
	    if ( muHLTMatchesFilter.size() > 0 ) {
	      std::string theLabel = theTriggerNames.at(iTr) + "_" + theCentralities.at(iCent);
            
	      if (_fillHistos) {
		if (_combineCategories) {
		  myRecoMuonHistos->Fill(muon, "All_"+theLabel);
		  if (isBarrel)
		    myRecoMuonHistos->Fill(muon, "Barrel_"+theLabel);
		  else
		    myRecoMuonHistos->Fill(muon, "EndCap_"+theLabel);
		}
		else if ( muType==Glb || muType==GlbTrk ) {
		  myRecoGlbMuonHistos->Fill(muon, "All_"+theLabel);
		  if (isBarrel)
		    myRecoGlbMuonHistos->Fill(muon, "Barrel_"+theLabel);
		  else
		    myRecoGlbMuonHistos->Fill(muon, "EndCap_"+theLabel);
		}
		else if ( muType==Trk || muType==GlbOrTrk || muType==All ) {
		  myRecoTrkMuonHistos->Fill(muon, "All_"+theLabel);
		  if (isBarrel)
		    myRecoTrkMuonHistos->Fill(muon, "Barrel_"+theLabel);
		  else
		    myRecoTrkMuonHistos->Fill(muon, "EndCap_"+theLabel);
		}
	      }

	      trigBits += pow(2,iTr-1);

	      if (iTr==1) nL1DoubleMu0Muons++;
	    }
	  }
	  if (_fillTree)
	    this->fillTreeMuon(muon, muType, trigBits);
	}
      }
    }
  }
  
  hGoodMuonsNoTrig->Fill(nGoodMuonsNoTrig);
  hGoodMuons->Fill(nGoodMuons);
  hL1DoubleMu0->Fill(nL1DoubleMu0Muons);

  return;
};