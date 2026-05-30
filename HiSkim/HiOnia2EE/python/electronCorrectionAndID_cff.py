import FWCore.ParameterSet.Config as cms

def configureElectronCorrectionAndID(process, isMC=False):
    """
    Configure electron energy corrections and HI-specific ID/Isolation
    
    This function sets up:
    1. Energy scale corrections via CorrectedElectronProducer
    2. MVA-based ID and isolation via HIElectronInfoProducer  
    
    Args:
        process: CMSSW process
        isMC: True for MC, False for data
    """
    
    # Step 1: Apply energy scale corrections
    process.correctedElectrons = cms.EDProducer(
        'CorrectedElectronProducer',
        semiDeterministic = cms.bool(True),
        src = cms.InputTag('slimmedElectrons'),
        centrality = cms.InputTag('centralityBin', 'HFtowers'),
        correctionFile = cms.string('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/SS2023PbPbMC.dat') if isMC else cms.string('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/SSHIRun2023A.dat'),
        minPt = cms.double(0.0),
        epCombConfig = cms.PSet(
            ecalTrkRegressionConfig = cms.PSet(
                rangeMinLowEt = cms.double(0.2),
                rangeMaxLowEt = cms.double(3.0),
                lowEtHighEtBoundary = cms.double(20.0),
                rangeMinHighEt = cms.double(0.0),
                rangeMaxHighEt = cms.double(3.0),
                forceHighEnergyTrainingIfSaturated = cms.bool(False),
            ),
            ecalTrkRegressionUncertConfig = cms.PSet(
                rangeMinLowEt = cms.double(0.0002),
                rangeMaxLowEt = cms.double(0.5),
                lowEtHighEtBoundary = cms.double(20.0),
                rangeMinHighEt = cms.double(0.0002),
                rangeMaxHighEt = cms.double(0.5),
                forceHighEnergyTrainingIfSaturated = cms.bool(False),
            ),
            maxEcalEnergyForComb = cms.double(200.0),
            minEOverPForComb = cms.double(0.025),
            maxEPDiffInSigmaForComb = cms.double(15.0),
            maxRelTrkMomErrForComb = cms.double(10.0),
        ),
    )
    
    # Step 2: Apply HI-specific electron ID and isolation
    process.hiElectrons = cms.EDProducer(
        'HIElectronInfoProducer',
        electrons = cms.InputTag('correctedElectrons'),
        pfCandidates = cms.InputTag('packedPFCandidates'),
        centrality = cms.InputTag('centralityBin', 'HFtowers'),
        etaMap = cms.InputTag('hiFJRhoProducerFinerBins', 'mapEtaEdges'),
        rhoMap = cms.InputTag('hiFJRhoProducerFinerBins', 'mapToRho'),
        pf_maxAbsEta = cms.double(2.8),
        sk_radius = cms.double(0.4),
        electron_minPt = cms.double(0.0),
        iso_rVeto = cms.double(0.026),
        iso_rCone = cms.double(0.3),
        file_idModel = cms.FileInPath('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/eleid_BDT.ubj'),
        file_isoModel = cms.FileInPath('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/eleiso_BDT.ubj'),
        file_corr = cms.FileInPath('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/SS2023PbPbMC.dat') if isMC else cms.FileInPath('HeavyIonsAnalysis/EGMAnalysis/data/Run3_2023_PbPb/SSHIRun2023A.dat'),
    )
    
    # Create the sequence
    process.electronCorrectionAndIDSequence = cms.Sequence(
        process.correctedElectrons *
        process.hiElectrons
    )
    
    return process

def updateOnia2EEProducer(process, producerName='onia2ElectronElectronPatGlbGlb'):
    """
    Update the Onia2EE producer to use corrected electrons with ID/ISO
    
    Args:
        process: CMSSW process
        producerName: Name of the onia2EE producer module
    """
    if hasattr(process, producerName):
        producer = getattr(process, producerName)
        # Update electron source to use corrected+ID electrons
        producer.electrons = cms.InputTag('hiElectrons')
    
    return process
