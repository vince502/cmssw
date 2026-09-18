#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Visibility.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "PhotonConversionTrajectorySeedProducerFromSingleLegAlgo.h"
//#include "UserUtilities/TimingPerformance/interface/TimeReport.h"

class dso_hidden PhotonConversionTrajectorySeedProducerFromSingleLeg final : public edm::stream::EDProducer<> {
public:
  PhotonConversionTrajectorySeedProducerFromSingleLeg(const edm::ParameterSet&);
  ~PhotonConversionTrajectorySeedProducerFromSingleLeg() override { delete _theFinder; }
  PhotonConversionTrajectorySeedProducerFromSingleLeg(const PhotonConversionTrajectorySeedProducerFromSingleLeg&) =
      delete;
  PhotonConversionTrajectorySeedProducerFromSingleLeg& operator=(
      const PhotonConversionTrajectorySeedProducerFromSingleLeg&) = delete;
  void produce(edm::Event&, const edm::EventSetup&) override;

private:
  std::string _newSeedCandidates;
  PhotonConversionTrajectorySeedProducerFromSingleLegAlgo* _theFinder;
  bool _applyHFTowerSumCut;
  edm::EDGetTokenT<reco::Centrality> _centralityToken;
  double _maxHFTowerSum;
};

PhotonConversionTrajectorySeedProducerFromSingleLeg::PhotonConversionTrajectorySeedProducerFromSingleLeg(
    const edm::ParameterSet& conf)
    : _newSeedCandidates(conf.getParameter<std::string>("newSeedCandidates")),
      _applyHFTowerSumCut(conf.getParameter<bool>("applyHFTowerSumCut")),
      _centralityToken(_applyHFTowerSumCut
                           ? consumes<reco::Centrality>(conf.getParameter<edm::InputTag>("centrality"))
                           : edm::EDGetTokenT<reco::Centrality>()),
      _maxHFTowerSum(conf.getParameter<double>("maxHFTowerSum")) {
  _theFinder = new PhotonConversionTrajectorySeedProducerFromSingleLegAlgo(conf, consumesCollector());
  produces<TrajectorySeedCollection>(_newSeedCandidates);
}

void PhotonConversionTrajectorySeedProducerFromSingleLeg::produce(edm::Event& ev, const edm::EventSetup& es) {
  auto result = std::make_unique<TrajectorySeedCollection>();

  if (_applyHFTowerSumCut) {
    if (ev.get(_centralityToken).EtHFtowerSum() >= _maxHFTowerSum) {
      ev.put(std::move(result), _newSeedCandidates);
      return;
    }
  }

  _theFinder->find(ev, es, *result);
  result->shrink_to_fit();
  ev.put(std::move(result), _newSeedCandidates);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PhotonConversionTrajectorySeedProducerFromSingleLeg);
