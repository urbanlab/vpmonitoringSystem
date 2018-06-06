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
#define CODE_ADDR 32
#define CODE_ADDR2 64
#define BITS_ADDR 16
#define OFF_ADDR 128

const int inter=15;       
uint16_t RECV_PIN = 14;
const int IR_LED = 13;
const int A_LED = 12;
int lecture;
int TestCode;
int TestCode2;
int CodeRecep=0;
int EEPROMoff;


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
String htmlHeader(String title) {
 return "<html>\
          <head>\
            <title>Erasme VP</title>\
            <style>\
              body { font-family: Arial, Helvetica, Sans-Serif; Color: #4CAF50;}\
               #carre  {box-sizing: content-box; width: auto;height: auto; padding: 15px; border: 2px solid #4CAF50};\
            </style>\
          </head>\
          <body>\
          <h1>"+ title +"</h1>";
        }

String htmlFooter() {
 return "</body>\
        </html>";
}

// --------------------------------------------------------------------------------------
// Affichage de la page web de base 
// --------------------------------------------------------------------------------------
void handleRoot() {
  CodeRecep=0;
  String page;
  page = htmlHeader("Home");
  page +=
        "<h3>Allumage et extinction</h3>\
            <div id='carre'>\
                  <h4>1.Codes identiques</h4>\
               <p><a input type href='on' >Allumer le projecteur</a>\
                  <a input type href='off1'>Eteindre le projecteur</a>\
                  <a input type href='off2'>Eteindre avec confirmation</a></p>\
                  <h4>2.Codes differents</h4>\  
               <p><a input type href='on'>Allumer le projecteur</a>\
                  <a input type href='off3'>Eteindre le projecteur</a>\
                  <a input type href='off'>Eteindre avec l'EEPROM</a></p>\
            </div>\
          <h3>Parametrage</h3>\
            <div id='carre'>\
               <h4>Nom du DNS</h4>\
                  <p><input type='text' id='nom'>.local</a>\
                    <a input type href='xxxx'>enregistrer</a>\
               <h4>Protocole d'extinction</h4>\  
                    <p><a input type href='/param/protocole/extinction'>Eteindre</a></p>\
                    <p><a input type href='/param/protocole/extinction/confirmation'>Eteindre avec confirmation</a></p>\
                    <p><a input type href='/param/protocole/codediff'>Code d'extinction different de l'allumage</a></p>\
                    <p><a input type href='erase'>Tout effacer</a></p>";

  page += htmlFooter();

  
  server.send ( 200, "text/html", page );

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

// ------------------------------------------------------------------------------------------------------------
// Allumage et extinction 
// ------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// Allumer
// --------------------------------------------------------------------------------------
  server.on("/on", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));      //On envoie le code qui correspond au code de la télécommande
    
    String page2;
           page2 = htmlHeader("Projecteur allume");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );
    
    clignote(2, 200);

  });

// --------------------------------------------------------------------------------------
// Eteindre avec l'EEPROM
// --------------------------------------------------------------------------------------
  server.on("/off", [](){

    if (EEPROMReadlong(OFF_ADDR)==1){
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    clignote(4, 50);
    }
    
    else if (EEPROMReadlong(OFF_ADDR)==2){
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));      //On envoie le code qui correspond au code de la télécommande
    clignote(4, 50);
    
    delay(1500);
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    clignote(4, 50);
    }
    
    String page2;
           page2 = htmlHeader("Projecteur eteint en fonction de l'EEPROM");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );
  });

// --------------------------------------------------------------------------------------
// Eteindre
// --------------------------------------------------------------------------------------
  server.on("/off1", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    
    String page2;
           page2 = htmlHeader("Projecteur eteint");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );

    Serial.println();
            Serial.print("Code voulu : ");
            Serial.println(TestCode);

            Serial.print("Code obtenu : ");
            Serial.println(EEPROMReadlong(CODE_ADDR)); 
   
    clignote(4, 50);
  });

// --------------------------------------------------------------------------------------
// Eteindre avec confirmation
// --------------------------------------------------------------------------------------
  server.on("/off2", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));      //On envoie le code qui correspond au code de la télécommande
    clignote(4, 50);
    
    delay(1500);
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    clignote(4, 50);
    
    String page2;
               page2 = htmlHeader("Projecteur eteint avec confirmation");
               page2 +="<a input type href='/'>Retour</a>";
               page2 += htmlFooter();
    
        server.send ( 200, "text/html", page2 );

    
    

  });

// --------------------------------------------------------------------------------------
// Eteindre (2 boutons)
// --------------------------------------------------------------------------------------
  server.on("/off3", [](){
    
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR2), EEPROMReadlong(BITS_ADDR));

    String page2;
           page2 = htmlHeader("Projecteur eteint");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );

          Serial.println();
                  Serial.print("Code voulu : ");
                  Serial.println(TestCode2);
      
                  Serial.print("Code obtenu : ");
                  Serial.println(EEPROMReadlong(CODE_ADDR2)); 
      
  });

// ------------------------------------------------------------------------------------------------------------
// Fin allumage et extinction 
// ------------------------------------------------------------------------------------------------------------
  
  server.onNotFound(handleNotFound);  //Si on ne trouve pas la page

  server.begin();
  Serial.println("HTTP server started");

// ------------------------------------------------------------------------------------------------------------
// Paramétrage
// ------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// Eteindre (EEPROM)
// --------------------------------------------------------------------------------------
  server.on("/param/protocole/extinction", [](){

    for (int i = 128; i < 154; i++)
    EEPROM.write(i, 0);
    
    EEPROMoff=1;
    EEPROMWritelong(OFF_ADDR, EEPROMoff);
    
    String page2;
           page2 = htmlHeader("Protocole enregistre dans l'EEPROM");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );
    
    clignote(2, 200);

  });
  
// --------------------------------------------------------------------------------------
// Eteindre avec confirmation (EEPROM)
// --------------------------------------------------------------------------------------
  server.on("/param/protocole/extinction/confirmation", [](){

    for (int i = 128; i < 154; i++)
    EEPROM.write(i, 0);
    
    EEPROMoff=2;
    EEPROMWritelong(OFF_ADDR, EEPROMoff);

    
    String page2;
           page2 = htmlHeader("Protocole enregistre dans l'EEPROM");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );
    
    clignote(2, 200);

  });
  
// --------------------------------------------------------------------------------------
// Code d'extinction different de l'allumage
// --------------------------------------------------------------------------------------
  server.on("/param/protocole/codediff", [](){

    CodeRecep=1;
      
          String page2;
          page2 = htmlHeader("Recepteur");
          page2 +="<h3>Activation du recepteur</h3>\
                    <div id='carre'>\
                       <p>Passez le boitier en mode reception et enregistrer le code d'extinction</p>\
                       <p><a input type href='/'>Enregistrer</a></p></div>";
        
          page2 += htmlFooter();
      
        
        server.send ( 200, "text/html", page2 );
    

  });

// --------------------------------------------------------------------------------------
// Erase
// --------------------------------------------------------------------------------------
  server.on("/erase", [](){
    
    
    
      for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);

    String page2;
           page2 = htmlHeader("Memoire effacee");
           page2 +="<a input type href='/'>Retour</a>";
           page2 += htmlFooter();

    server.send ( 200, "text/html", page2 );
    
    clignote(10, 50);

  });

// ------------------------------------------------------------------------------------------------------------
// Fin paramétrage
// ------------------------------------------------------------------------------------------------------------

}


// --------------------------------------------------------------------------------------
// Fonction pour allumer la LED
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
// On lit en long int dans l'EEPROM
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


// --------------------------------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------------------------------
void loop(void){
  
  server.handleClient();      //On active le serveur
  lecture=digitalRead(inter); //Lecture du switch

  if(lecture==1 && CodeRecep==0){
 
          if (irrecv.decode(&results)) {

            for(int i = 0; i<1;i++) {
              Serial.println();
              Serial.println("Recepteur actif");
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
          if(lecture==1 && CodeRecep==1){
      
            if (irrecv.decode(&results)) {
      
                  for(int i = 0; i<1;i++) {
                    Serial.println();
                    Serial.println("Recepteur actif");
                  }
      
                  EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM
      
                  EEPROMWritelong(CODE_ADDR2, results.value);         //On écrit le code dans l'EEPROM
      
                  EEPROMWritelong(BITS_ADDR, results.bits);          //On écrit le nombre de bits dans l'EEPROM
      
                  TestCode2=(results.value);
                  
      
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
                  Serial.println(TestCode2);
      
                  Serial.print("Code obtenu : ");
                  Serial.println(EEPROMReadlong(CODE_ADDR2)); 
                  
                  clignote(4, 50);                //On fait clignoter la led 4 fois avec un intervalle de 50ms
      
            irrecv.resume();  // Reçoit la prochaine valeur
          }
        }
}
