#include <stdio.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define HTTP_REST_PORT 81
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50


const char* wifi_ssid = "SSID";
const char* wifi_passwd = "psswd";

struct Pump {
    byte id;
    byte port;
    byte status;
} pump_resource;

const size_t capacity = JSON_OBJECT_SIZE(3);

ESP8266WebServer http_rest_server(HTTP_REST_PORT);


void init_pump_resource()
{
    pump_resource.id = 1;
    pump_resource.port = 2;
    pump_resource.status = HIGH;
    pinMode(pump_resource.port, OUTPUT);
    digitalWrite(pump_resource.port, pump_resource.status);
}

int init_wifi() {
    int retries = 0;

    Serial.println("Connecting to WiFi AP..........");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_passwd);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print("#");
    }
    return WiFi.status(); // return the WiFi connection status
}

void get_pump() {
        DynamicJsonDocument  doc(capacity);
        doc["id"] = pump_resource.id;
        doc["port"] = pump_resource.port;
        doc["status"] = pump_resource.status;
        String buf;
        serializeJson(doc, buf);
        http_rest_server.send(200, F("application/json"), buf);
}

void get_pump_on(){
   pump_resource.status = LOW;
   digitalWrite(pump_resource.port, pump_resource.status);
   DynamicJsonDocument  doc(capacity);
   doc["id"] = pump_resource.id;
   doc["port"] = pump_resource.port;
   doc["status"] = pump_resource.status;
   String buf;
   serializeJson(doc, buf);
   http_rest_server.send(200, F("application/json"), buf);
}

void get_pump_off(){
   pump_resource.status = HIGH;
   digitalWrite(pump_resource.port, pump_resource.status);
   DynamicJsonDocument  doc(capacity);
   doc["id"] = pump_resource.id;
   doc["port"] = pump_resource.port;
   doc["status"] = pump_resource.status;
   String buf;
   serializeJson(doc, buf);
   http_rest_server.send(200, F("application/json"), buf);
}

void config_rest_server_routing() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "Welcome to Minimal REST Server Example");
    });
    http_rest_server.on("/pump", HTTP_GET, get_pump);
    http_rest_server.on("/pump/on", HTTP_GET, get_pump_on);
    http_rest_server.on("/pump/off", HTTP_GET, get_pump_off);
}

void setup() {
  Serial.begin(115200);

    init_pump_resource();
    if (init_wifi() == WL_CONNECTED) {
        Serial.print("Connected to ");
        Serial.print(wifi_ssid);
        Serial.print("--- IP: ");
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.print("Error connecting to: ");
        Serial.println(wifi_ssid);
    }

    config_rest_server_routing();

    http_rest_server.begin();
    Serial.println("HTTP REST Server Started");
}

void loop() {
  http_rest_server.handleClient();
}
