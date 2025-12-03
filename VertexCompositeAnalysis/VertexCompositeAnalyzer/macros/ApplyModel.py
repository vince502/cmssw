#!/usr/bin/env python3
"""
ApplyModel.py
Apply trained XGBoost model to new B meson data for inference

Features:
- Batch prediction on new ROOT files or Parquet data
- Model application with configurable thresholds
- Enhanced ROOT output with BDT scores
- Performance monitoring and validation
- A/B testing framework for model comparison

Usage:
    python ApplyModel.py --model models/bmeson_xgboost --input new_data.root --output scored_data.root
    python ApplyModel.py --model models/v1 --input data.parquet --output scores.parquet --format parquet
"""

import argparse
import json
import pickle
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any, Union
import warnings

import numpy as np
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
import uproot
import xgboost as xgb
from sklearn.metrics import roc_auc_score, average_precision_score

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)
warnings.filterwarnings("ignore", category=FutureWarning)

class BMesonModelApplicator:
    """Apply trained XGBoost model to new B meson data"""
    
    def __init__(self, model_path: str):
        self.model_path = Path(model_path)
        
        # Load model artifacts
        self.model = self._load_model()
        self.feature_names = self._load_feature_names()
        self.scaler = self._load_scaler()
        self.feature_selector = self._load_feature_selector()
        self.training_config = self._load_training_config()
        
        logger.info(f"Loaded model from {model_path}")
        logger.info(f"Model expects {len(self.feature_names)} features")
        
    def _load_model(self) -> xgb.XGBClassifier:
        """Load trained XGBoost model"""
        model_file = self.model_path / 'xgboost_model.pkl'
        if not model_file.exists():
            raise FileNotFoundError(f"Model file not found: {model_file}")
        
        with open(model_file, 'rb') as f:
            model = pickle.load(f)
        
        return model
        
    def _load_feature_names(self) -> List[str]:
        """Load feature names"""
        features_file = self.model_path / 'feature_names.json'
        if not features_file.exists():
            logger.warning("Feature names file not found, will use all available features")
            return None
        
        with open(features_file, 'r') as f:
            features = json.load(f)
        
        return features
    
    def _load_scaler(self):
        """Load feature scaler if exists"""
        scaler_file = self.model_path / 'scaler.pkl'
        if not scaler_file.exists():
            return None
        
        with open(scaler_file, 'rb') as f:
            scaler = pickle.load(f)
        
        return scaler
    
    def _load_feature_selector(self):
        """Load feature selector if exists"""
        selector_file = self.model_path / 'feature_selector.pkl'
        if not selector_file.exists():
            return None
        
        with open(selector_file, 'rb') as f:
            selector = pickle.load(f)
        
        return selector
    
    def _load_training_config(self) -> Dict[str, Any]:
        """Load training configuration"""
        config_file = self.model_path / 'training_config.yaml'
        if not config_file.exists():
            return {}
        
        import yaml
        with open(config_file, 'r') as f:
            config = yaml.safe_load(f)
        
        return config
    
    def apply_to_root_file(self, input_file: str, output_file: str, 
                          tree_name: str = "bDiMuMuFlatTree",
                          score_column: str = "bdt_score",
                          pred_column: str = "bdt_prediction",
                          threshold: float = 0.5,
                          chunk_size: int = 100000) -> Dict[str, Any]:
        """Apply model to ROOT file and create enhanced output"""
        logger.info(f"Applying model to ROOT file: {input_file}")
        
        # Read ROOT file
        with uproot.open(input_file) as file:
            if tree_name not in file:
                raise KeyError(f"Tree '{tree_name}' not found in {input_file}")
            
            tree = file[tree_name]
            total_entries = tree.num_entries
            
            if total_entries == 0:
                logger.warning(f"Empty tree in {input_file}")
                return {'n_processed': 0}
        
        logger.info(f"Processing {total_entries} entries in chunks of {chunk_size}")
        
        # Process in chunks and write to new ROOT file
        import ROOT
        ROOT.gROOT.SetBatch(True)
        
        # Create output file and tree
        output_root_file = ROOT.TFile(output_file, "RECREATE")
        output_tree = ROOT.TTree("bDiMuMuFlatTree", "Enhanced B meson flat tree with BDT scores")
        
        # Variables to store in output tree
        tree_vars = {}
        
        total_processed = 0
        predictions_summary = {'scores': [], 'predictions': []}
        
        for chunk_start in range(0, total_entries, chunk_size):
            chunk_end = min(chunk_start + chunk_size, total_entries)
            
            logger.info(f"Processing chunk {chunk_start}:{chunk_end}")
            
            # Read chunk
            with uproot.open(input_file) as file:
                tree = file[tree_name]
                df_chunk = tree.arrays(entry_start=chunk_start, entry_stop=chunk_end, library="pd")
            
            # Apply model
            scores, predictions = self._apply_model_to_dataframe(df_chunk, threshold)
            
            # Add scores to dataframe
            df_chunk[score_column] = scores
            df_chunk[pred_column] = predictions
            
            # Store predictions for summary
            predictions_summary['scores'].extend(scores.tolist())
            predictions_summary['predictions'].extend(predictions.tolist())
            
            # Write chunk to ROOT file
            if chunk_start == 0:
                # Setup tree structure on first chunk
                tree_vars = self._setup_root_tree_variables(output_tree, df_chunk)
            
            # Fill tree
            for idx, row in df_chunk.iterrows():
                for var_name, var_ref in tree_vars.items():
                    if var_name in row.index:
                        var_ref[0] = float(row[var_name]) if pd.notna(row[var_name]) else -999.0
                    else:
                        var_ref[0] = -999.0
                
                output_tree.Fill()
            
            total_processed += len(df_chunk)
        
        # Write and close output file
        output_tree.Write()
        output_root_file.Close()
        
        # Calculate summary statistics
        scores = np.array(predictions_summary['scores'])
        predictions = np.array(predictions_summary['predictions'])
        
        results = {
            'n_processed': total_processed,
            'n_predicted_signal': int(np.sum(predictions == 1)),
            'n_predicted_background': int(np.sum(predictions == 0)),
            'signal_fraction': float(np.mean(predictions)),
            'mean_score': float(np.mean(scores)),
            'score_std': float(np.std(scores)),
            'score_min': float(np.min(scores)),
            'score_max': float(np.max(scores)),
            'threshold_used': threshold
        }
        
        logger.info(f"Model application completed:")
        logger.info(f"  Processed: {total_processed:,} candidates")
        logger.info(f"  Predicted signal: {results['n_predicted_signal']:,} ({results['signal_fraction']:.3f})")
        logger.info(f"  Mean BDT score: {results['mean_score']:.4f} ± {results['score_std']:.4f}")
        logger.info(f"  Output saved to: {output_file}")
        
        return results
    
    def _setup_root_tree_variables(self, tree: 'ROOT.TTree', df: pd.DataFrame) -> Dict[str, Any]:
        """Setup ROOT tree variables"""
        import ROOT
        
        tree_vars = {}
        
        for column in df.columns:
            # Create appropriate ROOT variable type
            if df[column].dtype in ['float32', 'float64']:
                var_ref = ROOT.std.array('d', [0.0])  # double
                tree.Branch(column, var_ref, f"{column}/D")
            elif df[column].dtype in ['int8', 'int16', 'int32']:
                var_ref = ROOT.std.array('i', [0])  # int
                tree.Branch(column, var_ref, f"{column}/I")
            elif df[column].dtype in ['uint8', 'uint16', 'uint32']:
                var_ref = ROOT.std.array('i', [0])  # unsigned int
                tree.Branch(column, var_ref, f"{column}/i")
            elif df[column].dtype == 'bool':
                var_ref = ROOT.std.array('i', [0])  # bool as int
                tree.Branch(column, var_ref, f"{column}/O")
            else:
                # Default to double
                var_ref = ROOT.std.array('d', [0.0])
                tree.Branch(column, var_ref, f"{column}/D")
            
            tree_vars[column] = var_ref
        
        return tree_vars
    
    def apply_to_parquet_file(self, input_file: str, output_file: str,
                             score_column: str = "bdt_score",
                             pred_column: str = "bdt_prediction", 
                             threshold: float = 0.5,
                             chunk_size: Optional[int] = None) -> Dict[str, Any]:
        """Apply model to Parquet file"""
        logger.info(f"Applying model to Parquet file: {input_file}")
        
        # Read Parquet file
        table = pq.read_table(input_file)
        df = table.to_pandas()
        
        logger.info(f"Loaded {len(df)} candidates")
        
        # Apply model
        scores, predictions = self._apply_model_to_dataframe(df, threshold)
        
        # Add scores to dataframe
        df[score_column] = scores
        df[pred_column] = predictions
        
        # Save enhanced dataframe
        df.to_parquet(output_file, compression='snappy')
        
        # Calculate summary statistics
        results = {
            'n_processed': len(df),
            'n_predicted_signal': int(np.sum(predictions == 1)),
            'n_predicted_background': int(np.sum(predictions == 0)),
            'signal_fraction': float(np.mean(predictions)),
            'mean_score': float(np.mean(scores)),
            'score_std': float(np.std(scores)),
            'score_min': float(np.min(scores)),
            'score_max': float(np.max(scores)),
            'threshold_used': threshold
        }
        
        logger.info(f"Model application completed:")
        logger.info(f"  Processed: {len(df):,} candidates")
        logger.info(f"  Predicted signal: {results['n_predicted_signal']:,} ({results['signal_fraction']:.3f})")
        logger.info(f"  Mean BDT score: {results['mean_score']:.4f} ± {results['score_std']:.4f}")
        logger.info(f"  Output saved to: {output_file}")
        
        return results
    
    def _apply_model_to_dataframe(self, df: pd.DataFrame, threshold: float = 0.5) -> Tuple[np.ndarray, np.ndarray]:
        """Apply model to DataFrame and return scores and predictions"""
        
        # Prepare features
        X = self._prepare_features(df)
        
        # Get predictions
        scores = self.model.predict_proba(X)[:, 1]  # Probability of signal class
        predictions = (scores >= threshold).astype(int)
        
        return scores, predictions
    
    def _prepare_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Prepare features for model prediction"""
        
        # Select features used in training
        if self.feature_names:
            available_features = [col for col in self.feature_names if col in df.columns]
            missing_features = [col for col in self.feature_names if col not in df.columns]
            
            if missing_features:
                logger.warning(f"Missing features: {missing_features}")
                # Create missing features with default values
                for feature in missing_features:
                    df[feature] = -999.0
            
            X = df[self.feature_names].copy()
        else:
            # Use all available features except excluded ones
            exclude_cols = ['runNb', 'eventNb', 'lumiSection', 'signal_label', 'bdt_score', 'bdt_prediction']
            feature_cols = [col for col in df.columns if col not in exclude_cols]
            X = df[feature_cols].copy()
        
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
    
    def validate_on_labeled_data(self, data_file: str, 
                                label_column: str = "signal_label",
                                file_format: str = "auto") -> Dict[str, Any]:
        """Validate model performance on labeled data"""
        logger.info(f"Validating model on labeled data: {data_file}")
        
        # Load data
        if file_format == "auto":
            file_format = "parquet" if data_file.endswith('.parquet') else "root"
        
        if file_format == "parquet":
            table = pq.read_table(data_file)
            df = table.to_pandas()
        else:
            # ROOT file
            with uproot.open(data_file) as file:
                tree = file["bDiMuMuFlatTree"]
                df = tree.arrays(library="pd")
        
        # Filter valid labels
        if label_column in df.columns:
            valid_mask = df[label_column].isin([0, 1])
            df_valid = df[valid_mask].copy()
            y_true = df_valid[label_column].values
        else:
            logger.warning(f"Label column '{label_column}' not found. Cannot validate.")
            return {}
        
        logger.info(f"Validating on {len(df_valid)} labeled samples")
        
        # Apply model
        scores, predictions = self._apply_model_to_dataframe(df_valid)
        
        # Calculate performance metrics
        try:
            roc_auc = roc_auc_score(y_true, scores)
            avg_precision = average_precision_score(y_true, scores)
        except Exception as e:
            logger.error(f"Error calculating metrics: {e}")
            roc_auc = avg_precision = 0.0
        
        # Classification metrics at different thresholds
        thresholds = [0.1, 0.3, 0.5, 0.7, 0.9]
        threshold_metrics = []
        
        for thresh in thresholds:
            preds = (scores >= thresh).astype(int)
            
            tp = np.sum((preds == 1) & (y_true == 1))
            fp = np.sum((preds == 1) & (y_true == 0))
            tn = np.sum((preds == 0) & (y_true == 0))
            fn = np.sum((preds == 0) & (y_true == 1))
            
            precision = tp / (tp + fp) if (tp + fp) > 0 else 0
            recall = tp / (tp + fn) if (tp + fn) > 0 else 0
            f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0
            
            threshold_metrics.append({
                'threshold': thresh,
                'precision': precision,
                'recall': recall,
                'f1': f1
            })
        
        validation_results = {
            'n_samples': len(df_valid),
            'n_signal': int(np.sum(y_true == 1)),
            'n_background': int(np.sum(y_true == 0)),
            'roc_auc': float(roc_auc),
            'average_precision': float(avg_precision),
            'threshold_metrics': threshold_metrics,
            'score_distribution': {
                'signal_mean': float(np.mean(scores[y_true == 1])),
                'signal_std': float(np.std(scores[y_true == 1])),
                'background_mean': float(np.mean(scores[y_true == 0])),
                'background_std': float(np.std(scores[y_true == 0]))
            }
        }
        
        logger.info(f"Validation results:")
        logger.info(f"  ROC AUC: {roc_auc:.4f}")
        logger.info(f"  Average Precision: {avg_precision:.4f}")
        logger.info(f"  Signal score: {validation_results['score_distribution']['signal_mean']:.3f} ± {validation_results['score_distribution']['signal_std']:.3f}")
        logger.info(f"  Background score: {validation_results['score_distribution']['background_mean']:.3f} ± {validation_results['score_distribution']['background_std']:.3f}")
        
        return validation_results
    
    def batch_apply(self, input_pattern: str, output_dir: str,
                   file_format: str = "auto", threshold: float = 0.5,
                   chunk_size: int = 100000) -> Dict[str, Any]:
        """Apply model to multiple files matching a pattern"""
        logger.info(f"Batch applying model to files matching: {input_pattern}")
        
        import glob
        input_files = sorted(glob.glob(input_pattern))
        
        if not input_files:
            raise FileNotFoundError(f"No files found matching pattern: {input_pattern}")
        
        logger.info(f"Found {len(input_files)} files to process")
        
        # Create output directory
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        batch_results = []
        total_processed = 0
        
        for input_file in input_files:
            input_path = Path(input_file)
            output_file = output_path / f"scored_{input_path.name}"
            
            logger.info(f"Processing: {input_file} -> {output_file}")
            
            try:
                if file_format == "auto":
                    format_to_use = "parquet" if input_file.endswith('.parquet') else "root"
                else:
                    format_to_use = file_format
                
                if format_to_use == "parquet":
                    results = self.apply_to_parquet_file(input_file, str(output_file), 
                                                       threshold=threshold)
                else:
                    results = self.apply_to_root_file(input_file, str(output_file),
                                                    threshold=threshold, chunk_size=chunk_size)
                
                results['input_file'] = input_file
                results['output_file'] = str(output_file)
                batch_results.append(results)
                total_processed += results['n_processed']
                
            except Exception as e:
                logger.error(f"Error processing {input_file}: {e}")
                continue
        
        # Summary
        total_signal = sum(r['n_predicted_signal'] for r in batch_results)
        total_background = sum(r['n_predicted_background'] for r in batch_results)
        
        batch_summary = {
            'n_files_processed': len(batch_results),
            'total_candidates': total_processed,
            'total_predicted_signal': total_signal,
            'total_predicted_background': total_background,
            'overall_signal_fraction': total_signal / total_processed if total_processed > 0 else 0,
            'file_results': batch_results
        }
        
        logger.info(f"Batch processing completed:")
        logger.info(f"  Files processed: {len(batch_results)}")
        logger.info(f"  Total candidates: {total_processed:,}")
        logger.info(f"  Total predicted signal: {total_signal:,}")
        logger.info(f"  Overall signal fraction: {batch_summary['overall_signal_fraction']:.3f}")
        
        return batch_summary
    
    def compare_models(self, other_model_path: str, test_data: str,
                      label_column: str = "signal_label") -> Dict[str, Any]:
        """Compare this model with another model on test data"""
        logger.info(f"Comparing models: {self.model_path} vs {other_model_path}")
        
        # Load other model
        other_applicator = BMesonModelApplicator(other_model_path)
        
        # Load test data
        table = pq.read_table(test_data)
        df = table.to_pandas()
        
        # Filter valid labels
        valid_mask = df[label_column].isin([0, 1])
        df_valid = df[valid_mask].copy()
        y_true = df_valid[label_column].values
        
        # Apply both models
        scores1, _ = self._apply_model_to_dataframe(df_valid)
        scores2, _ = other_applicator._apply_model_to_dataframe(df_valid)
        
        # Calculate performance metrics for both models
        roc_auc1 = roc_auc_score(y_true, scores1)
        roc_auc2 = roc_auc_score(y_true, scores2)
        
        ap1 = average_precision_score(y_true, scores1)
        ap2 = average_precision_score(y_true, scores2)
        
        # Statistical significance test (DeLong test would be ideal, but using simple t-test)
        from scipy import stats
        score_diff = scores1 - scores2
        t_stat, p_value = stats.ttest_1samp(score_diff, 0)
        
        comparison_results = {
            'model1_path': str(self.model_path),
            'model2_path': str(other_model_path),
            'n_test_samples': len(df_valid),
            'model1_performance': {
                'roc_auc': float(roc_auc1),
                'average_precision': float(ap1),
                'mean_score': float(np.mean(scores1)),
                'std_score': float(np.std(scores1))
            },
            'model2_performance': {
                'roc_auc': float(roc_auc2),
                'average_precision': float(ap2),
                'mean_score': float(np.mean(scores2)),
                'std_score': float(np.std(scores2))
            },
            'comparison': {
                'auc_difference': float(roc_auc1 - roc_auc2),
                'ap_difference': float(ap1 - ap2),
                'score_correlation': float(np.corrcoef(scores1, scores2)[0, 1]),
                'statistical_test': {
                    't_statistic': float(t_stat),
                    'p_value': float(p_value),
                    'significant_at_0.05': bool(p_value < 0.05)
                }
            }
        }
        
        logger.info(f"Model comparison results:")
        logger.info(f"  Model 1 AUC: {roc_auc1:.4f}")
        logger.info(f"  Model 2 AUC: {roc_auc2:.4f}")
        logger.info(f"  AUC difference: {roc_auc1 - roc_auc2:.4f}")
        logger.info(f"  Statistical significance (p-value): {p_value:.4f}")
        
        return comparison_results


def main():
    parser = argparse.ArgumentParser(
        description="Apply trained XGBoost model to B meson data",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Apply to single ROOT file
  python ApplyModel.py --model models/bmeson_xgboost --input data.root --output scored_data.root
  
  # Apply to Parquet file with custom threshold
  python ApplyModel.py --model models/v1 --input data.parquet --output scores.parquet --threshold 0.7
  
  # Batch process multiple files
  python ApplyModel.py --model models/v1 --input "data/*.root" --output scored_data/ --batch
  
  # Validate on labeled data
  python ApplyModel.py --model models/v1 --input test_data.parquet --validate
        """
    )
    
    parser.add_argument('--model', required=True, help='Path to trained model directory')
    parser.add_argument('--input', required=True, help='Input data file or pattern')
    parser.add_argument('--output', help='Output file or directory')
    parser.add_argument('--format', choices=['auto', 'root', 'parquet'], default='auto',
                       help='Input/output file format')
    parser.add_argument('--threshold', type=float, default=0.5,
                       help='Classification threshold (default: 0.5)')
    parser.add_argument('--chunk-size', type=int, default=100000,
                       help='Chunk size for processing large files')
    parser.add_argument('--batch', action='store_true',
                       help='Batch process multiple files')
    parser.add_argument('--validate', action='store_true',
                       help='Validate model on labeled data')
    parser.add_argument('--compare-with', type=str,
                       help='Compare with another model')
    parser.add_argument('--label-column', default='signal_label',
                       help='Label column name for validation')
    
    args = parser.parse_args()
    
    # Initialize model applicator
    applicator = BMesonModelApplicator(args.model)
    
    if args.validate:
        # Validation mode
        results = applicator.validate_on_labeled_data(
            args.input, 
            label_column=args.label_column,
            file_format=args.format
        )
        
        # Save validation results
        if args.output:
            output_file = args.output
        else:
            output_file = f"validation_results_{Path(args.model).name}.json"
        
        with open(output_file, 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Validation results saved to {output_file}")
        
    elif args.compare_with:
        # Model comparison mode
        if not args.output:
            args.output = "model_comparison.json"
        
        results = applicator.compare_models(
            args.compare_with, 
            args.input,
            label_column=args.label_column
        )
        
        with open(args.output, 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Model comparison results saved to {args.output}")
        
    elif args.batch:
        # Batch processing mode
        if not args.output:
            args.output = "batch_scored_data"
        
        results = applicator.batch_apply(
            args.input,
            args.output,
            file_format=args.format,
            threshold=args.threshold,
            chunk_size=args.chunk_size
        )
        
        # Save batch results
        results_file = Path(args.output) / "batch_results.json"
        with open(results_file, 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Batch processing results saved to {results_file}")
        
    else:
        # Single file processing mode
        if not args.output:
            input_path = Path(args.input)
            args.output = f"scored_{input_path.name}"
        
        if args.format == "parquet" or args.input.endswith('.parquet'):
            results = applicator.apply_to_parquet_file(
                args.input,
                args.output,
                threshold=args.threshold
            )
        else:
            results = applicator.apply_to_root_file(
                args.input,
                args.output,
                threshold=args.threshold,
                chunk_size=args.chunk_size
            )
        
        # Save results summary
        results_file = Path(args.output).with_suffix('.json')
        with open(results_file, 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Processing results saved to {results_file}")


if __name__ == "__main__":
    main()