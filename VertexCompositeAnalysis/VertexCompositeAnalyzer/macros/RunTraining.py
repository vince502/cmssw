#!/usr/bin/env python3
"""
RunTraining.py
Main execution script for B meson XGBoost training pipeline

This script orchestrates the complete machine learning workflow:
1. Data conversion (ROOT → Arrow)
2. Feature engineering
3. Model training with hyperparameter optimization
4. Model evaluation and performance analysis
5. Model deployment preparation

Usage:
    python RunTraining.py --config TrainingConfig.yaml
    python RunTraining.py --config TrainingConfig.yaml --data-dir /path/to/root/files
    python RunTraining.py --quick-start  # Use default settings for quick testing
"""

import argparse
import logging
import sys
import time
from pathlib import Path
from typing import Optional, Dict, Any
import warnings

# Configure logging first
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('bmeson_training.log')
    ]
)
logger = logging.getLogger(__name__)

# Suppress warnings
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)

# Import our modules
try:
    from ConvertToArrow import ROOTToArrowConverter
    from FeatureEngineering import BMesonFeatureEngineer
    from XGBoostTrainer import BMesonXGBoostTrainer
    from ModelEvaluation import BMesonModelEvaluator
    from ApplyModel import BMesonModelApplicator
except ImportError as e:
    logger.error(f"Failed to import required modules: {e}")
    logger.error("Make sure all Python files are in the same directory")
    sys.exit(1)

class BMesonTrainingPipeline:
    """Complete B meson XGBoost training pipeline"""
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path
        self.start_time = time.time()
        
        # Pipeline components
        self.converter = None
        self.feature_engineer = None
        self.trainer = None
        self.evaluator = None
        
        # Results storage
        self.results = {
            'pipeline_start_time': self.start_time,
            'steps_completed': [],
            'conversion_results': None,
            'feature_engineering_results': None,
            'training_results': None,
            'evaluation_results': None
        }
        
        logger.info("=== B Meson XGBoost Training Pipeline ===")
        logger.info(f"Start time: {time.ctime(self.start_time)}")
        
    def step_1_convert_data(self, input_pattern: str, output_file: str, 
                           chunk_size: int = 100000) -> bool:
        """Step 1: Convert ROOT files to Arrow format"""
        logger.info("=== Step 1: Converting ROOT files to Arrow format ===")
        
        try:
            self.converter = ROOTToArrowConverter(
                chunk_size=chunk_size,
                compression='snappy'
            )
            
            # Check if output already exists
            if Path(output_file).exists():
                logger.info(f"Arrow file already exists: {output_file}")
                response = input("Do you want to overwrite? (y/N): ").lower()
                if response != 'y':
                    logger.info("Using existing Arrow file")
                    self.results['conversion_results'] = {'status': 'skipped', 'output_file': output_file}
                    self.results['steps_completed'].append('convert_data')
                    return True
            
            # Perform conversion
            self.converter.convert_files(input_pattern, output_file)
            
            self.results['conversion_results'] = {
                'status': 'completed',
                'input_pattern': input_pattern,
                'output_file': output_file,
                'chunk_size': chunk_size
            }
            self.results['steps_completed'].append('convert_data')
            
            logger.info("✓ Data conversion completed successfully")
            return True
            
        except Exception as e:
            logger.error(f"✗ Data conversion failed: {e}")
            return False
    
    def step_2_feature_engineering(self, input_file: str, output_file: str) -> bool:
        """Step 2: Create physics-motivated features"""
        logger.info("=== Step 2: Creating physics features ===")
        
        try:
            self.feature_engineer = BMesonFeatureEngineer()
            
            # Check if output already exists
            if Path(output_file).exists():
                logger.info(f"Feature-enhanced file already exists: {output_file}")
                response = input("Do you want to overwrite? (y/N): ").lower()
                if response != 'y':
                    logger.info("Using existing feature-enhanced file")
                    self.results['feature_engineering_results'] = {'status': 'skipped', 'output_file': output_file}
                    self.results['steps_completed'].append('feature_engineering')
                    return True
            
            # Load data
            import pyarrow.parquet as pq
            logger.info(f"Loading data from {input_file}")
            table = pq.read_table(input_file)
            df = table.to_pandas()
            
            logger.info(f"Loaded {len(df)} B meson candidates")
            
            # Create features
            df_with_features = self.feature_engineer.create_features(df)
            
            # Create signal labels
            labels = self.feature_engineer.create_signal_labels(df_with_features)
            df_with_features['signal_label'] = labels
            
            # Save enhanced dataset
            df_with_features.to_parquet(output_file, compression='snappy')
            
            # Results summary
            n_signal = sum(labels == 1)
            n_background = sum(labels == 0) 
            n_excluded = sum(labels == -1)
            n_features = len(df_with_features.columns) - len(df.columns)
            
            self.results['feature_engineering_results'] = {
                'status': 'completed',
                'input_file': input_file,
                'output_file': output_file,
                'n_candidates': len(df_with_features),
                'n_signal': n_signal,
                'n_background': n_background,
                'n_excluded': n_excluded,
                'n_new_features': n_features,
                'feature_categories': self.feature_engineer.get_feature_importance_groups()
            }
            self.results['steps_completed'].append('feature_engineering')
            
            logger.info(f"✓ Feature engineering completed successfully")
            logger.info(f"  Created {n_features} new features")
            logger.info(f"  Signal candidates: {n_signal:,}")
            logger.info(f"  Background candidates: {n_background:,}")
            logger.info(f"  Enhanced dataset saved to: {output_file}")
            
            return True
            
        except Exception as e:
            logger.error(f"✗ Feature engineering failed: {e}")
            return False
    
    def step_3_train_model(self, config_path: Optional[str] = None) -> bool:
        """Step 3: Train XGBoost model"""
        logger.info("=== Step 3: Training XGBoost model ===")
        
        try:
            self.trainer = BMesonXGBoostTrainer(config_path=config_path)
            
            # Train model
            self.trainer.train_model()
            
            self.results['training_results'] = {
                'status': 'completed',
                'config_path': config_path,
                'model_dir': str(self.trainer.output_dir),
                'final_auc': self.trainer.results.get('test_auc', 0),
                'best_params': self.trainer.results.get('best_params', {}),
                'cv_results': self.trainer.results.get('cv_results', {})
            }
            self.results['steps_completed'].append('train_model')
            
            logger.info(f"✓ Model training completed successfully")
            logger.info(f"  Final test AUC: {self.trainer.results.get('test_auc', 0):.4f}")
            logger.info(f"  Model saved to: {self.trainer.output_dir}")
            
            return True
            
        except Exception as e:
            logger.error(f"✗ Model training failed: {e}")
            return False
    
    def step_4_evaluate_model(self, model_dir: str, test_data: str) -> bool:
        """Step 4: Comprehensive model evaluation"""
        logger.info("=== Step 4: Evaluating trained model ===")
        
        try:
            self.evaluator = BMesonModelEvaluator(
                model_dir=model_dir,
                data_file=test_data
            )
            
            # Generate comprehensive evaluation report
            evaluation_results = self.evaluator.generate_evaluation_report(test_data)
            
            self.results['evaluation_results'] = {
                'status': 'completed',
                'model_dir': model_dir,
                'test_data': test_data,
                'roc_auc': evaluation_results.get('roc_analysis', {}).get('roc_auc', 0),
                'average_precision': evaluation_results.get('pr_analysis', {}).get('avg_precision', 0),
                'n_test_samples': evaluation_results.get('model_info', {}).get('n_test_samples', 0)
            }
            self.results['steps_completed'].append('evaluate_model')
            
            logger.info(f"✓ Model evaluation completed successfully")
            logger.info(f"  ROC AUC: {evaluation_results.get('roc_analysis', {}).get('roc_auc', 0):.4f}")
            logger.info(f"  Average Precision: {evaluation_results.get('pr_analysis', {}).get('avg_precision', 0):.4f}")
            logger.info(f"  Evaluation plots saved to: {model_dir}")
            
            return True
            
        except Exception as e:
            logger.error(f"✗ Model evaluation failed: {e}")
            return False
    
    def step_5_prepare_deployment(self, model_dir: str, test_data: str) -> bool:
        """Step 5: Prepare model for deployment"""
        logger.info("=== Step 5: Preparing model for deployment ===")
        
        try:
            # Test model application
            applicator = BMesonModelApplicator(model_dir)
            
            # Validate on test data
            validation_results = applicator.validate_on_labeled_data(test_data)
            
            # Create deployment package
            deployment_dir = Path(model_dir) / 'deployment'
            deployment_dir.mkdir(exist_ok=True)
            
            # Copy essential files for deployment
            import shutil
            essential_files = [
                'xgboost_model.pkl',
                'feature_names.json',
                'scaler.pkl',
                'feature_selector.pkl',
                'training_config.yaml'
            ]
            
            for file_name in essential_files:
                src_file = Path(model_dir) / file_name
                if src_file.exists():
                    shutil.copy2(src_file, deployment_dir / file_name)
            
            # Save deployment metadata
            deployment_metadata = {
                'model_version': '1.0',
                'deployment_ready': True,
                'validation_auc': validation_results.get('roc_auc', 0),
                'validation_samples': validation_results.get('n_samples', 0),
                'deployment_date': time.ctime(),
                'required_features': applicator.feature_names,
                'preprocessing_steps': {
                    'feature_selection': applicator.feature_selector is not None,
                    'scaling': applicator.scaler is not None
                }
            }
            
            import json
            with open(deployment_dir / 'deployment_metadata.json', 'w') as f:
                json.dump(deployment_metadata, f, indent=2)
            
            # Create usage example
            usage_example = f"""
# Example usage of deployed B meson XGBoost model

from ApplyModel import BMesonModelApplicator

# Initialize model
applicator = BMesonModelApplicator('{deployment_dir}')

# Apply to new data
results = applicator.apply_to_parquet_file(
    'new_data.parquet', 
    'scored_data.parquet',
    threshold=0.5
)

print(f"Processed {{results['n_processed']}} candidates")
print(f"Predicted signal: {{results['n_predicted_signal']}}")
print(f"Mean BDT score: {{results['mean_score']:.3f}}")
"""
            
            with open(deployment_dir / 'usage_example.py', 'w') as f:
                f.write(usage_example)
            
            self.results['deployment_results'] = {
                'status': 'completed',
                'deployment_dir': str(deployment_dir),
                'validation_auc': validation_results.get('roc_auc', 0),
                'deployment_metadata': deployment_metadata
            }
            self.results['steps_completed'].append('prepare_deployment')
            
            logger.info(f"✓ Deployment preparation completed successfully")
            logger.info(f"  Validation AUC: {validation_results.get('roc_auc', 0):.4f}")
            logger.info(f"  Deployment package: {deployment_dir}")
            
            return True
            
        except Exception as e:
            logger.error(f"✗ Deployment preparation failed: {e}")
            return False
    
    def run_complete_pipeline(self, root_data_pattern: str, 
                             config_path: Optional[str] = None,
                             output_dir: str = "bmeson_ml_output") -> bool:
        """Run the complete training pipeline"""
        logger.info("=== Running Complete B Meson Training Pipeline ===")
        
        # Create output directory
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Define intermediate file paths
        arrow_file = output_path / "bmeson_data.parquet"
        features_file = output_path / "bmeson_data_with_features.parquet"
        
        # Step 1: Convert data
        if not self.step_1_convert_data(root_data_pattern, str(arrow_file)):
            return False
        
        # Step 2: Feature engineering
        if not self.step_2_feature_engineering(str(arrow_file), str(features_file)):
            return False
        
        # Update config to use our feature file
        if config_path:
            import yaml
            with open(config_path, 'r') as f:
                config = yaml.safe_load(f)
            config['data']['input_file'] = str(features_file)
            
            # Save updated config
            updated_config_path = output_path / "updated_config.yaml"
            with open(updated_config_path, 'w') as f:
                yaml.dump(config, f, default_flow_style=False)
            config_path = str(updated_config_path)
        
        # Step 3: Train model
        if not self.step_3_train_model(config_path):
            return False
        
        model_dir = self.trainer.output_dir
        
        # Step 4: Evaluate model
        if not self.step_4_evaluate_model(str(model_dir), str(features_file)):
            return False
        
        # Step 5: Prepare deployment
        if not self.step_5_prepare_deployment(str(model_dir), str(features_file)):
            return False
        
        return True
    
    def generate_final_report(self, output_file: str = "pipeline_report.json") -> None:
        """Generate final pipeline report"""
        logger.info("=== Generating Final Pipeline Report ===")
        
        # Calculate total time
        end_time = time.time()
        total_time = end_time - self.start_time
        
        self.results['pipeline_end_time'] = end_time
        self.results['total_runtime_seconds'] = total_time
        self.results['total_runtime_formatted'] = self._format_time(total_time)
        
        # Success status
        expected_steps = ['convert_data', 'feature_engineering', 'train_model', 'evaluate_model', 'prepare_deployment']
        completed_steps = self.results['steps_completed']
        success_rate = len(completed_steps) / len(expected_steps)
        
        self.results['pipeline_success'] = success_rate == 1.0
        self.results['success_rate'] = success_rate
        self.results['completed_steps_count'] = len(completed_steps)
        self.results['total_steps_count'] = len(expected_steps)
        
        # Save report
        import json
        with open(output_file, 'w') as f:
            json.dump(self.results, f, indent=2, default=str)
        
        # Print summary
        logger.info("=== PIPELINE SUMMARY ===")
        logger.info(f"Status: {'✓ SUCCESS' if self.results['pipeline_success'] else '✗ PARTIAL/FAILED'}")
        logger.info(f"Completed steps: {len(completed_steps)}/{len(expected_steps)}")
        logger.info(f"Total runtime: {self._format_time(total_time)}")
        
        if self.results.get('training_results'):
            logger.info(f"Final model AUC: {self.results['training_results'].get('final_auc', 0):.4f}")
        
        if self.results.get('evaluation_results'):
            logger.info(f"Test AUC: {self.results['evaluation_results'].get('roc_auc', 0):.4f}")
        
        logger.info(f"Pipeline report saved to: {output_file}")
    
    def _format_time(self, seconds: float) -> str:
        """Format time duration"""
        hours = int(seconds // 3600)
        minutes = int((seconds % 3600) // 60)
        secs = int(seconds % 60)
        
        if hours > 0:
            return f"{hours}h {minutes}m {secs}s"
        elif minutes > 0:
            return f"{minutes}m {secs}s"
        else:
            return f"{secs}s"


def create_quick_start_config() -> str:
    """Create a quick-start configuration for testing"""
    config = {
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
            'n_features': 30,
            'selection_method': 'mutual_info',
            'scaling': True,
            'scaler_type': 'robust'
        },
        'sampling': {
            'balance_classes': True,
            'sampling_strategy': 'smote',
            'smote_k_neighbors': 5
        },
        'model': {
            'objective': 'binary:logistic',
            'eval_metric': 'auc',
            'use_gpu': False,
            'early_stopping_rounds': 50,
            'n_estimators': 500
        },
        'hyperparameter_optimization': {
            'enabled': True,
            'n_trials': 20,  # Reduced for quick testing
            'timeout': 600,  # 10 minutes
            'cv_folds': 3,
            'scoring': 'roc_auc'
        },
        'cross_validation': {
            'enabled': True,
            'cv_folds': 3,
            'stratified': True
        },
        'output': {
            'model_dir': 'models/bmeson_quick_test',
            'save_model': True,
            'save_features': True,
            'save_scaler': True,
            'save_plots': True
        }
    }
    
    quick_config_path = "QuickStartConfig.yaml"
    import yaml
    with open(quick_config_path, 'w') as f:
        yaml.dump(config, f, default_flow_style=False)
    
    return quick_config_path


def main():
    parser = argparse.ArgumentParser(
        description="Complete B meson XGBoost training pipeline",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Full pipeline with custom config
  python RunTraining.py --config TrainingConfig.yaml --data-dir /path/to/root/files

  # Quick start with default settings
  python RunTraining.py --quick-start --data-pattern "*.root"

  # Run individual steps
  python RunTraining.py --step convert --input "data/*.root" --output data.parquet
  python RunTraining.py --step features --input data.parquet --output features.parquet
  python RunTraining.py --step train --config TrainingConfig.yaml
        """
    )
    
    # Main operation modes
    parser.add_argument('--config', type=str, help='Path to training configuration file')
    parser.add_argument('--quick-start', action='store_true', 
                       help='Quick start with default settings')
    
    # Data input
    parser.add_argument('--data-dir', type=str, 
                       help='Directory containing ROOT files')
    parser.add_argument('--data-pattern', type=str, default="BDiMuMu_Flat_*.root",
                       help='File pattern for ROOT files')
    
    # Output settings
    parser.add_argument('--output-dir', type=str, default="bmeson_ml_output",
                       help='Output directory for all results')
    
    # Individual step execution
    parser.add_argument('--step', choices=['convert', 'features', 'train', 'evaluate', 'deploy'],
                       help='Run only a specific step')
    parser.add_argument('--input', type=str, help='Input file for individual step')
    parser.add_argument('--output', type=str, help='Output file for individual step')
    parser.add_argument('--model-dir', type=str, help='Model directory for evaluation/deployment')
    
    # Performance settings
    parser.add_argument('--chunk-size', type=int, default=100000,
                       help='Chunk size for data processing')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose logging')
    
    args = parser.parse_args()
    
    # Configure logging level
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # Initialize pipeline
    pipeline = BMesonTrainingPipeline(config_path=args.config)
    
    try:
        if args.step:
            # Run individual step
            logger.info(f"Running individual step: {args.step}")
            
            if args.step == 'convert':
                if not args.input or not args.output:
                    parser.error("--input and --output required for convert step")
                success = pipeline.step_1_convert_data(args.input, args.output, args.chunk_size)
                
            elif args.step == 'features':
                if not args.input or not args.output:
                    parser.error("--input and --output required for features step")
                success = pipeline.step_2_feature_engineering(args.input, args.output)
                
            elif args.step == 'train':
                success = pipeline.step_3_train_model(args.config)
                
            elif args.step == 'evaluate':
                if not args.model_dir or not args.input:
                    parser.error("--model-dir and --input required for evaluate step")
                success = pipeline.step_4_evaluate_model(args.model_dir, args.input)
                
            elif args.step == 'deploy':
                if not args.model_dir or not args.input:
                    parser.error("--model-dir and --input required for deploy step")
                success = pipeline.step_5_prepare_deployment(args.model_dir, args.input)
            
            if success:
                logger.info(f"✓ Step '{args.step}' completed successfully")
            else:
                logger.error(f"✗ Step '{args.step}' failed")
                sys.exit(1)
        
        elif args.quick_start:
            # Quick start mode
            logger.info("Running quick start pipeline")
            
            # Create quick config
            config_path = create_quick_start_config()
            logger.info(f"Created quick start config: {config_path}")
            
            # Determine data pattern
            if args.data_dir:
                data_pattern = f"{args.data_dir}/*.root"
            else:
                data_pattern = args.data_pattern
            
            # Run complete pipeline
            success = pipeline.run_complete_pipeline(
                data_pattern, 
                config_path,
                args.output_dir
            )
            
            if success:
                logger.info("✓ Quick start pipeline completed successfully!")
            else:
                logger.error("✗ Quick start pipeline failed")
                sys.exit(1)
        
        else:
            # Full pipeline mode
            if not args.config:
                parser.error("--config required for full pipeline (or use --quick-start)")
            
            # Determine data pattern
            if args.data_dir:
                data_pattern = f"{args.data_dir}/*.root" 
            else:
                data_pattern = args.data_pattern
            
            # Run complete pipeline
            success = pipeline.run_complete_pipeline(
                data_pattern,
                args.config, 
                args.output_dir
            )
            
            if success:
                logger.info("✓ Complete pipeline finished successfully!")
            else:
                logger.error("✗ Pipeline failed")
                sys.exit(1)
    
    except KeyboardInterrupt:
        logger.warning("Pipeline interrupted by user")
        sys.exit(1)
    
    except Exception as e:
        logger.error(f"Pipeline failed with unexpected error: {e}")
        sys.exit(1)
    
    finally:
        # Always generate final report
        try:
            report_file = Path(args.output_dir) / "pipeline_report.json"
            pipeline.generate_final_report(str(report_file))
        except Exception as e:
            logger.error(f"Failed to generate final report: {e}")


if __name__ == "__main__":
    main()