/*
 * LoRa E32-TTL-100
 * Get configuration and receive DHT11 sensor data.
 * E32-TTL-100----- Arduino UNO
 * M0         ----- 3.3v
 * M1         ----- 3.3v
 * TX         ----- RX PIN 11 (PullUP)
 * RX         ----- TX PIN 10 (PullUP & Voltage divider)
 * AUX        ----- Not connected (12)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 */

#include "Arduino.h"
#include "LoRa_E32.h"
#include <SoftwareSerial.h>
#include "DHT.h"

// DHT11 setup
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LoRa setup
SoftwareSerial mySerial(11,10); // RX, TX
// &mySerial: UART
// 12: AUX
// 9: M0
// 8: M1
LoRa_E32 e32ttl100(&mySerial, 12,9,8); 

void receiveData();
void readDHT();
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

// ---------------------------------------------
// Struct for message data (parsed from LoRa)
// ---------------------------------------------
struct Message {
  char name[20];        // Device name
  float temperature;    // Temperature in Celsius
  float humidity;       // Humidity in percentage
  char text[20];        // Optional: raw message
};
Message msg;  // global variable to hold the received data
Message msg1; // global variable to hold the send data

// ---------------------------------------------
// receiveData(): Receive LoRa messages and parse data
// ---------------------------------------------
void receiveData(){
 if (e32ttl100.available() > 1) {
    ResponseContainer rs = e32ttl100.receiveMessage();

    if (rs.status.code == 1) {
      String message = rs.data;  // full received message
      Serial.println("===Received Data===");
      Serial.println("Raw message: " + message);

      // --- Parse name ---
      int colonIndex = message.indexOf(':');
      String nameStr = message.substring(0, colonIndex);
      nameStr.trim();
      nameStr.toCharArray(msg.name, sizeof(msg.name));

      // --- Parse temperature ---
      int tempIndex = message.indexOf("Temp") + 5; 
      int tempEnd = message.indexOf("C", tempIndex);
      String tempStr = message.substring(tempIndex, tempEnd);
      tempStr.trim();
      msg.temperature = tempStr.toFloat();

      // --- Parse humidity ---
      int humIndex = message.indexOf("Hum") + 4;
      int humEnd = message.indexOf("%", humIndex);
      String humStr = message.substring(humIndex, humEnd);
      humStr.trim();
      msg.humidity = humStr.toFloat();

      // --- Print parsed struct data ---
      Serial.println("=== Information ===");
      Serial.print("Name: "); Serial.println(msg.name);
      Serial.print("Temperature: "); Serial.println(msg.temperature);
      Serial.print("Humidity: "); Serial.println(msg.humidity);
      Serial.println("===================");
    } else {
      Serial.print("Receive error: ");
      Serial.println(rs.status.getResponseDescription());
    }
  }
}

// ---------------------------------------------
// sendData(): Read DHT11 and send data via LoRa
// ---------------------------------------------
void sendData() {
    // Read DHT11 and store values into msg1
    msg1.humidity = dht.readHumidity();
    msg1.temperature = dht.readTemperature();
    strcpy(msg1.name, "Node1");  // assign node name

    // Format message string
    String message = String(msg1.name) +
                     ": Temp " + String(msg1.temperature, 1) + " C " +
                     "Hum " + String(msg1.humidity, 1) + " %";

    // Send via LoRa
    ResponseStatus rs = e32ttl100.sendMessage(message);

    if (rs.code == 1) {
        Serial.println("=== Send Data ===");
        Serial.println("Send success: " + message);
    } else {
        Serial.print("Send error: ");
        Serial.println(rs.getResponseDescription());
    }
}


// ---------------------------------------------
// readDHT11(): Read values from DHT11 sensor
// ---------------------------------------------
void readDHT11() {
  float humidity = dht.readHumidity(); 
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }

  // Print humidity and temperature values
  Serial.println("=== Read Data from sensor ===");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
}

// ---------------------------------------------------
// printParameters(): Print LoRa configuration values
// ---------------------------------------------------
void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD BIN: "));  
  Serial.print(configuration.HEAD, BIN);
  Serial.print(" "); Serial.print(configuration.HEAD, DEC);
  Serial.print(" "); Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));

  Serial.print(F("AddH BIN: "));  Serial.println(configuration.ADDH, BIN);
  Serial.print(F("AddL BIN: "));  Serial.println(configuration.ADDL, BIN);
  Serial.print(F("Chan BIN: "));  Serial.print(configuration.CHAN, DEC); 
  Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));

  Serial.print(F("SpeedParityBit BIN    : "));  
  Serial.print(configuration.SPED.uartParity, BIN);
  Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());

  Serial.print(F("SpeedUARTDataRate BIN : "));  
  Serial.print(configuration.SPED.uartBaudRate, BIN);
  Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());

  Serial.print(F("SpeedAirDataRate BIN  : "));  
  Serial.print(configuration.SPED.airDataRate, BIN);
  Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

  Serial.print(F("OptionTrans BIN       : "));  
  Serial.print(configuration.OPTION.fixedTransmission, BIN);
  Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());

  Serial.print(F("OptionPullup BIN      : "));  
  Serial.print(configuration.OPTION.ioDriveMode, BIN);
  Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());

  Serial.print(F("OptionWakeup BIN      : "));  
  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);
  Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());

  Serial.print(F("OptionFEC BIN         : "));  
  Serial.print(configuration.OPTION.fec, BIN);
  Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());

  Serial.print(F("OptionPower BIN       : "));  
  Serial.print(configuration.OPTION.transmissionPower, BIN);
  Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

  Serial.println("----------------------------------------");
}

// ---------------------------------------------------
// printModuleInformation(): Print LoRa module info
// ---------------------------------------------------
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD BIN: "));  
  Serial.print(moduleInformation.HEAD, BIN);
  Serial.print(" "); Serial.print(moduleInformation.HEAD, DEC);
  Serial.print(" "); Serial.println(moduleInformation.HEAD, HEX);

  Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
  Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");
}

// ---------------------------------------------
// setup(): Initialize serial, LoRa, and DHT11
// ---------------------------------------------
void setup() {

  pinMode(12,INPUT_PULLUP); // AUX not used but set as input

  pinMode(8,INPUT_PULLUP);  // M1 = 0
  pinMode(9,INPUT_PULLUP);  // M0 = 0
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);

  Serial.begin(9600);

  delay(1000); // wait for LoRa module to stabilize

  // Start LoRa UART communication
  e32ttl100.begin();

  // Initialize DHT sensor
  dht.begin();

  // Get LoRa module configuration
  ResponseStructContainer c;
  c = e32ttl100.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;

  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  // Print default parameters
  printParameters(configuration);

  // Set custom configuration values
  configuration.ADDL = 0x00;
  configuration.ADDH = 0x00;
  configuration.CHAN = 0x06;

  configuration.OPTION.fec = FEC_1_ON;
  configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
  configuration.OPTION.transmissionPower = POWER_20;  
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;

  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;

  // Write configuration to LoRa module
  ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);

  // Print new parameters
  printParameters(configuration);
  c.close();
}

// ---------------------------------------------
// loop(): main execution loop
// ---------------------------------------------
void loop() {
    receiveData();            // Receive LoRa message
    delay(2000);              // Wait 2 seconds before next receive

    readDHT11();                // Read DHT11 sensor
    delay(2000);              // Wait 2 seconds to avoid reading too frequently

    sendData();        // Send data via LoRa
    delay(5000);              // Wait 5 seconds before sending again
}
