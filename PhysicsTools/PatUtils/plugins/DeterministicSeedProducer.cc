// -*- C++ -*-

//
// Package:     PhysicsTools/PatUtils
// Class:       DeterministicSeedProducer
// Description: Creates seeds for various objects to ensure reproducible random operations.
//
// Implementation:
//     Per object, seeds are created using only integer variables as they are not subject to
//     architecture dependent floating point inaccuracy. An intermediate seed is generated by
//     applying integer-safe operations to these numbers, such as addition, multiplication or bit
//     shifts. To cover the entire 4-byte range of possible seeds (e.g. for TRandom), this seed is
//     then stringified and passed to the openssl SHA1 hashing algorithm. The hexadecimal
//     representation of that hash is truncated at 8 characters whereas the maximum value 'ffffffff'
//     corresponds to the maximum value a uint32_t can hold.
//
// Original Author:  Marcel Rieger
//         Created:  Wed, 15 Feb 2017 16:26:16 GMT
//

#include <memory>

#include "openssl/sha.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"


uint32_t transformSeed(uint32_t seed)
{
    // safe convert to unsigned char array as required by SHA1
    std::string s = std::to_string(seed);
    unsigned char c[s.size()];
    for (size_t i = 0; i < s.size(); i++)
    {
        c[i] = (unsigned char)s[i];
    }

    // create the sha1 hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(c, sizeof(c), hash);

    // transform the hash into a hex string
    std::stringstream ss;
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        ss << std::hex << static_cast<unsigned>(hash[i]);
    }
    std::string hex = ss.str();

    // convert the first 8 characters to 4-bytes int with base 16
    // ffffffff equals the max unsigned int so no need for modulo
    uint32_t transformedSeed = strtoul(hex.substr(0, 8).c_str(), NULL, 16);

    return transformedSeed;
}

class DeterministicSeedProducer : public edm::stream::EDProducer<>
{
public:
    explicit DeterministicSeedProducer(const edm::ParameterSet&);
    ~DeterministicSeedProducer();

    template <typename T>
    void setupProducts(edm::InputTag& collection, edm::EDGetTokenT<std::vector<T> >& token,
        std::string& seedName);

    template <typename T>
    void createSeeds(edm::Event& iEvent, edm::EDGetTokenT<std::vector<T> >& token,
        std::string& seedName, uint32_t (*seedFunc)(edm::Event&, const T&)) const;

    static uint32_t createElectronSeed(edm::Event& iEvent, const pat::Electron& electron);
    static uint32_t createMuonSeed(edm::Event& iEvent, const pat::Muon& muon);
    static uint32_t createTauSeed(edm::Event& iEvent, const pat::Tau& tau);
    static uint32_t createPhotonSeed(edm::Event& iEvent, const pat::Photon& photon);
    static uint32_t createJetSeed(edm::Event& iEvent, const pat::Jet& jet);
    static uint32_t createMETSeed(edm::Event& iEvent, const pat::MET& MET);

private:
    virtual void produce(edm::Event&, const edm::EventSetup&) override;

    bool produceCollections_;
    bool produceValueMaps_;

    edm::InputTag electronCollection_;
    std::string electronSeedName_;
    edm::EDGetTokenT<std::vector<pat::Electron> > electronToken_;

    edm::InputTag muonCollection_;
    std::string muonSeedName_;
    edm::EDGetTokenT<std::vector<pat::Muon> > muonToken_;

    edm::InputTag tauCollection_;
    std::string tauSeedName_;
    edm::EDGetTokenT<std::vector<pat::Tau> > tauToken_;

    edm::InputTag photonCollection_;
    std::string photonSeedName_;
    edm::EDGetTokenT<std::vector<pat::Photon> > photonToken_;

    edm::InputTag jetCollection_;
    std::string jetSeedName_;
    edm::EDGetTokenT<std::vector<pat::Jet> > jetToken_;

    edm::InputTag METCollection_;
    std::string METSeedName_;
    edm::EDGetTokenT<std::vector<pat::MET> > METToken_;
};

DeterministicSeedProducer::DeterministicSeedProducer(const edm::ParameterSet& iConfig)
    : produceCollections_(iConfig.getParameter<bool>("produceCollections"))
    , produceValueMaps_(iConfig.getParameter<bool>("produceValueMaps"))
    , electronCollection_(iConfig.getParameter<edm::InputTag>("electronCollection"))
    , electronSeedName_(iConfig.getParameter<std::string>("electronSeedName"))
    , muonCollection_(iConfig.getParameter<edm::InputTag>("muonCollection"))
    , muonSeedName_(iConfig.getParameter<std::string>("muonSeedName"))
    , tauCollection_(iConfig.getParameter<edm::InputTag>("tauCollection"))
    , tauSeedName_(iConfig.getParameter<std::string>("tauSeedName"))
    , photonCollection_(iConfig.getParameter<edm::InputTag>("photonCollection"))
    , photonSeedName_(iConfig.getParameter<std::string>("photonSeedName"))
    , jetCollection_(iConfig.getParameter<edm::InputTag>("jetCollection"))
    , jetSeedName_(iConfig.getParameter<std::string>("jetSeedName"))
    , METCollection_(iConfig.getParameter<edm::InputTag>("METCollection"))
    , METSeedName_(iConfig.getParameter<std::string>("METSeedName"))
{
    setupProducts<pat::Electron>(electronCollection_, electronToken_, electronSeedName_);
    setupProducts<pat::Muon>(muonCollection_, muonToken_, muonSeedName_);
    setupProducts<pat::Tau>(tauCollection_, tauToken_, tauSeedName_);
    setupProducts<pat::Photon>(photonCollection_, photonToken_, photonSeedName_);
    setupProducts<pat::Jet>(jetCollection_, jetToken_, jetSeedName_);
    setupProducts<pat::MET>(METCollection_, METToken_, METSeedName_);
}

DeterministicSeedProducer::~DeterministicSeedProducer()
{
}

template <typename T>
void DeterministicSeedProducer::setupProducts(edm::InputTag& collection,
    edm::EDGetTokenT<std::vector<T> >& token, std::string& seedName)
{
    if (!collection.label().empty())
    {
        token = consumes<std::vector<T> >(collection);
        if (produceCollections_)
        {
            produces<std::vector<T> >(seedName);
        }
        if (produceValueMaps_)
        {
            produces<edm::ValueMap<uint32_t> >(seedName);
        }
    }
}

void DeterministicSeedProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    if (!produceCollections_ && !produceValueMaps_)
    {
        return;
    }

    if (!electronCollection_.label().empty())
    {
        createSeeds<pat::Electron>(iEvent, electronToken_, electronSeedName_,
            &DeterministicSeedProducer::createElectronSeed);
    }
    if (!muonCollection_.label().empty())
    {
        createSeeds<pat::Muon>(iEvent, muonToken_, muonSeedName_,
            &DeterministicSeedProducer::createMuonSeed);
    }
    if (!tauCollection_.label().empty())
    {
        createSeeds<pat::Tau>(iEvent, tauToken_, tauSeedName_,
            &DeterministicSeedProducer::createTauSeed);
    }
    if (!photonCollection_.label().empty())
    {
        createSeeds<pat::Photon>(iEvent, photonToken_, photonSeedName_,
            &DeterministicSeedProducer::createPhotonSeed);
    }
    if (!jetCollection_.label().empty())
    {
        createSeeds<pat::Jet>(iEvent, jetToken_, jetSeedName_,
            &DeterministicSeedProducer::createJetSeed);
    }
    if (!METCollection_.label().empty())
    {
        createSeeds<pat::MET>(iEvent, METToken_, METSeedName_,
            &DeterministicSeedProducer::createMETSeed);
    }
}

template <typename T>
void DeterministicSeedProducer::createSeeds(edm::Event& iEvent,
    edm::EDGetTokenT<std::vector<T> >& token, std::string& seedName,
    uint32_t (*seedFunc)(edm::Event&, const T&)) const
{
    edm::Handle<std::vector<T> > handle;
    iEvent.getByToken(token, handle);

    std::vector<uint32_t> seeds;
    for (const T& obj : *handle)
    {
        uint32_t seed = transformSeed(seedFunc(iEvent, obj));
        seeds.push_back(seed);
    }

    if (produceCollections_)
    {
        std::auto_ptr<std::vector<T> > out(new std::vector<T>(*handle));
        for (size_t i = 0; i < out->size(); i++)
        {
            out->at(i).addUserInt(seedName, seeds[i]);
        }
        iEvent.put(out, seedName);
    }

    if (produceValueMaps_)
    {
        std::auto_ptr<edm::ValueMap<uint32_t> > out(new edm::ValueMap<uint32_t>());
        edm::ValueMap<uint32_t>::Filler filler(*out);
        filler.insert(handle, seeds.begin(), seeds.end());
        filler.fill();
        iEvent.put(out, seedName);
    }
}

uint32_t DeterministicSeedProducer::createElectronSeed(edm::Event& iEvent, const pat::Electron& electron)
{
    uint32_t seed = 0;

    // event number
    seed += 99991 * iEvent.id().event();

    // ieta/ix and iphi/iy of super cluster seed
    int i1(1);
    int i2(1);
    const reco::SuperClusterRef& superCluster = electron.superCluster();
    const reco::CaloClusterPtr& seedCaloCluster = superCluster->seed();
    const DetId& detId = seedCaloCluster->seed();
    if (detId.subdetId() == EcalSubdetector::EcalBarrel)
    {
        const EBDetId ebDetId = EBDetId(detId.rawId());
        i1 = ebDetId.ieta();
        i2 = ebDetId.iphi();
    }
    else if (detId.subdetId() == EcalSubdetector::EcalEndcap)
    {
        const EEDetId eeDetId = EEDetId(detId.rawId());
        i1 = eeDetId.ix();
        i2 = eeDetId.iy();
    }
    seed += 401 * i1 * electron.basicClustersSize();
    seed += (99139 * i2) << 16;

    // number of valid strip hits
    const reco::HitPattern& hitPattern = electron.gsfTrack()->hitPattern();
    seed += 81701 * 16057 * hitPattern.numberOfValidStripHits();

    // number of valid pixel hits
    seed += hitPattern.numberOfValidPixelHits();

    return seed;
}

uint32_t DeterministicSeedProducer::createMuonSeed(edm::Event& iEvent, const pat::Muon& muon)
{
    uint32_t seed = 0;

    // event number
    seed += 83987 * iEvent.id().event();

    // number of hits, chambers, matches and station gap distance
    seed += 7537 * muon.numberOfValidHits();
    seed += muon.numberOfChambers() * muon.stationGapMaskDistance();
    seed += muon.numberOfMatches() << 8;

    // track information
    uint32_t tWM = 1;
    const auto& trackRef = muon.track();
    if (!trackRef.isNull()) tWM = trackRef.get()->hitPattern().trackerLayersWithMeasurement();
    seed += 17 * tWM;

    uint32_t vPH = 1;
    const auto& innerTrackRef = muon.innerTrack();
    if (!innerTrackRef.isNull()) vPH = innerTrackRef.get()->hitPattern().numberOfValidPixelHits();
    seed += vPH;

    return seed;
}

uint32_t DeterministicSeedProducer::createTauSeed(edm::Event& iEvent, const pat::Tau& tau)
{
    return 1;
}

uint32_t DeterministicSeedProducer::createPhotonSeed(edm::Event& iEvent, const pat::Photon& photon)
{
    return 1;
}

uint32_t DeterministicSeedProducer::createJetSeed(edm::Event& iEvent, const pat::Jet& jet)
{
    return 1;
}

uint32_t DeterministicSeedProducer::createMETSeed(edm::Event& iEvent, const pat::MET& MET)
{
    return 1;
}

DEFINE_FWK_MODULE(DeterministicSeedProducer);
