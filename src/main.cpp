/*
Este programa apaga un LED por un tiempo y luego hace un fade de subida
y bajada por un tiempo especificado por el usuario. El usuario configura
un MQTT con un JSON de ambos tiempo. Los lee, hace el bucle y publica en 
MQTT los valores de los tiempos transcurridos
*/


//IMPORTACIÓN DE MÓDULOS
#include <Arduino.h>
#include <ArduinoJson.h>

//Declaración de constantes para el LED
unsigned int tiempoApagado = 5000;
unsigned int tiempoFade = 5000;
float delayIT = tiempoFade/(255*2);
int freq = 5000;
int ledChannel = 0;
int resolution = 8;


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