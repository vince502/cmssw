#include "VertexCompositeAnalysis/VertexCompositeAnalyzer/plugins/PATCompositeTreeProducer2.h"

//#define DEBUG false

//#define DEBUG true
#define PI 3.1416
// #define MAXCAN 500000

PATCompositeTreeProducer2::PATCompositeTreeProducer2(const edm::ParameterSet& iConfig)
{
    //options
    doRecoNtuple_ = iConfig.getUntrackedParameter<bool>("doRecoNtuple");
    doGenNtuple_ = iConfig.getUntrackedParameter<bool>("doGenNtuple");
    twoLayerDecay_ = iConfig.getUntrackedParameter<bool>("twoLayerDecay");
    threeProngDecay_ = iConfig.getUntrackedParameter<bool>("threeProngDecay");
    doGenMatching_ = iConfig.getUntrackedParameter<bool>("doGenMatching");
    doGenMatchingTOF_ = iConfig.getUntrackedParameter<bool>("doGenMatchingTOF");
    hasSwap_ = iConfig.getUntrackedParameter<bool>("hasSwap");
    decayInGen_ = iConfig.getUntrackedParameter<bool>("decayInGen");
    doMuon_ = iConfig.getUntrackedParameter<bool>("doMuon");
    doMuonFull_ = iConfig.getUntrackedParameter<bool>("doMuonFull");
    PID_ = iConfig.getUntrackedParameter<int>("PID");
    PID_dau1_ = iConfig.getUntrackedParameter<int>("PID_dau1");
    PID_dau2_ = iConfig.getUntrackedParameter<int>("PID_dau2");
    if(threeProngDecay_) PID_dau3_ = iConfig.getUntrackedParameter<int>("PID_dau3");
    
    saveTree_ = iConfig.getUntrackedParameter<bool>("saveTree");
    saveHistogram_ = iConfig.getUntrackedParameter<bool>("saveHistogram");
    saveAllHistogram_ = iConfig.getUntrackedParameter<bool>("saveAllHistogram");
    massHistPeak_ = iConfig.getUntrackedParameter<double>("massHistPeak");
    massHistWidth_ = iConfig.getUntrackedParameter<double>("massHistWidth");
    massHistBins_ = iConfig.getUntrackedParameter<int>("massHistBins");

    useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
    isSkimMVA_ = iConfig.getUntrackedParameter<bool>("isSkimMVA"); 

    //cut variables
    multMax_ = iConfig.getUntrackedParameter<double>("multMax", -1);
    multMin_ = iConfig.getUntrackedParameter<double>("multMin", -1);
    deltaR_ = iConfig.getUntrackedParameter<double>("deltaR", 0.03);

    pTBins_ = iConfig.getUntrackedParameter< std::vector<double> >("pTBins");
    yBins_  = iConfig.getUntrackedParameter< std::vector<double> >("yBins");

    //input tokens
    tok_offlinePV_ = consumes<reco::VertexCollection>(iConfig.getUntrackedParameter<edm::InputTag>("VertexCollection"));
    tok_generalTrk_ = consumes<reco::TrackCollection>(iConfig.getUntrackedParameter<edm::InputTag>("TrackCollection"));
    PATCompositeCandidateCollection_Token_ = consumes<CCC>(iConfig.getUntrackedParameter<edm::InputTag>("CompositeCollection"));
    MVAValues_Token_ = consumes<MVACollection>(iConfig.getParameter<edm::InputTag>("MVACollection"));
    tok_muon_ = consumes<reco::MuonCollection>(iConfig.getUntrackedParameter<edm::InputTag>("MuonCollection"));
    Dedx_Token1_ = consumes<edm::ValueMap<reco::DeDxData> >(edm::InputTag("dedxHarmonic2"));
    Dedx_Token2_ = consumes<edm::ValueMap<reco::DeDxData> >(edm::InputTag("dedxTruncated40"));
    tok_genParticle_ = consumes<reco::GenParticleCollection>(edm::InputTag(iConfig.getUntrackedParameter<edm::InputTag>("GenParticleCollection")));

    isCentrality_ = false;
    if(iConfig.exists("isCentrality")) isCentrality_ = iConfig.getParameter<bool>("isCentrality");
    if(isCentrality_)
    {
      tok_centBinLabel_ = consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinLabel"));
      tok_centSrc_ = consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralitySrc"));
    }

    isEventPlane_ = false;
    if(iConfig.exists("isEventPlane")) isEventPlane_ = iConfig.getParameter<bool>("isEventPlane");
    if(isEventPlane_)
    {
      tok_eventplaneSrc_ = consumes<reco::EvtPlaneCollection>(iConfig.getParameter<edm::InputTag>("eventplaneSrc"));
    }

    if(useAnyMVA_ && iConfig.exists("MVACollection"))
      MVAValues_Token_ = consumes<MVACollection>(iConfig.getParameter<edm::InputTag>("MVACollection"));
}


PATCompositeTreeProducer2::~PATCompositeTreeProducer2()
{
 
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
PATCompositeTreeProducer2::analyze(const edm::Event& iEvent, const edm::EventSetup&
iSetup)
{
    using std::vector;
    using namespace edm;
    using namespace reco;

    if(doGenNtuple_) fillGEN(iEvent,iSetup);
    if(doRecoNtuple_) fillRECO(iEvent,iSetup);

    if(saveTree_) PATCompositeNtuple->Fill();
}

void
PATCompositeTreeProducer2::fillRECO(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
#ifdef DEBUG
    using std::cout;
    using std::endl;
#endif
    //get collections
    edm::Handle<reco::VertexCollection> vertices;
    iEvent.getByToken(tok_offlinePV_,vertices);
    
    edm::Handle<reco::TrackCollection> tracks;
    iEvent.getByToken(tok_generalTrk_, tracks);

    edm::Handle<CCC> v0candidates;
    iEvent.getByToken(PATCompositeCandidateCollection_Token_,v0candidates);
    const CCC * v0candidates_ = v0candidates.product();
    
    edm::Handle<MVACollection> mvavalues;
    if(useAnyMVA_)
    {
      iEvent.getByToken(MVAValues_Token_,mvavalues);
      assert( (*mvavalues).size() == v0candidates->size() );
    }

    edm::Handle<reco::GenParticleCollection> genpars;
    if(doGenMatching_ || doGenMatchingTOF_) iEvent.getByToken(tok_genParticle_,genpars);

    edm::Handle<edm::ValueMap<reco::DeDxData> > dEdxHandle1;
    iEvent.getByToken(Dedx_Token1_, dEdxHandle1);
    
    edm::Handle<edm::ValueMap<reco::DeDxData> > dEdxHandle2;
    iEvent.getByToken(Dedx_Token2_, dEdxHandle2);
    
    centrality=-1;
    if(isCentrality_)
    {
      edm::Handle<reco::Centrality> cent;
      iEvent.getByToken(tok_centSrc_, cent);

      iEvent.getByToken(tok_centBinLabel_,cbin_);
      centrality = *cbin_;  

      HFsumETPlus = cent->EtHFtowerSumPlus();
      HFsumETMinus = cent->EtHFtowerSumMinus();
      Npixel = cent->multiplicityPixel();
      ZDCPlus = cent->zdcSumPlus();
      ZDCMinus = cent->zdcSumMinus();
    }

    if(isEventPlane_)
    {
      edm::Handle<reco::EvtPlaneCollection> eventplanes;
      iEvent.getByToken(tok_eventplaneSrc_,eventplanes);

      const reco::EvtPlane & ephfp1 = (*eventplanes)[0];
      const reco::EvtPlane & ephfm1 = (*eventplanes)[1];
      const reco::EvtPlane & ephfp2 = (*eventplanes)[6];
      const reco::EvtPlane & ephfm2 = (*eventplanes)[7];
      const reco::EvtPlane & ephfp3 = (*eventplanes)[13];
      const reco::EvtPlane & ephfm3 = (*eventplanes)[14];
     
      ephfpAngle[0] = ephfp1.angle(2);
      ephfpAngle[1] = ephfp2.angle(2);
      ephfpAngle[2] = ephfp3.angle(2);

      ephfmAngle[0] = ephfm1.angle(2);
      ephfmAngle[1] = ephfm2.angle(2);
      ephfmAngle[2] = ephfm3.angle(2);

      ephfpQ[0] = ephfp1.q(2);
      ephfpQ[1] = ephfp2.q(2);
      ephfpQ[2] = ephfp3.q(2);

      ephfmQ[0] = ephfm1.q(2);
      ephfmQ[1] = ephfm2.q(2);
      ephfmQ[2] = ephfm3.q(2);

      ephfpSumW = ephfp2.sumw();
      ephfmSumW = ephfm2.sumw();
    }

    //best vertex
    bestvz=-999.9; bestvx=-999.9; bestvy=-999.9;
    double bestvzError=-999.9, bestvxError=-999.9, bestvyError=-999.9;
    const reco::Vertex & vtx = (*vertices)[0];
    bestvz = vtx.z(); bestvx = vtx.x(); bestvy = vtx.y();
    bestvzError = vtx.zError(); bestvxError = vtx.xError(); bestvyError = vtx.yError();
    
    //Ntrkoffline
    Ntrkoffline = 0;
    if(multMax_!=-1 && multMin_!=-1)
    {
      for(unsigned it=0; it<tracks->size(); ++it){
        
        const reco::Track & trk = (*tracks)[it];
        
        math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
        
        double dzvtx = trk.dz(bestvtx);
        double dxyvtx = trk.dxy(bestvtx);
        double dzerror = sqrt(trk.dzError()*trk.dzError()+bestvzError*bestvzError);
        double dxyerror = sqrt(trk.d0Error()*trk.d0Error()+bestvxError*bestvyError);
        
        if(!trk.quality(reco::TrackBase::highPurity)) continue;
        if(fabs(trk.ptError())/trk.pt()>0.10) continue;
        if(fabs(dzvtx/dzerror) > 3) continue;
        if(fabs(dxyvtx/dxyerror) > 3) continue;
        
        double eta = trk.eta();
        double pt  = trk.pt();
        
        if(fabs(eta)>2.4) continue;
        if(pt<=0.4) continue;
        Ntrkoffline++;
      }
    }

    std::vector<reco::GenParticleRef> genRefs;
    if(doGenMatching_){
        if(!genpars.isValid())
        { cout<<"Gen matching cannot be done without Gen collection!!"<<endl; return; }
        for(unsigned int it=0; it<genpars->size(); ++it){
            const reco::GenParticle & trk = (*genpars)[it];
            int id = trk.pdgId();
            if(fabs(id)!=PID_) continue; //check is target
            if(decayInGen_ && trk.numberOfDaughters()!=2 && !threeProngDecay_) continue; //check 2-pron decay if target decays in Gen
            if(decayInGen_ && trk.numberOfDaughters()!=3 && threeProngDecay_) continue; //check 2-pron decay if target decays in Gen

            // wrong when considering two layer decay
            int nDau = threeProngDecay_ ? 3 : 2;
            std::vector<unsigned int> idxs;
            std::vector<unsigned int> permutations(nDau);
            std::iota(permutations.begin(), permutations.end(), 0);
            std::sort(permutations.begin(), permutations.end());
            if (!threeProngDecay_) {
              do {
                auto Dd1 = trk.daughter( permutations.at(0) );
                // if(fabs(Dd1->pdgId())!=421){cout << "wrongmatching: " << Dd1->pdgId() << endl;}
                auto Dd2 = trk.daughter( permutations.at(1) );
                // if(fabs(Dd2->pdgId())==421){cout << "wrongmatching: " << Dd2->pdgId() << " Dd1? " << Dd1->pdgId() << endl;}
                if (abs(Dd1->pdgId()) == PID_dau1_ && abs(Dd2->pdgId()) == PID_dau2_) {
                  if(twoLayerDecay_){
                    // Magic numbers, _permutations -> number of D0 daughters;
                    std::vector<unsigned int> _permutations(2);
                    std::iota(_permutations.begin(), _permutations.end(), 0);
                    std::sort(_permutations.begin(), _permutations.end());
                    do {
                      auto Ddd1 = Dd1->daughter( _permutations.at(0) );
                      auto Ddd2 = Dd1->daughter( _permutations.at(1) );
                      if (abs(Ddd1->pdgId()) == 211 && abs(Ddd2->pdgId()) == 321) {
                        idxs = permutations;
                        break;
                      }
                    } while (std::next_permutation(_permutations.begin(), _permutations.end()));
                    if(!idxs.empty()) break;
                  } else {
                    if (abs(Dd1->pdgId()) == PID_dau1_
                        && abs(Dd2->pdgId()) == PID_dau2_
                        ) {
                      idxs = permutations;
                      break;
                    } while (std::next_permutation(permutations.begin(), permutations.end()));
                    if(!idxs.empty()) break;
                  }
                }
              } while (std::next_permutation(permutations.begin(), permutations.end()));
            } else {
              do {
                auto Dd1 = trk.daughter( permutations.at(0) );
                auto Dd2 = trk.daughter( permutations.at(1) );
                auto Dd3 = trk.daughter( permutations.at(2) );

                if (abs(Dd1->pdgId()) == PID_dau1_
                    && abs(Dd2->pdgId()) == PID_dau2_
                    && abs(Dd3->pdgId() == PID_dau3_)) {
                  idxs = permutations;
                  break;
                }
              } while (std::next_permutation(permutations.begin(), permutations.end()));
            }
            if (decayInGen_ && idxs.empty()) continue;
            genRefs.push_back(reco::GenParticleRef(genpars, it));
        }
        //if (genRefs.size()>1) std::cout << "More than one target of generated particles\n";
    }

    //RECO Candidate info
    candSize = v0candidates_->size();
    #ifdef DEBUG
    cout << "candSize : " << candSize << endl;
    #endif

    for(unsigned it=0; it<v0candidates_->size(); ++it){
        
        const CC & trk = (*v0candidates_)[it];
        
        double secvz=-999.9, secvx=-999.9, secvy=-999.9;
        secvz = trk.vz(); secvx = trk.vx(); secvy = trk.vy();

        eta[it] = trk.eta();
        y[it] = trk.rapidity();
        pt[it] = trk.pt();
        #ifdef DEBUG
        #endif
        phi[it] = trk.phi();
        flavor[it] = trk.pdgId()/abs(trk.pdgId());

        mva[it] = 0.0;
        if(useAnyMVA_) mva[it] = (*mvavalues)[it];
        if(trk.hasUserFloat("D0mva")) mva[it] = trk.userFloat("D0mva");
        if(trk.hasUserFloat("mva")) mva[it] = trk.userFloat("mva");

        double px = trk.px();
        double py = trk.py();
        double pz = trk.pz();
        mass[it] = trk.mass();
        
        // Need cases where this is not true
        const reco::Candidate * d1 = trk.daughter(0);
        // const CC* d1 = trk.daughter(0);
        const reco::Candidate * gd1;
        const reco::Candidate * gd2;
        if(twoLayerDecay_){
          gd1 = d1->daughter(0);
          gd2 = d1->daughter(1);
        }
        const reco::Candidate * d2 = trk.daughter(1);
        const reco::Candidate * d3 = 0;        
        if(threeProngDecay_) d3 = trk.daughter(2);

        //Gen match
        if(doGenMatching_ )
        {
          if( twoLayerDecay_ ){
            matchGEN[it] = false;
            unsigned int nGen = genRefs.size();
            isSwap[it] = false;
            idmom_reco[it] = -77;
            idBAnc_reco[it] = -77;

            for( unsigned int igen=0; igen<nGen; igen++){
              auto const theGenDStar = genRefs.at(igen);
              unsigned int idxD0 = -1;
              if( abs(theGenDStar->daughter(0)->pdgId()) == 421 ) idxD0 = 0;
              auto const* theGenD0 = genRefs.at(igen)->daughter(idxD0);
              auto const* theGenPion = genRefs.at(igen)->daughter(1- idxD0);
              // Only works for 2 body two layer decay
              reco::Candidate const* recoD1;
              reco::Candidate const* recoPi;
              unsigned int idxRecoD0 = -1;
              if (abs(trk.daughter(0)->pdgId())== 421) idxRecoD0 = 0;
              recoD1 = trk.daughter(idxRecoD0);
              recoPi = trk.daughter(1-idxRecoD0);
              const auto nGenDau = theGenD0->numberOfDaughters();


             //if(debug_ ) std::cout << "nGenDau: " << nGenDau<< std::endl;

              matchGEN[it] = matchGEN[it] || (matchHadron(recoD1, *theGenD0,true) && matchHadron(recoPi, *theGenPion,false));
              #ifdef DEBUG
              cout << "matchGEN[it]: " << matchGEN[it] << endl;
                #endif
                #ifdef DEBUG
                if(genRefs.at(igen)->daughter(0)->pdgId()!=421){
                cout << "genRefs.at(igen) : " << genRefs.at(igen)->pdgId() << endl;
                cout << "nGenDau : " << nGenDau << endl;
                cout << "D0_dau1 : " << theGenD0->daughter(0)->pdgId() << endl;
                cout << "D0_dau2 : " << theGenD0->daughter(1)->pdgId() << endl;
                cout << "GenDstar_dau1 : " << theGenD0->pdgId() << endl;
                cout << "GenDstar_dau2 : " << theGenPion->pdgId() << endl;
                cout << "RecoDstar_dau1 : " << recoD1->pdgId() << endl;
                cout << "RecoDstar_dau2 : " << recoPi->pdgId() << endl;
                cout << "match D0 : " << matchHadron(recoD1, *theGenD0,true) << endl;
                cout << "match Pion : " << matchHadron(recoPi, *theGenPion,false) << endl;
                }
                #endif
                if(matchGEN[it]){

                #ifdef DEBUG
                cout << "genRefs.at(igen) : " << genRefs.at(igen)->pdgId() << endl;
                cout << "DStar pdgID : " << theGenDStar->pdgId() << endl;
                cout << "D0_dau1 : " << theGenD0->daughter(0)->pdgId() << endl;
                cout << "D0_dau2 : " << theGenD0->daughter(1)->pdgId() << endl;
                cout << "GenDstar_dau1 : " << theGenD0->pdgId() << endl;
                cout << "GenDstar_dau2 : " << theGenPion->pdgId() << endl;
                cout << "RecoDstar_dau1 : " << recoD1->pdgId() << endl;
                cout << "RecoDstar_dau2 : " << recoPi->pdgId() << endl;
                cout << "match D0 : " << matchHadron(recoD1, *theGenD0,true) << endl;
                cout << "match Pion : " << matchHadron(recoPi, *theGenPion,false) << endl;
                #endif
                  isSwap[it] = checkSwap(recoD1, *theGenD0);
                  auto mom_ref = findMother(theGenDStar);
                  if (mom_ref.isNonnull()) idmom_reco[it] = mom_ref->pdgId();
                  int __count_anc__ = 0;
                  auto __ref_anc__ = mom_ref;
                  while ( __ref_anc__.isNonnull() && __count_anc__ < 50 ){
                  #ifdef DEBUG
                    cout << "ref ancestor: " <<  __ref_anc__->pdgId() << endl;
                  #endif
                    __ref_anc__ = findMother(__ref_anc__);
                    if( __ref_anc__.isNonnull()){
                      if( ((int) abs(__ref_anc__->pdgId())) % 1000 / 100 == 5){ 
                        idBAnc_reco[it] = __ref_anc__->pdgId();
                  } } }
                  matchGen_DStarpT_[it] = theGenDStar->pt();
                  matchGen_DStareta_[it] = theGenDStar->eta();
                  matchGen_DStarphi_[it] = theGenDStar->phi();
                  matchGen_DStarmass_[it] = theGenDStar->mass();
                  matchGen_DStary_[it] = theGenDStar->rapidity();
                  matchGen_DStarcharge_[it] = theGenDStar->charge();
                  matchGen_DStarpdgId_[it] = theGenDStar->pdgId();

                  matchGen_D0pT_[it] = theGenD0->pt();
                  matchGen_D0eta_[it] = theGenD0->eta();
                  matchGen_D0phi_[it] = theGenD0->phi();
                  matchGen_D0mass_[it] = theGenD0->mass();
                  matchGen_D0y_[it] = theGenD0->rapidity();
                  matchGen_D0charge_[it] = theGenD0->charge();
                  matchGen_D0pdgId_[it] = theGenD0->pdgId();

                  genDecayLength(*theGenDStar, matchGen_D1decayLength2D_[it], matchGen_D1decayLength3D_[it], matchGen_D1angle2D_[it], matchGen_D1angle3D_[it] );
                  getAncestorId(*theGenDStar, matchGen_D1ancestorId_[it], matchGen_D1ancestorFlavor_[it] );

                  const auto* genDau0 = theGenD0->daughter(0);
                  const auto* genDau1 = theGenD0->daughter(1);

                  matchGen_D0Dau1_pT_[it] = genDau0->pt();
                  matchGen_D0Dau1_eta_[it] = genDau0->eta();
                  matchGen_D0Dau1_phi_[it] = genDau0->phi();
                  matchGen_D0Dau1_mass_[it] = genDau0->mass();
                  matchGen_D0Dau1_y_[it] = genDau0->rapidity();
                  matchGen_D0Dau1_charge_[it] = genDau0->charge();
                  matchGen_D0Dau1_pdgId_[it] = genDau0->pdgId();

                  matchGen_D0Dau2_pT_[it] = genDau1->pt();
                  matchGen_D0Dau2_eta_[it] = genDau1->eta();
                  matchGen_D0Dau2_phi_[it] = genDau1->phi();
                  matchGen_D0Dau2_mass_[it] = genDau1->mass();
                  matchGen_D0Dau2_y_[it] = genDau1->rapidity();
                  matchGen_D0Dau2_charge_[it] = genDau1->charge();
                  matchGen_D0Dau2_pdgId_[it] = genDau1->pdgId();

                  matchGen_D1pT_[it] = theGenPion->pt();
                  matchGen_D1eta_[it] = theGenPion->eta();
                  matchGen_D1phi_[it] = theGenPion->phi();
                  matchGen_D1mass_[it] = theGenPion->mass();
                  matchGen_D1y_[it] = theGenPion->rapidity();
                  matchGen_D1charge_[it] = theGenPion->charge();
                  matchGen_D1pdgId_[it] = theGenPion->pdgId();
                  break;
                }
              } // END for nGen
            }
            else {
              matchGEN[it] = false;
              unsigned int nGen = genRefs.size();
              isSwap[it] = false;
              idmom_reco[it] = -77;
              idBAnc_reco[it] = -77;

              for( unsigned int igen=0; igen<nGen; igen++){
                auto const theGenP = genRefs.at(igen);
                #ifdef DEBUG 
                cout << "theGenP pdgId : " << theGenP->pdgId() << endl;
                #endif
                matchGEN[it] = matchGEN[it] || matchHadron(&trk, *theGenP,true);
                if(matchGEN[it]){
                  isSwap[it] = checkSwap(&trk, *theGenP);
                  auto mom_ref = findMother(theGenP);
                  if (mom_ref.isNonnull()) idmom_reco[it] = mom_ref->pdgId();
                  int __count_anc__ = 0;
                  auto __ref_anc__ = mom_ref;
                  while ( __ref_anc__.isNonnull() && __count_anc__ < 50 ){
                    __ref_anc__ = findMother(__ref_anc__);
                    if( __ref_anc__.isNonnull()){
                      if( ((int) abs(__ref_anc__->pdgId())) % 1000 / 100 == 5){ 
                        idBAnc_reco[it] = __ref_anc__->pdgId();
                  } } }

                  matchGen_D0pT_[it] = theGenP->pt();
                  matchGen_D0eta_[it] = theGenP->eta();
                  matchGen_D0phi_[it] = theGenP->phi();
                  matchGen_D0mass_[it] = theGenP->mass();
                  matchGen_D0y_[it] = theGenP->rapidity();
                  matchGen_D0charge_[it] = theGenP->charge();
                  matchGen_D0pdgId_[it] = theGenP->pdgId();

                  genDecayLength(*theGenP, matchGen_D1decayLength2D_[it], matchGen_D1decayLength3D_[it], matchGen_D1angle2D_[it], matchGen_D1angle3D_[it] );
                  getAncestorId(*theGenP, matchGen_D1ancestorId_[it], matchGen_D1ancestorFlavor_[it] );

                  const auto* genDau0 = theGenP->daughter(0);
                  const auto* genDau1 = theGenP->daughter(1);

                  matchGen_D0Dau1_pT_[it] = genDau0->pt();
                  matchGen_D0Dau1_eta_[it] = genDau0->eta();
                  matchGen_D0Dau1_phi_[it] = genDau0->phi();
                  matchGen_D0Dau1_mass_[it] = genDau0->mass();
                  matchGen_D0Dau1_y_[it] = genDau0->rapidity();
                  matchGen_D0Dau1_charge_[it] = genDau0->charge();
                  matchGen_D0Dau1_pdgId_[it] = genDau0->pdgId();

                  matchGen_D0Dau2_pT_[it] = genDau1->pt();
                  matchGen_D0Dau2_eta_[it] = genDau1->eta();
                  matchGen_D0Dau2_phi_[it] = genDau1->phi();
                  matchGen_D0Dau2_mass_[it] = genDau1->mass();
                  matchGen_D0Dau2_y_[it] = genDau1->rapidity();
                  matchGen_D0Dau2_charge_[it] = genDau1->charge();
                  matchGen_D0Dau2_pdgId_[it] = genDau1->pdgId();
                  break;
                }
              } // END for nGen
            }
          }
          
          double pxd1 = d1->px();
          double pyd1 = d1->py();
          double pzd1 = d1->pz();
          double pxd2 = d2->px();
          double pyd2 = d2->py();
          double pzd2 = d2->pz();
          
          TVector3 dauvec1(pxd1,pyd1,pzd1);
          TVector3 dauvec2(pxd2,pyd2,pzd2);
          
          //pt
          pt1[it] = d1->pt();
          pt2[it] = d2->pt();
          
          //momentum
          p1[it] = d1->p();
          p2[it] = d2->p();
          
          //eta
          eta1[it] = d1->eta();
          eta2[it] = d2->eta();
          
          //phi
          phi1[it] = d1->phi();
          phi2[it] = d2->phi();
          
          //charge
          charge1[it] = d1->charge();
          charge2[it] = d2->charge();
          
          double pxd3 = -999.9;
          double pyd3 = -999.9;
          double pzd3 = -999.9;
          if(threeProngDecay_ && d3)
          {
            pxd3 = d3->px();
            pyd3 = d3->py();
            pzd3 = d3->pz();
            pt3[it] = d3->pt();
            p3[it] = d3->p();
            eta3[it] = d3->eta();
            phi3[it] = d3->phi();
            charge3[it] = d3->charge();
          }
          TVector3 dauvec3(pxd3,pyd3,pzd3);

          pid1[it] = -99999;
          pid2[it] = -99999;
          if(doGenMatchingTOF_)
          {
            for(unsigned it=0; it<genpars->size(); ++it){

                const reco::GenParticle & trk = (*genpars)[it];

                if(trk.pt()<0.001) continue;

                int id = trk.pdgId();
                TVector3 trkvect(trk.px(),trk.py(),trk.pz());

                if(fabs(id)!=PID_ && trk.charge())
                {
                  // matching daughter 1
                  double deltaR = trkvect.DeltaR(dauvec1);
                  if(deltaR < deltaR_ && fabs((trk.pt()-pt1[it])/pt1[it]) < 0.5 && trk.charge()==charge1[it] && pid1[it]==-99999)
                  {
                    pid1[it] = id;
                  } 

                  // matching daughter 2
                  deltaR = trkvect.DeltaR(dauvec2);
                  if(deltaR < deltaR_ && fabs((trk.pt()-pt2[it])/pt2[it]) < 0.5 && trk.charge()==charge2[it] && pid2[it]==-99999)
                  {
                    pid2[it] = id;
                  }
                }

                if(fabs(id)==PID_ && trk.numberOfDaughters()==2)
                {
                  const reco::Candidate * Dd1 = trk.daughter(0);
                  const reco::Candidate * Dd2 = trk.daughter(1);
                  TVector3 d1vect(Dd1->px(),Dd1->py(),Dd1->pz());
                  TVector3 d2vect(Dd2->px(),Dd2->py(),Dd2->pz());
                  int id1 = Dd1->pdgId();
                  int id2 = Dd2->pdgId();
                
                  double deltaR = d1vect.DeltaR(dauvec1);
                  if(deltaR < deltaR_ && fabs((Dd1->pt()-pt1[it])/pt1[it]) < 0.5 && Dd1->charge()==charge1[it] && pid1[it]==-99999)
                  {
                    pid1[it] = id1;
                  }
                  deltaR = d2vect.DeltaR(dauvec1);
                  if(deltaR < deltaR_ && fabs((Dd2->pt()-pt1[it])/pt1[it]) < 0.5 && Dd2->charge()==charge1[it] && pid1[it]==-99999)
                  {
                    pid1[it] = id1;
                  }

                  deltaR = d1vect.DeltaR(dauvec2);
                  if(deltaR < deltaR_ && fabs((Dd1->pt()-pt2[it])/pt2[it]) < 0.5 && Dd1->charge()==charge2[it] && pid2[it]==-99999)
                  {
                    pid2[it] = id2;
                  }
                  deltaR = d2vect.DeltaR(dauvec2);
                  if(deltaR < deltaR_ && fabs((Dd2->pt()-pt2[it])/pt2[it]) < 0.5 && Dd2->charge()==charge2[it] && pid2[it]==-99999)
                  {
                    pid2[it] = id2;
                  }
                }

                if(pid1[it]!=-99999 && pid2[it]!=-99999) break;
            }
          }

          //vtxChi2
          // reco::Vertex& vtx = trk.userData("DStarVtx");
          vtxChi2[it] = trk.userFloat("VtxChi2");
          ndf[it] = trk.userFloat("VtxNdof");
          VtxProb[it] = TMath::Prob(vtxChi2[it],ndf[it]);
          
          //PAngle
          TVector3 ptosvec(secvx-bestvx,secvy-bestvy,secvz-bestvz);
          TVector3 secvec(px,py,pz);
          
          TVector3 ptosvec2D(secvx-bestvx,secvy-bestvy,0);
          TVector3 secvec2D(px,py,0);
          
          // agl[it] = cos(secvec.Angle(ptosvec));
          // agl_abs[it] = secvec.Angle(ptosvec);
          // agl2D[it] = cos(secvec2D.Angle(ptosvec2D));
          // agl2D_abs[it] = secvec2D.Angle(ptosvec2D);

          agl[it] = cos(trk.userFloat("alpha3D"));
          agl_abs[it] = trk.userFloat("alpha3D");
          agl2D[it] = cos(trk.userFloat("alpha2D"));
          agl2D_abs[it] = trk.userFloat("alpha2D");
          
          //Decay length 3D
          // typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3> > SMatrixSym3D;
          // typedef ROOT::Math::SVector<double, 3> SVector3;
          // typedef ROOT::Math::SVector<double, 6> SVector6;
          
          // SMatrixSym3D totalCov = vtx.covariance() + trk.vertexCovariance();
          // SVector3 distanceVector(secvx-bestvx,secvy-bestvy,secvz-bestvz);
          
          // dl[it] = ROOT::Math::Mag(distanceVector);
          // dlerror[it] = sqrt(ROOT::Math::Similarity(totalCov, distanceVector))/dl[it];

          dl[it] = trk.userFloat("decaylength3D");
          dlos[it] = trk.userFloat("decaylengthsignif3D");
          
          dlerror[it] = dl[it]/dlos[it];
          
          //Decay length 2D
          // SVector6 v1(vtx.covariance(0,0), vtx.covariance(0,1),vtx.covariance(1,1),0,0,0);
          // SVector6 v2(trk.vertexCovariance(0,0), trk.vertexCovariance(0,1),trk.vertexCovariance(1,1),0,0,0);
          
          // SMatrixSym3D sv1(v1);
          // SMatrixSym3D sv2(v2);
          
          // SMatrixSym3D totalCov2D = sv1 + sv2;
          // SVector3 distanceVector2D(secvx-bestvx,secvy-bestvy,0);
          
          dl2D[it] = trk.userFloat("decaylength2D");
          // double dl2Derror = sqrt(ROOT::Math::Similarity(totalCov2D, distanceVector2D))/dl2D[it];
          
          dlos2D[it] = trk.userFloat("decaylengthsignif2D");
          trk3Ddca[it] = twoLayerDecay_? ((CC*)d1)->userFloat("track3DDCA") : trk.userFloat("track3DDCA");
          trk3DdcaErr[it] = twoLayerDecay_? ((CC*)d1)->userFloat("track3DDCAErr") : trk.userFloat("track3DDCAErr");
          dca3D[it] = twoLayerDecay_? ((CC*)d1)->userFloat("dca3D") : trk.userFloat("dca3D");
          dca3DErr[it] = twoLayerDecay_? ((CC*)d1)->userFloat("dca3DErr") : trk.userFloat("dca3DErr");

          //trk info
          auto dau1 = d1->get<reco::TrackRef>();
          if(!twoLayerDecay_)
          {
              //trk quality
              trkquality1[it] = dau1->quality(reco::TrackBase::highPurity);
              
              //trk dEdx
              H2dedx1[it] = -999.9;
              
              if(dEdxHandle1.isValid()){
                  const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle1.product();
                  H2dedx1[it] = dEdxTrack[dau1].dEdx();
              }
              
              T4dedx1[it] = -999.9;
              
              if(dEdxHandle2.isValid()){
                  const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle2.product();
                  T4dedx1[it] = dEdxTrack[dau1].dEdx();
              }
              
              //track Chi2
              trkChi1[it] = dau1->normalizedChi2();
              
              //track pT error
              ptErr1[it] = dau1->ptError();
              
              //vertexCovariance 00-xError 11-y 22-z
              secvz = trk.vz(); secvx = trk.vx(); secvy = trk.vy();
              
              //trkNHits
              nhit1[it] = dau1->numberOfValidHits();
              
              //DCA
              math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
              
              double dzbest1 = dau1->dz(bestvtx);
              double dxybest1 = dau1->dxy(bestvtx);
              double dzerror1 = sqrt(dau1->dzError()*dau1->dzError()+bestvzError*bestvzError);
              double dxyerror1 = sqrt(dau1->d0Error()*dau1->d0Error()+bestvxError*bestvyError);
              
              dzos1[it] = dzbest1/dzerror1;
              dxyos1[it] = dxybest1/dxyerror1;
              dzval1[it] = dzbest1;
              dxyval1[it] = dxybest1;
          }
          
          auto dau2 = d2->get<reco::TrackRef>();
          
          //trk quality
          trkquality2[it] = dau2->quality(reco::TrackBase::highPurity);
          
          //trk dEdx
          H2dedx2[it] = -999.9;
          
          if(dEdxHandle1.isValid()){
              const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle1.product();
              H2dedx2[it] = dEdxTrack[dau2].dEdx();
          }
          
          T4dedx2[it] = -999.9;
          
          if(dEdxHandle2.isValid()){
              const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle2.product();
              T4dedx2[it] = dEdxTrack[dau2].dEdx();
          }
          
          //track Chi2
          trkChi2[it] = dau2->normalizedChi2();
          
          //track pT error
          ptErr2[it] = dau2->ptError();
          
          //vertexCovariance 00-xError 11-y 22-z
          secvz = trk.vz(); secvx = trk.vx(); secvy = trk.vy();
          
          //trkNHits
          nhit2[it] = dau2->numberOfValidHits();
          
          //DCA
          math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
          
          double dzbest2 = dau2->dz(bestvtx);
          double dxybest2 = dau2->dxy(bestvtx);
          double dzerror2 = sqrt(dau2->dzError()*dau2->dzError()+bestvzError*bestvzError);
          double dxyerror2 = sqrt(dau2->d0Error()*dau2->d0Error()+bestvxError*bestvyError);
          
          dzos2[it] = dzbest2/dzerror2;
          dxyos2[it] = dxybest2/dxyerror2;
          dzval2[it] = dzbest2;
          dxyval2[it] = dxybest2;
          
          if(doMuon_)
          {
            edm::Handle<reco::MuonCollection> theMuonHandle;
            iEvent.getByToken(tok_muon_, theMuonHandle);
              
            nmatchedch1[it] = -1;
            nmatchedst1[it] = -1;
            matchedenergy1[it] = -1;
            nmatchedch2[it] = -1;
            nmatchedst2[it] = -1;
            matchedenergy2[it] = -1;
              
            double x_exp = -999.;
            double y_exp = -999.;
            double xerr_exp = -999.;
            double yerr_exp = -999.;
            double dxdz_exp = -999.;
            double dydz_exp = -999.;
            double dxdzerr_exp = -999.;
            double dydzerr_exp = -999.;
              
            double x_seg = -999.;
            double y_seg = -999.;
            double xerr_seg = -999.;
            double yerr_seg = -999.;
            double dxdz_seg = -999.;
            double dydz_seg = -999.;
            double dxdzerr_seg = -999.;
            double dydzerr_seg = -999.;
              
            double dx_seg = 999.;
            double dy_seg = 999.;
            double dxerr_seg = 999.;
            double dyerr_seg = 999.;
            double dxSig_seg = 999.;
            double dySig_seg = 999.;
            double ddxdz_seg = 999.;
            double ddydz_seg = 999.;
            double ddxdzerr_seg = 999.;
            double ddydzerr_seg = 999.;
            double ddxdzSig_seg = 999.;
            double ddydzSig_seg = 999.;
              
            onestmuon1[it] = false;
            pfmuon1[it] = false;
            glbmuon1[it] = false;
            trkmuon1[it] = false;
            calomuon1[it] = false; 
            softmuon1[it] = false;
            onestmuon2[it] = false;
            pfmuon2[it] = false;
            glbmuon2[it] = false;
            trkmuon2[it] = false;
            calomuon2[it] = false;
            softmuon2[it] = false;

            const int muId1 = muAssocToTrack( dau1, theMuonHandle );
            const int muId2 = muAssocToTrack( dau2, theMuonHandle );

            if( muId1 != -1 )
            {
              const reco::Muon& cand = (*theMuonHandle)[muId1];

              onestmuon1[it] = muon::isGoodMuon(cand, muon::selectionTypeFromString("TMOneStationTight"));
              pfmuon1[it] =  cand.isPFMuon();
              glbmuon1[it] =  cand.isGlobalMuon();
              trkmuon1[it] =  cand.isTrackerMuon();
              calomuon1[it] =  cand.isCaloMuon();

              if( 
                  //glbmuon1[it] && 
                  trkmuon1[it] &&
                  cand.innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 && 
                  cand.innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0 && 
                  fabs(cand.innerTrack()->dxy(vtx.position())) < 0.3 &&
                  fabs(cand.innerTrack()->dz(vtx.position())) < 20.
                ) softmuon1[it] = true;
            }

            if( muId2 != -1 )
            {
              const reco::Muon& cand = (*theMuonHandle)[muId2];

              onestmuon2[it] = muon::isGoodMuon(cand, muon::selectionTypeFromString("TMOneStationTight"));
              pfmuon2[it] =  cand.isPFMuon();
              glbmuon2[it] =  cand.isGlobalMuon();
              trkmuon2[it] =  cand.isTrackerMuon();
              calomuon2[it] =  cand.isCaloMuon();

              if(
                  //glbmuon2[it] && 
                  trkmuon2[it] &&
                  cand.innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
                  cand.innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0 &&
                  fabs(cand.innerTrack()->dxy(vtx.position())) < 0.3 &&
                  fabs(cand.innerTrack()->dz(vtx.position())) < 20.
                ) softmuon2[it] = true;
            }

            if(doMuonFull_)
            {

            if( muId1 != -1 )
            {
              const reco::Muon& cand = (*theMuonHandle)[muId1];

              nmatchedch1[it] = cand.numberOfMatches();
              nmatchedst1[it] = cand.numberOfMatchedStations();
                    
              reco::MuonEnergy muenergy = cand.calEnergy();
              matchedenergy1[it] = muenergy.hadMax;
                      
              const std::vector<reco::MuonChamberMatch>& muchmatches = cand.matches();
                    
              for(unsigned int ich=0;ich<muchmatches.size();ich++)
              {
                x_exp = muchmatches[ich].x;
                y_exp = muchmatches[ich].y;
                xerr_exp = muchmatches[ich].xErr;
                yerr_exp = muchmatches[ich].yErr;
                dxdz_exp = muchmatches[ich].dXdZ;
                dydz_exp = muchmatches[ich].dYdZ;
                dxdzerr_exp = muchmatches[ich].dXdZErr;
                dydzerr_exp = muchmatches[ich].dYdZErr;
                          
                std::vector<reco::MuonSegmentMatch> musegmatches = muchmatches[ich].segmentMatches;
                          
                if(!musegmatches.size()) continue;
                for(unsigned int jseg=0;jseg<musegmatches.size();jseg++)
                {
                  x_seg = musegmatches[jseg].x;
                  y_seg = musegmatches[jseg].y;
                  xerr_seg = musegmatches[jseg].xErr;
                  yerr_seg = musegmatches[jseg].yErr;
                  dxdz_seg = musegmatches[jseg].dXdZ;
                  dydz_seg = musegmatches[jseg].dYdZ;
                  dxdzerr_seg = musegmatches[jseg].dXdZErr;
                  dydzerr_seg = musegmatches[jseg].dYdZErr;
                              
                  if(sqrt((x_seg-x_exp)*(x_seg-x_exp)+(y_seg-y_exp)*(y_seg-y_exp))<sqrt(dx_seg*dx_seg+dy_seg*dy_seg))
                  {
                    dx_seg = x_seg - x_exp;
                    dy_seg = y_seg - y_exp;
                    dxerr_seg = sqrt(xerr_seg*xerr_seg+xerr_exp*xerr_exp);
                    dyerr_seg = sqrt(yerr_seg*yerr_seg+yerr_exp*yerr_exp);
                    dxSig_seg = dx_seg / dxerr_seg;
                    dySig_seg = dy_seg / dyerr_seg;
                    ddxdz_seg = dxdz_seg - dxdz_exp;
                    ddydz_seg = dydz_seg - dydz_exp;
                    ddxdzerr_seg = sqrt(dxdzerr_seg*dxdzerr_seg+dxdzerr_exp*dxdzerr_exp);
                    ddydzerr_seg = sqrt(dydzerr_seg*dydzerr_seg+dydzerr_exp*dydzerr_exp);
                    ddxdzSig_seg = ddxdz_seg / ddxdzerr_seg;
                    ddydzSig_seg = ddydz_seg / ddydzerr_seg;
                  }
                }
                        
                dx1_seg_[it]=dx_seg;
                dy1_seg_[it]=dy_seg;
                dxSig1_seg_[it]=dxSig_seg;
                dySig1_seg_[it]=dySig_seg;
                ddxdz1_seg_[it]=ddxdz_seg;
                ddydz1_seg_[it]=ddydz_seg;
                ddxdzSig1_seg_[it]=ddxdzSig_seg;
                ddydzSig1_seg_[it]=ddydzSig_seg;
              }
            } 

            if( muId2 != -1 )
            {
              const reco::Muon& cand = (*theMuonHandle)[muId2];

              nmatchedch2[it] = cand.numberOfMatches();
              nmatchedst2[it] = cand.numberOfMatchedStations();
                      
              reco::MuonEnergy muenergy = cand.calEnergy();
              matchedenergy2[it] = muenergy.hadMax;
                      
              const std::vector<reco::MuonChamberMatch>& muchmatches = cand.matches();
              for(unsigned int ich=0;ich<muchmatches.size();ich++)
                          //                        for(unsigned int ich=0;ich<1;ich++)
              {
                x_exp = muchmatches[ich].x;
                y_exp = muchmatches[ich].y;
                xerr_exp = muchmatches[ich].xErr;
                yerr_exp = muchmatches[ich].yErr;
                dxdz_exp = muchmatches[ich].dXdZ;
                dydz_exp = muchmatches[ich].dYdZ;
                dxdzerr_exp = muchmatches[ich].dXdZErr;
                dydzerr_exp = muchmatches[ich].dYdZErr;
                          
                std::vector<reco::MuonSegmentMatch> musegmatches = muchmatches[ich].segmentMatches;
                          
                if(!musegmatches.size()) continue;
                for(unsigned int jseg=0;jseg<musegmatches.size();jseg++)
                {
                  x_seg = musegmatches[jseg].x;
                  y_seg = musegmatches[jseg].y;
                  xerr_seg = musegmatches[jseg].xErr;
                  yerr_seg = musegmatches[jseg].yErr;
                  dxdz_seg = musegmatches[jseg].dXdZ;
                  dydz_seg = musegmatches[jseg].dYdZ;
                  dxdzerr_seg = musegmatches[jseg].dXdZErr;
                  dydzerr_seg = musegmatches[jseg].dYdZErr;
                              
                  if(sqrt((x_seg-x_exp)*(x_seg-x_exp)+(y_seg-y_exp)*(y_seg-y_exp))<sqrt(dx_seg*dx_seg+dy_seg*dy_seg))
                  {
                    dx_seg = x_seg - x_exp;
                    dy_seg = y_seg - y_exp;
                    dxerr_seg = sqrt(xerr_seg*xerr_seg+xerr_exp*xerr_exp);
                    dyerr_seg = sqrt(yerr_seg*yerr_seg+yerr_exp*yerr_exp);
                    dxSig_seg = dx_seg / dxerr_seg;
                    dySig_seg = dy_seg / dyerr_seg;
                    ddxdz_seg = dxdz_seg - dxdz_exp;
                    ddydz_seg = dydz_seg - dydz_exp;
                    ddxdzerr_seg = sqrt(dxdzerr_seg*dxdzerr_seg+dxdzerr_exp*dxdzerr_exp);
                    ddydzerr_seg = sqrt(dydzerr_seg*dydzerr_seg+dydzerr_exp*dydzerr_exp);
                    ddxdzSig_seg = ddxdz_seg / ddxdzerr_seg;
                    ddydzSig_seg = ddydz_seg / ddydzerr_seg;
                  }
                }
                          
                dx2_seg_[it]=dx_seg;
                dy2_seg_[it]=dy_seg;
                dxSig2_seg_[it]=dxSig_seg;
                dySig2_seg_[it]=dySig_seg;
                ddxdz2_seg_[it]=ddxdz_seg;
                ddydz2_seg_[it]=ddydz_seg;
                ddxdzSig2_seg_[it]=ddxdzSig_seg;
                ddydzSig2_seg_[it]=ddydzSig_seg;
              }
            }
            } // doMuonFull
          }
          
          if(twoLayerDecay_)
          {
              grand_mass[it] = d1->mass();
              
              // const reco::Candidate * gd1 = d1->daughter(0);
              // const reco::Candidate * gd2 = d1->daughter(1);
              
              double gpxd1 = gd1->px();
              double gpyd1 = gd1->py();
              double gpzd1 = gd1->pz();
              double gpxd2 = gd2->px();
              double gpyd2 = gd2->py();
              double gpzd2 = gd2->pz();
              
              TVector3 gdauvec1(gpxd1,gpyd1,gpzd1);
              TVector3 gdauvec2(gpxd2,gpyd2,gpzd2);
              
              auto gdau1 = gd1->get<reco::TrackRef>();
              auto gdau2 = gd2->get<reco::TrackRef>();
              
              //trk quality
              
              grand_trkquality1[it] = gdau1->quality(reco::TrackBase::highPurity);
              grand_trkquality2[it] = gdau2->quality(reco::TrackBase::highPurity);
              
              //trk dEdx
              grand_H2dedx1[it] = -999.9;
              grand_H2dedx2[it] = -999.9;
              
              if(dEdxHandle1.isValid()){
                  const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle1.product();
                  grand_H2dedx1[it] = dEdxTrack[gdau1].dEdx();
                  grand_H2dedx2[it] = dEdxTrack[gdau2].dEdx();
              }
              
              grand_T4dedx1[it] = -999.9;
              grand_T4dedx2[it] = -999.9;
              
              if(dEdxHandle2.isValid()){
                  const edm::ValueMap<reco::DeDxData> dEdxTrack = *dEdxHandle2.product();
                  grand_T4dedx1[it] = dEdxTrack[gdau1].dEdx();
                  grand_T4dedx2[it] = dEdxTrack[gdau2].dEdx();
              }
              
              //track pt
              grand_pt1[it] = gd1->pt();
              grand_pt2[it] = gd2->pt();
              
              //track momentum
              grand_p1[it] = gd1->p();
              grand_p2[it] = gd2->p();
              
              //track eta
              grand_eta1[it] = gd1->eta();
              grand_eta2[it] = gd2->eta();
              
              //track charge
              grand_charge1[it] = gd1->charge();
              grand_charge2[it] = gd2->charge();
              
              //track Chi2
              grand_trkChi1[it] = gdau1->normalizedChi2();
              grand_trkChi2[it] = gdau2->normalizedChi2();
              
              //track pT error
              grand_ptErr1[it] = gdau1->ptError();
              grand_ptErr2[it] = gdau2->ptError();
              
              //vertexCovariance 00-xError 11-y 22-z
              secvz = d1->vz(); secvx = d1->vx(); secvy = d1->vy();
              
              //trkNHits
              grand_nhit1[it] = gdau1->numberOfValidHits();
              grand_nhit2[it] = gdau2->numberOfValidHits();
              
              //DCA
              math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
              
              double gdzbest1 = gdau1->dz(bestvtx);
              double gdxybest1 = gdau1->dxy(bestvtx);
              double gdzerror1 = sqrt(gdau1->dzError()*gdau1->dzError()+bestvzError*bestvzError);
              double gdxyerror1 = sqrt(gdau1->d0Error()*gdau1->d0Error()+bestvxError*bestvyError);
              
              grand_dzos1[it] = gdzbest1/gdzerror1;
              grand_dxyos1[it] = gdxybest1/gdxyerror1;
              
              double gdzbest2 = gdau2->dz(bestvtx);
              double gdxybest2 = gdau2->dxy(bestvtx);
              double gdzerror2 = sqrt(gdau2->dzError()*gdau2->dzError()+bestvzError*bestvzError);
              double gdxyerror2 = sqrt(gdau2->d0Error()*gdau2->d0Error()+bestvxError*bestvyError);
              
              grand_dzos2[it] = gdzbest2/gdzerror2;
              grand_dxyos2[it] = gdxybest2/gdxyerror2;
              
              //vtxChi2 -> is D0 for D* analysis 
              // If we want to make it more general, should be a case where either the candidate is reco::Candidate or pat::CompositeCandidate
              // grand_vtxChi2[it] = d1->vertexChi2();
              // grand_ndf[it] = d1->vertexNdof();
              grand_vtxChi2[it] = ((CC*) d1)->userFloat("VtxChi2");
              grand_ndf[it] = ((CC*) d1)->userFloat("VtxNdof");
              grand_VtxProb[it] = TMath::Prob(grand_vtxChi2[it],grand_ndf[it]);
              
              //PAngle
              TVector3 ptosvec(secvx-bestvx,secvy-bestvy,secvz-bestvz);
              TVector3 secvec(d1->px(),d1->py(),d1->pz());
              
              TVector3 ptosvec2D(secvx-bestvx,secvy-bestvy,0);
              TVector3 secvec2D(d1->px(),d1->py(),0);
              
              grand_agl[it] = cos(secvec.Angle(ptosvec));
              grand_agl_abs[it] = secvec.Angle(ptosvec);
              
              grand_agl2D[it] = cos(secvec2D.Angle(ptosvec2D));
              grand_agl2D_abs[it] = secvec2D.Angle(ptosvec2D);
              
              //Decay length 3D
              typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3> > SMatrixSym3D;
              typedef ROOT::Math::SVector<double, 3> SVector3;
              typedef ROOT::Math::SVector<double, 6> SVector6;
              
              // const reco::Vertex* d1vtx;
              // d1vtx = ((CC*) d1)->userData<reco::Vertex>("D0Vtx");
              CC* d1CC = (CC*) d1;
              // SMatrixSym3D d1Cov = d1vtx.vertexCovariance(); 
              // SMatrixSym3D totalCov = vtx.covariance() + d1Cov;
              // SVector3 distanceVector(secvx-bestvx,secvy-bestvy,secvz-bestvz);
              
              // grand_dl[it] = ROOT::Math::Mag(distanceVector);
              // grand_dlerror[it] = sqrt(ROOT::Math::Similarity(totalCov, distanceVector))/grand_dl[it];
              
              // grand_dlos[it] = grand_dl[it]/grand_dlerror[it];

              grand_dl[it] = d1CC->userFloat("decaylength3D");
              grand_dlos[it] = d1CC->userFloat("decaylengthsignif3D");
              grand_dlerror[it] = grand_dl[it]/grand_dlos[it];
              
              //Decay length 2D
              // SVector6 v1(vtx.covariance(0,0), vtx.covariance(0,1),vtx.covariance(1,1),0,0,0);
              // SVector6 v2(d1->vertexCovariance(0,0), d1->vertexCovariance(0,1),d1->vertexCovariance(1,1),0,0,0);
              
              // SMatrixSym3D sv1(v1);
              // SMatrixSym3D sv2(v2);
              
              // SMatrixSym3D totalCov2D = sv1 + sv2;
              // SVector3 distanceVector2D(secvx-bestvx,secvy-bestvy,0);
              
              // double gdl2D = ROOT::Math::Mag(distanceVector2D);
              // double gdl2Derror = sqrt(ROOT::Math::Similarity(totalCov2D, distanceVector2D))/gdl2D;
              
              // grand_dlos2D[it] = gdl2D/gdl2Derror;

              double gdl2D = d1CC->userFloat("decaylength2D");
              grand_dlos2D[it] = d1CC->userFloat("decaylengthsignif2D");
              //D03Ddca[it] = trk.userFloat("D03DDCA");
              //D03DdcaErr[it] = trk.userFloat("D03DDCAErr");

              double gdl2Derror = gdl2D/grand_dlos2D[it];
              
          }
  #ifdef DEBUG
  cout << "Done reco single iter" << endl;
  #endif

          if(saveHistogram_)
          {
            for(unsigned int ipt=0;ipt<pTBins_.size()-1;ipt++)
              for(unsigned int iy=0;iy<yBins_.size()-1;iy++)
              {
                if(pt[it]<pTBins_[ipt+1] && pt[it]>pTBins_[ipt] && y[it]<yBins_[iy+1] && y[it]>yBins_[iy])
                {
                  hMassVsMVA[iy][ipt]->Fill(mva[it],mass[it]);
  //                h3DDCAVsMVA[iy][ipt]->Fill(mva[it],dl[it]*sin(agl_abs[it]));
  //                h2DDCAVsMVA[iy][ipt]->Fill(mva[it],dl2D[it]*sin(agl2D_abs[it]));

                  if(saveAllHistogram_)
                  {
                  hpTVsMVA[iy][ipt]->Fill(mva[it],pt[it]);
                  hetaVsMVA[iy][ipt]->Fill(mva[it],eta[it]);
                  hyVsMVA[iy][ipt]->Fill(mva[it],y[it]);
                  hVtxProbVsMVA[iy][ipt]->Fill(mva[it],VtxProb[it]);
                  h3DCosPointingAngleVsMVA[iy][ipt]->Fill(mva[it],agl[it]);
                  h3DPointingAngleVsMVA[iy][ipt]->Fill(mva[it],agl_abs[it]);
                  h2DCosPointingAngleVsMVA[iy][ipt]->Fill(mva[it],agl2D[it]);
                  h2DPointingAngleVsMVA[iy][ipt]->Fill(mva[it],agl2D_abs[it]);
                  h3DDecayLengthSignificanceVsMVA[iy][ipt]->Fill(mva[it],dlos[it]);
                  h3DDecayLengthVsMVA[iy][ipt]->Fill(mva[it],dl[it]);
                  h2DDecayLengthSignificanceVsMVA[iy][ipt]->Fill(mva[it],dlos2D[it]);
                  h2DDecayLengthVsMVA[iy][ipt]->Fill(mva[it],dl2D[it]);
                  hzDCASignificanceDaugther1VsMVA[iy][ipt]->Fill(mva[it],dzos1[it]);
                  hxyDCASignificanceDaugther1VsMVA[iy][ipt]->Fill(mva[it],dxyos1[it]);
                  hNHitD1VsMVA[iy][ipt]->Fill(mva[it],nhit1[it]);
                  hpTD1VsMVA[iy][ipt]->Fill(mva[it],pt1[it]);
                  hpTerrD1VsMVA[iy][ipt]->Fill(mva[it],ptErr1[it]/pt1[it]);
                  hEtaD1VsMVA[iy][ipt]->Fill(mva[it],eta1[it]);
                  hdedxHarmonic2D1VsMVA[iy][ipt]->Fill(mva[it],H2dedx1[it]);
                  hdedxHarmonic2D1VsP[iy][ipt]->Fill(p1[it],H2dedx1[it]);
                  hzDCASignificanceDaugther2VsMVA[iy][ipt]->Fill(mva[it],dzos2[it]);
                  hxyDCASignificanceDaugther2VsMVA[iy][ipt]->Fill(mva[it],dxyos2[it]);
                  hNHitD2VsMVA[iy][ipt]->Fill(mva[it],nhit2[it]);
                  hpTD2VsMVA[iy][ipt]->Fill(mva[it],pt2[it]);
                  hpTerrD2VsMVA[iy][ipt]->Fill(mva[it],ptErr2[it]/pt2[it]);
                  hEtaD2VsMVA[iy][ipt]->Fill(mva[it],eta2[it]);
                  hdedxHarmonic2D2VsMVA[iy][ipt]->Fill(mva[it],H2dedx2[it]);
                  hdedxHarmonic2D2VsP[iy][ipt]->Fill(p2[it],H2dedx2[it]);
                  if(threeProngDecay_)
                  {
                    hzDCASignificanceDaugther3VsMVA[iy][ipt]->Fill(mva[it],dzos3[it]);
                    hxyDCASignificanceDaugther3VsMVA[iy][ipt]->Fill(mva[it],dxyos3[it]);
                    hNHitD3VsMVA[iy][ipt]->Fill(mva[it],nhit3[it]);
                    hpTD3VsMVA[iy][ipt]->Fill(mva[it],pt3[it]);
                    hpTerrD3VsMVA[iy][ipt]->Fill(mva[it],ptErr3[it]/pt3[it]);
                    hEtaD3VsMVA[iy][ipt]->Fill(mva[it],eta3[it]);
                    hdedxHarmonic2D3VsMVA[iy][ipt]->Fill(mva[it],H2dedx3[it]);
                    hdedxHarmonic2D3VsP[iy][ipt]->Fill(p1[it],H2dedx3[it]);
                  }

                  }
                }
              }
          }

      }
  #ifdef DEBUG
  cout << "Fill reco done" << endl;
  #endif
  }

  void
  PATCompositeTreeProducer2::fillGEN(const edm::Event& iEvent, const edm::EventSetup& iSetup)
  {
      #ifdef DEBUG
      cout << "Fill GEN Start" << endl;
      #endif
      edm::Handle<reco::GenParticleCollection> genpars;
      iEvent.getByToken(tok_genParticle_,genpars);
      std::vector<reco::GenParticleRef> genRefs;
      for(unsigned it=0; it<genpars->size(); ++it){

          const reco::GenParticle & trk = (*genpars)[it];

          int id = trk.pdgId();

          if(fabs(id)!=PID_) continue; //check is target
          if(decayInGen_ && trk.numberOfDaughters()!=2 && !threeProngDecay_) continue; //check 2-pron decay if target decays in Gen
          if(decayInGen_ && trk.numberOfDaughters()!=3 && threeProngDecay_) continue; //check 2-pron decay if target decays in Gen
          int nDau = threeProngDecay_ ? 3 : 2;
          std::vector<unsigned int> idxs;
          std::vector<unsigned int> permutations(nDau);
          std::iota(permutations.begin(), permutations.end(), 0);
          std::sort(permutations.begin(), permutations.end());
          if (!threeProngDecay_) {
            do {
              auto Dd1 = trk.daughter( permutations.at(0) );
              auto Dd2 = trk.daughter( permutations.at(1) );
              if (abs(Dd1->pdgId()) == PID_dau1_ && abs(Dd2->pdgId()) == PID_dau2_) {
                if(twoLayerDecay_){
                  if (Dd1->numberOfDaughters() != 2) continue;
                  // Magic numbers, _permutations -> number of D0 daughters;
                  std::vector<unsigned int> _permutations(2);
                  std::iota(_permutations.begin(), _permutations.end(), 0);
                  std::sort(_permutations.begin(), _permutations.end());
                  do {
                    auto Ddd1 = Dd1->daughter( _permutations.at(0) );
                    auto Ddd2 = Dd1->daughter( _permutations.at(1) );
                    if (abs(Ddd1->pdgId()) == 211 && abs(Ddd2->pdgId()) == 321) {
                      idxs = permutations;
                      break;
                    }
                  } while (std::next_permutation(_permutations.begin(), _permutations.end()));
                  if(!idxs.empty()) break;
                } else {
                  if (abs(Dd1->pdgId()) == PID_dau1_
                      && abs(Dd2->pdgId()) == PID_dau2_
                      ) {
                    idxs = permutations;
                    break;
                  } while (std::next_permutation(permutations.begin(), permutations.end()));
                  if(!idxs.empty()) break;
                }
              }
            } while (std::next_permutation(permutations.begin(), permutations.end()));
          } else {
            do {
              auto Dd1 = trk.daughter( permutations.at(0) );
              auto Dd2 = trk.daughter( permutations.at(1) );
              auto Dd3 = trk.daughter( permutations.at(2) );

              if (abs(Dd1->pdgId()) == PID_dau1_
                  && abs(Dd2->pdgId()) == PID_dau2_
                  && abs(Dd3->pdgId() == PID_dau3_)) {
                idxs = permutations;
                break;
              }
            } while (std::next_permutation(permutations.begin(), permutations.end()));
          }
              if (decayInGen_ && idxs.empty()) continue;
              genRefs.push_back(reco::GenParticleRef(genpars, it));
      }
      unsigned int nGen = genRefs.size();
      candSize_gen = nGen;
      if(twoLayerDecay_){
        for( unsigned int igen=0; igen<nGen; igen++){
          #ifdef DEBUG
          cout <<"nGen : "<< nGen << endl;
          #endif
          auto const theGenDStar = genRefs.at(igen);
          if(abs(theGenDStar->pdgId())!=413) cout << "id : " << theGenDStar->pdgId() << endl;
          unsigned int idxD0 = -1;
          if( fabs(theGenDStar->daughter(0)->pdgId()) == 421 ) idxD0 = 0;
          auto const* theGenD0 = genRefs.at(igen)->daughter(idxD0);
          auto const* theGenPion = genRefs.at(igen)->daughter(1- idxD0);
          mass_gen[igen] = theGenDStar->mass();
          pt_gen[igen] = theGenDStar->pt();
          eta_gen[igen] = theGenDStar->eta(); 
          phi_gen[igen] = theGenDStar->phi();
          y_gen[igen] = theGenDStar->rapidity();
          status_gen[igen] = theGenDStar->status();
          pdgId_gen[igen] = theGenDStar->pdgId();
          charge_gen[igen] = theGenDStar->charge();
          idmom[igen] = -77;
          if(theGenDStar->numberOfMothers()!=0){
            const reco::Candidate * mom = theGenDStar->mother();
            idmom[igen] = mom->pdgId();
          }

          getAncestorId(*theGenDStar, gen_D0ancestorId_[igen], gen_D0ancestorFlavor_[igen] );

          gen_D0mass_[igen] = theGenD0->mass();
          gen_D0pT_[igen] = theGenD0->pt();
          gen_D0eta_[igen] = theGenD0->eta();
          gen_D0phi_[igen] = theGenD0->phi();
          gen_D0y_[igen] = theGenD0->rapidity();
          gen_D0pdgId_[igen] = theGenD0->pdgId();
          gen_D1mass_[igen] = theGenPion->mass(); 
          gen_D1pT_[igen] = theGenPion->pt();
          gen_D1eta_[igen] = theGenPion->eta();
          gen_D1phi_[igen] = theGenPion->phi();
          gen_D1y_[igen] = theGenPion->rapidity();
          gen_D1pdgId_[igen] = theGenPion->pdgId();

          const auto* genDau0 = theGenD0->daughter(0);
          const auto* genDau1 = theGenD0->daughter(1);
          gen_D0Dau1_pT_[igen] = genDau0->pt();
          gen_D0Dau1_eta_[igen] = genDau0->eta();
          gen_D0Dau1_phi_[igen] = genDau0->phi();
          gen_D0Dau1_y_[igen] = genDau0->rapidity();
          gen_D0Dau1_pdgId_[igen] = genDau0->pdgId();
          #ifdef DEBUG
          //cout << "D0 dau1 pdgId : " << genDau0->pdgId() << endl;
	     //cout <<"D0 Ancestor Id : " <<  gen_D0ancestorId_[igen] << endl;
          #endif
          // cout << "D0 dau1 pdgId : " <<  gen_D0Dau1_pdgId_[igen] << endl;
          gen_D0Dau2_pT_[igen] = genDau1->pt();

          gen_D0Dau2_eta_[igen] = genDau1->eta();
          gen_D0Dau2_phi_[igen] = genDau1->phi();
          gen_D0Dau2_y_[igen] = genDau1->rapidity();
          gen_D0Dau2_pdgId_[igen] = genDau1->pdgId();
      }
    }
    else{
        for( unsigned int igen=0; igen<nGen; igen++){
            auto const theGenP = genRefs.at(igen);
             pt_gen[igen] = theGenP->pt();
             eta_gen[igen] = theGenP->eta();
             phi_gen[igen] = theGenP->phi();
             mass_gen[igen] = theGenP->mass();
             y_gen[igen] = theGenP->rapidity();
             status_gen[igen] = theGenP->status();
             pdgId_gen[igen] = theGenP->pdgId();
             charge_gen[igen] = theGenP->charge();

             idmom[igen] = -77;
             if(theGenP->numberOfMothers()!=0){
               const reco::Candidate * mom = theGenP->mother();
               idmom[igen] = mom->pdgId();
             }

             // genDecayLength(*thegenP, Gen_D1decayLength2D_[igen], matchGen_D1decayLength3D_[it], matchGen_D1angle2D_[it], matchGen_D1angle3D_[it] );
             getAncestorId(*theGenP, gen_D0ancestorId_[igen], gen_D0ancestorFlavor_[igen] );

             const auto* genDau0 = theGenP->daughter(0);
             const auto* genDau1 = theGenP->daughter(1);

             gen_D0Dau1_pT_[igen] = genDau0->pt();
             gen_D0Dau1_eta_[igen] = genDau0->eta();
             gen_D0Dau1_phi_[igen] = genDau0->phi();
             gen_D0Dau1_mass_[igen] = genDau0->mass();
             gen_D0Dau1_y_[igen] = genDau0->rapidity();
             gen_D0Dau1_charge_[igen] = genDau0->charge();
             gen_D0Dau1_pdgId_[igen] = genDau0->pdgId();

             gen_D0Dau2_pT_[igen] = genDau1->pt();
             gen_D0Dau2_eta_[igen] = genDau1->eta();
             gen_D0Dau2_phi_[igen] = genDau1->phi();
             gen_D0Dau2_mass_[igen] = genDau1->mass();
             gen_D0Dau2_y_[igen] = genDau1->rapidity();
             gen_D0Dau2_charge_[igen] = genDau1->charge();
             gen_D0Dau2_pdgId_[igen] = genDau1->pdgId();
                }
              } // END for nGen
            }
  

          //if (genRefs.size()>1) std::cout << "More than one target of generated particles\n";
      
      //     if(trk.numberOfMothers()!=0)
      //     {
      //         const reco::Candidate * mom = trk.mother();
      //         idmom[candSize_gen-1] = mom->pdgId();
      //     }
              
      //         const reco::Candidate * Dd1 = trk.daughter(0);
      //         const reco::Candidate * Dd2 = trk.daughter(1);
          



      //     pt_gen[candSize_gen-1] = trk.pt();
      //     eta_gen[candSize_gen-1] = trk.eta();
      //     status_gen[candSize_gen-1] = trk.status();
      //     idmom[candSize_gen-1] = -77;
      //     y_gen[candSize_gen-1] = trk.rapidity();

      //     if(trk.numberOfMothers()!=0)
      //     {
      //         const reco::Candidate * mom = trk.mother();
      //         idmom[candSize_gen-1] = mom->pdgId();
      //     }

      //     if(!decayInGen_) continue;

      //     const reco::Candidate * Dd1 = trk.daughter(0);
      //     const reco::Candidate * Dd2 = trk.daughter(1);
      //     const reco::Candidate * Dd3 = trk.daughter(2);

      //     iddau1[candSize_gen-1] = fabs(Dd1->pdgId());
      //     iddau2[candSize_gen-1] = fabs(Dd2->pdgId());
      //     if(Dd3) iddau3[candSize_gen-1] = fabs(Dd3->pdgId());
      // }


  // ------------ method called once each job just before starting event
  //loop  ------------
  void
  PATCompositeTreeProducer2::beginJob()
  {
      TH1D::SetDefaultSumw2();
      
      if(!doRecoNtuple_ && !doGenNtuple_)
      {
          cout<<"No output for either RECO or GEN!! Fix config!!"<<endl; return;
      }

      if(twoLayerDecay_ && doMuon_)
      {
          cout<<"Muons cannot be coming from two layer decay!! Fix config!!"<<endl; return;
      }
      
      if(saveHistogram_) initHistogram();
      if(saveTree_) initTree();
  }

  void
  PATCompositeTreeProducer2::initHistogram()
  {
    for(unsigned int ipt=0;ipt<pTBins_.size()-1;ipt++)
    {
      for(unsigned int iy=0;iy<yBins_.size()-1;iy++)
    {
    hMassVsMVA[iy][ipt] = fs->make<TH2F>(Form("hMassVsMVA_y%d_pt%d",iy,ipt),";mva;mass(GeV)",100,-1.,1.,massHistBins_,massHistPeak_-massHistWidth_,massHistPeak_+massHistWidth_);
  //   h3DDCAVsMVA[iy][ipt] = fs->make<TH2F>(Form("h3DDCAVsMVA_y%d_pt%d",iy,ipt),";mva;3D DCA;",100,-1.,1.,1000,0,10);
  //   h2DDCAVsMVA[iy][ipt] = fs->make<TH2F>(Form("h2DDCAVsMVA_y%d_pt%d",iy,ipt),";mva;2D DCA;",100,-1.,1.,1000,0,10);

    if(saveAllHistogram_)
    {
    hpTVsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTVsMVA_y%d_pt%d",iy,ipt),";mva;pT;",100,-1,1,100,0,10);
    hetaVsMVA[iy][ipt] = fs->make<TH2F>(Form("hetaVsMVA_y%d_pt%d",iy,ipt),";mva;eta;",100,-1.,1.,40,-4,4);
    hyVsMVA[iy][ipt] = fs->make<TH2F>(Form("hyVsMVA_y%d_pt%d",iy,ipt),";mva;y;",100,-1.,1.,40,-4,4);
    hVtxProbVsMVA[iy][ipt] = fs->make<TH2F>(Form("hVtxProbVsMVA_y%d_pt%d",iy,ipt),";mva;VtxProb;",100,-1.,1.,100,0,1);
    h3DCosPointingAngleVsMVA[iy][ipt] = fs->make<TH2F>(Form("h3DCosPointingAngleVsMVA_y%d_pt%d",iy,ipt),";mva;3DCosPointingAngle;",100,-1.,1.,100,-1,1);
    h3DPointingAngleVsMVA[iy][ipt] = fs->make<TH2F>(Form("h3DPointingAngleVsMVA_y%d_pt%d",iy,ipt),";mva;3DPointingAngle;",100,-1.,1.,50,-3.14,3.14);
    h2DCosPointingAngleVsMVA[iy][ipt] = fs->make<TH2F>(Form("h2DCosPointingAngleVsMVA_y%d_pt%d",iy,ipt),";mva;2DCosPointingAngle;",100,-1.,1.,100,-1,1);
    h2DPointingAngleVsMVA[iy][ipt] = fs->make<TH2F>(Form("h2DPointingAngleVsMVA_y%d_pt%d",iy,ipt),";mva;2DPointingAngle;",100,-1.,1.,50,-3.14,3.14);
    h3DDecayLengthSignificanceVsMVA[iy][ipt] = fs->make<TH2F>(Form("h3DDecayLengthSignificanceVsMVA_y%d_pt%d",iy,ipt),";mva;3DDecayLengthSignificance;",100,-1.,1.,300,0,30);
    h2DDecayLengthSignificanceVsMVA[iy][ipt] = fs->make<TH2F>(Form("h2DDecayLengthSignificanceVsMVA_y%d_pt%d",iy,ipt),";mva;2DDecayLengthSignificance;",100,-1.,1.,300,0,30);
    h3DDecayLengthVsMVA[iy][ipt] = fs->make<TH2F>(Form("h3DDecayLengthVsMVA_y%d_pt%d",iy,ipt),";mva;3DDecayLength;",100,-1.,1.,300,0,30);
    h2DDecayLengthVsMVA[iy][ipt] = fs->make<TH2F>(Form("h2DDecayLengthVsMVA_y%d_pt%d",iy,ipt),";mva;2DDecayLength;",100,-1.,1.,300,0,30);
    hzDCASignificanceDaugther1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hzDCASignificanceDaugther1VsMVA_y%d_pt%d",iy,ipt),";mva;zDCASignificanceDaugther1;",100,-1.,1.,100,-10,10);
    hxyDCASignificanceDaugther1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hxyDCASignificanceDaugther1VsMVA_y%d_pt%d",iy,ipt),";mva;xyDCASignificanceDaugther1;",100,-1.,1.,100,-10,10);
    hNHitD1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hNHitD1VsMVA_y%d_pt%d",iy,ipt),";mva;NHitD1;",100,-1.,1.,100,0,100);
    hpTD1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTD1VsMVA_y%d_pt%d",iy,ipt),";mva;pTD1;",100,-1.,1.,100,0,10);
    hpTerrD1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTerrD1VsMVA_y%d_pt%d",iy,ipt),";mva;pTerrD1;",100,-1.,1.,50,0,0.5);
    hEtaD1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hEtaD1VsMVA_y%d_pt%d",iy,ipt),";mva;EtaD1;",100,-1.,1.,40,-4,4);
    hdedxHarmonic2D1VsMVA[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D1VsMVA_y%d_pt%d",iy,ipt),";mva;dedxHarmonic2D1;",100,-1.,1.,100,0,10);
    hdedxHarmonic2D1VsP[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D1VsP_y%d_pt%d",iy,ipt),";p (GeV);dedxHarmonic2D1",100,0,10,100,0,10);
    hzDCASignificanceDaugther2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hzDCASignificanceDaugther2VsMVA_y%d_pt%d",iy,ipt),";mva;zDCASignificanceDaugther2;",100,-1.,1.,100,-10,10);
    hxyDCASignificanceDaugther2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hxyDCASignificanceDaugther2VsMVA_y%d_pt%d",iy,ipt),";mva;xyDCASignificanceDaugther2;",100,-1.,1.,100,-10,10);
    hNHitD2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hNHitD2VsMVA_y%d_pt%d",iy,ipt),";mva;NHitD2;",100,-1.,1.,100,0,100);
    hpTD2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTD2VsMVA_y%d_pt%d",iy,ipt),";mva;pTD2;",100,-1.,1.,100,0,10);
    hpTerrD2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTerrD2VsMVA_y%d_pt%d",iy,ipt),";mva;pTerrD2;",100,-1.,1.,50,0,0.5);
    hEtaD2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hEtaD2VsMVA_y%d_pt%d",iy,ipt),";mva;EtaD2;",100,-1.,1.,40,-4,4);
    hdedxHarmonic2D2VsMVA[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D2VsMVA_y%d_pt%d",iy,ipt),";mva;dedxHarmonic2D2;",100,-1.,1.,100,0,10);
    hdedxHarmonic2D2VsP[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D2VsP_y%d_pt%d",iy,ipt),";p (GeV);dedxHarmonic2D2",100,0,10,100,0,10);

    if(threeProngDecay_)
    {
      hzDCASignificanceDaugther3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hzDCASignificanceDaugther3VsMVA_y%d_pt%d",iy,ipt),";mva;zDCASignificanceDaugther3;",100,-1.,1.,100,-10,10);
      hxyDCASignificanceDaugther3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hxyDCASignificanceDaugther3VsMVA_y%d_pt%d",iy,ipt),";mva;xyDCASignificanceDaugther3;",100,-1.,1.,100,-10,10);
      hNHitD3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hNHitD3VsMVA_y%d_pt%d",iy,ipt),";mva;NHitD3;",100,-1.,1.,100,0,100);
      hpTD3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTD3VsMVA_y%d_pt%d",iy,ipt),";mva;pTD3;",100,-1.,1.,100,0,10);
      hpTerrD3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hpTerrD3VsMVA_y%d_pt%d",iy,ipt),";mva;pTerrD3;",100,-1.,1.,50,0,0.5);
      hEtaD3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hEtaD3VsMVA_y%d_pt%d",iy,ipt),";mva;EtaD3;",100,-1.,1.,40,-4,4);
      hdedxHarmonic2D3VsMVA[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D3VsMVA_y%d_pt%d",iy,ipt),";mva;dedxHarmonic2D3;",100,-1.,1.,100,0,10);
      hdedxHarmonic2D3VsP[iy][ipt] = fs->make<TH2F>(Form("hdedxHarmonic2D3VsP_y%d_pt%d",iy,ipt),";p (GeV);dedxHarmonic2D3",100,0,10,100,0,10);
    }

    }
    }
  }
  }

  void 
  PATCompositeTreeProducer2::initTree()
  { 
      PATCompositeNtuple = fs->make< TTree>("PATCompositeNtuple","PATCompositeNtuple");
      
      if(doRecoNtuple_) 
      { 
    
      // Event info
      PATCompositeNtuple->Branch("Ntrkoffline",&Ntrkoffline,"Ntrkoffline/I");
      PATCompositeNtuple->Branch("Npixel",&Npixel,"Npixel/I");
      PATCompositeNtuple->Branch("HFsumETPlus",&HFsumETPlus,"HFsumETPlus/F");
      PATCompositeNtuple->Branch("HFsumETMinus",&HFsumETMinus,"HFsumETMinus/F");
      PATCompositeNtuple->Branch("ZDCPlus",&ZDCPlus,"ZDCPlus/F");
      PATCompositeNtuple->Branch("ZDCMinus",&ZDCMinus,"ZDCMinus/F");
      PATCompositeNtuple->Branch("bestvtxX",&bestvx,"bestvtxX/F");
      PATCompositeNtuple->Branch("bestvtxY",&bestvy,"bestvtxY/F");
      PATCompositeNtuple->Branch("bestvtxZ",&bestvz,"bestvtxZ/F");
      PATCompositeNtuple->Branch("candSize",&candSize,"candSize/I");
      if(isCentrality_) PATCompositeNtuple->Branch("centrality",&centrality,"centrality/I");
      if(isEventPlane_) 
      {
        PATCompositeNtuple->Branch("ephfpAngle",&ephfpAngle,"ephfpAngle[3]/F");
        PATCompositeNtuple->Branch("ephfmAngle",&ephfmAngle,"ephfmAngle[3]/F");
        PATCompositeNtuple->Branch("ephfpQ",&ephfpQ,"ephfpQ[3]/F");
        PATCompositeNtuple->Branch("ephfmQ",&ephfmQ,"ephfmQ[3]/F");
        PATCompositeNtuple->Branch("ephfpSumW",&ephfpSumW,"ephfpSumW/F");
        PATCompositeNtuple->Branch("ephfmSumW",&ephfmSumW,"ephfmSumW/F");
      }

      // particle info
      PATCompositeNtuple->Branch("pT",&pt,"pT[candSize]/F");
      PATCompositeNtuple->Branch("y",&y,"y[candSize]/F");
      PATCompositeNtuple->Branch("eta",&eta,"eta[candSize]/F");
      PATCompositeNtuple->Branch("phi",&phi,"phi[candSize]/F");
      PATCompositeNtuple->Branch("mass",&mass,"mass[candSize]/F");
      PATCompositeNtuple->Branch("mva",&mva,"mva[candSize]/F");

      if(!isSkimMVA_)  
      {
          //Composite candidate info RECO
          PATCompositeNtuple->Branch("flavor",&flavor,"flavor[candSize]/F");
          PATCompositeNtuple->Branch("VtxProb",&VtxProb,"VtxProb[candSize]/F");
          PATCompositeNtuple->Branch("VtxChi2",&vtxChi2,"VtxChi2[candSize]/F");
          PATCompositeNtuple->Branch("VtxNDF",&ndf,"VtxNDF[candSize]/F");
          PATCompositeNtuple->Branch("3DCosPointingAngle",&agl,"3DCosPointingAngle[candSize]/F");
          PATCompositeNtuple->Branch("3DPointingAngle",&agl_abs,"3DPointingAngle[candSize]/F");
          PATCompositeNtuple->Branch("2DCosPointingAngle",&agl2D,"2DCosPointingAngle[candSize]/F");
          PATCompositeNtuple->Branch("2DPointingAngle",&agl2D_abs,"2DPointingAngle[candSize]/F");
          PATCompositeNtuple->Branch("3DDecayLengthSignificance",&dlos,"3DDecayLengthSignificance[candSize]/F");
          PATCompositeNtuple->Branch("3DDecayLength",&dl,"3DDecayLength[candSize]/F");
          PATCompositeNtuple->Branch("2DDecayLengthSignificance",&dlos2D,"2DDecayLengthSignificance[candSize]/F");
          PATCompositeNtuple->Branch("2DDecayLength",&dl2D,"2DDecayLength[candSize]/F");
          PATCompositeNtuple->Branch("Trk3DDCA",&trk3Ddca,"Trk3DDCA[candSize]/F");
          PATCompositeNtuple->Branch("Trk3DDCAErr",&trk3DdcaErr,"Trk3DDCAErr[candSize]/F");
          PATCompositeNtuple->Branch("dca3D",&dca3D,"dca3D[candSize]/F");
          PATCompositeNtuple->Branch("dca3DErr",&dca3DErr,"dca3DErr[candSize]/F");
      
          if(doGenMatching_)
          {
              PATCompositeNtuple->Branch("isSwap",&isSwap,"isSwap[candSize]/O");
              PATCompositeNtuple->Branch("idmom_reco",&idmom_reco,"idmom_reco[candSize]/I");
              PATCompositeNtuple->Branch("idBAnc_reco",&idBAnc_reco,"idBAnc_reco[candSize]/I");
              PATCompositeNtuple->Branch("matchGEN",&matchGEN,"matchGEN[candSize]/O");
              PATCompositeNtuple->Branch("matchGen3DPointingAngle",&gen_agl_abs,"gen3DPointingAngle[candSize]/F");
              PATCompositeNtuple->Branch("matchGen2DPointingAngle",&gen_agl2D_abs,"gen2DPointingAngle[candSize]/F");
              PATCompositeNtuple->Branch("matchGen3DDecayLength",&gen_dl,"gen3DDecayLength[candSize]/F");
              PATCompositeNtuple->Branch("matchGen2DDecayLength",&gen_dl2D,"gen2DDecayLength[candSize]/F");
              // PATCompositeNtuple->Branch("matchgen_D0pT",&gen_D0pT_, "gen_D0pT[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0eta",&gen_D0eta_, "gen_D0eta[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0phi",&gen_D0phi_, "gen_D0phi[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0mass",&gen_D0mass_, "gen_D0mass[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0y",&gen_D0y_, "gen_D0y[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0charge",&gen_D0charge_, "gen_D0charge[candSize]F");
              // PATCompositeNtuple->Branch("matchgen_D0pdgId",&gen_D0pdgId_, "gen_D0pdgId[candSize]I");
              if(twoLayerDecay_){
                PATCompositeNtuple->Branch("matchGen_DStarpT",&matchGen_DStarpT_, "matchGen_DStarpT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStareta",&matchGen_DStareta_, "matchGen_DStareta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStarphi",&matchGen_DStarphi_, "matchGen_DStarphi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStarmass",&matchGen_DStarmass_, "matchGen_DStarmass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStary",&matchGen_DStary_, "matchGen_DStary[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStarcharge",&matchGen_DStarcharge_, "matchGen_DStarcharge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_DStarpdgId",&matchGen_DStarpdgId_, "matchGen_DStarpdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D0pT",&matchGen_D0pT_, "matchGen_D0pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0eta",&matchGen_D0eta_, "matchGen_D0eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0phi",&matchGen_D0phi_, "matchGen_D0phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0mass",&matchGen_D0mass_, "matchGen_D0mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0y",&matchGen_D0y_, "matchGen_D0y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0charge",&matchGen_D0charge_, "matchGen_D0charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0pdgId",&matchGen_D0pdgId_, "matchGen_D0pdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT_, "matchGen_D0Dau1_pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta_, "matchGen_D0Dau1_eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi_, "matchGen_D0Dau1_phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass_, "matchGen_D0Dau1_mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_y",&matchGen_D0Dau1_y_, "matchGen_D0Dau1_y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge_, "matchGen_D0Dau1_charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId_, "matchGen_D0Dau1_pdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT_, "matchGen_D0Dau2_pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta_, "matchGen_D0Dau2_eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi_, "matchGen_D0Dau2_phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass_, "matchGen_D0Dau2_mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_y",&matchGen_D0Dau2_y_, "matchGen_D0Dau2_y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge_, "matchGen_D0Dau2_charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId_, "matchGen_D0Dau2_pdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D1pT",&matchGen_D1pT_, "matchGen_D1pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1eta",&matchGen_D1eta_, "matchGen_D1eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1phi",&matchGen_D1phi_, "matchGen_D1phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1mass",&matchGen_D1mass_, "matchGen_D1mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1y",&matchGen_D1y_, "matchGen_D1y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1charge",&matchGen_D1charge_, "matchGen_D1charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D1pdgId",&matchGen_D1pdgId_, "matchGen_D1pdgId[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1decayLength2D_",&matchGen_D1decayLength2D_, "matchGen_D1decayLength2D_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1decayLength3D_",&matchGen_D1decayLength3D_, "matchGen_D1decayLength3D_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1angle2D_",&matchGen_D1angle2D_, "matchGen_D1angle2D_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1angle3D_",&matchGen_D1angle3D_, "matchGen_D1angle3D_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1ancestorId_",&matchGen_D1ancestorId_, "matchGen_D1ancestorId_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_, "matchGen_D1ancestorFlavor_[candSize]/I");
              }
              else{
                PATCompositeNtuple->Branch("matchGen_D0pT",&matchGen_D0pT_, "matchGen_D0pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0eta",&matchGen_D0eta_, "matchGen_D0eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0phi",&matchGen_D0phi_, "matchGen_D0phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0mass",&matchGen_D0mass_, "matchGen_D0mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0y",&matchGen_D0y_, "matchGen_D0y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0charge",&matchGen_D0charge_, "matchGen_D0charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0pdgId",&matchGen_D0pdgId_, "matchGen_D0pdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT_, "matchGen_D0Dau1_pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta_, "matchGen_D0Dau1_eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi_, "matchGen_D0Dau1_phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass_, "matchGen_D0Dau1_mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_y",&matchGen_D0Dau1_y_, "matchGen_D0Dau1_y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge_, "matchGen_D0Dau1_charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId_, "matchGen_D0Dau1_pdgId[candSize]/I");

                PATCompositeNtuple->Branch("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT_, "matchGen_D0Dau2_pT[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta_, "matchGen_D0Dau2_eta[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi_, "matchGen_D0Dau2_phi[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass_, "matchGen_D0Dau2_mass[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_y",&matchGen_D0Dau2_y_, "matchGen_D0Dau2_y[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge_, "matchGen_D0Dau2_charge[candSize]/F");
                PATCompositeNtuple->Branch("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId_, "matchGen_D0Dau2_pdgId[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1ancestorId_",&matchGen_D1ancestorId_, "matchGen_D1ancestorId_[candSize]/I");
                PATCompositeNtuple->Branch("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_, "matchGen_D1ancestorFlavor_[candSize]/I");
              }
          }
          
          if(doGenMatchingTOF_)
          {
            PATCompositeNtuple->Branch("PIDD1",&pid1,"PIDD1[candSize]/I");
            PATCompositeNtuple->Branch("PIDD2",&pid1,"PIDD2[candSize]/I");
            PATCompositeNtuple->Branch("TOFD1",&tof1,"TOFD1[candSize]/F");
            PATCompositeNtuple->Branch("TOFD2",&tof1,"TOFD2[candSize]/F");
          }

          //daugther & grand daugther info
          if(twoLayerDecay_)
          {
              PATCompositeNtuple->Branch("massDaugther1",&grand_mass,"massDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("pTD1",&pt1,"pTD1[candSize]/F");
              PATCompositeNtuple->Branch("EtaD1",&eta1,"EtaD1[candSize]/F");
              PATCompositeNtuple->Branch("PhiD1",&phi1,"PhiD1[candSize]/F");
              PATCompositeNtuple->Branch("VtxProbDaugther1",&grand_VtxProb,"VtxProbDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("VtxChi2Daugther1",&grand_vtxChi2,"VtxChi2Daugther1[candSize]/F");
              PATCompositeNtuple->Branch("VtxNDFDaugther1",&grand_ndf,"VtxNDFDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("3DCosPointingAngleDaugther1",&grand_agl,"3DCosPointingAngleDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("3DPointingAngleDaugther1",&grand_agl_abs,"3DPointingAngleDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("2DCosPointingAngleDaugther1",&grand_agl2D,"2DCosPointingAngleDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("2DPointingAngleDaugther1",&grand_agl2D_abs,"2DPointingAngleDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("3DDecayLengthSignificanceDaugther1",&grand_dlos,"3DDecayLengthSignificanceDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("3DDecayLengthDaugther1",&grand_dl,"3DDecayLengthDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("3DDecayLengthErrorDaugther1",&grand_dlerror,"3DDecayLengthErrorDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("2DDecayLengthSignificanceDaugther1",&grand_dlos2D,"2DDecayLengthSignificanceDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("zDCASignificanceDaugther2",&dzos2,"zDCASignificanceDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("xyDCASignificanceDaugther2",&dxyos2,"xyDCASignificanceDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("NHitD2",&nhit2,"NHitD2[candSize]/F");
              PATCompositeNtuple->Branch("HighPurityDaugther2",&trkquality2,"HighPurityDaugther2[candSize]/O");
              PATCompositeNtuple->Branch("pTD2",&pt2,"pTD2[candSize]/F");
              PATCompositeNtuple->Branch("EtaD2",&eta2,"EtaD2[candSize]/F");
              PATCompositeNtuple->Branch("PhiD2",&phi2,"PhiD2[candSize]/F");
              PATCompositeNtuple->Branch("pTerrD1",&ptErr2,"pTerrD2[candSize]/F");
              PATCompositeNtuple->Branch("pTerrD2",&ptErr2,"pTerrD2[candSize]/F");
              PATCompositeNtuple->Branch("dedxHarmonic2D2",&H2dedx2,"dedxHarmonic2D2[candSize]/F");
  //            PATCompositeNtuple->Branch("normalizedChi2Daugther2",&trkChi2,"normalizedChi2Daugther2[candSize]/F");
              PATCompositeNtuple->Branch("zDCASignificanceGrandDaugther1",&grand_dzos1,"zDCASignificanceGrandDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("zDCASignificanceGrandDaugther2",&grand_dzos2,"zDCASignificanceGrandDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("xyDCASignificanceGrandDaugther1",&grand_dxyos1,"xyDCASignificanceGrandDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("xyDCASignificanceGrandDaugther2",&grand_dxyos2,"xyDCASignificanceGrandDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("NHitGrandD1",&grand_nhit1,"NHitGrandD1[candSize]/F");
              PATCompositeNtuple->Branch("NHitGrandD2",&grand_nhit2,"NHitGrandD2[candSize]/F");
              PATCompositeNtuple->Branch("HighPurityGrandDaugther1",&grand_trkquality1,"HighPurityGrandDaugther1[candSize]/O");
              PATCompositeNtuple->Branch("HighPurityGrandDaugther2",&grand_trkquality2,"HighPurityGrandDaugther2[candSize]/O");
              PATCompositeNtuple->Branch("pTGrandD1",&grand_pt1,"pTGrandD1[candSize]/F");
              PATCompositeNtuple->Branch("pTGrandD2",&grand_pt2,"pTGrandD2[candSize]/F");
              PATCompositeNtuple->Branch("pTerrGrandD1",&grand_ptErr1,"pTerrGrandD1[candSize]/F");
              PATCompositeNtuple->Branch("pTerrGrandD2",&grand_ptErr2,"pTerrGrandD2[candSize]/F");
              PATCompositeNtuple->Branch("EtaGrandD1",&grand_eta1,"EtaGrandD1[candSize]/F");
              PATCompositeNtuple->Branch("EtaGrandD2",&grand_eta2,"EtaGrandD2[candSize]/F");
              PATCompositeNtuple->Branch("dedxHarmonic2GrandD1",&grand_H2dedx1,"dedxHarmonic2GrandD1[candSize]/F");
              PATCompositeNtuple->Branch("dedxHarmonic2GrandD2",&grand_H2dedx2,"dedxHarmonic2GrandD2[candSize]/F");
          //    PATCompositeNtuple->Branch("D03DDCA",&D03Ddca,"D03DDCA[candSize]/F");
          //    PATCompositeNtuple->Branch("D03DDCAErr",&D03DdcaErr,"D03DDCAErr[candSize]/F");
          }
          else
          {
              PATCompositeNtuple->Branch("zDCASignificanceDaugther1",&dzos1,"zDCASignificanceDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("xyDCASignificanceDaugther1",&dxyos1,"xyDCASignificanceDaugther1[candSize]/F");
              PATCompositeNtuple->Branch("zDCADaugther1",&dzval1,"zDCADaugther1[candSize]/F");
              PATCompositeNtuple->Branch("xyDCADaugther1",&dxyval1,"xyDCADaugther1[candSize]/F");
              PATCompositeNtuple->Branch("NHitD1",&nhit1,"NHitD1[candSize]/F");
              PATCompositeNtuple->Branch("HighPurityDaugther1",&trkquality1,"HighPurityDaugther1[candSize]/O");
              PATCompositeNtuple->Branch("pTD1",&pt1,"pTD1[candSize]/F");
              PATCompositeNtuple->Branch("pTerrD1",&ptErr1,"pTerrD1[candSize]/F");
  //            PATCompositeNtuple->Branch("pD1",&p1,"pD1[candSize]/F");
              PATCompositeNtuple->Branch("EtaD1",&eta1,"EtaD1[candSize]/F");
              PATCompositeNtuple->Branch("PhiD1",&phi1,"PhiD1[candSize]/F");
  //            PATCompositeNtuple->Branch("chargeD1",&charge1,"chargeD1[candSize]/I");
              PATCompositeNtuple->Branch("dedxHarmonic2D1",&H2dedx1,"dedxHarmonic2D1[candSize]/F");
  //            PATCompositeNtuple->Branch("dedxTruncated40Daugther1",&T4dedx1,"dedxTruncated40Daugther1[candSize]/F");
             PATCompositeNtuple->Branch("normalizedChi2Daugther1",&trkChi1,"normalizedChi2Daugther1[candSize]/F");
              PATCompositeNtuple->Branch("zDCASignificanceDaugther2",&dzos2,"zDCASignificanceDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("xyDCASignificanceDaugther2",&dxyos2,"xyDCASignificanceDaugther2[candSize]/F");
              PATCompositeNtuple->Branch("zDCADaugther2",&dzval2,"zDCADaugther2[candSize]/F");
              PATCompositeNtuple->Branch("xyDCADaugther2",&dxyval2,"xyDCADaugther2[candSize]/F");
              PATCompositeNtuple->Branch("NHitD2",&nhit2,"NHitD2[candSize]/F");
              PATCompositeNtuple->Branch("HighPurityDaugther2",&trkquality2,"HighPurityDaugther2[candSize]/O");
              PATCompositeNtuple->Branch("pTD2",&pt2,"pTD2[candSize]/F");
              PATCompositeNtuple->Branch("pTerrD2",&ptErr2,"pTerrD2[candSize]/F");
  //            PATCompositeNtuple->Branch("pD2",&p2,"pD2[candSize]/F");
              PATCompositeNtuple->Branch("EtaD2",&eta2,"EtaD2[candSize]/F");
              PATCompositeNtuple->Branch("PhiD2",&phi2,"PhiD2[candSize]/F");
  //            PATCompositeNtuple->Branch("chargeD2",&charge2,"chargeD2[candSize]/I");
              PATCompositeNtuple->Branch("dedxHarmonic2D2",&H2dedx2,"dedxHarmonic2D2[candSize]/F");
  //            PATCompositeNtuple->Branch("dedxTruncated40Daugther2",&T4dedx2,"dedxTruncated40Daugther2[candSize]/F");
             PATCompositeNtuple->Branch("normalizedChi2Daugther2",&trkChi2,"normalizedChi2Daugther2[candSize]/F");
              if(threeProngDecay_)
              {
                PATCompositeNtuple->Branch("zDCASignificanceDaugther3",&dzos3,"zDCASignificanceDaugther3[candSize]/F");
                PATCompositeNtuple->Branch("xyDCASignificanceDaugther3",&dxyos3,"xyDCASignificanceDaugther3[candSize]/F");
                PATCompositeNtuple->Branch("zDCADaugther3",&dzval3,"zDCADaugther3[candSize]/F");
                PATCompositeNtuple->Branch("xyDCADaugther3",&dxyval3,"xyDCADaugther3[candSize]/F");
                PATCompositeNtuple->Branch("NHitD3",&nhit3,"NHitD3[candSize]/F");
                PATCompositeNtuple->Branch("HighPurityDaugther3",&trkquality3,"HighPurityDaugther3[candSize]/O");
                PATCompositeNtuple->Branch("pTD3",&pt1,"pTD3[candSize]/F");
                PATCompositeNtuple->Branch("pTerrD3",&ptErr3,"pTerrD3[candSize]/F");
                PATCompositeNtuple->Branch("EtaD3",&eta1,"EtaD3[candSize]/F");
                PATCompositeNtuple->Branch("dedxHarmonic2D3",&H2dedx1,"dedxHarmonic2D3[candSize]/F");
              }
          }
          
          if(doMuon_)
          {
              PATCompositeNtuple->Branch("OneStMuon1",&onestmuon1,"OneStMuon1[candSize]/O");
              PATCompositeNtuple->Branch("OneStMuon2",&onestmuon2,"OneStMuon2[candSize]/O");
              PATCompositeNtuple->Branch("PFMuon1",&pfmuon1,"PFMuon1[candSize]/O");
              PATCompositeNtuple->Branch("PFMuon2",&pfmuon2,"PFMuon2[candSize]/O");
              PATCompositeNtuple->Branch("GlbMuon1",&glbmuon1,"GlbMuon1[candSize]/O");
              PATCompositeNtuple->Branch("GlbMuon2",&glbmuon2,"GlbMuon2[candSize]/O");
              PATCompositeNtuple->Branch("trkMuon1",&trkmuon1,"trkMuon1[candSize]/O");
              PATCompositeNtuple->Branch("trkMuon2",&trkmuon2,"trkMuon2[candSize]/O");
              PATCompositeNtuple->Branch("caloMuon1",&calomuon1,"caloMuon1[candSize]/O");
              PATCompositeNtuple->Branch("caloMuon2",&calomuon2,"caloMuon2[candSize]/O");
              PATCompositeNtuple->Branch("SoftMuon1",&softmuon1,"SoftMuon1[candSize]/O");
              PATCompositeNtuple->Branch("SoftMuon2",&softmuon2,"SoftMuon2[candSize]/O");

              if(doMuonFull_)
            {
              PATCompositeNtuple->Branch("nMatchedChamberD1",&nmatchedch1,"nMatchedChamberD1[candSize]/F");
              PATCompositeNtuple->Branch("nMatchedStationD1",&nmatchedst1,"nMatchedStationD1[candSize]/F");
              PATCompositeNtuple->Branch("EnergyDepositionD1",&matchedenergy1,"EnergyDepositionD1[candSize]/F");
              PATCompositeNtuple->Branch("nMatchedChamberD2",&nmatchedch2,"nMatchedChamberD2[candSize]/F");
              PATCompositeNtuple->Branch("nMatchedStationD2",&nmatchedst2,"nMatchedStationD2[candSize]/F");
              PATCompositeNtuple->Branch("EnergyDepositionD2",&matchedenergy2,"EnergyDepositionD2[candSize]/F");
              PATCompositeNtuple->Branch("dx1_seg",        &dx1_seg_, "dx1_seg[candSize]/F");
              PATCompositeNtuple->Branch("dy1_seg",        &dy1_seg_, "dy1_seg[candSize]/F");
              PATCompositeNtuple->Branch("dxSig1_seg",     &dxSig1_seg_, "dxSig1_seg[candSize]/F");
              PATCompositeNtuple->Branch("dySig1_seg",     &dySig1_seg_, "dySig1_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddxdz1_seg",     &ddxdz1_seg_, "ddxdz1_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddydz1_seg",     &ddydz1_seg_, "ddydz1_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddxdzSig1_seg",  &ddxdzSig1_seg_, "ddxdzSig1_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddydzSig1_seg",  &ddydzSig1_seg_, "ddydzSig1_seg[candSize]/F");
              PATCompositeNtuple->Branch("dx2_seg",        &dx2_seg_, "dx2_seg[candSize]/F");
              PATCompositeNtuple->Branch("dy2_seg",        &dy2_seg_, "dy2_seg[candSize]/F");
              PATCompositeNtuple->Branch("dxSig2_seg",     &dxSig2_seg_, "dxSig2_seg[candSize]/F");
              PATCompositeNtuple->Branch("dySig2_seg",     &dySig2_seg_, "dySig2_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddxdz2_seg",     &ddxdz2_seg_, "ddxdz2_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddydz2_seg",     &ddydz2_seg_, "ddydz2_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddxdzSig2_seg",  &ddxdzSig2_seg_, "ddxdzSig2_seg[candSize]/F");
              PATCompositeNtuple->Branch("ddydzSig2_seg",  &ddydzSig2_seg_, "ddydzSig2_seg[candSize]/F");
           }
        }
    }

    } // doRecoNtuple_

    if(doGenNtuple_)
    {
        PATCompositeNtuple->Branch("candSize_gen",&candSize_gen,"candSize_gen/I");
        PATCompositeNtuple->Branch("gen_mass",&mass_gen,"mass_gen[candSize_gen]/F");
        PATCompositeNtuple->Branch("gen_pT",&pt_gen,"pT_gen[candSize_gen]/F");
        PATCompositeNtuple->Branch("gen_eta",&eta_gen,"eta_gen[candSize_gen]/F");
        PATCompositeNtuple->Branch("gen_phi",&phi_gen,"phi_gen[candSize_gen]/F");
        PATCompositeNtuple->Branch("gen_y",&y_gen,"y_gen[candSize_gen]/F");
        PATCompositeNtuple->Branch("gen_status",&status_gen,"status_gen[candSize_gen]/I");
        PATCompositeNtuple->Branch("gen_pdgId",&pdgId_gen,"pdgId_gen[candSize_gen]/I");
        PATCompositeNtuple->Branch("gen_charge",&charge_gen,"charge_gen[candSize_gen]/I");
        PATCompositeNtuple->Branch("gen_MotherID",&idmom,"MotherID_gen[candSize_gen]/I");

        if(decayInGen_)
        {

            PATCompositeNtuple->Branch("gen_DauID1",&iddau1,"DauID1_gen[candSize_gen]/I");
            PATCompositeNtuple->Branch("gen_DauID2",&iddau2,"DauID2_gen[candSize_gen]/I");
            PATCompositeNtuple->Branch("gen_DauID3",&iddau3,"DauID3_gen[candSize_gen]/I");
        }
        if(twoLayerDecay_){
          PATCompositeNtuple->Branch("gen_D0pT",&gen_D0pT_, "gen_D0pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0eta",&gen_D0eta_, "gen_D0eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0phi",&gen_D0phi_, "gen_D0phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0mass",&gen_D0mass_, "gen_D0mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0y",&gen_D0y_, "gen_D0y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0charge",&gen_D0charge_, "gen_D0charge[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0pdgId",&gen_D0pdgId_, "gen_D0pdgId[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0ancestorId_",&gen_D0ancestorId_, "gen_D0ancestorId_[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0ancestorFlavor_",&gen_D0ancestorFlavor_, "gen_D0ancestorFlavor_[candSize_gen]/I");

          PATCompositeNtuple->Branch("gen_D0Dau1_pT",&gen_D0Dau1_pT_, "gen_D0Dau1_pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_eta",&gen_D0Dau1_eta_, "gen_D0Dau1_eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_phi",&gen_D0Dau1_phi_, "gen_D0Dau1_phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_mass",&gen_D0Dau1_mass_, "gen_D0Dau1_mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_y",&gen_D0Dau1_y_, "gen_D0Dau1_y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_charge",&gen_D0Dau1_charge_, "gen_D0Dau1_charge[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId_, "gen_D0Dau1_pdgId[candSize_gen]/I");

          PATCompositeNtuple->Branch("gen_D0Dau2_pT",&gen_D0Dau2_pT_, "gen_D0Dau2_pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_eta",&gen_D0Dau2_eta_, "gen_D0Dau2_eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_phi",&gen_D0Dau2_phi_, "gen_D0Dau2_phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_mass",&gen_D0Dau2_mass_, "gen_D0Dau2_mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_y",&gen_D0Dau2_y_, "gen_D0Dau2_y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_charge",&gen_D0Dau2_charge_, "gen_D0Dau2_charge[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId_, "gen_D0Dau2_pdgId[candSize_gen]/I");

          PATCompositeNtuple->Branch("gen_D1pT",&gen_D1pT_, "gen_D1pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D1eta",&gen_D1eta_, "gen_D1eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D1phi",&gen_D1phi_, "gen_D1phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D1mass",&gen_D1mass_, "gen_D1mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D1y",&gen_D1y_, "gen_D1y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D1charge",&gen_D1charge_, "gen_D1charge[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D1pdgId",&gen_D1pdgId_, "gen_D1pdgId[candSize_gen]/I");
        }
        else{
          PATCompositeNtuple->Branch("gen_D0ancestorId_",&gen_D0ancestorId_, "gen_D0ancestorId_[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0ancestorFlavor_",&gen_D0ancestorFlavor_, "gen_D0ancestorFlavor_[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0Dau1_pT",&gen_D0Dau1_pT_, "gen_D0Dau1_pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_eta",&gen_D0Dau1_eta_, "gen_D0Dau1_eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_phi",&gen_D0Dau1_phi_, "gen_D0Dau1_phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_mass",&gen_D0Dau1_mass_, "gen_D0Dau1_mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_y",&gen_D0Dau1_y_, "gen_D0Dau1_y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau1_charge",&gen_D0Dau1_charge_, "gen_D0Dau1_charge[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId_, "gen_D0Dau1_pdgId[candSize_gen]/I");

          PATCompositeNtuple->Branch("gen_D0Dau2_pT",&gen_D0Dau2_pT_, "gen_D0Dau2_pT[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_eta",&gen_D0Dau2_eta_, "gen_D0Dau2_eta[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_phi",&gen_D0Dau2_phi_, "gen_D0Dau2_phi[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_mass",&gen_D0Dau2_mass_, "gen_D0Dau2_mass[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_y",&gen_D0Dau2_y_, "gen_D0Dau2_y[candSize_gen]/F");
          PATCompositeNtuple->Branch("gen_D0Dau2_charge",&gen_D0Dau2_charge_, "gen_D0Dau2_charge[candSize_gen]/I");
          PATCompositeNtuple->Branch("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId_, "gen_D0Dau2_pdgId[candSize_gen]/I");
        }
    }
}

int PATCompositeTreeProducer2::
muAssocToTrack( const reco::TrackRef& trackref,
                const edm::Handle<reco::MuonCollection>& muonh) const {
  auto muon = std::find_if(muonh->cbegin(),muonh->cend(),
                           [&](const reco::Muon& m) {
                             return ( m.track().isNonnull() &&
                                      m.track() == trackref    );
                           });
  return ( muon != muonh->cend() ? std::distance(muonh->cbegin(),muon) : -1 );
}

// ------------ method called once each job just after ending the event
//loop  ------------
void 
PATCompositeTreeProducer2::endJob() {
    
}

//define this as a plug-in
DEFINE_FWK_MODULE(PATCompositeTreeProducer2);
