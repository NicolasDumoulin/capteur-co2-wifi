#include <SPI.h>
#include <WiFiNINA.h>
#include "Webpageindex.h"
#include "chart.js.h"
#include "SparkFun_SCD30_Arduino_Library.h"

int initWifiAP(const char* ssid, const char* password);

/**
 * Initialisation du Wifi
 */
int initWifi(const char* ssid, const char* password, const char* failbackAPssid, const char* failbackAPpassword, const int wifilocal_nb_attempts, const int wifilocal_attempts_wait);

/* Get the next line in c_string s */
char *sgets(char *s, int n, const char **strp);

/* Needed to split large files because of a limitation of print() */
// TODO use StreamLib as described on https://werner.rothschopf.net/microcontroller/202011_arduino_webserver_optimize_en.htm
void printContentLineByLine(WiFiClient* client, const char* content);

enum class Request {INDEX_HTML, DATA, CONFIG_GET, CONFIG_POST, CHARTJS};

int cstringEndsWith(const char *str, const char *suffix);

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String split(String data, char separator, int index);

void serveWeb(WiFiServer* server, SCD30* airSensor, float co2, float temp, float humidity, int* updateIntervall, int* altitude, bool* ledON);

