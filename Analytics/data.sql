Sentinel
| extend TaiwanTime = ingestion_time() + 8h
| extend 
    Verdict = tostring(payload.verdict),
    Confidence = todouble(payload.confidence),
    RSSI = tolong(payload.rssi),
    SSID = tostring(payload.ssid),
    MAC = tostring(payload.mac)
| project TaiwanTime, SSID, MAC, RSSI, Verdict, Confidence
| order by TaiwanTime desc