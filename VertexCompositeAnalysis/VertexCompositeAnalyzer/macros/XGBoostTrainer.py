#!/usr/bin/env python3
"""
XGBoostTrainer.py
Main XGBoost training framework for B meson signal vs background discrimination

Features:
- Physics-aware data splitting and cross-validation
- Hyperparameter optimization with Optuna
- Signal/background class balancing
- Feature selection and importance analysis
- Model interpretability with SHAP
- Early stopping and regularization

Usage:
    from XGBoostTrainer import BMesonXGBoostTrainer
    trainer = BMesonXGBoostTrainer(config_path='TrainingConfig.yaml')
    trainer.train_model()
"""

import argparse
import yaml
import json
import pickle
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
import warnings

import numpy as np
import pandas as pd
import pyarrow.parquet as pq
from sklearn.model_selection import train_test_split, StratifiedKFold, cross_val_score
from sklearn.preprocessing import StandardScaler, RobustScaler
from sklearn.metrics import (
    roc_auc_score, roc_curve, precision_recall_curve, 
    classification_report, confusion_matrix
)
from sklearn.feature_selection import SelectKBest, f_classif, mutual_info_classif
from imblearn.over_sampling import SMOTE
from imblearn.under_sampling import RandomUnderSampler

import xgboost as xgb
import optuna
from optuna.samplers import TPESampler
from optuna.pruners import MedianPruner

import matplotlib.pyplot as plt
import seaborn as sns
import shap

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Suppress warnings
warnings.filterwarnings("ignore", category=FutureWarning)
optuna.logging.set_verbosity(optuna.logging.WARNING)

class BMesonXGBoostTrainer:
    """XGBoost trainer for B meson signal vs background discrimination"""
    
    def __init__(self, config_path: Optional[str] = None):
        self.config = self._load_config(config_path)
        self.model = None
        self.scaler = None
        self.feature_selector = None
        self.study = None
        self.results = {}
        
        # Set up output directory
        self.output_dir = Path(self.config['output']['model_dir'])
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Configure matplotlib for non-interactive use
        plt.switch_backend('Agg')
        
    def _load_config(self, config_path: Optional[str]) -> Dict[str, Any]:
        """Load training configuration"""
        if config_path is None:
            # Use default configuration
            return self._get_default_config()
        
        with open(config_path, 'r') as f:
            config = yaml.safe_load(f)
        
        logger.info(f"Loaded configuration from {config_path}")
        return config
    
    def _get_default_config(self) -> Dict[str, Any]:
        """Get default training configuration"""
        return {
            'data': {
                'input_file': 'bmeson_data_with_features.parquet',
                'target_column': 'signal_label',
                'bmeson_type_column': 'bMesonType',
                'exclude_columns': ['runNb', 'eventNb', 'lumiSection'],
                'test_size': 0.2,
                'val_size': 0.2,
                'random_state': 42
            },
            'features': {
                'feature_selection': True,
                'n_features': 50,
                'selection_method': 'mutual_info',  # 'f_score', 'mutual_info'
                'scaling': True,
                'scaler_type': 'robust'  # 'standard', 'robust'
            },
            'sampling': {
                'balance_classes': True,
                'sampling_strategy': 'smote',  # 'undersample', 'smote', 'none'
                'smote_k_neighbors': 5
            },
            'model': {
                'objective': 'binary:logistic',
                'eval_metric': 'auc',
                'use_gpu': False,
                'early_stopping_rounds': 100,
                'n_estimators': 1000
            },
            'hyperparameter_optimization': {
                'enabled': True,
                'n_trials': 100,
                'timeout': 3600,  # 1 hour
                'cv_folds': 5,
                'scoring': 'roc_auc'
            },
            'cross_validation': {
                'enabled': True,
                'cv_folds': 5,
                'stratified': True
            },
            'output': {
                'model_dir': 'models/bmeson_xgboost',
                'save_model': True,
                'save_features': True,
                'save_scaler': True,
                'save_plots': True
            }
        }
    
    def load_data(self) -> pd.DataFrame:
        """Load and prepare training data"""
        logger.info(f"Loading data from {self.config['data']['input_file']}")
        
        # Load data
        table = pq.read_table(self.config['data']['input_file'])
        df = table.to_pandas()
        
        logger.info(f"Loaded {len(df)} samples with {len(df.columns)} features")
        
        # Filter valid labels (signal=1, background=0, exclude=-1)
        target_col = self.config['data']['target_column']
        valid_mask = df[target_col].isin([0, 1])
        df_clean = df[valid_mask].copy()
        
        logger.info(f"After filtering: {len(df_clean)} samples")
        logger.info(f"Signal samples: {sum(df_clean[target_col] == 1)}")
        logger.info(f"Background samples: {sum(df_clean[target_col] == 0)}")
        
        # Show B meson type distribution
        if self.config['data']['bmeson_type_column'] in df_clean.columns:
            type_dist = df_clean[self.config['data']['bmeson_type_column']].value_counts().sort_index()
            logger.info("B meson type distribution:")
            for btype, count in type_dist.items():
                type_name = {0: 'B+', 1: 'B0', 2: 'Bc'}.get(btype, f'Type{btype}')
                logger.info(f"  {type_name}: {count:,} candidates")
        
        return df_clean
    
    def prepare_features(self, df: pd.DataFrame) -> Tuple[pd.DataFrame, pd.Series]:
        """Prepare features and target for training"""
        logger.info("Preparing features and target")
        
        # Separate features and target
        target_col = self.config['data']['target_column']
        exclude_cols = self.config['data']['exclude_columns'] + [target_col]
        
        # Remove excluded columns
        feature_cols = [col for col in df.columns if col not in exclude_cols]
        X = df[feature_cols].copy()
        y = df[target_col].copy()
        
        logger.info(f"Selected {len(feature_cols)} features for training")
        
        # Handle missing values
        X = X.fillna(-999)
        
        # Replace infinite values
        X = X.replace([np.inf, -np.inf], [999, -999])
        
        return X, y
    
    def split_data(self, X: pd.DataFrame, y: pd.Series) -> Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame, pd.Series, pd.Series, pd.Series]:
        """Split data into train/validation/test sets"""
        logger.info("Splitting data")
        
        test_size = self.config['data']['test_size']
        val_size = self.config['data']['val_size']
        random_state = self.config['data']['random_state']
        
        # First split: train+val vs test
        X_temp, X_test, y_temp, y_test = train_test_split(
            X, y, test_size=test_size, stratify=y, 
            random_state=random_state
        )
        
        # Second split: train vs val
        val_size_adjusted = val_size / (1 - test_size)
        X_train, X_val, y_train, y_val = train_test_split(
            X_temp, y_temp, test_size=val_size_adjusted, stratify=y_temp,
            random_state=random_state
        )
        
        logger.info(f"Train set: {len(X_train)} samples")
        logger.info(f"Validation set: {len(X_val)} samples") 
        logger.info(f"Test set: {len(X_test)} samples")
        
        return X_train, X_val, X_test, y_train, y_val, y_test
    
    def apply_feature_selection(self, X_train: pd.DataFrame, y_train: pd.Series,
                               X_val: pd.DataFrame, X_test: pd.DataFrame) -> Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
        """Apply feature selection"""
        if not self.config['features']['feature_selection']:
            return X_train, X_val, X_test
        
        logger.info("Applying feature selection")
        
        n_features = self.config['features']['n_features']
        method = self.config['features']['selection_method']
        
        if method == 'f_score':
            selector = SelectKBest(score_func=f_classif, k=n_features)
        elif method == 'mutual_info':
            selector = SelectKBest(score_func=mutual_info_classif, k=n_features)
        else:
            raise ValueError(f"Unknown feature selection method: {method}")
        
        # Fit selector on training data
        X_train_selected = selector.fit_transform(X_train, y_train)
        X_val_selected = selector.transform(X_val)
        X_test_selected = selector.transform(X_test)
        
        # Get selected feature names
        selected_features = X_train.columns[selector.get_support()].tolist()
        logger.info(f"Selected {len(selected_features)} features using {method}")
        
        # Convert back to DataFrames
        X_train = pd.DataFrame(X_train_selected, columns=selected_features, index=X_train.index)
        X_val = pd.DataFrame(X_val_selected, columns=selected_features, index=X_val.index)
        X_test = pd.DataFrame(X_test_selected, columns=selected_features, index=X_test.index)
        
        self.feature_selector = selector
        
        return X_train, X_val, X_test
    
    def apply_scaling(self, X_train: pd.DataFrame, X_val: pd.DataFrame, 
                     X_test: pd.DataFrame) -> Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
        """Apply feature scaling"""
        if not self.config['features']['scaling']:
            return X_train, X_val, X_test
        
        logger.info("Applying feature scaling")
        
        scaler_type = self.config['features']['scaler_type']
        
        if scaler_type == 'standard':
            scaler = StandardScaler()
        elif scaler_type == 'robust':
            scaler = RobustScaler()
        else:
            raise ValueError(f"Unknown scaler type: {scaler_type}")
        
        # Fit scaler on training data
        X_train_scaled = scaler.fit_transform(X_train)
        X_val_scaled = scaler.transform(X_val)
        X_test_scaled = scaler.transform(X_test)
        
        # Convert back to DataFrames
        X_train = pd.DataFrame(X_train_scaled, columns=X_train.columns, index=X_train.index)
        X_val = pd.DataFrame(X_val_scaled, columns=X_val.columns, index=X_val.index)
        X_test = pd.DataFrame(X_test_scaled, columns=X_test.columns, index=X_test.index)
        
        self.scaler = scaler
        
        return X_train, X_val, X_test
    
    def balance_classes(self, X_train: pd.DataFrame, y_train: pd.Series) -> Tuple[pd.DataFrame, pd.Series]:
        """Apply class balancing"""
        if not self.config['sampling']['balance_classes']:
            return X_train, y_train
        
        logger.info("Applying class balancing")
        
        strategy = self.config['sampling']['sampling_strategy']
        
        if strategy == 'smote':
            k_neighbors = self.config['sampling']['smote_k_neighbors']
            sampler = SMOTE(k_neighbors=k_neighbors, random_state=self.config['data']['random_state'])
        elif strategy == 'undersample':
            sampler = RandomUnderSampler(random_state=self.config['data']['random_state'])
        else:
            return X_train, y_train
        
        X_resampled, y_resampled = sampler.fit_resample(X_train, y_train)
        
        logger.info(f"Before balancing - Signal: {sum(y_train == 1)}, Background: {sum(y_train == 0)}")
        logger.info(f"After balancing - Signal: {sum(y_resampled == 1)}, Background: {sum(y_resampled == 0)}")
        
        # Convert back to DataFrame
        X_resampled = pd.DataFrame(X_resampled, columns=X_train.columns)
        y_resampled = pd.Series(y_resampled)
        
        return X_resampled, y_resampled
    
    def optimize_hyperparameters(self, X_train: pd.DataFrame, y_train: pd.Series) -> Dict[str, Any]:
        """Optimize hyperparameters using Optuna"""
        if not self.config['hyperparameter_optimization']['enabled']:
            return self._get_default_params()
        
        logger.info("Starting hyperparameter optimization")
        
        def objective(trial):
            # Suggest hyperparameters
            params = {
                'objective': self.config['model']['objective'],
                'eval_metric': self.config['model']['eval_metric'],
                'tree_method': 'gpu_hist' if self.config['model']['use_gpu'] else 'hist',
                'random_state': self.config['data']['random_state'],
                
                # Hyperparameters to optimize
                'n_estimators': trial.suggest_int('n_estimators', 100, 2000),
                'max_depth': trial.suggest_int('max_depth', 3, 12),
                'learning_rate': trial.suggest_float('learning_rate', 0.01, 0.3),
                'subsample': trial.suggest_float('subsample', 0.6, 1.0),
                'colsample_bytree': trial.suggest_float('colsample_bytree', 0.6, 1.0),
                'reg_alpha': trial.suggest_float('reg_alpha', 0, 10),
                'reg_lambda': trial.suggest_float('reg_lambda', 1, 10),
                'min_child_weight': trial.suggest_int('min_child_weight', 1, 10),
                'gamma': trial.suggest_float('gamma', 0, 5)
            }
            
            # Cross-validation
            cv_folds = self.config['hyperparameter_optimization']['cv_folds']
            scoring = self.config['hyperparameter_optimization']['scoring']
            
            cv = StratifiedKFold(n_splits=cv_folds, shuffle=True, random_state=self.config['data']['random_state'])
            
            # Use XGBoost with early stopping
            model = xgb.XGBClassifier(**params)
            scores = cross_val_score(model, X_train, y_train, cv=cv, scoring=scoring)
            
            return scores.mean()
        
        # Create study
        sampler = TPESampler(seed=self.config['data']['random_state'])
        pruner = MedianPruner(n_startup_trials=10, n_warmup_steps=5)
        
        self.study = optuna.create_study(
            direction='maximize',
            sampler=sampler,
            pruner=pruner
        )
        
        # Optimize
        n_trials = self.config['hyperparameter_optimization']['n_trials']
        timeout = self.config['hyperparameter_optimization']['timeout']
        
        self.study.optimize(objective, n_trials=n_trials, timeout=timeout)
        
        logger.info(f"Optimization completed. Best score: {self.study.best_value:.4f}")
        logger.info(f"Best parameters: {self.study.best_params}")
        
        # Prepare final parameters
        best_params = self.study.best_params.copy()
        best_params.update({
            'objective': self.config['model']['objective'],
            'eval_metric': self.config['model']['eval_metric'],
            'tree_method': 'gpu_hist' if self.config['model']['use_gpu'] else 'hist',
            'random_state': self.config['data']['random_state']
        })
        
        return best_params
    
    def _get_default_params(self) -> Dict[str, Any]:
        """Get default XGBoost parameters"""
        return {
            'objective': self.config['model']['objective'],
            'eval_metric': self.config['model']['eval_metric'],
            'tree_method': 'gpu_hist' if self.config['model']['use_gpu'] else 'hist',
            'random_state': self.config['data']['random_state'],
            'n_estimators': self.config['model']['n_estimators'],
            'max_depth': 6,
            'learning_rate': 0.1,
            'subsample': 0.8,
            'colsample_bytree': 0.8,
            'reg_alpha': 0,
            'reg_lambda': 1,
            'min_child_weight': 1,
            'gamma': 0
        }
    
    def train_final_model(self, X_train: pd.DataFrame, y_train: pd.Series,
                         X_val: pd.DataFrame, y_val: pd.Series,
                         params: Dict[str, Any]) -> xgb.XGBClassifier:
        """Train final model with best parameters"""
        logger.info("Training final model")
        
        # Create model
        model = xgb.XGBClassifier(**params)
        
        # Train with early stopping
        model.fit(
            X_train, y_train,
            eval_set=[(X_val, y_val)],
            early_stopping_rounds=self.config['model']['early_stopping_rounds'],
            verbose=False
        )
        
        logger.info(f"Model trained with {model.n_estimators} trees")
        
        return model
    
    def evaluate_model(self, model: xgb.XGBClassifier, 
                      X_val: pd.DataFrame, y_val: pd.Series,
                      X_test: pd.DataFrame, y_test: pd.Series) -> Dict[str, Any]:
        """Evaluate model performance"""
        logger.info("Evaluating model performance")
        
        results = {}
        
        # Predictions
        y_val_pred = model.predict(X_val)
        y_val_proba = model.predict_proba(X_val)[:, 1]
        y_test_pred = model.predict(X_test)
        y_test_proba = model.predict_proba(X_test)[:, 1]
        
        # AUC scores
        val_auc = roc_auc_score(y_val, y_val_proba)
        test_auc = roc_auc_score(y_test, y_test_proba)
        
        results['val_auc'] = val_auc
        results['test_auc'] = test_auc
        
        logger.info(f"Validation AUC: {val_auc:.4f}")
        logger.info(f"Test AUC: {test_auc:.4f}")
        
        # Classification reports
        results['val_classification_report'] = classification_report(y_val, y_val_pred, output_dict=True)
        results['test_classification_report'] = classification_report(y_test, y_test_pred, output_dict=True)
        
        # Confusion matrices
        results['val_confusion_matrix'] = confusion_matrix(y_val, y_val_pred)
        results['test_confusion_matrix'] = confusion_matrix(y_test, y_test_pred)
        
        # ROC curves
        val_fpr, val_tpr, val_thresholds = roc_curve(y_val, y_val_proba)
        test_fpr, test_tpr, test_thresholds = roc_curve(y_test, y_test_proba)
        
        results['val_roc_curve'] = {'fpr': val_fpr, 'tpr': val_tpr, 'thresholds': val_thresholds}
        results['test_roc_curve'] = {'fpr': test_fpr, 'tpr': test_tpr, 'thresholds': test_thresholds}
        
        # Precision-recall curves
        val_precision, val_recall, val_pr_thresholds = precision_recall_curve(y_val, y_val_proba)
        test_precision, test_recall, test_pr_thresholds = precision_recall_curve(y_test, y_test_proba)
        
        results['val_pr_curve'] = {'precision': val_precision, 'recall': val_recall, 'thresholds': val_pr_thresholds}
        results['test_pr_curve'] = {'precision': test_precision, 'recall': test_recall, 'thresholds': test_pr_thresholds}
        
        # Feature importance
        feature_importance = pd.DataFrame({
            'feature': X_val.columns,
            'importance': model.feature_importances_
        }).sort_values('importance', ascending=False)
        
        results['feature_importance'] = feature_importance
        
        return results
    
    def cross_validate(self, X_train: pd.DataFrame, y_train: pd.Series, 
                      params: Dict[str, Any]) -> Dict[str, float]:
        """Perform cross-validation"""
        if not self.config['cross_validation']['enabled']:
            return {}
        
        logger.info("Performing cross-validation")
        
        cv_folds = self.config['cross_validation']['cv_folds']
        
        if self.config['cross_validation']['stratified']:
            cv = StratifiedKFold(n_splits=cv_folds, shuffle=True, 
                               random_state=self.config['data']['random_state'])
        else:
            cv = cv_folds
        
        model = xgb.XGBClassifier(**params)
        
        # Different scoring metrics
        scoring_metrics = ['roc_auc', 'precision', 'recall', 'f1']
        cv_results = {}
        
        for metric in scoring_metrics:
            scores = cross_val_score(model, X_train, y_train, cv=cv, scoring=metric)
            cv_results[f'{metric}_mean'] = scores.mean()
            cv_results[f'{metric}_std'] = scores.std()
            logger.info(f"CV {metric}: {scores.mean():.4f} ± {scores.std():.4f}")
        
        return cv_results
    
    def save_model_artifacts(self, model: xgb.XGBClassifier, 
                           feature_names: List[str],
                           results: Dict[str, Any]) -> None:
        """Save model and related artifacts"""
        if not self.config['output']['save_model']:
            return
        
        logger.info(f"Saving model artifacts to {self.output_dir}")
        
        # Save model
        model_path = self.output_dir / 'xgboost_model.json'
        model.save_model(str(model_path))
        logger.info(f"Model saved to {model_path}")
        
        # Save model as pickle (includes sklearn wrapper)
        pickle_path = self.output_dir / 'xgboost_model.pkl'
        with open(pickle_path, 'wb') as f:
            pickle.dump(model, f)
        
        # Save feature names
        if self.config['output']['save_features']:
            features_path = self.output_dir / 'feature_names.json'
            with open(features_path, 'w') as f:
                json.dump(feature_names, f, indent=2)
        
        # Save scaler
        if self.scaler is not None and self.config['output']['save_scaler']:
            scaler_path = self.output_dir / 'scaler.pkl'
            with open(scaler_path, 'wb') as f:
                pickle.dump(self.scaler, f)
        
        # Save feature selector
        if self.feature_selector is not None:
            selector_path = self.output_dir / 'feature_selector.pkl'
            with open(selector_path, 'wb') as f:
                pickle.dump(self.feature_selector, f)
        
        # Save results
        results_path = self.output_dir / 'training_results.json'
        # Convert numpy arrays to lists for JSON serialization
        results_serializable = self._make_json_serializable(results)
        with open(results_path, 'w') as f:
            json.dump(results_serializable, f, indent=2)
        
        # Save configuration
        config_path = self.output_dir / 'training_config.yaml'
        with open(config_path, 'w') as f:
            yaml.dump(self.config, f, default_flow_style=False)
    
    def _make_json_serializable(self, obj):
        """Convert numpy arrays and other non-serializable objects to JSON-compatible format"""
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
    
    def train_model(self) -> None:
        """Main training pipeline"""
        logger.info("Starting B meson XGBoost training pipeline")
        
        # Load data
        df = self.load_data()
        
        # Prepare features
        X, y = self.prepare_features(df)
        
        # Split data
        X_train, X_val, X_test, y_train, y_val, y_test = self.split_data(X, y)
        
        # Feature selection
        X_train, X_val, X_test = self.apply_feature_selection(X_train, y_train, X_val, X_test)
        
        # Feature scaling
        X_train, X_val, X_test = self.apply_scaling(X_train, X_val, X_test)
        
        # Class balancing
        X_train, y_train = self.balance_classes(X_train, y_train)
        
        # Hyperparameter optimization
        best_params = self.optimize_hyperparameters(X_train, y_train)
        
        # Cross-validation
        cv_results = self.cross_validate(X_train, y_train, best_params)
        
        # Train final model
        self.model = self.train_final_model(X_train, y_train, X_val, y_val, best_params)
        
        # Evaluate model
        self.results = self.evaluate_model(self.model, X_val, y_val, X_test, y_test)
        self.results['cv_results'] = cv_results
        self.results['best_params'] = best_params
        
        # Save artifacts
        self.save_model_artifacts(self.model, X_train.columns.tolist(), self.results)
        
        logger.info("Training pipeline completed successfully!")
        logger.info(f"Final test AUC: {self.results['test_auc']:.4f}")


def main():
    parser = argparse.ArgumentParser(description="Train XGBoost model for B meson signal vs background")
    parser.add_argument('--config', type=str, help='Path to configuration file')
    parser.add_argument('--data', type=str, help='Path to input data file')
    parser.add_argument('--output', type=str, help='Output directory for model artifacts')
    
    args = parser.parse_args()
    
    # Initialize trainer
    trainer = BMesonXGBoostTrainer(config_path=args.config)
    
    # Override config with command line arguments
    if args.data:
        trainer.config['data']['input_file'] = args.data
    if args.output:
        trainer.config['output']['model_dir'] = args.output
    
    # Train model
    trainer.train_model()


if __name__ == "__main__":
    main()