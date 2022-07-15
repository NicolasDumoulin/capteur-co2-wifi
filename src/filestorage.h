
// Mettre entre 1 et 4 pour avoir plus de logs sur le port série
#define _LFS_LOGLEVEL_          0
#define RP2040_FS_SIZE_KB       256

#include <LittleFS_Mbed_RP2040.h>
#define FORCE_REFORMAT false // default value
#define DATA_PATH MBED_LITTLEFS_FILE_PREFIX "/data.txt"
#define CONFIG_PATH MBED_LITTLEFS_FILE_PREFIX "/config.txt"

void appendFile(const char * path, const char * message);
void writeFile(const char * path, const char * message);
void readFile(const char * path);

void storeData(int timestep, const char* timestamp, float co2, float temp, float humidity);

void storeConfig(int updateIntervall, int level_medium, int level_high, int co2AltitudeCompensation);

void readConfig();

void readData();

void _writeFile(const char * path, const char * message, char* mode);

void deleteFile(const char * path);