import FWCore.ParameterSet.Config as cms


def ElectronUnpacker(**kwargs):
    mod = cms.EDProducer(
        "ElectronUnpacker",
        primaryElectrons=cms.InputTag("slimmedElectrons"),
        secondaryElectrons=cms.InputTag("slimmedLowPtElectrons"),
        maxDeltaR=cms.double(1e-4),
        maxRelPtDiff=cms.double(0.01),
        preferPrimary=cms.bool(True),
        mightGet=cms.optional.untracked.vstring,
    )
    for key, value in kwargs.items():
        setattr(mod, key, value)
    return mod
