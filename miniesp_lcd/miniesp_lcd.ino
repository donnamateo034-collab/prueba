#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_PASSWORD";

// IP del ESP32 central con la pantalla LCD
const char* servidorCentral = "http://192.168.1.100/actualizar"; 

const int pinSensor = 4; // Pin digital del mini ESP32 conectado al OUT del sensor TCRT5000
const int idPlaza = 1;   // Cambiar este número según la plaza (1, 2, 3, etc. para cada nodo)

int estadoAnterior = -1; 

void setup() {
  Serial.begin(115200);
  pinMode(pinSensor, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando nodo a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nNodo conectado al Wi-Fi con éxito!");
}

void loop() {
  int lecturaActual = digitalRead(pinSensor);
  
  // Lógica típica del sensor IR TCRT5000: 
  // LOW (0) = Objeto detectado (Ocupado), HIGH (1) = Libre.
  int estadoActual = (lecturaActual == LOW) ? 1 : 0;

  // Solo actúa y transmite si el estado CAMBIÓ respecto al ciclo anterior
  if (estadoActual != estadoAnterior) {
    estadoAnterior = estadoActual;

    Serial.print("¡Cambio en Plaza ");
    Serial.print(idPlaza);
    Serial.print("! Estado: ");
    Serial.println(estadoActual == 1 ? "OCUPADO" : "LIBRE");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(servidorCentral) + "?plaza=" + idPlaza + "&estado=" + estadoActual;
      
      http.begin(url);
      int codigoRespuesta = http.GET();
      
      if (codigoRespuesta > 0) {
        Serial.println("Aviso enviado correctamente a la central.");
      } else {
        Serial.println("Error al enviar el aviso.");
      }
      http.end();
    }
  }

  delay(500); // Pausa para evitar rebotes eléctricos
}
