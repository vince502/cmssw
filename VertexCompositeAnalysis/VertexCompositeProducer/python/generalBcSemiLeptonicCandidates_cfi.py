import FWCore.ParameterSet.Config as cms

# Visible semileptonic Bc candidates (neutrino is missing by construction).
# Channels produced by one module:
#   BcToJpsiMuMuMuNu : J/psi(mu mu) + mu
#   BcToJpsiMuMuENu  : J/psi(mu mu) + e
#   BcToJpsiEEMuNu   : J/psi(e e)   + mu
#   BcToJpsiEEENu    : J/psi(e e)   + e
#   BcToJpsiLepNu    : union of all channels above
#   BcToJpsiCrossLepNu : union of cross channels only
#                       = BcToJpsiMuMuENu + BcToJpsiEEMuNu

generalBcSemiLeptonicCandidates = cms.EDProducer(
    "BcSemiLeptonicProducer",
    jpsiMuMuSrc = cms.InputTag("onia2MuMuPatGlbGlb"),
    jpsiEESrc = cms.InputTag("onia2ElectronElectronPatGlbGlb"),
    muonSrc = cms.InputTag("patMuonsWithTrigger"),
    electronSrc = cms.InputTag("patElectrons"),

    # J/psi preselection (common to mu mu and e e inputs)
    minJpsiPt = cms.double(0.0),
    maxJpsiAbsY = cms.double(2.4),
    jpsiMassMin = cms.double(2.5),
    jpsiMassMax = cms.double(3.5),

    # Extra lepton selection
    minMuonPt = cms.double(1.0),
    maxMuonAbsEta = cms.double(2.4),
    requireGlobalMuon = cms.bool(False),
    requirePFMuon = cms.bool(False),
    minElectronPt = cms.double(1.0),
    maxElectronAbsEta = cms.double(2.5),

    # Visible Bc candidate selection (J/psi + extra lepton only)
    minBcVisiblePt = cms.double(0.0),
    maxBcVisibleAbsY = cms.double(2.4),
    minBcVisibleMass = cms.double(2.0),
    maxBcVisibleMass = cms.double(10.0),

    # Reject using a lepton that is one of the J/psi daughters.
    maxOverlapDR = cms.double(0.01),

    # Channel switches
    doJpsiMuMuMu = cms.bool(True),
    doJpsiMuMuE = cms.bool(True),
    doJpsiEEMu = cms.bool(True),
    doJpsiEEE = cms.bool(True),
)

# Typical MiniAOD-oriented clone (override as needed in your cfg).
generalBcSemiLeptonicCandidatesMiniAOD = generalBcSemiLeptonicCandidates.clone(
    muonSrc = cms.InputTag("unpackedMuons"),
    electronSrc = cms.InputTag("slimmedElectrons"),
)

# Cross channels only:
#   Bc -> J/psi(mu mu) + e
#   Bc -> J/psi(e e)   + mu
bcToJpsiMuMuEAndEEMuCandidates = generalBcSemiLeptonicCandidates.clone(
    doJpsiMuMuMu = cms.bool(False),
    doJpsiMuMuE = cms.bool(True),
    doJpsiEEMu = cms.bool(True),
    doJpsiEEE = cms.bool(False),
)

bcToJpsiMuMuEAndEEMuCandidatesMiniAOD = generalBcSemiLeptonicCandidatesMiniAOD.clone(
    doJpsiMuMuMu = cms.bool(False),
    doJpsiMuMuE = cms.bool(True),
    doJpsiEEMu = cms.bool(True),
    doJpsiEEE = cms.bool(False),
)
