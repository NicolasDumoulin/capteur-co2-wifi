#include <LittleFS.h>
#include "config.h"

#define DATA_PATH "/data.txt"
#define CONFIG_PATH "/config.txt"

void appendFile(const char * path, const char * message);
void writeFile(const char * path, const char * message);
void readFile(const char * path);

void storeData(int timestep, const char* timestamp, float co2, float temp, float humidity);

void storeConfig(configStruct config);

void readConfig(configStruct* config);

void readData();

void _writeFile(const char * path, const char * message, char* mode);

void deleteFile(const char * path);