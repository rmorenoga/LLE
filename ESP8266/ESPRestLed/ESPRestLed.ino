#include <stdio.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

struct Led {
    byte id;
    byte gpio;
    byte status;
} led_resource;

const char* wifi_ssid = "SSID";
const char* wifi_passwd = "psswd";

const size_t capacity = JSON_OBJECT_SIZE(3);

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

void init_led_resource()
{
    led_resource.id = 0;
    led_resource.gpio = 0;
    led_resource.status = LOW;
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

void get_leds() {
    DynamicJsonDocument  doc(capacity);

    if (led_resource.id == 0)
        http_rest_server.send(204);
    else {
        doc["id"] = led_resource.id;
        doc["gpio"] = led_resource.gpio;
        doc["status"] = led_resource.status;
        String buf;
        //json_to_resource(doc);
        serializeJson(doc, buf);
        http_rest_server.send(200, F("application/json"), buf);
    }
}


// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += http_rest_server.uri();
  message += "\nMethod: ";
  message += (http_rest_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += http_rest_server.args();
  message += "\n";
  for (uint8_t i = 0; i < http_rest_server.args(); i++) {
    message += " " + http_rest_server.argName(i) + ": " + http_rest_server.arg(i) + "\n";
  }
  http_rest_server.send(404, "text/plain", message);
}
void json_to_resource(DynamicJsonDocument& doc) {
    int id, gpio, status;

    id = doc["id"];
    gpio = doc["gpio"];
    status = doc["status"];

    Serial.println(id);
    Serial.println(gpio);
    Serial.println(status);

    led_resource.id = id;
    led_resource.gpio = gpio;
    led_resource.status = status;
}

void post_put_leds() {
    DynamicJsonDocument doc(capacity + 20);
    String post_body = http_rest_server.arg("plain");
    Serial.println(post_body);

    //JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    DeserializationError err = deserializeJson(doc, post_body);
    Serial.print("HTTP Method: ");
    Serial.println(http_rest_server.method());
    
    if (err) {
        Serial.println("error in parsin json body");
        http_rest_server.send(400);
    }
    else {   
        if (http_rest_server.method() == HTTP_POST) {
            if ((doc["id"] != 0) && (doc["id"] != led_resource.id)) {
                json_to_resource(doc);
                http_rest_server.sendHeader("Location", "/leds/" + String(led_resource.id));
                http_rest_server.send(201);
                pinMode(led_resource.gpio, OUTPUT);
            }
            else if (doc["id"] == 0)
              http_rest_server.send(404);
            else if (doc["id"] == led_resource.id)
              http_rest_server.send(409);
        }
        else if (http_rest_server.method() == HTTP_PUT) {
            if (doc["id"] == led_resource.id) {
                json_to_resource(doc);
                http_rest_server.sendHeader("Location", "/leds/" + String(led_resource.id));
                http_rest_server.send(200);
                digitalWrite(led_resource.gpio, led_resource.status);
            }
            else
              http_rest_server.send(404);
        }
    }
}

void delete_leds(){
  DynamicJsonDocument doc(capacity + 20);
    String post_body = http_rest_server.arg("plain");
    Serial.println(post_body);

    //JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    DeserializationError err = deserializeJson(doc, post_body);
    Serial.print("HTTP Method: ");
    Serial.println(http_rest_server.method());
    
    if (err) {
        Serial.println("error in parsin json body");
        http_rest_server.send(400);
    }
    else {   
  if (doc["id"] == led_resource.id) {              
                http_rest_server.sendHeader("Location", "/leds/" + String(led_resource.id));
                http_rest_server.send(200);
                led_resource.id = 0;
            }
            else
              http_rest_server.send(404);
      }
 }


void config_rest_server_routing() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "Welcome to the ESP8266 REST Web Server");
    });
    http_rest_server.on("/leds", HTTP_GET, get_leds);
    http_rest_server.on("/leds", HTTP_POST, post_put_leds);
    http_rest_server.on("/leds", HTTP_PUT, post_put_leds);
    http_rest_server.on("/leds", HTTP_DELETE, delete_leds);
}

void setup(void) {
    Serial.begin(115200);

    init_led_resource();
    if (init_wifi() == WL_CONNECTED) {
        Serial.print("Connetted to ");
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

void loop(void) {
    http_rest_server.handleClient();
}
