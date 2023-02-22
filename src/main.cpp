/*
Este programa apaga un LED por un tiempo y luego hace un fade de subida
y bajada por un tiempo especificado por el usuario. El usuario configura
un MQTT con un JSON de ambos tiempo. Los lee, hace el bucle y publica en 
MQTT los valores de los tiempos transcurridos
*/


//IMPORTACIÓN DE MÓDULOS
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SerialCommand.h>
#include <WiFi.h>
#include <PubSubClient.h>

SerialCommand sCmd;

//Declaración de constantes necesarias para conectarse al MQTT
const char* ssid = "sercommBB5621";
const char* password =  "9746Y8BTSJTMFKMX";
const char* mqtt_server = "broker.eqmx.io";
const int port = 1883;
const char* mqtt_username = "IRENERG";
const char* mqtt_password = "holahola12_";

WiFiClient esp32Client;
PubSubClient client(esp32Client);


//Declaración de constantes para el LED
unsigned int tiempoApagado = 1000;
unsigned int tiempoFade = 1000;
float delayIT = tiempoFade/(255*2);
int freq = 5000;
int ledChannel = 0;
int resolution = 8;

//COMANDOS PARA LA PLACA
int cmdTiempo(char* param, uint8_t len, char* response) {
    sprintf(response, "El tiempo de apagado es %d  y el de FADE es %d", tiempoApagado, tiempoFade);
    return strlen(response);
}

int cmdFrecuencia(char* param, uint8_t len, char* response) {
    sprintf(response, "La frecuencia es %d", freq);
    return strlen(response);
}

int cmdLED(char* param, uint8_t len, char* response) {
    sprintf(response, "El LED está en el canal %d", ledChannel);
    return strlen(response);
}

void CommBegin() {
    sCmd.addCommand("T", cmdTiempo);
    sCmd.addCommand("F", cmdFrecuencia);
    sCmd.addCommand("L", cmdLED);
}

//Función callback para recibir los tiempos

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Mensaje recibido del topic: ");
  Serial.println(topic);
 
  Serial.print("Mensaje:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
}

//FUNCIÓN PARA CONECTARSE A LA WIFI
void wifi() {
  //Se conecta a la wifi definida al principio
  WiFi.begin(ssid, password);
  //mientras se conecta nos manda un mensaje
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Conectandose a la WiFi...");
  }
  //una vez conectado, lo indica
  Serial.println("Conectado a la red WiFi");
}

//FUNCIÓN PARA CONECTARSE AL SERVIDOR MQTT
void mqtt_connect() {
  while (!client.connected()) {
    String client_id = "pruebaesp32-";
    client_id += String(WiFi.macAddress());

    Serial.printf("El cliente %s se está conectando... \n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Se ha conectado al servidor MQTT");
    } 
    else {
        Serial.print("Error con el estado de cliente ");
        Serial.println(client.state());
        delay(2000);
    }
  }
}



//FUNCIÓN PARA ENVIAR UN JSON CON LOS TIEMPOS QUE TENDRÁ EL BUCLE
void sendjason() {
  //Mensaje Jason
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
 
  JSONencoder["Dispositivo"] = "ESP32";
  JSONencoder["OUTPUT"] = "LED en la placa";
  JsonArray& apagado = JSONencoder.createNestedArray("Duración de apagado");
  apagado.add(tiempoApagado);
  JsonArray& fade = JSONencoder.createNestedArray("Duración de Fade");
  fade.add(tiempoFade);

  Serial.println("Los tiempos de apagado y fade son: ");
  JSONencoder.prettyPrintTo(Serial);
  Serial.println();

  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Enviando mensaje a MQTT..");
  Serial.println(JSONmessageBuffer);

  if (client.publish("tiempoOUTPUT", JSONmessageBuffer) == true) {
    Serial.println("Se ha enviado el mensaje");
  } else {
    Serial.println("Error enviando el mensaje");
  }

}

void setup() {
  Serial.begin(115200);
  wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
  delay(1000);
  CommBegin();
  sendjason();
  Serial.println("-------------");
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LED_BUILTIN, ledChannel);
}

//FUNCIÓN QUE HACE EL FADE EN EL TIEMPO ESTABLECIDO
void FADE() {
  Serial.println("SUBIENDO INTENSIDAD");
  for (int intensidad = 0; intensidad <= 255; intensidad++) {

    ledcWrite(ledChannel, intensidad);
    delay(delayIT);
  }
  Serial.println("BAJANDO INTENSIDAD");
  for (int intensidad = 255; intensidad >= 0; intensidad--){
    ledcWrite(ledChannel, intensidad);
    delay(delayIT);
  }
}

void loop() {
  //Funcionamiento del LED
  Serial.println("Apagado");
  ledcWrite(ledChannel, LOW);
  delay(tiempoApagado);
  FADE(); 
}