#include <ostream>

#include "IOMC/ParticleGuns/interface/CloseByParticleGunProducer.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "DataFormats/Math/interface/Vector3D.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"
#include "CLHEP/Random/RandFlat.h"

using namespace edm;
using namespace std;

CloseByParticleGunProducer::CloseByParticleGunProducer(const ParameterSet& pset) :
   BaseFlatGunProducer(pset)
{

  ParameterSet defpset ;
  ParameterSet pgun_params =
    pset.getParameter<ParameterSet>("PGunParameters") ;

  fEnMax = pgun_params.getParameter<double>("EnMax");
  fEnMin = pgun_params.getParameter<double>("EnMin");
  fRhoMax = pgun_params.getParameter<double>("RhoMax");
  fRhoMin = pgun_params.getParameter<double>("RhoMin");
  fZMax = pgun_params.getParameter<double>("ZMax");
  fZMin = pgun_params.getParameter<double>("ZMin");
  fPhiMin = pgun_params.getParameter<double>("MinPhi");
  fPhiMax = pgun_params.getParameter<double>("MaxPhi");
  fDeltaR = pgun_params.getParameter<double>("DeltaR");
  fPointing = pgun_params.getParameter<bool>("Pointing");
  fRandomShoot = pgun_params.getParameter<bool>("RandomShoot");
  fNParticles = pgun_params.getParameter<int>("NParticles");
  fPartIDs = pgun_params.getParameter< vector<int> >("PartID");

  produces<HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();

}

CloseByParticleGunProducer::~CloseByParticleGunProducer()
{
   // no need to cleanup GenEvent memory - done in HepMCProduct
}

void CloseByParticleGunProducer::produce(Event &e, const EventSetup& es)
{
   edm::Service<edm::RandomNumberGenerator> rng;
   CLHEP::HepRandomEngine* engine = &rng->getEngine(e.streamID());

   if ( fVerbosity > 0 )
     {
       LogDebug("CloseByParticleGunProducer") << " CloseByParticleGunProducer : Begin New Event Generation" << endl ;
     }
   fEvt = new HepMC::GenEvent() ;

   // loop over particles
   //
   int barcode = 1 ;
   int numParticles = fRandomShoot ? CLHEP::RandFlat::shoot(engine, 1, fNParticles + 1) : fNParticles;
   std::vector<int> particles;

   for(int i=0; i<numParticles; i++){
     int partIdx = CLHEP::RandFlat::shoot(engine, 0, fPartIDs.size());
     particles.push_back(fPartIDs[partIdx]);
     }

   double phi = CLHEP::RandFlat::shoot(engine, fPhiMin, fPhiMax);
   double rho = CLHEP::RandFlat::shoot(engine, fRhoMin, fRhoMax);
   double z = CLHEP::RandFlat::shoot(engine, fZMin, fZMax);
   double eta = - log(tan(0.5 * atan(rho / z)));
   double phi0 = phi;
   double eta0 = eta;

   for (unsigned int ip=0; ip<particles.size(); ++ip)
   {
    if (ip > 0)
    {
      // create a random deltaR up to fDeltaR
      double deltaR = CLHEP::RandFlat::shoot(engine, 0., fDeltaR);

      // split delta R randomly in phi and rho directions
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
      // for eta = eta0 + deltaEta, this allows for a simple definition x = exp(-(eta0 + deltaEta))
      eta = eta0 + deltaEta;
      double x = exp(-eta);
      rho = -2 * z * x / (x * x - 1);
    }

     int PartID = particles[ip] ;
     const HepPDT::ParticleData *PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID))) ;
     double energy = CLHEP::RandFlat::shoot(engine, fEnMin, fEnMax);
     double mass   = PData->mass().value();
     double mom    = sqrt(energy * energy - mass * mass);
     double px     = 0.;
     double py     = 0.;
     double pz     = mom;

     // Compute Vertex Position
     double x = rho*cos(phi);
     double y = rho*sin(phi);
     double timeOffset = sqrt(x * x + y * y + z * z) * cm / c_light;
     HepMC::GenVertex* Vtx = new HepMC::GenVertex(HepMC::FourVector(x * cm, y * cm, z * cm, timeOffset));

     HepMC::FourVector p(px,py,pz,energy);
     // If we are requested to be pointing to (0,0,0), correct the momentum direction
     if (fPointing) {
       math::XYZVector direction(x,y,z);
       math::XYZVector momentum = direction.unit() * mom;
       p.setX(momentum.x());
       p.setY(momentum.y());
       p.setZ(momentum.z());
     }
     HepMC::GenParticle* Part = new HepMC::GenParticle(p,PartID,1);
     Part->suggest_barcode( barcode );
     barcode++;

     Vtx->add_particle_out(Part);

     if (fVerbosity > 0) {
       Vtx->print();
       Part->print();
     }
     fEvt->add_vertex(Vtx);
   }


   fEvt->set_event_number(e.id().event());
   fEvt->set_signal_process_id(20);

   if ( fVerbosity > 0 )
   {
      fEvt->print();
   }

   unique_ptr<HepMCProduct> BProduct(new HepMCProduct());
   BProduct->addHepMCData( fEvt );
   e.put(std::move(BProduct), "unsmeared");

   unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
   e.put(std::move(genEventInfo));

   if ( fVerbosity > 0 )
     {
       LogDebug("CloseByParticleGunProducer") << " CloseByParticleGunProducer : Event Generation Done " << endl;
     }

   particles.clear();
}
