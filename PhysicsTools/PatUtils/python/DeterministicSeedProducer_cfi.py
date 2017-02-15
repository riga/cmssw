# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
    produceCollections = cms.bool(True),
    produceValueMaps   = cms.bool(True),
    electronCollection = cms.InputTag("slimmedElectrons"),
    electronSeedName   = cms.string("electronDeterministicSeed"),
    muonCollection     = cms.InputTag("slimmedMuons"),
    muonSeedName       = cms.string("muonDeterministicSeed"),
    tauCollection      = cms.InputTag("slimmedTaus"),
    tauSeedName        = cms.string("tauDeterministicSeed"),
    photonCollection   = cms.InputTag("slimmedPhotons"),
    photonSeedName     = cms.string("photonDeterministicSeed"),
    jetCollection      = cms.InputTag("slimmedJets"),
    jetSeedName        = cms.string("jetDeterministicSeed"),
    METCollection      = cms.InputTag("slimmedMETs"),
    METSeedName        = cms.string("METDeterministicSeed")
)
