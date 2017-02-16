# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing("python")
options.setDefault("inputFiles", "root://xrootd-cms.infn.it//store/mc/RunIISpring15MiniAODv2/ttHTobb_M125_13TeV_powheg_pythia8/MINIAODSIM/74X_mcRun2_asymptotic_v2-v1/30000/00E46E74-C56D-E511-AA32-00266CFADD94.root")
options.setDefault("outputFile", "output.root")
options.setDefault("maxEvents", 10)
options.parseArguments()

process = cms.Process("TEST")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.deterministicSeeds = cms.EDProducer("DeterministicSeedProducer",
    produceCollections = cms.bool(True),
    produceValueMaps   = cms.bool(True),
    electronCollection = cms.InputTag("slimmedElectrons"),
    electronSeedName   = cms.string("electronDeterministicSeed"),
    electronSeedFactor = cms.int(1),
    muonCollection     = cms.InputTag("slimmedMuons"),
    muonSeedName       = cms.string("muonDeterministicSeed"),
    muonSeedFactor     = cms.int(1),
    tauCollection      = cms.InputTag("slimmedTaus"),
    tauSeedName        = cms.string("tauDeterministicSeed"),
    tauSeedFactor      = cms.int(1),
    photonCollection   = cms.InputTag("slimmedPhotons"),
    photonSeedName     = cms.string("photonDeterministicSeed"),
    photonSeedFactor   = cms.int(1),
    jetCollection      = cms.InputTag("slimmedJets"),
    jetSeedName        = cms.string("jetDeterministicSeed"),
    jetSeedFactor      = cms.int(1),
    METCollection      = cms.InputTag("slimmedMETs"),
    METSeedName        = cms.string("METDeterministicSeed"),
    METSeedFactor      = cms.int(1),
    debug              = cms.untracked.bool(True)
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string(options.outputFile)
)

process.p = cms.Path(process.deterministicSeeds)
process.e = cms.EndPath(process.out)
