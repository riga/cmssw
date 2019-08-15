#include <vector>

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "RecoHGCal/TICL/interface/ExtendedSimCluster.h"

using namespace ticl;

typedef std::vector<ExtendedSimCluster> ExtendedSimClusters;

class ExtendedSimClusterProducer : public edm::stream::EDProducer<> {
public:
  explicit ExtendedSimClusterProducer(const edm::ParameterSet&);
  ~ExtendedSimClusterProducer() override {}
  static void fillDescriptions(edm::ConfigurationDescriptions&);

  void produce(edm::Event&, const edm::EventSetup&) override;

private:
  edm::EDGetTokenT<std::vector<SimCluster>> simClusterToken_;
};
DEFINE_FWK_MODULE(ExtendedSimClusterProducer);

ExtendedSimClusterProducer::ExtendedSimClusterProducer(const edm::ParameterSet& pset)
    : simClusterToken_(consumes<std::vector<SimCluster>>(pset.getParameter<edm::InputTag>("simClusters"))) {
  produces<ExtendedSimClusters>();
}

void ExtendedSimClusterProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("simClusters", edm::InputTag("mix", "MergedCaloTruth"));
  descriptions.add("ExtendedSimClusterProducer", desc);
}

void ExtendedSimClusterProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
  std::unique_ptr<ExtendedSimClusters> clusters = std::make_unique<ExtendedSimClusters>();

  // TODO: logic

  event.put(std::move(clusters));
}
