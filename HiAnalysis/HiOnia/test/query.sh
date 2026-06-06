#/bin/bash

echo '' > datasets.txt

for i in {0..59}
do
	echo $i
	dasgoclient --query="file dataset=/HIPhysicsRawPrime$i/HIRun2026A-PbPbEW-PromptReco-v1/MINIAOD | grep file.nevents>0 | grep file.name" | awk '$1 != 0' | awk '{print $2}' >> datasets.txt
done
