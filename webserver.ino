#include <SPI.h>
#include <WiFiNINA.h>
#include "Webpageindex.h"
#include "chart.js.h"

/**
 * Initialisation du Wifi
 */
int initWifi(const char* ssid, const char* password, const char* failbackAPssid, const char* failbackAPpassword, const int wifilocal_nb_attempts, const int wifilocal_attempts_wait) {
  int status = WL_IDLE_STATUS;
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print("Current Firmware version: ");
    Serial.println(fv);
    Serial.println("Please upgrade the firmware");
  }
  int attempts = 0;
  // attempt to connect to WiFi network:
  while ((status != WL_CONNECTED) && (attempts < wifilocal_nb_attempts)) {
    Serial.print("Tentative de connexion au SSID : ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 3 seconds for connection:
    for (int i=0; i < wifilocal_attempts_wait; i++) {
      digitalWrite(LEDR,HIGH);
      delay(800);
      digitalWrite(LEDR,LOW);
      delay(200);
    }
    attempts++;
  }
  if (attempts == wifilocal_nb_attempts) {
    Serial.println("Échec de la connexion. Démarrage en mode Point d'Accès.");
    status = initWifiAP(failbackAPssid, failbackAPpassword);
  }
  return status;
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

int initWifiAP(const char* ssid, const char* password) {
  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  // WiFi.config(IPAddress(10, 0, 0, 1));
  // print the network name (SSID);
  Serial.print("Création du point d'accès : ");
  Serial.println(ssid);
  // Create open network. Change this line if you want to create an WEP network:
  int status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING) {
    Serial.println("Échec de la création du point d'accès");
    // don't continue
    while (true);
  }
  // wait 10 seconds for connection:
  delay(10000);
  return status;
}

/* Get the next line in c_string s */
char *sgets(char *s, int n, const char **strp){
    if(**strp == '\0')return NULL;
    int i;
    for(i=0;i<n-1;++i, ++(*strp)){
        s[i] = **strp;
        if(**strp == '\0')
            break;
        if(**strp == '\n'){
            s[i+1]='\0';
            ++(*strp);
            break;
        }
    }
    if(i==n-1)
        s[i] = '\0';
    return s;
}

/* Needed to split large files because of a limitation of print() */
// TODO use StreamLib as described on https://werner.rothschopf.net/microcontroller/202011_arduino_webserver_optimize_en.htm
void printContentLineByLine(WiFiClient* client, const char* content) {
  char buff[255];
  const char **p = &content;
  while(NULL!=sgets(buff, sizeof(buff), p))
      client->print(buff);
}

enum class Request {INDEX_HTML, DATA, CONFIG_GET, CONFIG_POST, CHARTJS};

int cstringEndsWith(const char *str, const char *suffix) {
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String split(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void serveWeb(WiFiServer* server, float co2, float temp, float humidity, int* updateIntervall, int* altitude) {
  WiFiClient client = server->available();
  if (client) {                             // if you get a client,
    String currentLine = "";                // make a String to hold incoming data from the client
    //char* currentLine = new char[50];                
    //int currentLinePos = 0;
    Request request = Request::INDEX_HTML;
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (currentLine.endsWith("GET /data")) {
        //if (cstringEndsWith(currentLine, "GET /data")) {
          request = Request::DATA;
        } else if (currentLine.endsWith("GET /config")) {
        //} else if (cstringEndsWith(currentLine, "GET /config")) {
          request = Request::CONFIG_GET;
        } else if (currentLine.endsWith("POST /config")) {
        //} else if (cstringEndsWith(currentLine, "POST /config")) {
          request = Request::CONFIG_POST;
        } else if (currentLine.endsWith("GET /chart.js")) {
        //} else if (cstringEndsWith(currentLine, "GET /chart.js")) {
          request = Request::CHARTJS;
        }
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
          //if (strlen(currentLine) == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // the content of the HTTP response follows the header:
            if (request == Request::DATA) {
              String data2send = "{\"COO\":\"" + String(co2)
               + "\", \"Temperature\":\"" + String(temp)
               + "\", \"Humidity\":\"" + String(humidity) + "\"}";
              printContentLineByLine(&client, data2send.c_str());
            } else if (request == Request::CONFIG_GET) {
              String data2send = "{\"updateIntervall\":\"" + String(*updateIntervall) + "\""
                  +",\"altitude\":\"" + String(*altitude) + "\""
                  + "}";
              printContentLineByLine(&client, data2send.c_str());
            } else if (request == Request::CONFIG_POST) {
              // lecture de la ligne de données POST jusqu'au caractère "/" (à envoyer ainsi depuis la requête)
              c='.';
              String postdata = "";
              while (c!='/') {
                c = client.read();
                postdata += c;
              }
              postdata.remove(postdata.length()-1);
              // parcours des paramètres présents dans les données, séparés par des ","
              String postelt, postvariable, postvalue;
              int postcpt = 0;
              while ((postelt = split(postdata,',',postcpt)) != "") {
                postcpt++;
                postvariable = split(postelt,'=',0);
                postvalue = split(postelt,'=',1);
                if (postvariable == "updateIntervall") {
                  if (*updateIntervall != postvalue.toInt()) {
                    *updateIntervall = postvalue.toInt();
                    airSensor.setMeasurementInterval(*updateIntervall);
                    Serial.print("Variable updateIntervall mise à jour avec : ");
                    Serial.println(postvalue);
                  }
                } else if (postvariable == "altitude") {
                  if (*altitude != postvalue.toInt()) {
                    *altitude = postvalue.toInt();
                    airSensor.setAltitudeCompensation(*altitude);
                    Serial.print("Variable altitude mise à jour avec : ");
                    Serial.println(postvalue);
                  }
                }
              }
              // TODO read POST data and update interval in main
            } else if (request == Request::CHARTJS) {
              printContentLineByLine(&client, CHARTJS);
            } else {
              printContentLineByLine(&client, MAIN_page);
            }
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            //currentLinePos = 0;
            //memset(&currentLine[0], 0, sizeof(currentLine));
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          //currentLine[currentLinePos++] = c;
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
  }
  client.stop();
}
