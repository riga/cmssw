#include <vector>

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "RecoHGCal/TICL/interface/ExtendedSimCluster.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

using namespace ticl;

typedef std::vector<ExtendedSimCluster> ExtendedSimClusters;

class ExtendedSimClusterProducer : public edm::stream::EDProducer<> {
public:
  static void fillDescriptions(edm::ConfigurationDescriptions&);

  explicit ExtendedSimClusterProducer(const edm::ParameterSet&);
  ~ExtendedSimClusterProducer() override {}

  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void produce(edm::Event&, const edm::EventSetup&) override;

  void fillRecHitData(edm::Event&);

private:
  edm::EDGetTokenT<std::vector<SimCluster>> simClusterToken_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> recHitTokens_;

  hgcal::RecHitTools recHitTools_;
};
DEFINE_FWK_MODULE(ExtendedSimClusterProducer);

void ExtendedSimClusterProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("simClusters", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<std::vector<edm::InputTag>>("recHits", {
    edm::InputTag("HGCalRecHit", "HGCEERecHits"),
    edm::InputTag("HGCalRecHit", "HGCHEFRecHits"),
    edm::InputTag("HGCalRecHit", "HGCHEBRecHits")
  });
  descriptions.add("extendedSimClusterProducer", desc);
}

ExtendedSimClusterProducer::ExtendedSimClusterProducer(const edm::ParameterSet& pset)
    : simClusterToken_(consumes<std::vector<SimCluster>>(pset.getParameter<edm::InputTag>("simClusters"))) {
  for (const edm::InputTag& inputTag : pset.getParameter<std::vector<edm::InputTag>>("recHits")) {
    recHitTokens_.push_back(consumes<HGCRecHitCollection>(inputTag));
  }

  produces<ExtendedSimClusters>();
  produces<std::map<DetId, float>>("recHitEta");
  produces<std::map<DetId, float>>("recHitPhi");
}

void ExtendedSimClusterProducer::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
  recHitTools_.getEventSetup(setup);
}

void ExtendedSimClusterProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
  // fill RecHit data (eta and phi) for easy access without RecHitTools later
  fillRecHitData(event);

  // get SimClusters
  edm::Handle<std::vector<SimCluster>> scHandle;
  event.getByToken(simClusterToken_, scHandle);

  std::unique_ptr<ExtendedSimClusters> clusters = std::make_unique<ExtendedSimClusters>();
  clusters->resize(1);

  // create a dummy cluster
  ExtendedSimCluster dummy(scHandle->at(0));
  dummy.showerRadius = 1.;
  (*clusters)[0] = dummy;

  // TODO: logic

  event.put(std::move(clusters));
}

void ExtendedSimClusterProducer::fillRecHitData(edm::Event& event) {
  // fill RecHit eta and phi values
  std::unique_ptr<std::map<DetId, float>> recHitEtaMap = std::make_unique<std::map<DetId, float>>();
  std::unique_ptr<std::map<DetId, float>> recHitPhiMap = std::make_unique<std::map<DetId, float>>();
  for (const edm::EDGetTokenT<HGCRecHitCollection>& recHitToken : recHitTokens_) {
    edm::Handle<HGCRecHitCollection> rhHandle;
    event.getByToken(recHitToken, rhHandle);

    for (const HGCRecHit& recHit : *rhHandle) {
      const DetId& detId = recHit.detid();
      if (recHitEtaMap->find(detId) != recHitEtaMap->end()) {
        continue;
      }

      const GlobalPoint position = recHitTools_.getPosition(detId);

      (*recHitEtaMap)[detId] = recHitTools_.getEta(position);
      (*recHitPhiMap)[detId] = recHitTools_.getPhi(position);
    }
  }
  event.put(std::move(recHitEtaMap), "recHitEta");
  event.put(std::move(recHitPhiMap), "recHitPhi");
}
