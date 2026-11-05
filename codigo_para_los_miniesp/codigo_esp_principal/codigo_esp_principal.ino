#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "RED_WIFI";
const char* password = "PASSWORD";

// IP fija opcional para la central (para asegurarnos de que siempre sea la 192.168.1.100)
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80); // Creamos un servidor web en el puerto 80

const int totalLugares = 4;           // Se supone una prueba con 4 lugares
int estadosPlazas[4] = {0, 0, 0, 0}; // Memoria local: 0 = Libre, 1 = Ocupado

void setup() {
  Serial.begin(115200);

  // Configuramos IP fija para que la central nunca cambie de dirección
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Falla al configurar la IP estática");
  }

  // Conectamos la central al Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando central a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nCentral conectada con éxito!");
  Serial.print("IP del Central: ");
  Serial.println(WiFi.localIP()); // Aquí mostrará 192.168.1.100

  // Definimos qué función se ejecuta cuando un mini ESP32 visite la ruta /actualizar
  server.on("/actualizar", handleActualizacion);
  
  server.begin();
  Serial.println("Servidor central listo y escuchando a los nodos.");
}

void loop() {
  server.handleClient(); // Mantiene el servidor activo atendiendo mensajes entrantes
}

void handleActualizacion() {
  // Verificamos si la petición del nodo trae los datos necesarios ("plaza" y "estado")
  if (server.hasArg("plaza") && server.hasArg("estado")) {
    int plaza = server.arg("plaza").toInt() - 1; // Ajustamos el índice (ej: plaza 1 pasa a posición 0)
    int estado = server.arg("estado").toInt();

    // Validamos que el número de plaza esté dentro del rango correcto
    if (plaza >= 0 && plaza < totalLugares) {
      estadosPlazas[plaza] = estado; // Actualizamos el estado en la memoria central

      // Recalculamos cuántos lugares están ocupados y libres
      int ocupados = 0;
      for (int i = 0; i < totalLugares; i++) {
        ocupados += estadosPlazas[i];
      }
      int libres = totalLugares - ocupados;

      // Imprimimos el estado actualizado en el Monitor Serie de la PC
      Serial.println("\n--- ACTUALIZACIÓN DE ESTACIONAMIENTO ---");
      Serial.print("Plaza "); Serial.print(plaza + 1); 
      Serial.println(estado == 1 ? " -> OCUPADA" : " -> LIBRE");
      Serial.print("Lugares Libres: "); Serial.println(libres);
      Serial.print("Lugares Ocupados: "); Serial.println(ocupados);
      Serial.println("----------------------------------------");

      // Respondemos al nodo que todo salió bien
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Plaza fuera de rango");
    }
  } else {
    server.send(400, "text/plain", "Faltan datos en la peticion");
  }
}
