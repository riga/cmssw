// Author: Marcel Rieger - marcel.rieger@cern.ch
// Date: 07/2019

#ifndef RecoHGCal_TICL_GlobalCache_H__
#define RecoHGCal_TICL_GlobalCache_H__

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

namespace tf = tensorflow;

namespace ticl {
  // base struct across ticl for objects hold in the edm::GlobalCache by plugins
  struct CacheBase {
    CacheBase(const edm::ParameterSet& params) {}

    virtual ~CacheBase() {}
  };

  // data structure hold by TrackstersProducer to store the TF graph for energy regression and ID
  struct TrackstersCache : CacheBase {
    TrackstersCache(const edm::ParameterSet& params) : CacheBase(params), eidGraphDef(nullptr) {}

    virtual ~TrackstersCache() {}

    std::atomic<tf::GraphDef*> eidGraphDef;
  };
}  // namespace ticl

#endif  // RecoHGCal_TICL_GlobalCache_H__
