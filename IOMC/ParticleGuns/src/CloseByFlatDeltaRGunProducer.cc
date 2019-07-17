#include <ostream>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "DataFormats/Math/interface/Vector3D.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "IOMC/ParticleGuns/interface/CloseByFlatDeltaRGunProducer.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"

using namespace edm;
using namespace std;

CloseByFlatDeltaRGunProducer::CloseByFlatDeltaRGunProducer(const ParameterSet& pset) : BaseFlatGunProducer(pset) {
  ParameterSet gunParams = pset.getParameter<ParameterSet>("PGunParameters");

  fPartIDs = gunParams.getParameter<vector<int> >("PartID");
  fNParticles = gunParams.getParameter<int>("NParticles");
  fShootPartIDs = gunParams.getParameter<bool>("ShootPartIDs");
  fRandomShoot = gunParams.getParameter<bool>("RandomShoot");

  fEnMax = gunParams.getParameter<double>("EnMax");
  fEnMin = gunParams.getParameter<double>("EnMin");
  fRhoMax = gunParams.getParameter<double>("RhoMax");
  fRhoMin = gunParams.getParameter<double>("RhoMin");
  fZMax = gunParams.getParameter<double>("ZMax");
  fZMin = gunParams.getParameter<double>("ZMin");
  fPhiMin = gunParams.getParameter<double>("MinPhi");
  fPhiMax = gunParams.getParameter<double>("MaxPhi");
  fDeltaR = gunParams.getParameter<double>("DeltaR");

  produces<HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();
}

CloseByFlatDeltaRGunProducer::~CloseByFlatDeltaRGunProducer() {}

void CloseByFlatDeltaRGunProducer::produce(Event& event, const EventSetup& setup) {
  edm::Service<edm::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine* engine = &(rng->getEngine(event.streamID()));

  if (fVerbosity > 0) {
    LogDebug("CloseByFlatDeltaRGunProducer") << " : Begin New Event Generation" << endl;
  }

  // create a new event to fill
  fEvt = new HepMC::GenEvent();

  // determine gun parameters for first particle to shoot (postfixed with "0")
  double phi = CLHEP::RandFlat::shoot(engine, fPhiMin, fPhiMax);
  double rho = CLHEP::RandFlat::shoot(engine, fRhoMin, fRhoMax);
  double z = CLHEP::RandFlat::shoot(engine, fZMin, fZMax);
  double eta = -log(tan(0.5 * atan(rho / z)));
  double phi0 = phi;
  double eta0 = eta;

  // determine the number of particles to shoot
  int n = 0;
  if (fShootPartIDs) {
    n = (int)fPartIDs.size();
  } else if (fRandomShoot) {
    n = CLHEP::RandFlat::shoot(engine, 1, fNParticles + 1);
  } else {
    n = fNParticles;
  }

  // shoot particles
  for (int i = 0; i < n; i++) {
    // find a new position relative to the first particle, obviously for all but the first one
    if (i > 0) {
      // create a random deltaR up to fDeltaR
      double deltaR = CLHEP::RandFlat::shoot(engine, 0., fDeltaR);

      // split delta R randomly in phi and eta directions
      double alpha = CLHEP::RandFlat::shoot(engine, 0., 2. * pi);
      double deltaPhi = sin(alpha) * deltaR;
      double deltaEta = cos(alpha) * deltaR;

      // update phi
      phi = phi0 + deltaPhi;

      // update rho
      // the approach is to transorm a difference in eta to a difference in rho using:
      // 1. tan(theta) = rho / z       <=> rho   = z * tan(theta)
      // 2. eta = -log(tan(theta / 2)) <=> theta = 2 * atan(exp(-eta))
      // 2. in 1.                       => rho   = z * tan(2 * atan(exp(-eta)))
      // using tan(atan(2 * x)) = - 2 * x (x^2 - 1) leads to
      //                                =>  rho = -2 * z * x / (x^2 - 1), with x = exp(-eta)
      // for eta = eta0 + deltaEta, this allows for defining x = exp(-(eta0 + deltaEta))
      eta = eta0 + deltaEta;
      double x = exp(-eta);
      rho = -2 * z * x / (x * x - 1);
    }

    // compute the gun / vertex position
    // the time offset is given in c*t and is used later on in the digitization
    double x = rho * cos(phi);
    double y = rho * sin(phi);
    double timeOffset = sqrt(x * x + y * y + z * z) * cm / c_light;
    HepMC::GenVertex* vtx = new HepMC::GenVertex(HepMC::FourVector(x * cm, y * cm, z * cm, timeOffset * c_light));

    // obtain kinematics
    int id = fShootPartIDs ? fPartIDs[i] : CLHEP::RandFlat::shoot(engine, 0, fPartIDs.size());
    const HepPDT::ParticleData* pData = fPDGTable->particle(HepPDT::ParticleID(abs(id)));
    double e = CLHEP::RandFlat::shoot(engine, fEnMin, fEnMax);
    double m = pData->mass().value();
    double p = sqrt(e * e - m * m);

    // determine momentum vector, that should point in the direction defined between
    // (0,0,0) and the gun position with a length of p
    math::XYZVector direction(x, y, z);
    math::XYZVector pVec = direction.unit() * p;

    // create the GenParticle
    HepMC::FourVector fVec(pVec.x(), pVec.y(), pVec.z(), e);
    HepMC::GenParticle* particle = new HepMC::GenParticle(fVec, id, 1);
    particle->suggest_barcode(i + 1);

    // add the particle to the vertex and the vertex to the event
    vtx->add_particle_out(particle);
    fEvt->add_vertex(vtx);

    if (fVerbosity > 0) {
      vtx->print();
      particle->print();
    }
  }

  // fill event attributes
  fEvt->set_event_number(event.id().event());
  fEvt->set_signal_process_id(20);

  if (fVerbosity > 0) {
    fEvt->print();
  }

  // store outputs
  unique_ptr<HepMCProduct> BProduct(new HepMCProduct());
  BProduct->addHepMCData(fEvt);
  event.put(std::move(BProduct), "unsmeared");
  unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
  event.put(std::move(genEventInfo));

  if (fVerbosity > 0) {
    LogDebug("CloseByFlatDeltaRGunProducer") << " : Event Generation Done " << endl;
  }
}
