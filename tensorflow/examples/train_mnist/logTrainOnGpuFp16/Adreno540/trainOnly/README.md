# Train on Mobile Phone (Snapdragon 835)
## 1. Explanation :

Train a batch of data and test the model with testing dataset.

The training process is accelerated by off-loading the MatMul operation from CPU
to GPU in TensorFlow framework.

## 1.1 Analysis: For the following network, MatMul of various matrices sizes are performed.
| Model Name | MatMul Operation in the Network                    |
| :---       | :---                                               |
| MLP        | [batchSize, 784] * [784, 50] * [50, 50] * [50, 10] |
| DNN        | [batchSize, 3136] * [3136, 1024] * [1024, 10]      |

## 2. Train MNIST dataset on mobile CPU (FP32)

| Model Name |  Overall Accuracy (%)  | Training Time (s) | Batch Size |
| :---       | :---                   | :---              | :---       |
| MLP        | 78.3435                | 5.34335           | 100        |
| DNN        | 96.7990                | 216.708           | 1000       |

## 3.1 Train MNIST dataset on mobile GPU (FP32)

### OpenCL Kernel Used: `MatMul_TN_1D_Fp32_Float4` + `MatTrans_1D_Fp32_Float4`
| Model Name |  Overall Accuracy (%)  | Training Time (s) | Batch Size |
| :---       | :---                   | :---              | :---       |
| MLP        | 79.2828                | 56.3333           | 100        |
| DNN        | 96.7909                | 508.305           | 1000       |

### OpenCL Kernel Used: `MatMul_TN_1D_Fp32_Float8` + `MatTrans_1D_Fp32_Float8`
| Model Name |  Overall Accuracy (%)  | Training Time (s) | Batch Size |
| :---       | :---                   | :---              | :---       |
| MLP        | 80.0404                | 56.5589           | 100        |
| DNN        | 97.2151                | 527.089           | 1000       |

### OpenCL Kernel Used: `MatMul_TN_1D_Fp32_Float16` + `MatTrans_1D_Fp32_Float16`
| Model Name |  Overall Accuracy (%)  | Training Time (s) | Batch Size |
| :---       | :---                   | :---              | :---       |
| MLP        | 78.8788                | 53.1919           | 100        |
| DNN        | 96.6364                | 388.855           | 1000       |

## 3.2 Train MNIST dataset on mobile GPU (FP16)

### OpenCL Kernel Used: `MatMul_TN_1D_Fp16_Float16` + `MatTrans_1D_Fp16_Float16`
| Model Name |  Overall Accuracy (%)  | Training Time (s) | Batch Size |
| :---       | :---                   | :---              | :---       |
| MLP        | 78.4747                | 77.4084           | 100        |
| DNN        | 96.1010                | 376.229           | 1000       |

## 4. Instructions:

Compile project by running script `./train_mnist_android.sh`

Run the following commands on Android:

### 1) MLP
`./train_mnist --graphName=mnist_mlp.pb --batchSize=100 --iteration=1`
#### * Change batchSize=100 for better accuracy
### 2) DNN
`./train_mnist --graphName=mnist_dnn.pb --dropProb=0.8 --batchSize=1000 --iteration=1`
