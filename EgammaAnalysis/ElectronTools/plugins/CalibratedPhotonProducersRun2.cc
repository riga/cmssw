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
  bool autoDataType;

  typedef edm::ValueMap<float> floatMap;
};

template<typename T>
CalibratedPhotonProducerRun2T<T>::CalibratedPhotonProducerRun2T( const edm::ParameterSet & conf ) :
  thePhotonToken(consumes<edm::View<T> >(conf.getParameter<edm::InputTag>("photons"))),
  theEnCorrectorRun2(conf.getParameter<bool>("isMC"), conf.getParameter<bool>("isSynchronization"), conf.getParameter<std::string >("correctionFile")),
  recHitCollectionEBToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEB"))),
  recHitCollectionEEToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEE"))),
  autoDataType((conf.existsAs<bool>("autoDataType") && !conf.getParameter<bool>("autoDataType") ) ? 0 : 1)
{
  produces<std::vector<T> >();
  produces<floatMap>("EGMscaleStatUpUncertainty");
  produces<floatMap>("EGMscaleStatDownUncertainty");
  produces<floatMap>("EGMscaleSystUpUncertainty");
  produces<floatMap>("EGMscaleSystDownUncertainty");
  produces<floatMap>("EGMscaleGainUpUncertainty");
  produces<floatMap>("EGMscaleGainDownUncertainty");
  produces<floatMap>("EGMresolutionRhoUpUncertainty");
  produces<floatMap>("EGMresolutionRhoDownUncertainty");
  produces<floatMap>("EGMresolutionPhiUpUncertainty");
  produces<floatMap>("EGMresolutionPhiDownUncertainty");
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
  std::vector<float> stat_up;
  std::vector<float> stat_dn;
  std::vector<float> syst_up;
  std::vector<float> syst_dn;
  std::vector<float> gain_up;
  std::vector<float> gain_dn;
  std::vector<float> rho_up;
  std::vector<float> rho_dn;
  std::vector<float> phi_up;
  std::vector<float> phi_dn;

  out->reserve(in->size());   
  stat_up.reserve(in->size());   
  stat_dn.reserve(in->size());   
  syst_up.reserve(in->size());   
  syst_dn.reserve(in->size());   
  gain_up.reserve(in->size());   
  gain_dn.reserve(in->size());   
  rho_up.reserve(in->size());   
  rho_dn.reserve(in->size());   
  phi_up.reserve(in->size());   
  phi_dn.reserve(in->size());   
  
  int eventIsMC = -1;
  if (autoDataType)
    eventIsMC = iEvent.isRealData() ? 0 : 1;

  for (const T &pho : *in) {
    out->push_back(pho);
    const EcalRecHitCollection* recHits = (pho.isEB()) ? recHitCollectionEBHandle.product() : recHitCollectionEEHandle.product();    
    std::vector<float> uncertainties = theEnCorrectorRun2.calibrate(out->back(), iEvent.id().run(), recHits, iEvent.streamID(), eventIsMC);
    stat_up.push_back(uncertainties[0]);
    stat_dn.push_back(uncertainties[1]);
    syst_up.push_back(uncertainties[2]);
    syst_dn.push_back(uncertainties[3]);
    gain_up.push_back(uncertainties[4]);
    gain_dn.push_back(uncertainties[5]);
    rho_up.push_back (uncertainties[6]);
    rho_dn.push_back (uncertainties[7]);
    phi_up.push_back (uncertainties[8]);
    phi_dn.push_back (uncertainties[9]);
  }
    
  auto calibratedHandle(iEvent.put(std::move(out)));
  std::unique_ptr<floatMap> statUpMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> statDownMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> systUpMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> systDownMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> gainUpMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> gainDownMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> rhoUpMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> rhoDownMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> phiUpMap = std::make_unique<floatMap>();
  std::unique_ptr<floatMap> phiDownMap = std::make_unique<floatMap>();

  floatMap::Filler statUpMapFiller(*statUpMap);
  statUpMapFiller.insert(calibratedHandle, stat_up.begin(), stat_up.end());
  statUpMapFiller.fill();
  iEvent.put(std::move(statUpMap), "EGMscaleStatUpUncertainty");

  floatMap::Filler statDownMapFiller(*statDownMap);
  statDownMapFiller.insert(calibratedHandle, stat_dn.begin(), stat_dn.end());
  statDownMapFiller.fill();
  iEvent.put(std::move(statDownMap), "EGMscaleStatDownUncertainty");

  floatMap::Filler systUpMapFiller(*systUpMap);
  systUpMapFiller.insert(calibratedHandle, syst_up.begin(), syst_up.end());
  systUpMapFiller.fill();
  iEvent.put(std::move(systUpMap), "EGMscaleSystUpUncertainty");

  floatMap::Filler systDownMapFiller(*systDownMap);
  systDownMapFiller.insert(calibratedHandle, syst_dn.begin(), syst_dn.end());
  systDownMapFiller.fill();
  iEvent.put(std::move(systDownMap), "EGMscaleSystDownUncertainty");

  floatMap::Filler gainUpMapFiller(*gainUpMap);
  gainUpMapFiller.insert(calibratedHandle, gain_up.begin(), gain_up.end());
  gainUpMapFiller.fill();
  iEvent.put(std::move(gainUpMap), "EGMscaleGainUpUncertainty");

  floatMap::Filler gainDownMapFiller(*gainDownMap);
  gainDownMapFiller.insert(calibratedHandle, gain_dn.begin(), gain_dn.end());
  gainDownMapFiller.fill();
  iEvent.put(std::move(gainDownMap), "EGMscaleGainDownUncertainty");

  floatMap::Filler rhoUpMapFiller(*rhoUpMap);
  rhoUpMapFiller.insert(calibratedHandle, rho_up.begin(), rho_up.end());
  rhoUpMapFiller.fill();
  iEvent.put(std::move(rhoUpMap), "EGMresolutionRhoUpUncertainty");

  floatMap::Filler rhoDownMapFiller(*rhoDownMap);
  rhoDownMapFiller.insert(calibratedHandle, rho_dn.begin(), rho_dn.end());
  rhoDownMapFiller.fill();
  iEvent.put(std::move(rhoDownMap), "EGMresolutionRhoDownUncertainty");

  floatMap::Filler phiUpMapFiller(*phiUpMap);
  phiUpMapFiller.insert(calibratedHandle, phi_up.begin(), phi_up.end());
  phiUpMapFiller.fill();
  iEvent.put(std::move(phiUpMap), "EGMresolutionPhiUpUncertainty");

  floatMap::Filler phiDownMapFiller(*phiDownMap);
  phiDownMapFiller.insert(calibratedHandle, phi_dn.begin(), phi_dn.end());
  phiDownMapFiller.fill();
  iEvent.put(std::move(phiDownMap), "EGMresolutionPhiDownUncertainty");
}

typedef CalibratedPhotonProducerRun2T<reco::Photon> CalibratedPhotonProducerRun2;
typedef CalibratedPhotonProducerRun2T<pat::Photon> CalibratedPatPhotonProducerRun2;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(CalibratedPhotonProducerRun2);
DEFINE_FWK_MODULE(CalibratedPatPhotonProducerRun2);

#endif
