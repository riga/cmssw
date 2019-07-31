/*
 * Particle gun that can be positioned in space given ranges in rho, z and phi.
 *
 * Authors:
 *   - Marcel Rieger <marcel.rieger@cern.ch>
 *   - Gerrit van Onsem <gerrit.van.onsem@cern.ch>
 */

#ifndef IOMC_PARTICLEGUN_CLOSEBYFLATDELTARGUNPRODUCER_H
#define IOMC_PARTICLEGUN_CLOSEBYFLATDELTARGUNPRODUCER_H

#include "IOMC/ParticleGuns/interface/BaseFlatGunProducer.h"

namespace edm {

  class CloseByFlatDeltaRGunProducer : public BaseFlatGunProducer {
  public:
    explicit CloseByFlatDeltaRGunProducer(const ParameterSet&);
    ~CloseByFlatDeltaRGunProducer() override;

  private:
    void produce(Event& e, const EventSetup& es) override;

  protected:
    // ids of particles to shoot
    std::vector<int> fPartIDs;

    // the number of particles to shoot
    int fNParticles;

    // flag that denotes that exactly the particles defined by fPartIds should be shot, with that order and quantity
    bool fExactShoot;

    // flag that denotes whether a random number of particles in the range [1, fNParticles] is shot
    bool fRandomShoot;

    // energy range
    double fEnMin;
    double fEnMax;

    // range of longitudinal gun position
    double fZMin;
    double fZMax;

    // range of radial gun position
    double fRhoMin;
    double fRhoMax;

    // phi range
    double fPhiMin;
    double fPhiMax;

    // deltaR parameters
    double fDeltaR;
  };

}  // namespace edm

#endif  // IOMC_PARTICLEGUN_CLOSEBYFLATDELTARGUNPRODUCER_H
