#!/bin/bash +e

for i in {0..24}
do

dasgoclient --query="file dataset=/PPRefZeroBiasPlusForward"$i"/Run2024J-PromptReco-v1/MINIAOD"
done
