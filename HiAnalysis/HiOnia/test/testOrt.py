import onnxruntime as ort
import numpy as np

# create input data in the float format (32 bit)
#data = np.arange(1, 18).astype(np.float32)
data = np.array([
0.118075, 1.94322, 0.0563177, 0.808029, 3.10458, -0.361092, -1.13143, 3.81912, -0.930483, 0.00159244, 0.0147527, 0.770337, 0.543453, 0.942741, 0.26027, 0.447334, 0.182519
], dtype=np.float32)
# Reshape to [1, 17] for ONNX input
#data = data.reshape(1, -1)
print(len(data))

# create inference session using ort.InferenceSession from a given model
ort_sess = ort.InferenceSession('../../../VertexCompositeAnalysis/VertexCompositeProducer/data//BDT_XGB_19Dec2025.onnx')

# run inference
outputs = ort_sess.run(None, {'float_input': np.array([data])})

# print input and output
print('input ->', data)
print('output ->', outputs)
