//Lib
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_INA219.h>



// DHT11 setup
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(9600);
  // Initialize DHT
  dht.begin();
}

void loop() {

  readDHT11();
  
  Serial.println("-----------------------------");
  delay(2000);
}

// read value in DHT11
void readDHT11() {
  // read humid and temp
  float humidity = dht.readHumidity(); 
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }

  // print humid and temp
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("*C");
}


