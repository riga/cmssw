// system include files
#include <memory>
#include <iostream>

// user include files

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HeavyIonEvent/interface/VoronoiBackground.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/LeafCandidate.h"

#include "CondFormats/HIObjects/interface/UETable.h"
#include "CondFormats/DataRecord/interface/HeavyIonUERcd.h"

// For DB entry using JetCorrector to store the vector of float
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"

#include "VoronoiAlgorithm.h"

using namespace std;
//
// class declaration
//

class VoronoiBackgroundProducer : public edm::EDProducer {
   public:
      explicit VoronoiBackgroundProducer(const edm::ParameterSet&);
      ~VoronoiBackgroundProducer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void produce(edm::Event&, const edm::EventSetup&);
      
      // ----------member data ---------------------------

   edm::EDGetTokenT<reco::CandidateView> src_;
   VoronoiAlgorithm* voronoi_;
   bool doEqualize_;
   double equalizeThreshold0_;
   double equalizeThreshold1_;
   double equalizeR_;
   bool useTextTable_;
   bool jetCorrectorFormat_;
   bool isCalo_;
   std::string tableLabel_;
   int etaBins_;
   int fourierOrder_;
   std::vector<reco::VoronoiBackground> vvm;

};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
VoronoiBackgroundProducer::VoronoiBackgroundProducer(const edm::ParameterSet& iConfig):
   voronoi_(0),
   doEqualize_(iConfig.getParameter<bool>("doEqualize")),
   equalizeThreshold0_(iConfig.getParameter<double>("equalizeThreshold0")),
   equalizeThreshold1_(iConfig.getParameter<double>("equalizeThreshold1")),
   equalizeR_(iConfig.getParameter<double>("equalizeR")),
   useTextTable_(iConfig.getParameter<bool>("useTextTable")),
   jetCorrectorFormat_(iConfig.getParameter<bool>("jetCorrectorFormat")),
   isCalo_(iConfig.getParameter<bool>("isCalo")),
   tableLabel_(iConfig.getParameter<std::string>("tableLabel")),
   etaBins_(iConfig.getParameter<int>("etaBins")),
   fourierOrder_(iConfig.getParameter<int>("fourierOrder"))
{

   src_ = consumes<reco::CandidateView>(iConfig.getParameter<edm::InputTag>("src"));
   //register your products

   produces<reco::VoronoiMap>();
   produces<std::vector<float> >();
}


VoronoiBackgroundProducer::~VoronoiBackgroundProducer()
{
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
VoronoiBackgroundProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   if(voronoi_ == 0){
	UECalibration *ue = NULL;
	if (useTextTable_) {
	 const bool isData = iEvent.isRealData();
	 const char *calibrationFile = NULL;
      if(isCalo_){
        if(isData) calibrationFile = "RecoHI/HiJetAlgos/data/ue_calibrations_hermite_calo_data.txt";
        if(!isData) calibrationFile = "RecoHI/HiJetAlgos/data/ue_calibrations_hermite_calo_mc.txt";
      }else{
        if(isData) calibrationFile = "RecoHI/HiJetAlgos/data/ue_calibrations_hermite_pf_data.txt";
        if(!isData) calibrationFile = "RecoHI/HiJetAlgos/data/ue_calibrations_hermite_pf_mc.txt";
      }
	ue = new UECalibration(calibrationFile);
	}
	else if (jetCorrectorFormat_) {
		edm::ESHandle<JetCorrectorParametersCollection> ueHandle;

		iSetup.get<JetCorrectionsRecord>().get(tableLabel_,ueHandle);

		const JetCorrectorParametersCollection *payload = ueHandle.product();
		std::vector<float> ue_vec = (*payload)[JetCorrectorParametersCollection::L1Offset].record(0).parameters();

		ue = new UECalibration(ue_vec);
	}
	else {
	 edm::ESHandle<UETable> ueHandle;

	 iSetup.get<HeavyIonUERcd>().get(tableLabel_,ueHandle);

	 const UETable *ueTable = ueHandle.product();
	 ue = new UECalibration(ueTable->values);
	}

     voronoi_ = new VoronoiAlgorithm(ue,equalizeR_,std::pair<double, double>(equalizeThreshold0_,equalizeThreshold1_),doEqualize_);
   }

   voronoi_->clear();
   vvm.clear();

   edm::Handle<reco::CandidateView> inputsHandle;
   iEvent.getByToken(src_,inputsHandle);

   for(unsigned int i = 0; i < inputsHandle->size(); ++i){
      reco::CandidateViewRef ref(inputsHandle,i);
      voronoi_->push_back_particle(ref->pt(),ref->eta(),ref->phi(),0);
   }

   std::vector<double> subtracted_momenta = voronoi_->subtracted_unequalized_perp();
   std::vector<double> equalized_momenta = voronoi_->subtracted_equalized_perp();
   std::vector<double> particle_area = voronoi_->particle_area();
   std::vector<double> voronoi_vn = voronoi_->perp_fourier();
   auto vnout = std::make_unique<std::vector<float>>(voronoi_vn.begin(), voronoi_vn.end());
   auto mapout = std::make_unique<reco::VoronoiMap>();
   reco::VoronoiMap::Filler filler(*mapout);

   for(unsigned int i = 0; i < inputsHandle->size(); ++i){
      reco::CandidateViewRef ref(inputsHandle,i);
      const double pre_eq_pt = subtracted_momenta[i];
      const double post_eq_pt = equalized_momenta[i];
      const double area = particle_area[i];
      const double mass_square = ref->massSqr();
      const double pre_eq_mt = sqrt(mass_square + pre_eq_pt * pre_eq_pt);
      const double post_eq_mt = sqrt(mass_square + post_eq_pt * post_eq_pt);

      reco::VoronoiBackground bkg(pre_eq_pt,post_eq_pt,pre_eq_mt,post_eq_mt,area);
      LogDebug("VoronoiBackgroundProducer")<<"Subtraction --- oldpt : "<<ref->pt()<<" --- newpt : "<<post_eq_pt<<endl;
      vvm.push_back(bkg);
   }

   filler.insert(inputsHandle,vvm.begin(),vvm.end());
   filler.fill();
   iEvent.put(std::move(vnout));
   iEvent.put(std::move(mapout));
 
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
VoronoiBackgroundProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(VoronoiBackgroundProducer);
