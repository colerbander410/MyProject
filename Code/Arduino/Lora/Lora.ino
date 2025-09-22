/*
 * LoRa E32-TTL-100
 * Get configuration.
 * https://www.mischianti.org/2019/10/29/lora-e32-device-for-arduino-esp32-or-esp8266-configuration-part-3/
 *
 * E32-TTL-100----- Arduino UNO
 * M0         ----- 3.3v
 * M1         ----- 3.3v
 * TX         ----- RX PIN 11 (PullUP)
 * RX         ----- TX PIN 10 (PullUP & Voltage divider)
 * AUX        ----- Not connected (12)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(11,10); // RX, TX

// &mySerial: UART
// 12: AUX
// 9: M0
// 8: M1
LoRa_E32 e32ttl100(&mySerial, 12,9,8); 
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {

  pinMode(12,INPUT_PULLUP);// ouput_AUX not used 

  pinMode(8,INPUT_PULLUP);// M1=0
  pinMode(9,INPUT_PULLUP);// M0=0
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);
	Serial.begin(9600);


	delay(1000); // wait for Lora 

	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);
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

	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);
	printParameters(configuration);
	c.close();
}
// struct definition
struct Message {
  char name[20];        // name, better to use char[] instead of int
  float temperature;
  float humidity;
  char text[20];        // optional: keep the raw message
};

Message msg;  // global variable to hold the data

void loop() {
  if (e32ttl100.available() > 1) {
    ResponseContainer rs = e32ttl100.receiveMessage();

    if (rs.status.code == 1) {
      String message = rs.data;  // full message
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

      // --- Print struct data ---
      Serial.println("=== Struct Data ===");
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



void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD BIN: "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH BIN: "));  Serial.println(configuration.ADDH, BIN);
	Serial.print(F("AddL BIN: "));  Serial.println(configuration.ADDL, BIN);
	Serial.print(F("Chan BIN: "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit BIN    : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDataRate BIN : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
	Serial.print(F("SpeedAirDataRate BIN  : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

	Serial.print(F("OptionTrans BIN       : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
	Serial.print(F("OptionPullup BIN      : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
	Serial.print(F("OptionWakeup BIN      : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
	Serial.print(F("OptionFEC BIN         : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
	Serial.print(F("OptionPower BIN       : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

	Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN);Serial.print(" ");Serial.print(moduleInformation.HEAD, DEC);Serial.print(" ");Serial.println(moduleInformation.HEAD, HEX);

	Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}
