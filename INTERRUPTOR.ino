#include <WiFi.h>

// El dispositivo Esp32 se conectará a la red wifi que le indiquemos con la respectiva contraseña 

const char* ssid = "SKULL";
const char* password = "SKULL123";

WiFiServer server(80);

String header; //variable para almacenar el encabezado de la solicitud HTTP

int limite = 50;
int lectura; // lectura de la capacitancia de la placa de aluminio 

const int outPin = 4; // pin de salida

String outState = "apagado"; //variables auxiliar para almacenar el estado actual de la salida

unsigned long lastTime, timeout = 2000;

void setup() {
  
  Serial.begin(115200);
  pinMode(outPin, OUTPUT);
  digitalWrite(outPin, HIGH);
  
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  

  Serial.println("");
  Serial.println("Dispositivo conectado.");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
}

void loop(){

   lectura = touchRead(T8); // pin de entrada sensor tactil

     

    if(lectura < limite){     //Al tocar la placa, el número de lectura del pin se reducirá dando lugar a que se cumpla la condición, de esta manera se encenderá y apagara la bombilla.                            
      digitalWrite(outPin, !digitalRead(outPin));   //esto permitirá que la condición se sostenga hasta que vuelva a cambiar la capacitancia,                                                   
      delay(500);                                   //de otra manera la bombilla se encenderá al tocar la placa, pero al retirar el dedo, volvería a apagarse la bombilla.
    }



  WiFiClient client = server.available();   //El ESP32 siempre está escuchando a los clientes entrantes
  if (client) {
                               
    lastTime = millis();
    
    Serial.println("Nuevo cliente.");      
    String currentLine = ""; // Cadena para contener los datos entrantes del cliente          
    while (client.connected() && millis() - lastTime <= timeout) 
    {
      
      if (client.available()) {        
        char c = client.read();            
        Serial.write(c);                  
        header += c;
        // Si recibe salto de linea y la currentLine esta vacia,
        // la peticion http finalizó, se responde:
        if (c == '\n') {   
                        
          if (currentLine.length() == 0) {

            // Los encabezados HTTP siempre comienzan con un código de respuesta (por ejemplo, HTTP/1.1 200 OK) 
            // y un tipo de contenido para que el cliente sepa lo que viene, luego una línea en blanco:
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();


            //si el estado del boton del html indica que la bombilla esta "apagada"
            //la bombilla se apaga

           
            if(header.indexOf("GET /4/off") >= 0) {  
              Serial.println("GPIO 4 apagado");
              outState = "apagado";
              digitalWrite(outPin, HIGH);                          
            }  
            
            //si el estado del boton del html indica que la bombilla esta "encendida"
            //la bombilla se enciende

            else if(header.indexOf("GET /4/on") >= 0){
              Serial.println("GPIO 4 encendido");
              outState = "encendido";
              digitalWrite(outPin, LOW); 
            } 

           
              

            // Pagina web
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".buttonOff {background-color: #555555;}</style></head>");
            
         
            client.println("<body><h1>Control Remoto</h1>");  // Titulo
            
  
            client.println("<p>Dispositivo: " + outState + "</p>"); // Estado del dispositivo
                  
            if (outState=="apagado") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">Encender</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button buttonOff\">Apagar</button></a></p>");
            } 
               
           

            client.println();
           
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;      
        }
      }
    }
    
    header = "";  
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
  
 

}
