#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== DATOS A COMPLETAR =====
const char* WIFI_SSID = "Estudiantes";
const char* WIFI_PASS = "educar_2018";
const char* BOT_TOKEN = "8696580398:AAEtB2K2pTBC7pj8Xsc-dy0GRXU3kNvkiQ0";
const char* CHAT_ID   = "6260469418";
// =============================

#define SENSOR_ENTRADA 25
#define SENSOR_SALIDA 35
#define SENSOR_LUGAR1 33
#define SENSOR_LUGAR2 32
#define SENSOR_LUGAR3 34
#define SERVO_ENTRADA 18
#define SERVO_SALIDA 19

Servo servoEntrada;
Servo servoSalida;

LiquidCrystal_I2C lcd(0x27, 20, 4);

WiFiClientSecure clienteTelegram;
UniversalTelegramBot bot(BOT_TOKEN, clienteTelegram);

bool entradaProcesada = false;
bool salidaProcesada = false;
int lugaresOcupados = 0;
int lugaresLibres = 3;

// Para avisar solo cuando algo CAMBIA
bool ocupadoAntes[3] = {false, false, false};
int libresAntes = 3;

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long inicio = millis();
  // Espera máximo 15 s, así las barreras funcionan aunque no haya WiFi
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nNo se pudo conectar al WiFi");
  }
}

void enviar(String texto) {
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, texto, "");
  }
}

void actualizarLCD() {
  lcd.setCursor(0, 1);
  lcd.print("LUGARES LIBRES: ");
  lcd.print(lugaresLibres);

  lcd.setCursor(0, 2);
  lcd.print("LUGARES OCUPADOS: ");
  lcd.print(lugaresOcupados);
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESTACIONAMIENTO");
  lcd.setCursor(0, 3);
  lcd.print(" ");
  actualizarLCD();

  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);
  pinMode(SENSOR_LUGAR1, INPUT);
  pinMode(SENSOR_LUGAR2, INPUT);
  pinMode(SENSOR_LUGAR3, INPUT);

ESP32PWM::allocateTimer(0);
ESP32PWM::allocateTimer(1);

servoEntrada.setPeriodHertz(50);
servoSalida.setPeriodHertz(50);
servoEntrada.attach(SERVO_ENTRADA, 500, 2400);
servoSalida.attach(SERVO_SALIDA, 500, 2400);
  servoEntrada.attach(SERVO_ENTRADA);
  servoSalida.attach(SERVO_SALIDA);
  servoEntrada.write(90);
  servoSalida.write(90);
  delay(2000);
  servoEntrada.write(0);
  servoSalida.write(0);
  delay(2000);

  conectarWiFi();
  clienteTelegram.setInsecure();
  enviar("ARGOS conectado. Lugares libres: 3");

  Serial.println("ESTACIONAMIENTO INICIADO");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }

  int entrada = digitalRead(SENSOR_ENTRADA);
  int salida = digitalRead(SENSOR_SALIDA);

  int lugares[3] = {
    digitalRead(SENSOR_LUGAR1),
    digitalRead(SENSOR_LUGAR2),
    digitalRead(SENSOR_LUGAR3)
  };

  lugaresOcupados = 0;
  for (int i = 0; i < 3; i++) {
    bool ocupado = (lugares[i] == LOW);
    if (ocupado) lugaresOcupados++;

    if (ocupado != ocupadoAntes[i]) {
      ocupadoAntes[i] = ocupado;
      enviar(String("Lugar ") + (i + 1) + (ocupado ? " ocupado" : " liberado"));
    }
  }

  lugaresLibres = 3 - lugaresOcupados;

  // Actualiza la pantalla solo si cambió la cantidad
  if (lugaresLibres != libresAntes) {
    actualizarLCD();
  }

  if (lugaresLibres == 0 && libresAntes != 0) {
    enviar("Estacionamiento LLENO. No hay lugares disponibles");
  }
  if (lugaresLibres > 0 && libresAntes == 0) {
    enviar(String("Se liberó un lugar. Libres: ") + lugaresLibres);
  }
  libresAntes = lugaresLibres;

  Serial.print("Entrada: ");
  Serial.print(entrada);
  Serial.print(" | Salida: ");
  Serial.print(salida);
  Serial.print(" | Lugar 1: ");
  Serial.print(lugares[0]);
  Serial.print(" | Lugar 2: ");
  Serial.print(lugares[1]);
  Serial.print(" | Lugar 3: ");
  Serial.print(lugares[2]);
  Serial.print(" | Libres: ");
  Serial.println(lugaresLibres);

  controlarEntrada(entrada);
  controlarSalida(salida);

  delay(200);
}

void controlarEntrada(int sensor) {
  if (sensor == LOW && !entradaProcesada) {
    if (lugaresLibres > 0) {
      Serial.println("AUTO ENTRADA - ABRIENDO BARRERA");
      servoEntrada.write(90);
      entradaProcesada = true;
    } else {
      Serial.println("ESTACIONAMIENTO LLENO - BARRERA CERRADA");
      
    }
  }

  if (sensor == HIGH && entradaProcesada) {
    delay(1000);
    servoEntrada.write(0);
    entradaProcesada = false;
    Serial.println("AUTO PASO - CERRANDO BARRERA ENTRADA");
    //lcd.print("AUTO PASO - CERRANDO BARRERA ENTRADA ");
  }
}

void controlarSalida(int sensor) {
  if (sensor == LOW && !salidaProcesada) {
    Serial.println("AUTO SALIDA - ABRIENDO BARRERA");
    //lcd.print("AUTO SALIDA - ABRIENDO BARRERA ");
    servoSalida.write(90);
    salidaProcesada = true;
  }

  if (sensor == HIGH && salidaProcesada) {
    delay(1000);
    servoSalida.write(0);
    salidaProcesada = false;
    Serial.println("AUTO PASO - CERRANDO BARRERA SALIDA");
    //lcd.print("AUTO PASO - CERRANDO BARRERA SALIDA ");
  }
}
