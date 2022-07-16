#include "filestorage.h"

// TODO implements functions

void storeData(int timestep, const char* timestamp, float co2, float temp, float humidity) {
  char line[32];
  snprintf(line, sizeof(line), "%d,%s,%f,%f,%f\n", timestep, timestamp, co2, temp, humidity);
  File file = LittleFS.open(DATA_PATH, "a");
  file.print(line);
  delay(1);
  file.close();
}

void storeConfig(configStruct config) {
  char line[64];
  snprintf(line, sizeof(line), "%d\n%d\n%d\n%d\n%s\n", config.co2MeasurementInterval, config.level_medium,
    config.level_high, config.co2AltitudeCompensation, config.ledON ? "true" : "false");
  File file = LittleFS.open(CONFIG_PATH, "w");
  file.print(line);
  delay(1);
  file.close();
}

void readConfig(configStruct* config) {
  Serial.println("Read Config");
  File file = LittleFS.open(CONFIG_PATH, "r");
  config->co2MeasurementInterval = file.readStringUntil('\n').toInt();
  config->level_medium = file.readStringUntil('\n').toInt();
  config->level_high = file.readStringUntil('\n').toInt();
  config->co2AltitudeCompensation = file.readStringUntil('\n').toInt();
  config->ledON = file.readStringUntil('\n') == "true";
}

void readData() {
  Serial.println("Read Data");
  //readFile(DATA_PATH);
}

void _writeFile(const char * path, const char * message, char* mode) {
  /*size_t messageSize = sizeof(message);
  FILE *file = fopen(path, mode); 
  if (!file) {
    Serial.print("Échec à l'ouverture du fichier ");
    Serial.print(path);
    Serial.println(" (appendFile)");
    return;
  }
  if (!fwrite((uint8_t *) message, 1, messageSize, file)) {
    Serial.print("Échec lors de l'écriture dans le fichier ");
    Serial.print(path);
    Serial.print(" (_writeFile mode ");
    Serial.print(mode);
    Serial.println(")");
  } 
  fclose(file);*/
}

void writeFile(const char * path, const char * message) {
  _writeFile(path, message, "w"); 
}

void appendFile(const char * path, const char * message) {
  _writeFile(path, message, "a"); 
}

// TODO return char*
void readFile(const char * path) {
  /*FILE *file = fopen(path, "r");
  if (!file) {
    Serial.print("Échec à l'ouverture du fichier ");
    Serial.print(path);
    Serial.println(" (readFile)");
    return;
  }
  char c;
  uint32_t numRead = 1;
  while (numRead) {
    numRead = fread((uint8_t *) &c, sizeof(c), 1, file);
    /*if (numRead)
      Serial.print(c);*/
  /*}
  fclose(file);*/
}

void deleteFile(const char * path) {
  /*if (!remove(path) == 0) {
    Serial.print("Échec lors de la suppression du fichier ");
    Serial.print(path);
    Serial.println(" (deleteFile)");
  }*/
}
