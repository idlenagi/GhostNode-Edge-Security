import serial
import time

# CONFIGURATION
COM_PORT = 'COM4'      
BAUD_RATE = 115200     
FILE_NAME = 'hotel_wifi_data.csv'

try:
    # Open the connection to the ESP32
    print(f"🔌 Connecting to Ghost Node on {COM_PORT}...")
    esp32 = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    time.sleep(2) # Give it a second to wake up
    
    print(f"Connected! Logging data to {FILE_NAME}")
    print("Press Ctrl+C to stop logging.\n")

    # Open the CSV file in 'append' mode so we don't overwrite old data
    with open(FILE_NAME, 'a') as file:
        
        while True:
            # Read a line of text from the USB cable
            if esp32.in_waiting > 0:
                raw_data = esp32.readline()
                
                # Decode the raw bytes into a standard string
                line = raw_data.decode('utf-8').strip()
                
                # Only save lines that look like our CSV data (contains commas)
                if "," in line:
                    file.write(line + '\n')
                    print(f"📝 Logged: {line}")
                    
except serial.SerialException:
    print(f"❌ ERROR: Could not open {COM_PORT}.")
    print("Is the Arduino Serial Monitor still open? Close it and try again!")
except KeyboardInterrupt:
    print("\n Data collection stopped by user.")
    print(f"All data securely saved to {FILE_NAME}")
finally:
    # Close the port cleanly
    if 'esp32' in locals() and esp32.is_open:
        esp32.close()