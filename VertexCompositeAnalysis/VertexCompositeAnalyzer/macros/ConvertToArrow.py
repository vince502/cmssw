#!/usr/bin/env python3
"""
ConvertToArrow.py
Convert flattened B meson ROOT trees to Apache Arrow/Parquet format
for efficient machine learning processing

Usage:
    python ConvertToArrow.py --input "BDiMuMu_Flat_*.root" --output "bmeson_data.parquet"
    python ConvertToArrow.py --input input.root --output output.parquet --chunk-size 100000
"""

import argparse
import glob
import os
import sys
from pathlib import Path
import logging
from typing import List, Dict, Any, Optional
import warnings

import numpy as np
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
import uproot
from tqdm import tqdm

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Suppress ROOT warnings
warnings.filterwarnings("ignore", message=".*TTree.*")

class ROOTToArrowConverter:
    """Convert ROOT trees to Apache Arrow format with optimized data types"""
    
    def __init__(self, chunk_size: int = 100000, compression: str = 'snappy'):
        self.chunk_size = chunk_size
        self.compression = compression
        
        # Define optimized data types for each variable category
        self.dtype_mapping = {
            # Event info
            'runNb': pa.uint32(),
            'eventNb': pa.uint64(), 
            'lumiSection': pa.uint32(),
            'centrality': pa.int16(),
            'Ntrkoffline': pa.int32(),
            'nPV': pa.int16(),
            
            # Floating point variables (use float32 for space efficiency)
            'bestvx': pa.float32(),
            'bestvy': pa.float32(),
            'bestvz': pa.float32(),
            'bestvxError': pa.float32(),
            'bestvyError': pa.float32(),
            'bestvzError': pa.float32(),
            
            # B meson info
            'bMesonType': pa.int8(),  # 0=B+, 1=B0, 2=Bc
            'bMesonPt': pa.float32(),
            'bMesonEta': pa.float32(),
            'bMesonPhi': pa.float32(),
            'bMesonY': pa.float32(),
            'bMesonMass': pa.float32(),
            'bMesonCharge': pa.float32(),
            'bMesonMVA': pa.float32(),
            'bMesonMatchGEN': pa.bool_(),
            
            # Vertex quality
            'bMesonVtxChi2': pa.float32(),
            'bMesonVtxNdof': pa.float32(),
            'bMesonVtxProb': pa.float32(),
            'bMesonVtxX': pa.float32(),
            'bMesonVtxY': pa.float32(),
            'bMesonVtxZ': pa.float32(),
            'bMesonVtxXError': pa.float32(),
            'bMesonVtxYError': pa.float32(),
            'bMesonVtxZError': pa.float32(),
            
            # Topological variables
            'bMesonPointingAngle2D': pa.float32(),
            'bMesonPointingAngle3D': pa.float32(),
            'bMesonDecayLength2D': pa.float32(),
            'bMesonDecayLength3D': pa.float32(),
            'bMesonDecayLengthSig2D': pa.float32(),
            'bMesonDecayLengthSig3D': pa.float32(),
            
            # Dimuon info
            'dimuonPt': pa.float32(),
            'dimuonEta': pa.float32(),
            'dimuonPhi': pa.float32(),
            'dimuonY': pa.float32(),
            'dimuonMass': pa.float32(),
            'dimuonCharge': pa.float32(),
            
            # Muon info (individual muons)
            'muon1Pt': pa.float32(),
            'muon1Eta': pa.float32(),
            'muon1Phi': pa.float32(),
            'muon1Charge': pa.float32(),
            'muon1IsGlobal': pa.bool_(),
            'muon1IsTracker': pa.bool_(),
            'muon1IsPF': pa.bool_(),
            'muon1Dxy': pa.float32(),
            'muon1Dz': pa.float32(),
            'muon1NHits': pa.int16(),
            'muon1NPixelHits': pa.int16(),
            'muon1NStations': pa.int16(),
            
            'muon2Pt': pa.float32(),
            'muon2Eta': pa.float32(),
            'muon2Phi': pa.float32(),
            'muon2Charge': pa.float32(),
            'muon2IsGlobal': pa.bool_(),
            'muon2IsTracker': pa.bool_(),
            'muon2IsPF': pa.bool_(),
            'muon2Dxy': pa.float32(),
            'muon2Dz': pa.float32(),
            'muon2NHits': pa.int16(),
            'muon2NPixelHits': pa.int16(),
            'muon2NStations': pa.int16(),
            
            # Track info
            'nTracks': pa.uint8(),
            'track1Pt': pa.float32(),
            'track1Eta': pa.float32(),
            'track1Phi': pa.float32(),
            'track1Charge': pa.float32(),
            'track1Dxy': pa.float32(),
            'track1Dz': pa.float32(),
            'track1PtError': pa.float32(),
            'track1NHits': pa.int16(),
            'track1NPixelHits': pa.int16(),
            'track1Chi2': pa.float32(),
            'track1HighPurity': pa.bool_(),
            'track1PID': pa.int16(),
            
            'track2Pt': pa.float32(),
            'track2Eta': pa.float32(),
            'track2Phi': pa.float32(),
            'track2Charge': pa.float32(),
            'track2Dxy': pa.float32(),  
            'track2Dz': pa.float32(),
            'track2PtError': pa.float32(),
            'track2NHits': pa.int16(),
            'track2NPixelHits': pa.int16(),
            'track2Chi2': pa.float32(),
            'track2HighPurity': pa.bool_(),
            'track2PID': pa.int16(),
            
            # K*0 info (B0 only)
            'kstarPt': pa.float32(),
            'kstarEta': pa.float32(),
            'kstarPhi': pa.float32(),
            'kstarMass': pa.float32(),
        }
        
        # Variable descriptions for metadata
        self.variable_descriptions = {
            'runNb': 'Run number',
            'eventNb': 'Event number', 
            'lumiSection': 'Luminosity section',
            'centrality': 'Event centrality',
            'Ntrkoffline': 'Number of offline tracks',
            'nPV': 'Number of primary vertices',
            'bestvx': 'Best primary vertex x position [cm]',
            'bestvy': 'Best primary vertex y position [cm]',
            'bestvz': 'Best primary vertex z position [cm]',
            'bMesonType': 'B meson type (0=B+, 1=B0, 2=Bc)',
            'bMesonPt': 'B meson transverse momentum [GeV/c]',
            'bMesonEta': 'B meson pseudorapidity',
            'bMesonPhi': 'B meson azimuthal angle [rad]',
            'bMesonY': 'B meson rapidity',
            'bMesonMass': 'B meson invariant mass [GeV/c²]',
            'bMesonCharge': 'B meson charge',
            'bMesonMVA': 'B meson MVA discriminator score',
            'bMesonMatchGEN': 'B meson matched to generator level',
            'bMesonVtxChi2': 'B meson vertex chi-squared',
            'bMesonVtxNdof': 'B meson vertex degrees of freedom',
            'bMesonVtxProb': 'B meson vertex probability',
            'bMesonPointingAngle2D': 'B meson 2D pointing angle [rad]',
            'bMesonPointingAngle3D': 'B meson 3D pointing angle [rad]',
            'bMesonDecayLength2D': 'B meson 2D decay length [cm]',
            'bMesonDecayLength3D': 'B meson 3D decay length [cm]',
            'bMesonDecayLengthSig2D': 'B meson 2D decay length significance',
            'bMesonDecayLengthSig3D': 'B meson 3D decay length significance',
            'dimuonPt': 'Dimuon transverse momentum [GeV/c]',
            'dimuonEta': 'Dimuon pseudorapidity',
            'dimuonPhi': 'Dimuon azimuthal angle [rad]',
            'dimuonY': 'Dimuon rapidity',
            'dimuonMass': 'Dimuon invariant mass (J/ψ) [GeV/c²]',
            'dimuonCharge': 'Dimuon charge',
            'muon1Pt': 'Leading muon transverse momentum [GeV/c]',
            'muon1Eta': 'Leading muon pseudorapidity',
            'muon1Phi': 'Leading muon azimuthal angle [rad]',
            'muon1Charge': 'Leading muon charge',
            'muon1IsGlobal': 'Leading muon is global muon',
            'muon1IsTracker': 'Leading muon is tracker muon',
            'muon1IsPF': 'Leading muon is particle flow muon',
            'muon2Pt': 'Subleading muon transverse momentum [GeV/c]',
            'muon2Eta': 'Subleading muon pseudorapidity', 
            'muon2Phi': 'Subleading muon azimuthal angle [rad]',
            'muon2Charge': 'Subleading muon charge',
            'nTracks': 'Number of additional tracks',
            'track1Pt': 'First track transverse momentum [GeV/c]',
            'track1Eta': 'First track pseudorapidity',
            'track1Phi': 'First track azimuthal angle [rad]',
            'track1Charge': 'First track charge',
            'track1PID': 'First track particle ID (PDG code)',
            'track2Pt': 'Second track transverse momentum [GeV/c]',
            'track2Eta': 'Second track pseudorapidity',
            'track2Phi': 'Second track azimuthal angle [rad]',
            'track2Charge': 'Second track charge',
            'track2PID': 'Second track particle ID (PDG code)',
            'kstarPt': 'K*0 transverse momentum [GeV/c] (B0 only)',
            'kstarEta': 'K*0 pseudorapidity (B0 only)',
            'kstarPhi': 'K*0 azimuthal angle [rad] (B0 only)',
            'kstarMass': 'K*0 invariant mass [GeV/c²] (B0 only)',
        }

    def get_file_list(self, input_pattern: str) -> List[str]:
        """Get list of ROOT files matching the input pattern"""
        if '*' in input_pattern:
            files = glob.glob(input_pattern)
        else:
            files = [input_pattern]
        
        # Verify files exist
        valid_files = []
        for file_path in files:
            if os.path.exists(file_path):
                valid_files.append(file_path)
            else:
                logger.warning(f"File not found: {file_path}")
        
        if not valid_files:
            raise FileNotFoundError(f"No valid ROOT files found matching: {input_pattern}")
        
        logger.info(f"Found {len(valid_files)} ROOT files to process")
        return sorted(valid_files)

    def get_tree_info(self, file_path: str) -> Dict[str, Any]:
        """Get information about the ROOT tree structure"""
        try:
            with uproot.open(file_path) as file:
                tree_name = "bDiMuMuFlatTree"
                if tree_name not in file:
                    raise KeyError(f"Tree '{tree_name}' not found in {file_path}")
                
                tree = file[tree_name]
                branches = list(tree.keys())
                num_entries = tree.num_entries
                
                logger.info(f"Tree info - Entries: {num_entries}, Branches: {len(branches)}")
                return {
                    'tree_name': tree_name,
                    'branches': branches,
                    'num_entries': num_entries
                }
        except Exception as e:
            logger.error(f"Error reading ROOT file {file_path}: {e}")
            raise

    def optimize_dataframe(self, df: pd.DataFrame) -> pd.DataFrame:
        """Optimize DataFrame data types for memory efficiency"""
        optimized_df = df.copy()
        
        for column in df.columns:
            if column in self.dtype_mapping:
                # Convert to the target Arrow type via pandas
                if self.dtype_mapping[column] == pa.float32():
                    optimized_df[column] = df[column].astype('float32')
                elif self.dtype_mapping[column] == pa.int8():
                    optimized_df[column] = df[column].astype('int8')
                elif self.dtype_mapping[column] == pa.int16():
                    optimized_df[column] = df[column].astype('int16')
                elif self.dtype_mapping[column] == pa.int32():
                    optimized_df[column] = df[column].astype('int32')
                elif self.dtype_mapping[column] == pa.uint8():
                    optimized_df[column] = df[column].astype('uint8')
                elif self.dtype_mapping[column] == pa.uint16():
                    optimized_df[column] = df[column].astype('uint16')
                elif self.dtype_mapping[column] == pa.uint32():
                    optimized_df[column] = df[column].astype('uint32')
                elif self.dtype_mapping[column] == pa.uint64():
                    optimized_df[column] = df[column].astype('uint64')
                elif self.dtype_mapping[column] == pa.bool_():
                    optimized_df[column] = df[column].astype('bool')
        
        return optimized_df

    def create_arrow_schema(self, df: pd.DataFrame) -> pa.Schema:
        """Create Arrow schema with optimized data types and metadata"""
        fields = []
        
        for column in df.columns:
            if column in self.dtype_mapping:
                arrow_type = self.dtype_mapping[column]
            else:
                # Default to float32 for unknown numeric columns
                arrow_type = pa.float32()
            
            # Add metadata for each field
            metadata = {}
            if column in self.variable_descriptions:
                metadata['description'] = self.variable_descriptions[column]
            
            field = pa.field(column, arrow_type, metadata=metadata)
            fields.append(field)
        
        # Add global metadata
        schema_metadata = {
            'source': 'BDiMuMuNtuplizer flattened ROOT trees',
            'conversion_tool': 'ConvertToArrow.py',
            'data_format': 'B meson analysis - one row per candidate',
            'physics_process': 'B+ → J/ψ K+, B0 → J/ψ K*0, Bc → J/ψ π+',
        }
        
        return pa.schema(fields, metadata=schema_metadata)

    def convert_file(self, file_path: str, output_path: str, append: bool = False):
        """Convert a single ROOT file to Arrow/Parquet format"""
        logger.info(f"Converting {file_path}")
        
        try:
            with uproot.open(file_path) as file:
                tree = file["bDiMuMuFlatTree"]
                total_entries = tree.num_entries
                
                if total_entries == 0:
                    logger.warning(f"Empty tree in {file_path}, skipping")
                    return
                
                # Process in chunks
                parquet_writer = None
                total_processed = 0
                
                for chunk_start in tqdm(range(0, total_entries, self.chunk_size), 
                                       desc=f"Processing {Path(file_path).name}"):
                    chunk_end = min(chunk_start + self.chunk_size, total_entries)
                    
                    # Read chunk as pandas DataFrame
                    df = tree.arrays(entry_start=chunk_start, entry_stop=chunk_end, 
                                   library="pd")
                    
                    # Optimize data types
                    df = self.optimize_dataframe(df)
                    
                    # Convert to Arrow
                    if parquet_writer is None:
                        schema = self.create_arrow_schema(df)
                        table = pa.Table.from_pandas(df, schema=schema)
                        
                        # Initialize parquet writer
                        if append and os.path.exists(output_path):
                            parquet_writer = pq.ParquetWriter(output_path, schema, 
                                                            compression=self.compression,
                                                            version='2.6')
                        else:
                            parquet_writer = pq.ParquetWriter(output_path, schema,
                                                            compression=self.compression,
                                                            version='2.6')
                    else:
                        table = pa.Table.from_pandas(df, schema=schema)
                    
                    # Write chunk
                    parquet_writer.write_table(table)
                    total_processed += len(df)
                
                if parquet_writer:
                    parquet_writer.close()
                
                logger.info(f"Processed {total_processed} entries from {file_path}")
                
        except Exception as e:
            logger.error(f"Error processing {file_path}: {e}")
            raise

    def convert_files(self, input_pattern: str, output_path: str):
        """Convert multiple ROOT files to a single Parquet file"""
        files = self.get_file_list(input_pattern)
        
        logger.info(f"Converting {len(files)} files to {output_path}")
        logger.info(f"Chunk size: {self.chunk_size}, Compression: {self.compression}")
        
        # Process first file
        self.convert_file(files[0], output_path, append=False)
        
        # Append remaining files
        for file_path in files[1:]:
            self.convert_file(file_path, output_path, append=True)
        
        # Verify output
        if os.path.exists(output_path):
            file_size = os.path.getsize(output_path) / (1024**3)  # GB
            logger.info(f"Output file: {output_path}")
            logger.info(f"File size: {file_size:.2f} GB")
            
            # Read and display basic statistics
            table = pq.read_table(output_path)
            logger.info(f"Total entries: {len(table)}")
            logger.info(f"Total columns: {len(table.column_names)}")
            
            # Show B meson type distribution
            df_sample = table.select(['bMesonType']).to_pandas()
            type_counts = df_sample['bMesonType'].value_counts().sort_index()
            logger.info("B meson type distribution:")
            for btype, count in type_counts.items():
                type_name = {0: 'B+', 1: 'B0', 2: 'Bc'}.get(btype, f'Type{btype}')
                logger.info(f"  {type_name}: {count:,} candidates")
        else:
            raise FileNotFoundError(f"Output file was not created: {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Convert flattened B meson ROOT trees to Arrow/Parquet format",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python ConvertToArrow.py --input "BDiMuMu_Flat_*.root" --output "bmeson_data.parquet"
  python ConvertToArrow.py --input input.root --output output.parquet --chunk-size 50000
  python ConvertToArrow.py --input "data/*.root" --output "combined.parquet" --compression lz4
        """
    )
    
    parser.add_argument('--input', required=True,
                       help='Input ROOT file pattern (supports wildcards)')
    parser.add_argument('--output', required=True,
                       help='Output Parquet file path')
    parser.add_argument('--chunk-size', type=int, default=100000,
                       help='Number of entries to process per chunk (default: 100000)')
    parser.add_argument('--compression', choices=['snappy', 'gzip', 'lz4', 'brotli'], 
                       default='snappy',
                       help='Compression algorithm (default: snappy)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose logging')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # Create output directory if needed
    output_dir = Path(args.output).parent
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Initialize converter
    converter = ROOTToArrowConverter(
        chunk_size=args.chunk_size,
        compression=args.compression
    )
    
    try:
        # Convert files
        converter.convert_files(args.input, args.output)
        logger.info("Conversion completed successfully!")
        
    except Exception as e:
        logger.error(f"Conversion failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()