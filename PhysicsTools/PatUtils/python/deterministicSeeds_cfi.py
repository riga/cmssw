# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
    produceCollections = cms.bool(False),
    produceValueMaps   = cms.bool(True),
    electronCollection = cms.InputTag("slimmedElectrons"),
    electronSeedName   = cms.string("electronDeterministicSeed"),
    electronSeedFactpr = cms.int(1),
    muonCollection     = cms.InputTag("slimmedMuons"),
    muonSeedName       = cms.string("muonDeterministicSeed"),
    muonSeedFactpr     = cms.int(1),
    tauCollection      = cms.InputTag("slimmedTaus"),
    tauSeedName        = cms.string("tauDeterministicSeed"),
    tauSeedFactpr      = cms.int(1),
    photonCollection   = cms.InputTag("slimmedPhotons"),
    photonSeedName     = cms.string("photonDeterministicSeed"),
    photonSeedFactpr   = cms.int(1),
    jetCollection      = cms.InputTag("slimmedJets"),
    jetSeedName        = cms.string("jetDeterministicSeed"),
    jetSeedFactpr      = cms.int(1),
    METCollection      = cms.InputTag("slimmedMETs"),
    METSeedName        = cms.string("METDeterministicSeed"),
    METSeedFactpr      = cms.int(1),
    debug              = cms.untracked.bool(False)
)
