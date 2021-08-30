
#include <SPI.h>
#include <WiFiNINA.h>
#include <LittleFS_Mbed_RP2040.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

/************** Web config **************/
bool AP = 0; // mettre à 1 pour passer en AP (Access Point) par défaut sans tenter la connexion au wifi local
const int wifilocal_nb_attempts = 3; // nombre de tentatives de connexion au wifi local
const int wifilocal_attempts_wait = 5; // temps d'attente (secondes) entre chaque tentative
#include "wifi_credentials.h"
// Mettre dans le fichier wifi_credentials.h la définition des 4 variables suivantes
//char ssid[] = "wifiSSID";        // Wifi SSID pour se connecter au réseau local
//char pass[] = "wifiPass";    // Wifi Mot de passe
//char ssidAP[] = "APwifiSSID";        // AP SSID
//char passAP[] = "APwifiSSID"";    // AP Mot de passe
/************** CO2 Sensor config **************/
const int level_medium = 700;
const int level_high = 1000;
int co2MeasurementInterval = 20;
int co2AltitudeCompensation = 393; // Beaumont, 63
/************** Variables **************/
LittleFS_MBED *myFS;
WiFiServer server(80);
SCD30 airSensor;
float co2 = 0.0, temp = 0.0, humidity = 0.0;
int timestep = 0;
char timestamp[] = "";
/************** Init **************/
void setup() {
  Serial.begin(9600);
  delay(1000); // attente de l'initialisation du moniteur série si présent et du capteur
  // Pour forcer l'attente du moniteur série : while (!Serial)
  Wire.begin(); // initialise le bus I2C (capteur CO2)
  // initalise la LED RGB de la carte
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  // Teste si le capteur est détecté
  if (!airSensor.begin()) {
    Serial.println("Capteur de CO2 non détecté. Vérifiez le câblage. Blocage du système …");
    while (1) {
      // Clignotement de la LED rouge
      digitalWrite(LEDR,HIGH);
      delay(500);
      digitalWrite(LEDR, LOW);
      delay(500);
    }
  };
  // initialise le stockage
  myFS = new LittleFS_MBED();
  if (!myFS->init()) {
    Serial.println("Échec de l'initialisation de LITTLEFS");
    while (1);
  }
  // TODO read config from file (if present)
  airSensor.setMeasurementInterval(co2MeasurementInterval);
  airSensor.setAltitudeCompensation(co2AltitudeCompensation);
  int status = initWifi(ssid, pass, ssidAP, passAP, wifilocal_nb_attempts, wifilocal_attempts_wait);
  server.begin();
  printWiFiStatus();
}
/************** LOOP **************/
void loop() {
  if (airSensor.dataAvailable()) {
    co2 = airSensor.getCO2();
    temp = airSensor.getTemperature();
    humidity = airSensor.getHumidity();
    Serial.print("co2(ppm) : ");
    Serial.print(co2);
    Serial.print(" / temp(C) : ");
    Serial.print(temp, 1);
    Serial.print(" / humidité(%) : ");
    Serial.print(humidity, 1);
    Serial.println();
    digitalWrite(LEDR, co2 > level_medium ? HIGH : LOW);
    digitalWrite(LEDG, co2 < level_medium && co2 > 0.0 ? HIGH : LOW);
    digitalWrite(LEDB, co2 > level_medium && co2 < level_high ? HIGH : LOW);
    storeData(timestep, timestamp, co2, temp, humidity);
    timestep++;
    // On attend le temps nécessaire à la prochaine acquisition de données et on écoute les requêtes web pendant ce temps.
    for (int i=0; i<co2MeasurementInterval*100;i++) {
      serveWeb(&server, co2, temp, humidity, &co2MeasurementInterval, &co2AltitudeCompensation);
      delay(10);
    }
  } else {
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDB, LOW);
    Serial.println("En attente de nouvelles données");
    delay(1000);
  }
}
