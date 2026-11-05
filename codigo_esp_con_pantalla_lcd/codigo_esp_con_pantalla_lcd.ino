#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configurar la pantalla LCD I2C (Dirección 0x27 o 0x3F, tamaño 20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_PASSWORD";

// IP fija para la central (para que los mini ESP32 sepan siempre a dónde enviar los datos)
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

const int totalLugares = 4;           // Cantidad de plazas de la prueba
int estadosPlazas[4] = {0, 0, 0, 0}; // 0 = Libre, 1 = Ocupado

void setup() {
  Serial.begin(115200);

  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print(" Iniciar Sistema... ");
  delay(1500);
  lcd.clear();

  // Configurar IP fija de la central
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Falla al configurar la IP estática");
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando central a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Mostrar IP en pantalla y monitor
  lcd.setCursor(0, 0);
  lcd.print(" Central Conectada  ");
  delay(2000);
  lcd.clear();

  server.on("/actualizar", handleActualizacion);
  server.begin();
  
  // Actualizar la pantalla LCD con el estado inicial
  actualizarPantallaLCD();
}

void loop() {
  server.handleClient();
}

void handleActualizacion() {
  if (server.hasArg("plaza") && server.hasArg("estado")) {
    int plaza = server.arg("plaza").toInt() - 1; // Ajustar índice (0 a 3)
    int estado = server.arg("estado").toInt();

    if (plaza >= 0 && plaza < totalLugares) {
      estadosPlazas[plaza] = estado;

      // Actualizar la pantalla LCD con los nuevos datos
      actualizarPantallaLCD();

      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Plaza fuera de rango");
    }
  } else {
    server.send(400, "text/plain", "Faltan datos en la peticion");
  }
}

void actualizarPantallaLCD() {
  int ocupados = 0;
  for (int i = 0; i < totalLugares; i++) {
    ocupados += estadosPlazas[i];
  }
  int libres = totalLugares - ocupados;

  // Dibujar la información en el cartel de la entrada (LCD 20x4)
  lcd.setCursor(0, 0);
  lcd.print(" ESTACIONAMIENTO    ");
  
  lcd.setCursor(0, 1);
  lcd.print("Libres:   ");
  lcd.print(libres);
  lcd.print("   "); // Espacios para limpiar restos

  lcd.setCursor(0, 2);
  lcd.print("Ocupados: ");
  lcd.print(ocupados);
  lcd.print("   ");

  lcd.setCursor(0, 3);
  if (libres == 0) {
    lcd.print("ESTADO: COMPLETO    ");
  } else {
    lcd.print("ESTADO: DISPONIBLE  ");
  }
}
