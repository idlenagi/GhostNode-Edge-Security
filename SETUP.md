# Setup & Deployment Guide: Ghost Node

This guide provides the necessary steps to replicate the Ghost Node environment, from flashing the hardware to configuring the Azure cloud backend.

## 1. Prerequisites

### Hardware
* **Microcontroller:** ESP32 (WROOM or DevKit V1 recommended)
* **Antenna:** Standard 2.4GHz (Internal or external)

### Software & Accounts
* **Arduino IDE** or **VS Code (PlatformIO)**
* **Edge Impulse CLI** (for model deployment)
*  **Python 3.9+** 
* **EMQX Cloud** (Free Tier) or a local MQTT Broker
* **Microsoft Azure Subscription** (Free/Student tier is sufficient)

---

## 2. Hardware Firmware (Edge)

1. **Install Dependencies:**
   - In Arduino IDE, install the `WiFi`, `PubSubClient`, and `ArduinoJson` libraries.
2. **Edge Impulse Integration:**
   - Export your trained model as an **Arduino Library** from the Edge Impulse Studio.
   - Include the `.zip` library in your IDE: `Sketch > Include Library > Add .ZIP Library`.
3. **Configuration:**
   - Open `firmware/ghost_node_main.ino`.
   - Update your `ssid`, `password`, and `mqtt_server` credentials.
4. **Flash:** - Select your ESP32 board and hit **Upload**.

---

## 3. Cloud Infrastructure (Backend)

### MQTT Broker (EMQX)
- Create a new dashboard in EMQX.
- Define a topic named `threat/detection`.
- Create a user with **Publish/Subscribe** permissions for the ESP32.

### Azure Logic Apps
1. Create a new **Logic App (Consumption)**.
2. **Trigger:** Set an HTTP Request trigger to receive data from the MQTT broker (or use the built-in MQTT connector).
3. **Concurrency Control:** (CRITICAL)
   - Go to **Settings > Transition Settings**.
   - Enable **Concurrency Control**.
   - Set the **Degree of Parallelism** to `20` to prevent `429 Too Many Requests` errors during high-velocity scans.

---

## 4. Analytics (Kusto / ADX)
1. Provision an **Azure Data Explorer (ADX)** cluster.
2. Ingest the logs from your Blob Storage.
3. Use the queries provided in the `/analytics` folder to generate the real-time threat maps.
