# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
    produceCollections = cms.bool(False),
    produceValueMaps   = cms.bool(True),
    electronCollection = cms.InputTag("slimmedElectrons"),
    electronSeedFactor = cms.int32(1),
    muonCollection     = cms.InputTag("slimmedMuons"),
    muonSeedFactor     = cms.int32(1),
    tauCollection      = cms.InputTag("slimmedTaus"),
    tauSeedFactor      = cms.int32(1),
    photonCollection   = cms.InputTag("slimmedPhotons"),
    photonSeedFactor   = cms.int32(1),
    jetCollection      = cms.InputTag("slimmedJets"),
    jetSeedFactor      = cms.int32(1),
    METCollection      = cms.InputTag("slimmedMETs"),
    METSeedFactor      = cms.int32(1),
    seedUserInt        = cms.string("deterministicSeed"),
    debug              = cms.untracked.bool(False)
)
