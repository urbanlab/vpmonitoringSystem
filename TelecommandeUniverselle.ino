// --------------------------------------------------------------------------------------
// Librairies
// --------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <port.h>
#include <EEPROM.h>

// --------------------------------------------------------------------------------------
// Déclaration des variables
// --------------------------------------------------------------------------------------
#define TYPE_ADDR 0         //On définit les adresses de départ pour l'EEPROM
#define CODE_ADDR 128
#define BITS_ADDR 64

const int inter=15;       
uint16_t RECV_PIN = 14;
const int IR_LED = 13;
const int A_LED = 12;
int lecture;
int TestCode;

// --------------------------------------------------------------------------------------
// Identifiant de connexion au wifi
// --------------------------------------------------------------------------------------
const char* ssid = "erasme-guests";
const char* password = "guests@erasme";


// --------------------------------------------------------------------------------------
// "Activation" des librairies
// --------------------------------------------------------------------------------------
ESP8266WebServer server(80);

IRrecv irrecv(RECV_PIN);
IRsend irsend(IR_LED);

decode_results results;

// --------------------------------------------------------------------------------------
// Affichage de la page web de base 
// --------------------------------------------------------------------------------------
void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <a input type href=\"/on\"/ >/Allumer le projecteur/</a>\
    <a href=\"/off\"/ >Eteindre le projecteur/</a>\
  </body>\
</html>",

    hr, min % 60, sec % 60
  );
  server.send ( 200, "text/html", temp );

}


// --------------------------------------------------------------------------------------
// Affichage de la page web non trouvée
// --------------------------------------------------------------------------------------
void handleNotFound(){

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


// --------------------------------------------------------------------------------------------------------------
// Setup
// --------------------------------------------------------------------------------------------------------------
void setup(){

  irsend.begin();                 //On démarre les bilbliothèques
  irrecv.enableIRIn();
  
  pinMode(A_LED, OUTPUT);         //Déclaration des leds
  digitalWrite(A_LED, LOW);
  pinMode(inter,INPUT_PULLUP);
  
  Serial.begin(115200);           //Activation du monitor
  EEPROM.begin(512);
  
  WiFi.mode(WIFI_STA);            //Connexion au wifi
  WiFi.begin(ssid, password);
  Serial.println("");

                              
  while (WiFi.status() != WL_CONNECTED) {   // On attend la connexion
    delay(500);
    Serial.print(".");
  }
  Serial.println("");                   
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("erasmeVP")) {             //Définition du DNS
    Serial.println("MDNS responder started");
  }

    server.on("/", handleRoot);

    
// --------------------------------------------------------------------------------------
// Led activée
// --------------------------------------------------------------------------------------
  server.on("/on", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));      //On envoie le code qui correspond au code de la télécommande
    
    Serial.println("Projecteur allumee");
    
    Serial.println(EEPROMReadlong(TYPE_ADDR));     //On affiche le numéro du type de la télécommande
    Serial.println(EEPROMReadlong(CODE_ADDR));     //On affiche le code du bouton de la télécommande
    Serial.println(EEPROMReadlong(BITS_ADDR));     //On affiche le nombre de bits
    Serial.println(TestCode);

    server.send(200, "text/plain", "Projecteur allumee");
    
    clignote(1, 200);

  });


// --------------------------------------------------------------------------------------
// Led éteinte
// --------------------------------------------------------------------------------------
  server.on("/off", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    
    Serial.println("Projecteur eteint");
    server.send(200, "text/plain", "Projecteur eteint");
    
    eteint();
  });
  
  
  server.onNotFound(handleNotFound);  //Si on ne trouve pas la page

  server.begin();
  Serial.println("HTTP server started");

  }


// --------------------------------------------------------------------------------------
// Raccourci LED
// --------------------------------------------------------------------------------------
  void allumee(){
    digitalWrite(A_LED,HIGH);
  }
  
  void eteint(){
    digitalWrite(A_LED,LOW);
  }


  void clignote(int a, int b){

   for(int i = 0; i<2*a;i++) {
       
    digitalWrite(A_LED,!digitalRead(A_LED));
    delay(b);
  }
}

// --------------------------------------------------------------------------------------
// On écrit en long int dans l'EEPROM
// --------------------------------------------------------------------------------------
void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }

// --------------------------------------------------------------------------------------
// On lit en int dans l'EEPROM
// --------------------------------------------------------------------------------------
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

/*      
// --------------------------------------------------------------------------------------
// On écrit en int dans l'EEPROM
// --------------------------------------------------------------------------------------
void eeprom_write(int addr, uint64_t f) {
unsigned char *buf = (unsigned char*)(&f);
for ( int i = 0 ; i < (uint64_t)sizeof(f) ; i++ ) {
EEPROM.write(addr+i, buf[i]);
}
}

// --------------------------------------------------------------------------------------
// On lit en int dans l'EEPROM
// --------------------------------------------------------------------------------------
float eeprom_read(int addr) {
uint64_t f;
unsigned char *buf = (unsigned char*)(&f);
for ( int i = 0 ; i < (uint64_t)sizeof(f) ; i++ ) {
buf[i] = EEPROM.read(addr+i);
}
return f;
}*/

// --------------------------------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------------------------------
void loop(void){
  
  server.handleClient();      //On active le serveur
  lecture=digitalRead(inter); //Lecture du switch

  if(lecture==1){
 
          if (irrecv.decode(&results)) {

            for(int i = 0; i<1;i++) {
              Serial.println("Recepteur actif");
            }

            for (int i = 0; i < 512; i++) {       //Vide la mémoire de l'EEPROM
              EEPROMWritelong(i, 0);
            }

            EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM

            EEPROMWritelong(CODE_ADDR, results.value);         //On écrit le code dans l'EEPROM

            EEPROMWritelong(BITS_ADDR, results.bits);          //On écrit le nombre de bits dans l'EEPROM

            TestCode=(results.value);
            

            EEPROM.commit();
            
            Serial.print("Numero du type d'encodage : ");
            Serial.println(results.decode_type);

            Serial.print("Code hexadecimal : ");
            serialPrintUint64(results.value, 16);
            Serial.println();

            Serial.print("Nombre de bits : ");
            Serial.println(uint64ToString(results.bits));
            

            Serial.println();
            Serial.print("Code voulu : ");
            Serial.println(TestCode);

            Serial.print("Code obtenu : ");
            Serial.println(EEPROMReadlong(CODE_ADDR)); 
            
            clignote(4, 50);                //On fait clignoter la led 4 fois avec un intervalle de 50ms

      irrecv.resume();  // Reçoit la prochaine valeur
    }
  }
}
