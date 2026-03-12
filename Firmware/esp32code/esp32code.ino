#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Ghost_Node_inferencing.h>
const char* ssid = "YOUR-WIFI NAME";
const char* password = "YOUR PASSWORD";
const char* mqtt_server = "SERVER NAME"; 
const char* mqtt_user = "MQTT-USERNAME";
const char* mqtt_pass = "MQTT-PASSWORD";

// Paste the content of your .pem file here
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);
void setup() {
  Serial.begin(115200);
  Serial.println("Ghost Node Sentinel: Booting");
  while (!Serial);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("\n Connecting to Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); }
  //Serial.println("Ghost Node Sentinel: Active and Ready");
  espClient.setCACert(ca_cert); // Set the certificate for secure connection
  client.setServer(mqtt_server, 8883); // Port 8883 is MANDATORY for Serverless
  Serial.println("Ghost Node Sentinel: Active and Ready");
}

void loop() {
  // 1. Ensure we are connected to the Cloud Broker
  if (!client.connected()) {
    String client_id = "esp32-sentinel-" + String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Sentinel Online!");
      client.publish("sentinel/status", "online"); 
    }
  }
  client.loop(); 

  // 2. Perform the WiFi Scan
  Serial.println("\n--- Ghost Node Sentinel: Scanning ---");
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("Found %d networks. Running AI Analysis...\n", n);

    for (int i = 0; i < n; ++i) {
      // --- PART A: PREPARE DATA FOR AI ---
      float rssi = (float)WiFi.RSSI(i);
      int channel = WiFi.channel(i);
      int enc_type = WiFi.encryptionType(i);

      // Mapping model (Matching the training data)
      float enc_num = 0.0; 
      if (enc_type == WIFI_AUTH_WEP) enc_num = 1.0;
      else if (enc_type == WIFI_AUTH_WPA_PSK) enc_num = 2.0;
      else if (enc_type == WIFI_AUTH_WPA2_PSK) enc_num = 3.0;
      else if (enc_type == WIFI_AUTH_WPA3_PSK) enc_num = 4.0;
      else enc_num = 0.0; 

      float features[] = { rssi, (float)channel, enc_num };
      
      signal_t signal;
      numpy::signal_from_buffer(features, 3, &signal);

      // --- PART B: RUN THE AI INFERENCE ---
      ei_impulse_result_t result = { 0 };
      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

      if (res != EI_IMPULSE_OK) continue;

      // Determine the winning verdict (highest confidence)
      String verdict = "UNKNOWN";
      float highest_confidence = 0.0;

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > highest_confidence) {
          highest_confidence = result.classification[ix].value;
          verdict = String(result.classification[ix].label);
        }
      }

      // --- PART C: FORMAT JSON AND SEND TO CLOUD ---
      // Create a JSON string including the AI's verdict and confidence
      String payload = "{";
      payload += "\"ssid\":\"" + WiFi.SSID(i) + "\", ";
      payload += "\"rssi\":" + String(rssi) + ", ";
      payload += "\"verdict\":\"" + verdict + "\", ";
      payload += "\"confidence\":" + String(highest_confidence * 100, 1);
      payload += "}";

      // Publish to EMQX
      if (client.publish("sentinel/scan", payload.c_str())) {
        Serial.printf("Published: %s | Verdict: %s (%.1f%%)\n", 
                      WiFi.SSID(i).c_str(), verdict.c_str(), highest_confidence * 100);
      }
      
      delay(100); // Small pause to prevent network congestion
    }
  }

  // Clean up and wait for the next scan cycle
  WiFi.scanDelete(); 
  Serial.println("Scan complete. Next scan in 10 seconds...");
  delay(10000); 
}
