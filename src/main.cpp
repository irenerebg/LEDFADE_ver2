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
SerialCommand sCmd;

//Declaración de constantes para el LED
unsigned int tiempoApagado = 1000;
unsigned int tiempoFade = 1000;
float delayIT = tiempoFade/(255*2);
int freq = 5000;
int ledChannel = 0; //placa D1 R32 es 0, placa esp32 c3 devkitm 1 es
int resolution = 8;

//COMANDOS PARA LA PLACA
#define START_CHAR '$'
#define END_CHAR ';'

int cmdTiempo(char* param, uint8_t len, char* response) {
    sprintf(response, "$El tiempo de apagado es %d  y el de FADE es %d;", tiempoApagado, tiempoFade);
    return strlen(response);
}

int cmdFrecuencia(char* param, uint8_t len, char* response) {
    sprintf(response, "$La frecuencia es %d;", freq);
    return strlen(response);
}

int cmdLED(char* param, uint8_t len, char* response) {
    sprintf(response, "$El LED está en el canal %d;", ledChannel);
    return strlen(response);
}

void CommBegin() {
    sCmd.addCommand("T", cmdTiempo);
    sCmd.addCommand("F", cmdFrecuencia);
    sCmd.addCommand("L", cmdLED);
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
}

void setup() {
  Serial.begin(115200);
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

void processcommand() {
    char c;
    char response[100];
    static char cmd_buffer[100];
    static unsigned int cmd_p = 0;

    while (Serial.available()) {
        c = char(Serial.read());

        if (c == START_CHAR)
            cmd_p = 0;

        cmd_buffer[cmd_p] = c;
        cmd_p += 1;
        cmd_p %= 20;

        if (c == END_CHAR) {
            cmd_buffer[cmd_p] = 0;
            cmd_p = 0;
            sCmd.processCommand(cmd_buffer, response);
            Serial.println(response);
        }
    }
}


void loop() {
  //Funcionamiento del LED
  Serial.println("Apagado");
  ledcWrite(ledChannel, LOW);
  delay(tiempoApagado);
  FADE();
  //Comandos de datos
  processcommand();
}