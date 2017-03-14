#ifndef CalibratedElectronProducerRun2_h
#define CalibratedElectronProducerRun2_h

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Common/interface/ValueMap.h"

#include "CondFormats/DataRecord/interface/GBRDWrapperRcd.h"
#include "CondFormats/EgammaObjects/interface/GBRForestD.h"
#include "EgammaAnalysis/ElectronTools/interface/EpCombinationToolSemi.h"
#include "EgammaAnalysis/ElectronTools/interface/ElectronEnergyCalibratorRun2.h"

#include <vector>
#include <memory>

template<typename T>
class CalibratedElectronProducerRun2T: public edm::stream::EDProducer<>
{
public:
  explicit CalibratedElectronProducerRun2T( const edm::ParameterSet & ) ;
  virtual ~CalibratedElectronProducerRun2T();
  virtual void produce( edm::Event &, const edm::EventSetup & ) override ;
  
private:
  edm::EDGetTokenT<edm::View<T> > theElectronToken;
  std::vector<std::string>        theGBRForestName;
  std::vector<const GBRForestD* > theGBRForestHandle;
  
  EpCombinationToolSemi        theEpCombinationTool;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBToken_;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEEToken_;
  typedef edm::ValueMap<float> floatMap;

protected:
  ElectronEnergyCalibratorRun2 theEnCorrectorRun2;

};

template<typename T>
CalibratedElectronProducerRun2T<T>::CalibratedElectronProducerRun2T( const edm::ParameterSet & conf ) :
  theElectronToken(consumes<edm::View<T> >(conf.getParameter<edm::InputTag>("electrons"))),
  theGBRForestName(conf.getParameter< std::vector<std::string> >("gbrForestName")),
  theEpCombinationTool(),
  theEnCorrectorRun2(theEpCombinationTool, conf.getParameter<bool>("isMC"), conf.getParameter<bool>("isSynchronization"), conf.getParameter<std::string>("correctionFile")),
  recHitCollectionEBToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEB"))),
  recHitCollectionEEToken_(consumes<EcalRecHitCollection>(conf.getParameter<edm::InputTag>("recHitCollectionEE")))
{
  produces<std::vector<T> >();
  produces<floatMap>("EGMscaleStatUncertainty");
  produces<floatMap>("EGMscaleSystUncertainty");
  produces<floatMap>("EGMscaleGainUncertainty");
}

template<typename T>
CalibratedElectronProducerRun2T<T>::~CalibratedElectronProducerRun2T()
{
}

template<typename T>
void CalibratedElectronProducerRun2T<T>::loopHook(T& ele)
{
}

template<typename T>
void
CalibratedElectronProducerRun2T<T>::produce( edm::Event & iEvent, const edm::EventSetup & iSetup ) 
{
  
  for (auto&& forestName : theGBRForestName) {
    edm::ESHandle<GBRForestD> forestHandle;
    iSetup.get<GBRDWrapperRcd>().get(forestName, forestHandle);
    theGBRForestHandle.emplace_back(forestHandle.product());      
  }
  
  theEpCombinationTool.init(theGBRForestHandle);
  
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(theElectronToken, in);
  
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
  
  for (const T &ele : *in) {
    out->push_back(ele);
    loopHook(out->back());
    const EcalRecHitCollection* recHits = (ele.isEB()) ? recHitCollectionEBHandle.product() : recHitCollectionEEHandle.product();
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

typedef CalibratedElectronProducerRun2T<reco::GsfElectron> CalibratedElectronProducerRun2;
typedef CalibratedElectronProducerRun2T<pat::Electron> CalibratedPatElectronProducerRun2Base;

// the classdef for pat::Electron was renamed here
// we want to use userInt which is not allowed in templates that might get implemented with
// reco::GsfElectron (also, polymorphism would make much more sense here instead of creating a
// a class template for two types that are related via inheritance; the ElectronEnergyCalibratorRun2
// is doing this as well)
// as a result, the base class must be extended
class CalibratedPatElectronProducerRun2: public CalibratedPatElectronProducerRun2Base
{
public:
    explicit CalibratedPatElectronProducerRun2(const edm::ParameterSet&);
    virtual ~CalibratedPatElectronProducerRun2();

    virtual void loopHook(pat::Electron& ele);

private:
    std::string seedUserInt_;
    bool useUserIntSeed_;
    TRandom3* rnd_;
};

CalibratedPatElectronProducerRun2::CalibratedPatElectronProducerRun2(const edm::ParameterSet & conf)
    : CalibratedPatElectronProducerRun2Base(conf)
    , seedUserInt_(conf.getParameter<std::string>("seedUserInt"))
    , rnd_(0)
{
    useUserIntSeed_ = !seedUserInt_.empty();
    if (useUserIntSeed_)
    {
        rnd_ = new TRandom3();
        theEnCorrectorRun2.initPrivateRng(rnd_);
    }
}

CalibratedPatElectronProducerRun2::~CalibratedPatElectronProducerRun2()
{
    if (rnd_)
    {
        delete rnd_;
    }
}

void CalibratedPatElectronProducerRun2::loopHook(pat::Electron& ele)
{
    // store the original pt as a userFloat
    // this value can be used later to derive the original p4 as the correction is multiplicative
    ele.addUserFloat("ptBeforeRun2Calibration", ele.pt());

    // reset the seed
    if (useUserIntSeed_)
    {
        if (!ele.hasUserInt(seedUserInt_))
        {
            throw cms::Exception("ElectronUserInt") << "missing seed userInt " << seedUserInt_;
        }
        rnd_->SetSeed((uint32_t)ele.userInt(seedUserInt_));
    }
}

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(CalibratedElectronProducerRun2);
DEFINE_FWK_MODULE(CalibratedPatElectronProducerRun2);

#endif
