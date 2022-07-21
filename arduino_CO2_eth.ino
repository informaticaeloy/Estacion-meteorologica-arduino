#include "Adafruit_CCS811.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Ethernet.h>
// the sensor communicates using SPI, so include the library:
#include <SPI.h>

Adafruit_CCS811 ccs;
 
Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

// assign a MAC address for the Ethernet controller.
// fill in your address here:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// assign an IP address for the controller:
IPAddress ip(192, 168, 1, 200);
IPAddress ms(255, 255, 255, 0);
IPAddress gw(192, 168, 1, 1);
IPAddress dns(192, 168, 1, 1);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

String json,solicitud;
float fTemperatura, fHumedad, fPresion;
int iCO2, iVOC;

void setup() {
  Serial.begin(9600);
  
// ********************************* CONFIGURACIÓN DEL SENSOR DE BME280 (TEMPERATURA, HUMEDAD Y PRESIÓN
  if (!bme.begin(0x76)) {
    Serial.println(F("No se ha encontrado el sensor BME280"));
    while (1) delay(10);
  }
  
  bme_temp->printSensorDetails();
  bme_pressure->printSensorDetails();
  bme_humidity->printSensorDetails();


// ********************************* CONFIGURACIÓN DEL EYHERNET SHIELD Y LOS PARÁMETROS DE RED  
  // start the Ethernet connection
  Ethernet.begin(mac, ip, dns, gw, ms);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  // start listening for clients
  server.begin();

// ********************************* CONFIGURACIÓN DEL SENSOR CCS811 (CO2 Y GASES) 
  Serial.println("CCS811 test");

  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // Wait for the sensor to be ready
  while(!ccs.available());
}

void loop() {
  // listen for incoming Ethernet connections:
  listenForEthernetClients();
}

// ********************************* FUNCIÓN QUE ESPERA LA CONEXIÓN DE UN CLIENTE
void listenForEthernetClients()
{ 
  EthernetClient cliente = server.available();
  char c;
  if (cliente.available())
  {
    c = cliente.read();
    Serial.print(c);
    if ( solicitud.length() < 100 )
    {
      solicitud += c; 
    }
  }
  TemHumPre();
  LeeCO2();
  if ( c == '\n' )
  {
    json = "{";
    json += "\"Temperatura\":\"" + (String)fTemperatura + "\",";
    json += "\"Humedad\":\"" + (String)fHumedad + "\",";
    json += "\"Presion\":\"" + (String)fPresion + "\",";
    json += "\"CO2\":\"" + (String)iCO2 + "\",";
    json += "\"VOC\":\"" + (String)iVOC + "\",";
    json += "\"uptime\":\"" + (String)millis() + "\"";
    json += "}\n";
    
    cliente.println("HTTP/1.1 200 OK"); // enviamos cabeceras
    cliente.println("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
    cliente.println("Content-Type: text/javascript");
    cliente.println("Access-Control-Allow-Origin: *");
    cliente.println();
    cliente.println(json); //imprimimos datos
    
    delay(100); // esperamos un poco
    cliente.stop(); //cerramos la conexión
  }
}

// ********************************* FUNCIÓN QUE LEE LA TEMPERATURA, LA HUMEDAD Y LA PRESION
void TemHumPre()
{
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);
  
  Serial.print(F("Temperatura = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");
  
  fTemperatura = temp_event.temperature;
    
  Serial.print(F("Humedad = "));
  Serial.print(humidity_event.relative_humidity);
  Serial.println(" %");

  fHumedad = humidity_event.relative_humidity;
  
  Serial.print(F("Presion = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");

  fPresion = pressure_event.pressure;
  
//  delay(1000);
}

// ********************************* FUNCIÓN QUE LEE EL NIVEL DE CO2
void LeeCO2()
{
  // int iCO2, iTVOC;
  ///delay(10000);
  if(ccs.available())
  {
    if(!ccs.readData())
    {
      Serial.print("CO2: ");
      iCO2 = ccs.geteCO2();
      Serial.print(iCO2);
      Serial.print("ppm, TVOC: ");
      iVOC = ccs.getTVOC();
      Serial.println(iVOC);
    }
    else
    {
      Serial.println("ERROR!");
      while(1);
    }
  }
 // delay(500);
}
