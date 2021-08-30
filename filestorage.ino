// Mettre entre 1 et 4 pour avoir plus de logs sur le port série
#define _LFS_LOGLEVEL_          0
#define RP2040_FS_SIZE_KB       256

#include <LittleFS_Mbed_RP2040.h>
#define FORCE_REFORMAT false // default value
#define DATA_PATH MBED_LITTLEFS_FILE_PREFIX "/data.txt"
#define CONFIG_PATH MBED_LITTLEFS_FILE_PREFIX "/config.txt"

void storeData(int timestep, const char* timestamp, float co2, float temp, float humidity) {
  char line[32];
  snprintf(line, sizeof(line), "%d,%s,%f,%f,%f\n", timestep, timestamp, co2, temp, humidity);
  appendFile(DATA_PATH, line);
}

void storeConfig(int updateIntervall, int level_medium, int level_high, int co2AltitudeCompensation) {
  char line[64];
  snprintf(line, sizeof(line), "%d\n%d\n%d\n%d\n", updateIntervall, level_medium, level_high, co2AltitudeCompensation);
  writeFile(DATA_PATH, line);
}

void _writeFile(const char * path, const char * message, char* mode) {
  size_t messageSize = sizeof(message);
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
  fclose(file);
}

void writeFile(const char * path, const char * message) {
  _writeFile(path, message, "w"); 
}

void appendFile(const char * path, const char * message) {
  _writeFile(path, message, "a"); 
}

// TODO return char*
void readFile(const char * path) {
  FILE *file = fopen(path, "r");
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
    if (numRead)
      Serial.print(c);
  }
  fclose(file);
}

void deleteFile(const char * path) {
  if (!remove(path) == 0) {
    Serial.print("Échec lors de la suppression du fichier ");
    Serial.print(path);
    Serial.println(" (deleteFile)");
  }
}
