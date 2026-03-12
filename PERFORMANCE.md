# Edge AI Performance Benchmarks

Ghost Node is optimized for low-latency inference on resource-constrained hardware. Below are the benchmarks recorded during testing on an ESP32-WROOM-32.

## Model Architecture
- **Type:** Fully Connected Neural Network (DNN)
- **Input:** 2.4GHz Signal Spectral Data (DSP processed)
- **Classes:** `SAFE`, `VULNERABLE`, `UNSAFE`

## Inference Metrics
| Metric | Result | Notes |
| :--- | :--- | :--- |
| **On-Chip Inference Latency** | **12ms - 18ms** | Varies based on network density |
| **Total Cycle Time** | **250ms** | Includes Scan + Inference + MQTT Pub |
| **Peak RAM Usage** | **28.4 KB** | Well within ESP32 520KB limit |
| **Flash Memory Usage** | **145 KB** | Includes model weights and DSP code |
| **Classification Accuracy** | **94.2%** | Validated against known Rogue AP signatures |

## Optimization Techniques
1. **Quantization:** Used **INT8 Quantization** via Edge Impulse to reduce model size by 4x with <1% accuracy loss.
2. **DSP Pre-processing:** Implemented a spectral analysis window to reduce the raw data input size before it hits the neural network.
3. **Managed Ingestion:** Leveraged Azure Concurrency Control to maintain a **100% success rate** for telemetry delivery during high-velocity bursts.
