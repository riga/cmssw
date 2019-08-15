// Extended definition of SimClusters that contain properties to identify overlap.

#ifndef RecoHGCal_TICL_ExtendedSimCluster_h
#define RecoHGCal_TICL_ExtendedSimCluster_h

#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

namespace ticl {

  class ExtendedSimCluster : public SimCluster {
  public:
    ExtendedSimCluster() : SimCluster() { resetExtensions(); }

    ExtendedSimCluster(const SimTrack &simtrk) : SimCluster(simtrk) { resetExtensions(); }

    ExtendedSimCluster(EncodedEventId eventID, uint32_t particleID) : SimCluster(eventID, particleID) {
      resetExtensions();
    }

    ~ExtendedSimCluster() {}

    inline void resetExtensions() {
      axisPhi = 0.;
      axisEta = 0.;
      showerRadius = 0.;
    }

    float axisPhi;
    float axisEta;
    float showerRadius;
  };
}  // namespace ticl

#endif  // RecoHGCal_TICL_ExtendedSimCluster_h
