#!/usr/bin/env python3
"""
ModelEvaluation.py
Comprehensive model evaluation and visualization for B meson XGBoost models

Features:
- ROC and PR curve analysis
- Feature importance visualization and SHAP analysis
- Signal efficiency vs background rejection curves
- Mass window optimization
- Model interpretability and error analysis
- Physics-motivated performance metrics

Usage:
    from ModelEvaluation import BMesonModelEvaluator
    evaluator = BMesonModelEvaluator(model_dir='models/bmeson_xgboost')
    evaluator.generate_evaluation_report()
"""

import json
import pickle
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
import warnings

import numpy as np
import pandas as pd
import pyarrow.parquet as pq
import xgboost as xgb
from sklearn.metrics import (
    roc_curve, auc, precision_recall_curve, average_precision_score,
    confusion_matrix, classification_report, accuracy_score,
    matthews_corrcoef, log_loss
)

import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import seaborn as sns
import shap
from scipy import stats
from scipy.optimize import minimize_scalar

# Configure logging and plotting
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)
plt.style.use('seaborn-v0_8')
warnings.filterwarnings("ignore", category=FutureWarning)

class BMesonModelEvaluator:
    """Comprehensive evaluation of B meson XGBoost models"""
    
    def __init__(self, model_dir: str, data_file: Optional[str] = None):
        self.model_dir = Path(model_dir)
        self.data_file = data_file
        
        # Load model artifacts
        self.model = self._load_model()
        self.feature_names = self._load_feature_names()
        self.scaler = self._load_scaler()
        self.feature_selector = self._load_feature_selector()
        self.training_results = self._load_training_results()
        
        # Configure matplotlib for publication-quality plots
        plt.rcParams.update({
            'font.size': 12,
            'axes.titlesize': 14,
            'axes.labelsize': 12,
            'xtick.labelsize': 10,
            'ytick.labelsize': 10,
            'legend.fontsize': 10,
            'figure.titlesize': 16,
            'figure.dpi': 100,
            'savefig.dpi': 300,
            'savefig.bbox': 'tight'
        })
        
        # Physics constants
        self.BPLUS_MASS = 5.27932  # GeV/c²
        self.BZERO_MASS = 5.27964  # GeV/c²
        self.BC_MASS = 6.2756     # GeV/c²
    
    def _load_model(self) -> xgb.XGBClassifier:
        """Load trained XGBoost model"""
        model_path = self.model_dir / 'xgboost_model.pkl'
        if not model_path.exists():
            raise FileNotFoundError(f"Model file not found: {model_path}")
        
        with open(model_path, 'rb') as f:
            model = pickle.load(f)
        
        logger.info("Loaded trained XGBoost model")
        return model
    
    def _load_feature_names(self) -> List[str]:
        """Load feature names"""
        features_path = self.model_dir / 'feature_names.json'
        if not features_path.exists():
            logger.warning("Feature names file not found")
            return None
        
        with open(features_path, 'r') as f:
            features = json.load(f)
        
        return features
    
    def _load_scaler(self):
        """Load feature scaler if exists"""
        scaler_path = self.model_dir / 'scaler.pkl'
        if not scaler_path.exists():
            return None
        
        with open(scaler_path, 'rb') as f:
            scaler = pickle.load(f)
        
        return scaler
    
    def _load_feature_selector(self):
        """Load feature selector if exists"""
        selector_path = self.model_dir / 'feature_selector.pkl'
        if not selector_path.exists():
            return None
        
        with open(selector_path, 'rb') as f:
            selector = pickle.load(f)
        
        return selector
    
    def _load_training_results(self) -> Dict[str, Any]:
        """Load training results"""
        results_path = self.model_dir / 'training_results.json'
        if not results_path.exists():
            logger.warning("Training results file not found")
            return {}
        
        with open(results_path, 'r') as f:
            results = json.load(f)
        
        return results
    
    def load_test_data(self, data_file: Optional[str] = None) -> Tuple[pd.DataFrame, pd.Series]:
        """Load test data for evaluation"""
        if data_file is None:
            data_file = self.data_file
        
        if data_file is None:
            raise ValueError("No data file specified")
        
        logger.info(f"Loading test data from {data_file}")
        
        # Load data
        table = pq.read_table(data_file)
        df = table.to_pandas()
        
        # Filter valid labels
        valid_mask = df['signal_label'].isin([0, 1])
        df_clean = df[valid_mask].copy()
        
        # Prepare features and target
        if self.feature_names:
            # Use the same features as training
            feature_cols = [col for col in self.feature_names if col in df_clean.columns]
            X = df_clean[feature_cols].copy()
        else:
            # Use all available features
            exclude_cols = ['runNb', 'eventNb', 'lumiSection', 'signal_label']
            feature_cols = [col for col in df_clean.columns if col not in exclude_cols]
            X = df_clean[feature_cols].copy()
        
        y = df_clean['signal_label'].copy()
        
        # Apply preprocessing
        X = self._preprocess_features(X)
        
        logger.info(f"Loaded {len(X)} test samples with {len(X.columns)} features")
        
        return X, y, df_clean
    
    def _preprocess_features(self, X: pd.DataFrame) -> pd.DataFrame:
        """Apply same preprocessing as training"""
        # Handle missing values
        X = X.fillna(-999)
        X = X.replace([np.inf, -np.inf], [999, -999])
        
        # Apply feature selection
        if self.feature_selector is not None:
            X_selected = self.feature_selector.transform(X)
            selected_features = X.columns[self.feature_selector.get_support()].tolist()
            X = pd.DataFrame(X_selected, columns=selected_features, index=X.index)
        
        # Apply scaling
        if self.scaler is not None:
            X_scaled = self.scaler.transform(X)
            X = pd.DataFrame(X_scaled, columns=X.columns, index=X.index)
        
        return X
    
    def generate_roc_analysis(self, X: pd.DataFrame, y: pd.Series, 
                             save_plots: bool = True) -> Dict[str, Any]:
        """Generate ROC curve analysis"""
        logger.info("Generating ROC analysis")
        
        # Get predictions
        y_proba = self.model.predict_proba(X)[:, 1]
        
        # Calculate ROC curve
        fpr, tpr, thresholds = roc_curve(y, y_proba)
        roc_auc = auc(fpr, tpr)
        
        # Find optimal threshold (Youden's J statistic)
        j_scores = tpr - fpr
        optimal_idx = np.argmax(j_scores)
        optimal_threshold = thresholds[optimal_idx]
        optimal_tpr = tpr[optimal_idx]
        optimal_fpr = fpr[optimal_idx]
        
        # Calculate metrics at different thresholds
        threshold_metrics = []
        for threshold in [0.1, 0.3, 0.5, 0.7, 0.9]:
            y_pred = (y_proba >= threshold).astype(int)
            accuracy = accuracy_score(y, y_pred)
            precision = np.sum((y_pred == 1) & (y == 1)) / np.sum(y_pred == 1) if np.sum(y_pred == 1) > 0 else 0
            recall = np.sum((y_pred == 1) & (y == 1)) / np.sum(y == 1) if np.sum(y == 1) > 0 else 0
            f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0
            
            threshold_metrics.append({
                'threshold': threshold,
                'accuracy': accuracy,
                'precision': precision,
                'recall': recall,
                'f1': f1
            })
        
        results = {
            'roc_auc': roc_auc,
            'fpr': fpr,
            'tpr': tpr,
            'thresholds': thresholds,
            'optimal_threshold': optimal_threshold,
            'optimal_tpr': optimal_tpr,
            'optimal_fpr': optimal_fpr,
            'threshold_metrics': threshold_metrics
        }
        
        if save_plots:
            self._plot_roc_curve(results)
        
        logger.info(f"ROC AUC: {roc_auc:.4f}")
        logger.info(f"Optimal threshold: {optimal_threshold:.3f} (TPR: {optimal_tpr:.3f}, FPR: {optimal_fpr:.3f})")
        
        return results
    
    def _plot_roc_curve(self, roc_results: Dict[str, Any]) -> None:
        """Plot ROC curve"""
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
        
        # ROC curve
        ax1.plot(roc_results['fpr'], roc_results['tpr'], 
                label=f'ROC Curve (AUC = {roc_results["roc_auc"]:.3f})', 
                linewidth=2, color='blue')
        ax1.plot([0, 1], [0, 1], 'k--', alpha=0.5, label='Random Classifier')
        ax1.plot(roc_results['optimal_fpr'], roc_results['optimal_tpr'], 
                'ro', markersize=8, label=f'Optimal Threshold = {roc_results["optimal_threshold"]:.3f}')
        
        ax1.set_xlabel('False Positive Rate')
        ax1.set_ylabel('True Positive Rate')
        ax1.set_title('Receiver Operating Characteristic (ROC) Curve')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        
        # Threshold metrics
        metrics_df = pd.DataFrame(roc_results['threshold_metrics'])
        ax2.plot(metrics_df['threshold'], metrics_df['accuracy'], 'o-', label='Accuracy')
        ax2.plot(metrics_df['threshold'], metrics_df['precision'], 's-', label='Precision')
        ax2.plot(metrics_df['threshold'], metrics_df['recall'], '^-', label='Recall')
        ax2.plot(metrics_df['threshold'], metrics_df['f1'], 'd-', label='F1 Score')
        ax2.axvline(roc_results['optimal_threshold'], color='red', linestyle='--', alpha=0.7)
        
        ax2.set_xlabel('Classification Threshold')
        ax2.set_ylabel('Metric Value')
        ax2.set_title('Performance Metrics vs Threshold')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'roc_analysis.png')
        plt.close()
    
    def generate_precision_recall_analysis(self, X: pd.DataFrame, y: pd.Series,
                                         save_plots: bool = True) -> Dict[str, Any]:
        """Generate precision-recall curve analysis"""
        logger.info("Generating precision-recall analysis")
        
        y_proba = self.model.predict_proba(X)[:, 1]
        
        # Calculate PR curve
        precision, recall, thresholds = precision_recall_curve(y, y_proba)
        avg_precision = average_precision_score(y, y_proba)
        
        # Find optimal threshold (maximize F1 score)
        f1_scores = 2 * precision[:-1] * recall[:-1] / (precision[:-1] + recall[:-1])
        f1_scores = np.nan_to_num(f1_scores)
        optimal_idx = np.argmax(f1_scores)
        optimal_threshold = thresholds[optimal_idx]
        optimal_precision = precision[optimal_idx]
        optimal_recall = recall[optimal_idx]
        optimal_f1 = f1_scores[optimal_idx]
        
        results = {
            'avg_precision': avg_precision,
            'precision': precision,
            'recall': recall,
            'thresholds': thresholds,
            'optimal_threshold': optimal_threshold,
            'optimal_precision': optimal_precision,
            'optimal_recall': optimal_recall,
            'optimal_f1': optimal_f1
        }
        
        if save_plots:
            self._plot_pr_curve(results, y)
        
        logger.info(f"Average Precision: {avg_precision:.4f}")
        logger.info(f"Optimal F1 threshold: {optimal_threshold:.3f} (F1: {optimal_f1:.3f})")
        
        return results
    
    def _plot_pr_curve(self, pr_results: Dict[str, Any], y: pd.Series) -> None:
        """Plot precision-recall curve"""
        fig, ax = plt.subplots(1, 1, figsize=(8, 6))
        
        baseline = np.sum(y) / len(y)  # Random classifier baseline
        
        ax.plot(pr_results['recall'], pr_results['precision'], 
               label=f'PR Curve (AP = {pr_results["avg_precision"]:.3f})', 
               linewidth=2, color='blue')
        ax.axhline(baseline, color='red', linestyle='--', alpha=0.7, 
                  label=f'Random Classifier (AP = {baseline:.3f})')
        ax.plot(pr_results['optimal_recall'], pr_results['optimal_precision'], 
               'ro', markersize=8, 
               label=f'Optimal F1 = {pr_results["optimal_f1"]:.3f}')
        
        ax.set_xlabel('Recall (Signal Efficiency)')
        ax.set_ylabel('Precision')
        ax.set_title('Precision-Recall Curve')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'precision_recall_curve.png')
        plt.close()
    
    def generate_feature_importance_analysis(self, X: pd.DataFrame, y: pd.Series,
                                           save_plots: bool = True) -> Dict[str, Any]:
        """Generate comprehensive feature importance analysis"""
        logger.info("Generating feature importance analysis")
        
        results = {}
        
        # XGBoost native feature importance
        feature_importance = pd.DataFrame({
            'feature': X.columns,
            'importance': self.model.feature_importances_
        }).sort_values('importance', ascending=False)
        
        results['xgb_importance'] = feature_importance.to_dict('records')
        
        # SHAP analysis
        try:
            logger.info("Computing SHAP values (this may take a while...)")
            explainer = shap.TreeExplainer(self.model)
            
            # Use a subset for SHAP computation to save time
            sample_size = min(1000, len(X))
            X_sample = X.sample(n=sample_size, random_state=42)
            shap_values = explainer.shap_values(X_sample)
            
            # SHAP feature importance
            shap_importance = pd.DataFrame({
                'feature': X.columns,
                'shap_importance': np.abs(shap_values).mean(axis=0)
            }).sort_values('shap_importance', ascending=False)
            
            results['shap_importance'] = shap_importance.to_dict('records')
            results['shap_values'] = shap_values
            results['shap_data'] = X_sample
            
        except Exception as e:
            logger.warning(f"SHAP analysis failed: {e}")
            results['shap_importance'] = None
            results['shap_values'] = None
        
        if save_plots:
            self._plot_feature_importance(results)
        
        return results
    
    def _plot_feature_importance(self, importance_results: Dict[str, Any]) -> None:
        """Plot feature importance analysis"""
        # Determine figure layout
        has_shap = importance_results['shap_values'] is not None
        fig_width = 20 if has_shap else 10
        ncols = 3 if has_shap else 1
        
        fig, axes = plt.subplots(1, ncols, figsize=(fig_width, 8))
        if not has_shap:
            axes = [axes]
        
        # XGBoost feature importance
        xgb_imp = pd.DataFrame(importance_results['xgb_importance']).head(20)
        axes[0].barh(range(len(xgb_imp)), xgb_imp['importance'])
        axes[0].set_yticks(range(len(xgb_imp)))
        axes[0].set_yticklabels(xgb_imp['feature'])
        axes[0].set_xlabel('XGBoost Feature Importance')
        axes[0].set_title('Top 20 Features (XGBoost)')
        axes[0].invert_yaxis()
        
        if has_shap:
            # SHAP feature importance
            shap_imp = pd.DataFrame(importance_results['shap_importance']).head(20)
            axes[1].barh(range(len(shap_imp)), shap_imp['shap_importance'])
            axes[1].set_yticks(range(len(shap_imp)))
            axes[1].set_yticklabels(shap_imp['feature'])
            axes[1].set_xlabel('SHAP Feature Importance')
            axes[1].set_title('Top 20 Features (SHAP)')
            axes[1].invert_yaxis()
            
            # SHAP summary plot
            shap_values = importance_results['shap_values']
            X_sample = importance_results['shap_data']
            
            # Create SHAP summary plot in the third subplot
            plt.sca(axes[2])
            shap.summary_plot(shap_values, X_sample, plot_type="bar", 
                            max_display=20, show=False)
            axes[2].set_title('SHAP Summary Plot')
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'feature_importance.png')
        plt.close()
        
        # Create separate detailed SHAP plots if available
        if has_shap:
            self._create_detailed_shap_plots(importance_results)
    
    def _create_detailed_shap_plots(self, importance_results: Dict[str, Any]) -> None:
        """Create detailed SHAP analysis plots"""
        shap_values = importance_results['shap_values']
        X_sample = importance_results['shap_data']
        
        # SHAP summary plot (beeswarm)
        plt.figure(figsize=(10, 8))
        shap.summary_plot(shap_values, X_sample, max_display=20, show=False)
        plt.tight_layout()
        plt.savefig(self.model_dir / 'shap_summary_beeswarm.png')
        plt.close()
        
        # SHAP waterfall plot for a single prediction
        plt.figure(figsize=(10, 8))
        shap.waterfall_plot(shap.Explanation(values=shap_values[0], 
                                           base_values=self.model.predict_proba(X_sample)[0, 0],
                                           data=X_sample.iloc[0].values,
                                           feature_names=X_sample.columns.tolist()),
                          show=False)
        plt.tight_layout()
        plt.savefig(self.model_dir / 'shap_waterfall_example.png')
        plt.close()
    
    def generate_mass_window_analysis(self, X: pd.DataFrame, y: pd.Series, 
                                    df_full: pd.DataFrame, save_plots: bool = True) -> Dict[str, Any]:
        """Analyze model performance across B meson mass windows"""
        logger.info("Generating mass window analysis")
        
        # Get predictions
        y_proba = self.model.predict_proba(X)[:, 1]
        
        # Define mass windows
        mass_windows = np.arange(-0.3, 0.31, 0.02)  # 20 MeV windows
        
        results = []
        for mass_center in mass_windows:
            window_mask = (
                (df_full['bmeson_mass_window'] >= mass_center - 0.01) &
                (df_full['bmeson_mass_window'] < mass_center + 0.01)
            )
            
            if np.sum(window_mask) < 10:  # Skip windows with too few events
                continue
            
            y_window = y[window_mask]
            y_proba_window = y_proba[window_mask]
            
            if len(np.unique(y_window)) < 2:  # Skip if only one class
                continue
            
            # Calculate metrics for this window
            from sklearn.metrics import roc_auc_score
            try:
                auc_score = roc_auc_score(y_window, y_proba_window)
            except:
                auc_score = 0.5
            
            signal_frac = np.mean(y_window)
            mean_score = np.mean(y_proba_window)
            n_events = len(y_window)
            
            results.append({
                'mass_center': mass_center,
                'auc_score': auc_score,
                'signal_fraction': signal_frac,
                'mean_model_score': mean_score,
                'n_events': n_events
            })
        
        results_df = pd.DataFrame(results)
        
        if save_plots and len(results_df) > 0:
            self._plot_mass_window_analysis(results_df, df_full, y_proba)
        
        return {'mass_window_analysis': results_df.to_dict('records')}
    
    def _plot_mass_window_analysis(self, results_df: pd.DataFrame, 
                                  df_full: pd.DataFrame, y_proba: np.ndarray) -> None:
        """Plot mass window analysis"""
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
        
        # AUC vs mass window
        ax1.plot(results_df['mass_center'], results_df['auc_score'], 'bo-')
        ax1.set_xlabel('Mass Window Center [GeV/c²]')
        ax1.set_ylabel('ROC AUC')
        ax1.set_title('Model Performance vs Mass Window')
        ax1.grid(True, alpha=0.3)
        ax1.axhline(0.5, color='red', linestyle='--', alpha=0.5)
        
        # Signal fraction vs mass window
        ax2.plot(results_df['mass_center'], results_df['signal_fraction'], 'go-')
        ax2.set_xlabel('Mass Window Center [GeV/c²]')
        ax2.set_ylabel('Signal Fraction')
        ax2.set_title('Signal Fraction vs Mass Window')
        ax2.grid(True, alpha=0.3)
        
        # Model score distribution by mass region
        signal_region = np.abs(df_full['bmeson_mass_window']) < 0.05
        sideband_region = (
            ((df_full['bmeson_mass_window'] >= -0.15) & (df_full['bmeson_mass_window'] <= -0.1)) |
            ((df_full['bmeson_mass_window'] >= 0.1) & (df_full['bmeson_mass_window'] <= 0.15))
        )
        
        ax3.hist(y_proba[signal_region], bins=50, alpha=0.7, label='Signal Region', density=True)
        ax3.hist(y_proba[sideband_region], bins=50, alpha=0.7, label='Sideband Region', density=True)
        ax3.set_xlabel('Model Score')
        ax3.set_ylabel('Density')
        ax3.set_title('Model Score Distribution')
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        
        # Mass distribution colored by model score
        scatter = ax4.scatter(df_full['bMesonMass'], df_full['bmeson_mass_window'], 
                             c=y_proba, cmap='viridis', alpha=0.6, s=1)
        ax4.set_xlabel('B Meson Mass [GeV/c²]')
        ax4.set_ylabel('Mass Window [GeV/c²]')
        ax4.set_title('Mass vs Model Score')
        plt.colorbar(scatter, ax=ax4, label='Model Score')
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'mass_window_analysis.png')
        plt.close()
    
    def generate_bmeson_type_analysis(self, X: pd.DataFrame, y: pd.Series,
                                     df_full: pd.DataFrame, save_plots: bool = True) -> Dict[str, Any]:
        """Analyze model performance by B meson type"""
        logger.info("Generating B meson type analysis")
        
        y_proba = self.model.predict_proba(X)[:, 1]
        
        results = {}
        
        for btype in [0, 1, 2]:  # B+, B0, Bc
            type_name = {0: 'B+', 1: 'B0', 2: 'Bc'}[btype]
            mask = df_full['bMesonType'] == btype
            
            if np.sum(mask) < 50:  # Skip types with too few events
                continue
            
            y_type = y[mask]
            y_proba_type = y_proba[mask]
            
            if len(np.unique(y_type)) < 2:
                continue
            
            # Calculate metrics
            from sklearn.metrics import roc_auc_score, average_precision_score
            auc_score = roc_auc_score(y_type, y_proba_type)
            ap_score = average_precision_score(y_type, y_proba_type)
            
            results[type_name] = {
                'n_total': np.sum(mask),
                'n_signal': np.sum(y_type == 1),
                'n_background': np.sum(y_type == 0),
                'signal_fraction': np.mean(y_type),
                'auc_score': auc_score,
                'ap_score': ap_score,
                'mean_score_signal': np.mean(y_proba_type[y_type == 1]),
                'mean_score_background': np.mean(y_proba_type[y_type == 0])
            }
        
        if save_plots:
            self._plot_bmeson_type_analysis(results, X, y, df_full, y_proba)
        
        return results
    
    def _plot_bmeson_type_analysis(self, results: Dict[str, Any], 
                                  X: pd.DataFrame, y: pd.Series,
                                  df_full: pd.DataFrame, y_proba: np.ndarray) -> None:
        """Plot B meson type analysis"""
        fig, axes = plt.subplots(2, 2, figsize=(15, 12))
        
        # Performance by type
        types = list(results.keys())
        auc_scores = [results[t]['auc_score'] for t in types]
        ap_scores = [results[t]['ap_score'] for t in types]
        
        x_pos = np.arange(len(types))
        width = 0.35
        
        axes[0, 0].bar(x_pos - width/2, auc_scores, width, label='ROC AUC', alpha=0.8)
        axes[0, 0].bar(x_pos + width/2, ap_scores, width, label='Average Precision', alpha=0.8)
        axes[0, 0].set_xlabel('B Meson Type')
        axes[0, 0].set_ylabel('Score')
        axes[0, 0].set_title('Model Performance by B Meson Type')
        axes[0, 0].set_xticks(x_pos)
        axes[0, 0].set_xticklabels(types)
        axes[0, 0].legend()
        axes[0, 0].grid(True, alpha=0.3)
        
        # Signal fraction by type
        signal_fractions = [results[t]['signal_fraction'] for t in types]
        axes[0, 1].bar(types, signal_fractions, alpha=0.8, color='orange')
        axes[0, 1].set_xlabel('B Meson Type')
        axes[0, 1].set_ylabel('Signal Fraction')
        axes[0, 1].set_title('Signal Fraction by B Meson Type')
        axes[0, 1].grid(True, alpha=0.3)
        
        # Score distributions by type
        colors = ['blue', 'green', 'red']
        for i, btype in enumerate([0, 1, 2]):
            type_name = {0: 'B+', 1: 'B0', 2: 'Bc'}[btype]
            if type_name not in results:
                continue
            
            mask = df_full['bMesonType'] == btype
            scores = y_proba[mask]
            
            axes[1, 0].hist(scores, bins=30, alpha=0.6, label=type_name, 
                           color=colors[i], density=True)
        
        axes[1, 0].set_xlabel('Model Score')
        axes[1, 0].set_ylabel('Density')
        axes[1, 0].set_title('Model Score Distribution by B Meson Type')
        axes[1, 0].legend()
        axes[1, 0].grid(True, alpha=0.3)
        
        # ROC curves by type
        from sklearn.metrics import roc_curve
        for i, btype in enumerate([0, 1, 2]):
            type_name = {0: 'B+', 1: 'B0', 2: 'Bc'}[btype]
            if type_name not in results:
                continue
            
            mask = df_full['bMesonType'] == btype
            y_type = y[mask]
            y_proba_type = y_proba[mask]
            
            if len(np.unique(y_type)) < 2:
                continue
            
            fpr, tpr, _ = roc_curve(y_type, y_proba_type)
            auc_score = results[type_name]['auc_score']
            
            axes[1, 1].plot(fpr, tpr, label=f'{type_name} (AUC = {auc_score:.3f})', 
                           color=colors[i], linewidth=2)
        
        axes[1, 1].plot([0, 1], [0, 1], 'k--', alpha=0.5)
        axes[1, 1].set_xlabel('False Positive Rate')
        axes[1, 1].set_ylabel('True Positive Rate')
        axes[1, 1].set_title('ROC Curves by B Meson Type')
        axes[1, 1].legend()
        axes[1, 1].grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'bmeson_type_analysis.png')
        plt.close()
    
    def generate_working_point_analysis(self, X: pd.DataFrame, y: pd.Series,
                                       save_plots: bool = True) -> Dict[str, Any]:
        """Generate working point analysis for different signal efficiencies"""
        logger.info("Generating working point analysis")
        
        y_proba = self.model.predict_proba(X)[:, 1]
        
        # Define target signal efficiencies
        target_efficiencies = [0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.60, 0.50]
        
        working_points = []
        
        for target_eff in target_efficiencies:
            # Find threshold for target signal efficiency
            signal_scores = y_proba[y == 1]
            threshold = np.percentile(signal_scores, (1 - target_eff) * 100)
            
            # Calculate metrics at this threshold
            y_pred = (y_proba >= threshold).astype(int)
            
            # True positives, false positives, etc.
            tp = np.sum((y_pred == 1) & (y == 1))
            fp = np.sum((y_pred == 1) & (y == 0))
            tn = np.sum((y_pred == 0) & (y == 0))
            fn = np.sum((y_pred == 0) & (y == 1))
            
            signal_efficiency = tp / (tp + fn) if (tp + fn) > 0 else 0
            background_rejection = tn / (tn + fp) if (tn + fp) > 0 else 0
            precision = tp / (tp + fp) if (tp + fp) > 0 else 0
            
            working_points.append({
                'target_efficiency': target_eff,
                'threshold': threshold,
                'actual_signal_efficiency': signal_efficiency,
                'background_rejection': background_rejection,
                'precision': precision,
                'tp': int(tp), 'fp': int(fp), 'tn': int(tn), 'fn': int(fn)
            })
        
        results = {'working_points': working_points}
        
        if save_plots:
            self._plot_working_points(working_points)
        
        return results
    
    def _plot_working_points(self, working_points: List[Dict[str, Any]]) -> None:
        """Plot working point analysis"""
        df_wp = pd.DataFrame(working_points)
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
        
        # Signal efficiency vs background rejection
        ax1.plot(df_wp['actual_signal_efficiency'], df_wp['background_rejection'], 'bo-', markersize=8)
        for i, row in df_wp.iterrows():
            ax1.annotate(f'{row["target_efficiency"]:.0%}', 
                        (row['actual_signal_efficiency'], row['background_rejection']),
                        xytext=(5, 5), textcoords='offset points', fontsize=9)
        
        ax1.set_xlabel('Signal Efficiency')
        ax1.set_ylabel('Background Rejection')
        ax1.set_title('Signal Efficiency vs Background Rejection')
        ax1.grid(True, alpha=0.3)
        
        # Threshold vs efficiency
        ax2.plot(df_wp['threshold'], df_wp['actual_signal_efficiency'], 'go-', label='Signal Efficiency')
        ax2.plot(df_wp['threshold'], df_wp['background_rejection'], 'ro-', label='Background Rejection')
        ax2.set_xlabel('Classification Threshold')
        ax2.set_ylabel('Efficiency/Rejection')
        ax2.set_title('Efficiency/Rejection vs Threshold')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # Precision vs signal efficiency
        ax3.plot(df_wp['actual_signal_efficiency'], df_wp['precision'], 'mo-', markersize=8)
        ax3.set_xlabel('Signal Efficiency (Recall)')
        ax3.set_ylabel('Precision')
        ax3.set_title('Precision vs Recall')
        ax3.grid(True, alpha=0.3)
        
        # Working points table
        ax4.axis('tight')
        ax4.axis('off')
        
        table_data = []
        for wp in working_points:
            table_data.append([
                f"{wp['target_efficiency']:.0%}",
                f"{wp['threshold']:.3f}",
                f"{wp['actual_signal_efficiency']:.3f}",
                f"{wp['background_rejection']:.3f}",
                f"{wp['precision']:.3f}"
            ])
        
        table = ax4.table(cellText=table_data,
                         colLabels=['Target Eff.', 'Threshold', 'Actual Eff.', 'Bkg. Rej.', 'Precision'],
                         cellLoc='center',
                         loc='center')
        table.auto_set_font_size(False)
        table.set_fontsize(10)
        table.scale(1.2, 1.5)
        ax4.set_title('Working Points Summary')
        
        plt.tight_layout()
        plt.savefig(self.model_dir / 'working_points_analysis.png')
        plt.close()
    
    def generate_evaluation_report(self, data_file: Optional[str] = None) -> Dict[str, Any]:
        """Generate comprehensive evaluation report"""
        logger.info("Generating comprehensive evaluation report")
        
        # Load test data
        X, y, df_full = self.load_test_data(data_file)
        
        # Generate all analyses
        evaluation_results = {}
        
        # Basic performance metrics
        evaluation_results['roc_analysis'] = self.generate_roc_analysis(X, y)
        evaluation_results['pr_analysis'] = self.generate_precision_recall_analysis(X, y)
        
        # Feature analysis
        evaluation_results['feature_importance'] = self.generate_feature_importance_analysis(X, y)
        
        # Physics-specific analyses
        evaluation_results['mass_window_analysis'] = self.generate_mass_window_analysis(X, y, df_full)
        evaluation_results['bmeson_type_analysis'] = self.generate_bmeson_type_analysis(X, y, df_full)
        
        # Working points
        evaluation_results['working_points'] = self.generate_working_point_analysis(X, y)
        
        # Add model info
        evaluation_results['model_info'] = {
            'n_features': len(X.columns),
            'feature_names': X.columns.tolist(),
            'n_test_samples': len(X),
            'n_signal': int(np.sum(y == 1)),
            'n_background': int(np.sum(y == 0)),
            'signal_fraction': float(np.mean(y))
        }
        
        # Save comprehensive results
        output_path = self.model_dir / 'evaluation_report.json'
        with open(output_path, 'w') as f:
            json.dump(self._make_json_serializable(evaluation_results), f, indent=2)
        
        logger.info(f"Evaluation report saved to {output_path}")
        logger.info("All evaluation plots saved to model directory")
        
        return evaluation_results
    
    def _make_json_serializable(self, obj):
        """Convert numpy arrays and other objects to JSON-serializable format"""
        if isinstance(obj, dict):
            return {key: self._make_json_serializable(value) for key, value in obj.items()}
        elif isinstance(obj, list):
            return [self._make_json_serializable(item) for item in obj]
        elif isinstance(obj, np.ndarray):
            return obj.tolist()
        elif isinstance(obj, (np.integer, np.floating)):
            return obj.item()
        elif isinstance(obj, pd.DataFrame):
            return obj.to_dict('records')
        else:
            return obj


def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Evaluate B meson XGBoost model")
    parser.add_argument('--model-dir', required=True, help='Directory containing trained model')
    parser.add_argument('--data', help='Path to test data file')
    parser.add_argument('--output-dir', help='Output directory for evaluation results')
    
    args = parser.parse_args()
    
    # Initialize evaluator
    evaluator = BMesonModelEvaluator(model_dir=args.model_dir, data_file=args.data)
    
    # Generate evaluation report
    results = evaluator.generate_evaluation_report(args.data)
    
    # Print summary
    roc_auc = results['roc_analysis']['roc_auc']
    avg_precision = results['pr_analysis']['avg_precision']
    
    print(f"\n=== Evaluation Summary ===")
    print(f"ROC AUC: {roc_auc:.4f}")
    print(f"Average Precision: {avg_precision:.4f}")
    print(f"Test samples: {results['model_info']['n_test_samples']:,}")
    print(f"Signal candidates: {results['model_info']['n_signal']:,}")
    print(f"Background candidates: {results['model_info']['n_background']:,}")
    print(f"Evaluation results saved to: {args.model_dir}")


if __name__ == "__main__":
    main()