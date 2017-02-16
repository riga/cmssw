# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
    produceCollections = cms.bool(False),
    produceValueMaps   = cms.bool(True),
    electronCollection = cms.InputTag("slimmedElectrons"),
    electronSeedName   = cms.string("electronDeterministicSeed"),
    electronSeedFactor = cms.int32(1),
    muonCollection     = cms.InputTag("slimmedMuons"),
    muonSeedName       = cms.string("muonDeterministicSeed"),
    muonSeedFactor     = cms.int32(1),
    tauCollection      = cms.InputTag("slimmedTaus"),
    tauSeedName        = cms.string("tauDeterministicSeed"),
    tauSeedFactor      = cms.int32(1),
    photonCollection   = cms.InputTag("slimmedPhotons"),
    photonSeedName     = cms.string("photonDeterministicSeed"),
    photonSeedFactor   = cms.int32(1),
    jetCollection      = cms.InputTag("slimmedJets"),
    jetSeedName        = cms.string("jetDeterministicSeed"),
    jetSeedFactor      = cms.int32(1),
    METCollection      = cms.InputTag("slimmedMETs"),
    METSeedName        = cms.string("METDeterministicSeed"),
    METSeedFactor      = cms.int32(1),
    debug              = cms.untracked.bool(False)
)
