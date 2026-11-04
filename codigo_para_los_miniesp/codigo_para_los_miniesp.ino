#include <WiFi.h>
#include <HTTPClient.h> // 

const char* ssid     = "RED_WIFI";
const char* password = "PASSWORD";

// IP del ESP32 principal (central) en la red local
const char* servidorCentral = "http://192.168.1.100/actualizar"; 

const int pinSensor = 4; // Pin digital donde conecto el OUT del sensor infrarrojo
const int idPlaza = 1;   // Cambiar el número según  plaza (1, 2, 3, etc.)

int estadoAnterior = -1; // Para detectar solo cuando hay un cambio

void setup() {
  Serial.begin(115200);
  pinMode(pinSensor, INPUT);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red!");
}

void loop() {
  int lecturaActual = digitalRead(pinSensor);
  
  // LOW = Objeto detectado (Ocupado), HIGH = Libre
  int estadoActual = (lecturaActual == LOW) ? 1 : 0;

  // Solo actúa si el estado CAMBIÓ respecto al ciclo anterior
  if (estadoActual != estadoAnterior) {
    estadoAnterior = estadoActual; // Actualiza el estado guardado

    Serial.print("¡Cambio detectado en Plaza ");
    Serial.print(idPlaza);
    Serial.print("! Nuevo estado: ");
    Serial.println(estadoActual == 1 ? "OCUPADO" : "LIBRE");

    // Enviar la información al ESP32 central por Wi-Fi si hay conexión
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      // Creamos una URL con los datos (ej: http://192.168.1.100/actualizar?plaza=1&estado=1)
      String url = String(servidorCentral) + "?plaza=" + idPlaza + "&estado=" + estadoActual;
      
      http.begin(url);
      int codigoRespuesta = http.GET();
      
      if (codigoRespuesta > 0) {
        Serial.println("Aviso enviado con éxito al central.");
      } else {
        Serial.println("Error al enviar el aviso.");
      }
      http.end();
    }
  }

  // Pequeña pausa para evitar lecturas falsas por rebotes rápidos
  delay(500); 
}
