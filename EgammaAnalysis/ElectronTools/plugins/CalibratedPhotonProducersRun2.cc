#ifndef CalibratedPhotonProducer_h
#define CalibratedPhotonProducer_h

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "EgammaAnalysis/ElectronTools/interface/PhotonEnergyCalibratorRun2.h"

#include <vector>

template<typename T>
class CalibratedPhotonProducerRun2T: public edm::stream::EDProducer<> {
public:
  explicit CalibratedPhotonProducerRun2T( const edm::ParameterSet & ) ;
  virtual ~CalibratedPhotonProducerRun2T();
  virtual void produce( edm::Event &, const edm::EventSetup & ) override ;

private:
  edm::EDGetTokenT<edm::View<T> > thePhotonToken;
  PhotonEnergyCalibratorRun2      theEnCorrectorRun2;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBToken_;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEEToken_;
  typedef edm::ValueMap<float> floatMap;

};

template<typename T>
CalibratedPhotonProducerRun2T<T>::CalibratedPhotonProducerRun2T( const edm::ParameterSet & conf ) :
  thePhotonToken(consumes<edm::View<T> >(conf.getParameter<edm::InputTag>("photons"))),
  theEnCorrectorRun2(conf.getParameter<bool>("isMC"), conf.getParameter<bool>("isSynchronization"), conf.getParameter<std::string >("correctionFile")),
  recHitCollectionEBToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEB"))),
  recHitCollectionEEToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEE")))
{
  produces<std::vector<T> >();
  produces<floatMap>("EGMscaleStatUncertainty");
  produces<floatMap>("EGMscaleSystUncertainty");
  produces<floatMap>("EGMscaleGainUncertainty");
}

template<typename T>
CalibratedPhotonProducerRun2T<T>::~CalibratedPhotonProducerRun2T()
{}

template<typename T>
void
CalibratedPhotonProducerRun2T<T>::produce( edm::Event & iEvent, const edm::EventSetup & iSetup ) {

  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(thePhotonToken, in);
  
  edm::Handle<EcalRecHitCollection> recHitCollectionEBHandle;
  edm::Handle<EcalRecHitCollection> recHitCollectionEEHandle;
  
  iEvent.getByToken(recHitCollectionEBToken_, recHitCollectionEBHandle);
  iEvent.getByToken(recHitCollectionEEToken_, recHitCollectionEEHandle);
  
  std::unique_ptr<std::vector<T> > out = std::make_unique<std::vector<T> >();
  std::vector<float> stat;
  std::vector<float> syst;
  std::vector<float> gain;

  out->reserve(in->size());   
  stat.reserve(in->size());   
  syst.reserve(in->size());   
  gain.reserve(in->size());   
  
  for (const T &pho : *in) {
    out->push_back(pho);
    const EcalRecHitCollection* recHits = (pho.isEB()) ? recHitCollectionEBHandle.product() : recHitCollectionEEHandle.product();    
    std::vector<float> scale_uncs = theEnCorrectorRun2.calibrate(out->back(), iEvent.id().run(), recHits, iEvent.streamID());
    stat.push_back(scale_uncs[0]);
    syst.push_back(scale_uncs[1]);
    gain.push_back(scale_uncs[2]);
  }
    
  auto calibratedHandle(iEvent.put(std::move(out)));
  std::unique_ptr<floatMap> statMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> systMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> gainMap = std::make_unique<floatMap>();

  floatMap::Filler statMapFiller(*statMap);
  statMapFiller.insert(calibratedHandle, stat.begin(), stat.end());
  statMapFiller.fill();
  iEvent.put(std::move(statMap), "EGMscaleStatUncertainty");

  floatMap::Filler systMapFiller(*systMap);
  systMapFiller.insert(calibratedHandle, syst.begin(), syst.end());
  systMapFiller.fill();
  iEvent.put(std::move(systMap), "EGMscaleSystUncertainty");

  floatMap::Filler gainMapFiller(*gainMap);
  gainMapFiller.insert(calibratedHandle, gain.begin(), gain.end());
  gainMapFiller.fill();
  iEvent.put(std::move(gainMap), "EGMscaleGainUncertainty");
}

typedef CalibratedPhotonProducerRun2T<reco::Photon> CalibratedPhotonProducerRun2;
typedef CalibratedPhotonProducerRun2T<pat::Photon> CalibratedPatPhotonProducerRun2;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(CalibratedPhotonProducerRun2);
DEFINE_FWK_MODULE(CalibratedPatPhotonProducerRun2);

#endif
