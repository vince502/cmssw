#!/usr/bin/env python3
"""
FeatureEngineering.py
Create physics-motivated features for B meson signal vs background discrimination

Features include:
- Kinematic variables (pT ratios, angular separations, invariant masses)
- Topological variables (decay length significance, pointing angles)
- Quality variables (vertex probabilities, track quality)
- Composite variables optimized for each B meson type

Usage:
    from FeatureEngineering import BMesonFeatureEngineer
    engineer = BMesonFeatureEngineer()
    df_with_features = engineer.create_features(df)
"""

import numpy as np
import pandas as pd
import logging
from typing import Dict, List, Optional, Union
from pathlib import Path
import warnings

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Suppress warnings
warnings.filterwarnings("ignore", category=RuntimeWarning)

class BMesonFeatureEngineer:
    """Create physics-motivated features for B meson analysis"""
    
    def __init__(self):
        # Physics constants
        self.MUON_MASS = 0.10566  # GeV/c²
        self.PION_MASS = 0.13957  # GeV/c²
        self.KAON_MASS = 0.49368  # GeV/c²
        self.JPSI_MASS = 3.09692  # GeV/c² (PDG)
        self.KSTAR_MASS = 0.89166  # GeV/c² (K*0)
        
        # B meson masses (PDG)
        self.BPLUS_MASS = 5.27932  # GeV/c²
        self.BZERO_MASS = 5.27964  # GeV/c²
        self.BC_MASS = 6.2756     # GeV/c²
        
        # Feature categories for organization
        self.feature_categories = {
            'kinematic': [],
            'topological': [],
            'quality': [],
            'angular': [],
            'composite': [],
            'bmeson_specific': []
        }
    
    def create_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create all engineered features for the dataset"""
        logger.info(f"Creating features for {len(df)} B meson candidates")
        
        df_features = df.copy()
        
        # Basic kinematic features
        df_features = self._create_kinematic_features(df_features)
        
        # Angular features
        df_features = self._create_angular_features(df_features)
        
        # Topological features
        df_features = self._create_topological_features(df_features)
        
        # Quality features
        df_features = self._create_quality_features(df_features)
        
        # Composite features
        df_features = self._create_composite_features(df_features)
        
        # B meson type-specific features
        df_features = self._create_bmeson_specific_features(df_features)
        
        # Clean up NaN and inf values
        df_features = self._clean_features(df_features)
        
        n_new_features = len(df_features.columns) - len(df.columns)
        logger.info(f"Created {n_new_features} new features")
        
        return df_features
    
    def _create_kinematic_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create kinematic-based features"""
        logger.debug("Creating kinematic features")
        
        # pT ratios and fractions
        df['muon_pt_ratio'] = np.where(df['muon2Pt'] > 0, df['muon1Pt'] / df['muon2Pt'], -1)
        df['muon_pt_sum'] = df['muon1Pt'] + df['muon2Pt']
        df['muon_pt_diff'] = df['muon1Pt'] - df['muon2Pt']
        df['dimuon_pt_frac'] = np.where(df['bMesonPt'] > 0, df['dimuonPt'] / df['bMesonPt'], -1)
        
        # Track pT features
        df['track_pt_sum'] = df['track1Pt'].fillna(0) + df['track2Pt'].fillna(0)
        df['track_pt_max'] = np.maximum(df['track1Pt'].fillna(0), df['track2Pt'].fillna(0))
        df['track_pt_min'] = np.where(
            (df['track1Pt'].fillna(0) > 0) & (df['track2Pt'].fillna(0) > 0),
            np.minimum(df['track1Pt'], df['track2Pt']),
            np.where(df['track1Pt'].fillna(0) > 0, df['track1Pt'], 
                    np.where(df['track2Pt'].fillna(0) > 0, df['track2Pt'], 0))
        )
        
        # Track vs B meson pT ratios
        df['track1_pt_frac'] = np.where(df['bMesonPt'] > 0, df['track1Pt'].fillna(0) / df['bMesonPt'], -1)
        df['track2_pt_frac'] = np.where(df['bMesonPt'] > 0, df['track2Pt'].fillna(0) / df['bMesonPt'], -1)
        
        # Invariant mass calculations
        df['dimuon_mass_diff_jpsi'] = np.abs(df['dimuonMass'] - self.JPSI_MASS)
        
        # K*0 mass difference (for B0)
        df['kstar_mass_diff'] = np.abs(df['kstarMass'].fillna(-999) - self.KSTAR_MASS)
        
        # Transverse mass
        df['bmeson_mt'] = np.sqrt(df['bMesonMass']**2 + df['bMesonPt']**2)
        
        # Energy fractions
        # Calculate energies assuming massless particles for simplicity
        df['muon1_energy'] = np.sqrt(df['muon1Pt']**2 + df['muon1Eta']**2) * np.cosh(df['muon1Eta'])
        df['muon2_energy'] = np.sqrt(df['muon2Pt']**2 + df['muon2Eta']**2) * np.cosh(df['muon2Eta'])
        df['dimuon_energy'] = df['muon1_energy'] + df['muon2_energy']
        
        self.feature_categories['kinematic'].extend([
            'muon_pt_ratio', 'muon_pt_sum', 'muon_pt_diff', 'dimuon_pt_frac',
            'track_pt_sum', 'track_pt_max', 'track_pt_min', 'track1_pt_frac', 'track2_pt_frac',
            'dimuon_mass_diff_jpsi', 'kstar_mass_diff', 'bmeson_mt',
            'muon1_energy', 'muon2_energy', 'dimuon_energy'
        ])
        
        return df
    
    def _create_angular_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create angular separation and correlation features"""
        logger.debug("Creating angular features")
        
        # Angular separations
        df['muon_deta'] = np.abs(df['muon1Eta'] - df['muon2Eta'])
        df['muon_dphi'] = self._delta_phi(df['muon1Phi'], df['muon2Phi'])
        df['muon_dr'] = np.sqrt(df['muon_deta']**2 + df['muon_dphi']**2)
        
        # Track-muon angular separations
        df['track1_muon1_dr'] = np.sqrt(
            (df['track1Eta'].fillna(0) - df['muon1Eta'])**2 + 
            self._delta_phi(df['track1Phi'].fillna(0), df['muon1Phi'])**2
        )
        df['track1_muon2_dr'] = np.sqrt(
            (df['track1Eta'].fillna(0) - df['muon2Eta'])**2 + 
            self._delta_phi(df['track1Phi'].fillna(0), df['muon2Phi'])**2
        )
        
        # Dimuon-track angular separations
        df['dimuon_track1_deta'] = np.abs(df['dimuonEta'] - df['track1Eta'].fillna(0))
        df['dimuon_track1_dphi'] = self._delta_phi(df['dimuonPhi'], df['track1Phi'].fillna(0))
        df['dimuon_track1_dr'] = np.sqrt(df['dimuon_track1_deta']**2 + df['dimuon_track1_dphi']**2)
        
        # B meson opening angles
        df['bmeson_opening_angle'] = np.arccos(
            np.clip(
                np.cos(df['muon1Phi'] - df['muon2Phi']) * 
                np.cos(df['muon1Eta'] - df['muon2Eta']), -1, 1
            )
        )
        
        # Helicity angles (simplified)
        df['muon1_cos_theta'] = np.tanh(df['muon1Eta'])
        df['muon2_cos_theta'] = np.tanh(df['muon2Eta'])
        
        self.feature_categories['angular'].extend([
            'muon_deta', 'muon_dphi', 'muon_dr',
            'track1_muon1_dr', 'track1_muon2_dr',
            'dimuon_track1_deta', 'dimuon_track1_dphi', 'dimuon_track1_dr',
            'bmeson_opening_angle', 'muon1_cos_theta', 'muon2_cos_theta'
        ])
        
        return df
    
    def _create_topological_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create topological and vertex-based features"""
        logger.debug("Creating topological features")
        
        # Decay length features
        df['decay_length_ratio'] = np.where(
            df['bMesonDecayLength3D'] > 0,
            df['bMesonDecayLength2D'] / df['bMesonDecayLength3D'],
            -1
        )
        
        # Significance ratios
        df['decay_length_sig_ratio'] = np.where(
            df['bMesonDecayLengthSig3D'] > 0,
            df['bMesonDecayLengthSig2D'] / df['bMesonDecayLengthSig3D'],
            -1
        )
        
        # Pointing angle combinations
        df['pointing_angle_diff'] = np.abs(df['bMesonPointingAngle3D'] - df['bMesonPointingAngle2D'])
        df['cos_pointing_angle_2d'] = np.cos(df['bMesonPointingAngle2D'])
        df['cos_pointing_angle_3d'] = np.cos(df['bMesonPointingAngle3D'])
        
        # Normalized decay length (by momentum)
        df['decay_length_norm'] = np.where(
            df['bMesonPt'] > 0,
            df['bMesonDecayLength2D'] * df['bMesonMass'] / df['bMesonPt'],
            -1
        )
        
        # Impact parameter-like features
        df['vertex_displacement'] = np.sqrt(
            (df['bMesonVtxX'] - df['bestvx'])**2 + 
            (df['bMesonVtxY'] - df['bestvy'])**2 + 
            (df['bMesonVtxZ'] - df['bestvz'])**2
        )
        
        df['vertex_displacement_2d'] = np.sqrt(
            (df['bMesonVtxX'] - df['bestvx'])**2 + 
            (df['bMesonVtxY'] - df['bestvy'])**2
        )
        
        # Vertex error ratios
        df['vertex_error_ratio_xy'] = np.where(
            df['bMesonVtxYError'] > 0,
            df['bMesonVtxXError'] / df['bMesonVtxYError'],
            -1
        )
        
        df['vertex_error_ratio_z'] = np.where(
            df['bMesonVtxZError'] > 0,
            np.sqrt(df['bMesonVtxXError']**2 + df['bMesonVtxYError']**2) / df['bMesonVtxZError'],
            -1
        )
        
        self.feature_categories['topological'].extend([
            'decay_length_ratio', 'decay_length_sig_ratio', 'pointing_angle_diff',
            'cos_pointing_angle_2d', 'cos_pointing_angle_3d', 'decay_length_norm',
            'vertex_displacement', 'vertex_displacement_2d',
            'vertex_error_ratio_xy', 'vertex_error_ratio_z'
        ])
        
        return df
    
    def _create_quality_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create track and vertex quality features"""
        logger.debug("Creating quality features")
        
        # Vertex quality
        df['vertex_chi2_ndof'] = np.where(df['bMesonVtxNdof'] > 0, 
                                         df['bMesonVtxChi2'] / df['bMesonVtxNdof'], -1)
        df['vertex_chi2_prob_log'] = np.where(df['bMesonVtxProb'] > 0,
                                             -np.log10(df['bMesonVtxProb']), 10)
        
        # Muon quality scores
        df['muon1_quality_score'] = (
            df['muon1IsGlobal'].astype(int) + 
            df['muon1IsTracker'].astype(int) + 
            df['muon1IsPF'].astype(int) +
            np.where(df['muon1NHits'] >= 10, 1, 0) +
            np.where(df['muon1NPixelHits'] >= 2, 1, 0) +
            np.where(df['muon1NStations'] >= 2, 1, 0)
        )
        
        df['muon2_quality_score'] = (
            df['muon2IsGlobal'].astype(int) + 
            df['muon2IsTracker'].astype(int) + 
            df['muon2IsPF'].astype(int) +
            np.where(df['muon2NHits'] >= 10, 1, 0) +
            np.where(df['muon2NPixelHits'] >= 2, 1, 0) +
            np.where(df['muon2NStations'] >= 2, 1, 0)
        )
        
        df['dimuon_quality_score'] = df['muon1_quality_score'] + df['muon2_quality_score']
        
        # Track quality features
        df['track1_quality_score'] = (
            df['track1HighPurity'].fillna(False).astype(int) +
            np.where(df['track1NHits'].fillna(0) >= 8, 1, 0) +
            np.where(df['track1NPixelHits'].fillna(0) >= 2, 1, 0) +
            np.where(df['track1Chi2'].fillna(999) < 5, 1, 0)
        )
        
        df['track2_quality_score'] = (
            df['track2HighPurity'].fillna(False).astype(int) +
            np.where(df['track2NHits'].fillna(0) >= 8, 1, 0) +
            np.where(df['track2NPixelHits'].fillna(0) >= 2, 1, 0) +
            np.where(df['track2Chi2'].fillna(999) < 5, 1, 0)
        )
        
        # Impact parameter significance
        df['muon1_dxy_sig'] = np.where(df['muon1Dxy'] != 0, np.abs(df['muon1Dxy']) / 0.01, 0)  # Assume 100μm resolution
        df['muon2_dxy_sig'] = np.where(df['muon2Dxy'] != 0, np.abs(df['muon2Dxy']) / 0.01, 0)
        df['track1_dxy_sig'] = np.where(df['track1Dxy'].fillna(0) != 0, 
                                       np.abs(df['track1Dxy'].fillna(0)) / 0.01, 0)
        
        # Overall quality score
        df['overall_quality_score'] = (
            df['dimuon_quality_score'] + 
            df['track1_quality_score'] +
            df['track2_quality_score'] +
            np.where(df['vertex_chi2_ndof'] < 3, 2, 0) +
            np.where(df['bMesonVtxProb'] > 0.01, 2, 0)
        )
        
        self.feature_categories['quality'].extend([
            'vertex_chi2_ndof', 'vertex_chi2_prob_log',
            'muon1_quality_score', 'muon2_quality_score', 'dimuon_quality_score',
            'track1_quality_score', 'track2_quality_score',
            'muon1_dxy_sig', 'muon2_dxy_sig', 'track1_dxy_sig',
            'overall_quality_score'
        ])
        
        return df
    
    def _create_composite_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create composite features combining multiple variables"""
        logger.debug("Creating composite features")
        
        # BDT-style discriminating variables
        df['signal_discriminant_1'] = (
            df['cos_pointing_angle_3d'] * np.log1p(df['bMesonDecayLengthSig3D']) * 
            df['bMesonVtxProb'] * df['dimuon_quality_score']
        )
        
        df['signal_discriminant_2'] = (
            df['bMesonPt'] * df['cos_pointing_angle_2d'] / 
            np.maximum(df['dimuon_mass_diff_jpsi'], 0.001)
        )
        
        # Kinematic discriminant
        df['kinematic_discriminant'] = (
            df['bMesonPt'] * df['dimuonPt'] * df['track1Pt'].fillna(0) / 
            np.maximum(df['bMesonMass'] - 5.0, 0.1)**2
        )
        
        # Topology discriminant  
        df['topology_discriminant'] = (
            df['bMesonDecayLengthSig2D'] * df['cos_pointing_angle_2d'] * 
            df['bMesonVtxProb'] / np.maximum(df['vertex_chi2_ndof'], 0.1)
        )
        
        # Quality discriminant
        df['quality_discriminant'] = (
            df['overall_quality_score'] * df['bMesonVtxProb'] * 
            np.exp(-df['vertex_chi2_ndof'])
        )
        
        # Mass window features
        df['bmeson_mass_window'] = self._get_mass_window(df['bMesonMass'], df['bMesonType'])
        df['in_signal_window'] = (np.abs(df['bmeson_mass_window']) < 0.05).astype(int)
        
        # Isolation-like features
        df['muon_isolation'] = df['dimuonPt'] / (df['dimuonPt'] + df['track_pt_sum'])
        
        self.feature_categories['composite'].extend([
            'signal_discriminant_1', 'signal_discriminant_2', 'kinematic_discriminant',
            'topology_discriminant', 'quality_discriminant', 'bmeson_mass_window',
            'in_signal_window', 'muon_isolation'
        ])
        
        return df
    
    def _create_bmeson_specific_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Create B meson type-specific features"""
        logger.debug("Creating B meson type-specific features")
        
        # B meson type indicators
        df['is_bplus'] = (df['bMesonType'] == 0).astype(int)
        df['is_bzero'] = (df['bMesonType'] == 1).astype(int)
        df['is_bc'] = (df['bMesonType'] == 2).astype(int)
        
        # B+ specific features (J/ψ + K)
        df['bplus_kaon_pt'] = np.where(df['is_bplus'] == 1, df['track1Pt'].fillna(0), 0)
        df['bplus_jpsi_kaon_pt_ratio'] = np.where(
            (df['is_bplus'] == 1) & (df['track1Pt'].fillna(0) > 0),
            df['dimuonPt'] / df['track1Pt'], -1
        )
        
        # B0 specific features (J/ψ + K*0)
        df['bzero_kstar_quality'] = np.where(
            df['is_bzero'] == 1,
            np.exp(-df['kstar_mass_diff'].fillna(999)) * df['kstarPt'].fillna(0),
            0
        )
        
        df['bzero_kstar_pt_ratio'] = np.where(
            (df['is_bzero'] == 1) & (df['kstarPt'].fillna(0) > 0),
            df['dimuonPt'] / df['kstarPt'], -1
        )
        
        # Bc specific features (J/ψ + π)
        df['bc_pion_pt'] = np.where(df['is_bc'] == 1, df['track1Pt'].fillna(0), 0)
        df['bc_jpsi_pion_pt_ratio'] = np.where(
            (df['is_bc'] == 1) & (df['track1Pt'].fillna(0) > 0),
            df['dimuonPt'] / df['track1Pt'], -1
        )
        
        # Mass-optimized features for each type
        df['bplus_mass_pull'] = np.where(
            df['is_bplus'] == 1,
            (df['bMesonMass'] - self.BPLUS_MASS) / 0.05,  # Assume 50 MeV resolution
            0
        )
        
        df['bzero_mass_pull'] = np.where(
            df['is_bzero'] == 1,
            (df['bMesonMass'] - self.BZERO_MASS) / 0.05,
            0
        )
        
        df['bc_mass_pull'] = np.where(
            df['is_bc'] == 1,
            (df['bMesonMass'] - self.BC_MASS) / 0.08,  # Assume 80 MeV resolution for Bc
            0
        )
        
        self.feature_categories['bmeson_specific'].extend([
            'is_bplus', 'is_bzero', 'is_bc',
            'bplus_kaon_pt', 'bplus_jpsi_kaon_pt_ratio',
            'bzero_kstar_quality', 'bzero_kstar_pt_ratio',
            'bc_pion_pt', 'bc_jpsi_pion_pt_ratio',
            'bplus_mass_pull', 'bzero_mass_pull', 'bc_mass_pull'
        ])
        
        return df
    
    def _delta_phi(self, phi1: pd.Series, phi2: pd.Series) -> pd.Series:
        """Calculate delta phi with proper wrapping"""
        dphi = phi1 - phi2
        dphi = np.where(dphi > np.pi, dphi - 2*np.pi, dphi)
        dphi = np.where(dphi < -np.pi, dphi + 2*np.pi, dphi)
        return np.abs(dphi)
    
    def _get_mass_window(self, mass: pd.Series, btype: pd.Series) -> pd.Series:
        """Get distance from expected B meson mass for each type"""
        expected_mass = np.where(btype == 0, self.BPLUS_MASS,
                               np.where(btype == 1, self.BZERO_MASS,
                                      np.where(btype == 2, self.BC_MASS, 5.28)))
        return mass - expected_mass
    
    def _clean_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Clean features by handling NaN and inf values"""
        logger.debug("Cleaning features")
        
        # Replace inf with large finite values
        df = df.replace([np.inf, -np.inf], [999, -999])
        
        # Fill NaN values with appropriate defaults
        for col in df.columns:
            if df[col].dtype in ['float32', 'float64']:
                df[col] = df[col].fillna(-999)
            elif df[col].dtype in ['int8', 'int16', 'int32', 'int64']:
                df[col] = df[col].fillna(-1)
            elif df[col].dtype == 'bool':
                df[col] = df[col].fillna(False)
        
        return df
    
    def get_feature_list(self, category: Optional[str] = None) -> List[str]:
        """Get list of created features by category"""
        if category is None:
            # Return all features
            all_features = []
            for cat_features in self.feature_categories.values():
                all_features.extend(cat_features)
            return all_features
        elif category in self.feature_categories:
            return self.feature_categories[category]
        else:
            raise ValueError(f"Unknown category: {category}. Available: {list(self.feature_categories.keys())}")
    
    def get_feature_importance_groups(self) -> Dict[str, List[str]]:
        """Get feature groups for importance analysis"""
        return {
            'Kinematic': self.feature_categories['kinematic'],
            'Angular': self.feature_categories['angular'], 
            'Topological': self.feature_categories['topological'],
            'Quality': self.feature_categories['quality'],
            'Composite': self.feature_categories['composite'],
            'B Meson Specific': self.feature_categories['bmeson_specific']
        }
    
    def create_signal_labels(self, df: pd.DataFrame, 
                           mass_window: float = 0.05,
                           sideband_low: tuple = (-0.15, -0.1),
                           sideband_high: tuple = (0.1, 0.15)) -> pd.Series:
        """Create signal/background labels based on mass windows"""
        
        # Calculate mass difference from expected value
        mass_diff = self._get_mass_window(df['bMesonMass'], df['bMesonType'])
        
        # Signal region
        signal_mask = np.abs(mass_diff) < mass_window
        
        # Sideband regions
        sideband_mask = (
            ((mass_diff >= sideband_low[0]) & (mass_diff <= sideband_low[1])) |
            ((mass_diff >= sideband_high[0]) & (mass_diff <= sideband_high[1]))
        )
        
        # Create labels: 1 for signal, 0 for background, -1 for excluded
        labels = np.full(len(df), -1)
        labels[signal_mask] = 1
        labels[sideband_mask] = 0
        
        return pd.Series(labels, index=df.index)


def main():
    """Example usage of the feature engineering"""
    import pyarrow.parquet as pq
    
    # Load sample data
    input_file = "bmeson_data.parquet"
    if not Path(input_file).exists():
        logger.error(f"Input file {input_file} not found. Run ConvertToArrow.py first.")
        return
    
    logger.info(f"Loading data from {input_file}")
    table = pq.read_table(input_file)
    df = table.to_pandas()
    
    # Create features
    engineer = BMesonFeatureEngineer()
    df_with_features = engineer.create_features(df)
    
    # Create labels
    labels = engineer.create_signal_labels(df_with_features)
    df_with_features['signal_label'] = labels
    
    # Show statistics
    logger.info(f"Dataset shape: {df_with_features.shape}")
    logger.info(f"Signal candidates: {sum(labels == 1)}")
    logger.info(f"Background candidates: {sum(labels == 0)}")
    logger.info(f"Excluded candidates: {sum(labels == -1)}")
    
    # Show feature categories
    for category, features in engineer.get_feature_importance_groups().items():
        logger.info(f"{category} features ({len(features)}): {features[:3]}...")
    
    # Save enhanced dataset
    output_file = "bmeson_data_with_features.parquet"
    df_with_features.to_parquet(output_file, compression='snappy')
    logger.info(f"Enhanced dataset saved to {output_file}")


if __name__ == "__main__":
    main()