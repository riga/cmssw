# -*- coding: utf-8 -*-

import FWCore.ParameterSet.Config as cms


process.ttHFGenFilter = cms.EDFilter("ttHFGenFilter",
    genBHadFlavour            = cms.InputTag("matchGenBHadron", "genBHadFlavour"),
    genBHadFromTopWeakDecay   = cms.InputTag("matchGenBHadron", "genBHadFromTopWeakDecay"),
    genBHadPlusMothers        = cms.InputTag("matchGenBHadron", "genBHadPlusMothers"),
    genBHadIndex              = cms.InputTag("matchGenBHadron", "genBHadIndex"),
    OnlyHardProcessBHadrons   = cms.bool(False),
    genBHadPlusMothersIndices = cms.InputTag("matchGenBHadron", "genBHadPlusMothersIndices"),
    genParticles              = cms.InputTag("genParticles"),
    taggingMode               = cms.bool(False)
)
