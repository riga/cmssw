# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


process = cms.Process("TEST")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        "root://xrootd-cms.infn.it//store/mc/RunIISummer16MiniAODv2/ttHTobb_M125_TuneCUETP8M2_ttHtranche3_13TeV-powheg-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/50000/0C0796F5-CFC6-E611-ADB6-008CFA0A58B4.root"
    )
)

process.deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
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
    METSeedName        = cms.string("METDeterministicSeed"),
    debug              = cms.untracked.bool(True)
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string("output.root")
)

process.p = cms.Path(process.deterministicSeeds)
process.e = cms.EndPath(process.out)
