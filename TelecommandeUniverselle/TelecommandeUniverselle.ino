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
#//include <port.h>
#include <EEPROM.h>
#include "DataEEPROM.h"
#include "OTAfunctions.h"
#include "config.h"

// --------------------------------------------------------------------------------------
// Déclaration des variables
// --------------------------------------------------------------------------------------
// On définit les adresses de départ pour l'EEPROM
#define TYPE_ADDR 0   // Adresse de stockage du type d'encodage lié à un vidéo-projecteur (= marque du VP)
#define NB_BITS_ADDR 16  // Adresse de stockage du nb de bits d'encodage des code IR
#define TIMEOUT_ADDR 32 // Adresse de stockage de la durée entre 2 émissions de code IR
#define CODE_OFF_ADDR 48   // Adresse de sctockage du code d'extinction
#define DNS_LENGTH_ADDR 64 
#define CODE_ADDR 80       // Adresse du code d'allumage/extinction 
#define CODE_ADDR2 144     // Adresse du code d'extinction, si différent de celui de l'allumage
#define MDNS_ADDR 208

// I/O definitions
#define SW_PIN 15             // Interrupteur sur la PIN 15
#define RECV_PIN 14           // Recepteur sur la PIN 14
#define IR_LED 13             // LED infrarouge sur la PIN 13
#define FEEDBACK_LED 12       // LED sur la PIN 12


int CodeRecep;                // Varaible d'état sur le mode Emission ou Réception IR (0=Emission, 1=Réception)
int EEPROMoff = 0;            // Variable du deuxième bouton dans l'EEPROM
int delai = 2000;

// Web Server
ESP8266WebServer server(80);

// Receiver and sender IR
IRrecv irrecv(RECV_PIN);
IRsend irsend(IR_LED);
decode_results results;

// --------------------------------------------------------------------------------------
// Fonction pour afficher l'adresse IP en String
// --------------------------------------------------------------------------------------
String humanReadableIp(IPAddress ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

// --------------------------------------------------------------
// Convertion String en Char*
// --------------------------------------------------------------
char* strToChar(String s) {
  unsigned int bufSize = s.length() + 1; //String length + null terminator
  char* ret = new char[bufSize];
  s.toCharArray(ret, bufSize);
  return ret;
}

// --------------------------------------------------------------------------------------
// Fonction pour faire clignoter la LED
// --------------------------------------------------------------------------------------
void LedBlink(int a, int b) {

  for (int i = 0; i < 2 * a; i++) {

    digitalWrite(FEEDBACK_LED, !digitalRead(FEEDBACK_LED));
    delay(b);
  }
}

// --------------------------------------------------------------------------------------
// On affiche le détail du code enregistré par le récepteur
// --------------------------------------------------------------------------------------
void printRecordedCode() {
  Serial.print("Numero du type d'encodage : ");
  Serial.println(results.decode_type);

  Serial.print("Code hexadecimal : ");
  serialPrintUint64(results.value, 16);
  Serial.println();

  Serial.print("Nombre de bits : ");
  Serial.println(uint64ToString(results.bits));

  LedBlink(4, 50);
}

// --------------------------------------------------------------------------------------
// Fonction qui renvoie un header HTML
// --------------------------------------------------------------------------------------
String htmlHeader(String title) {
  return "<html>\
          <head>\
          <meta charset='utf-8' />\
            <title>" + getMDNSname() + "</title>\
            " + css() + "\
          </head>\
          <body>\
          <h1>" + title + "</h1>";
}

// --------------------------------------------------------------------------------------
// Fonction qui renvoie un footer HTML
// --------------------------------------------------------------------------------------
String htmlFooter() {
  return "</body>\
        </html>";
}


// --------------------------------------------------------------
// Index html
// --------------------------------------------------------------
String GetHTMLIndex() {
  CodeRecep = 0;                      //On désactive le récepteur

  String page;

  String messages = "Nom DNS : ";
  messages += getMDNSname();
  messages += " / Adresse IP : ";
  messages += humanReadableIp(WiFi.localIP());

  page = htmlHeader(messages);
  page += "<h3>Allumage et extinction</h3>\
         <div class='carre'>\
             <table>\
                <tr><h4>Tester</h4>\
                    <FORM action='/on'>\
                      <th><INPUT TYPE='submit' class='button' VALUE='Allumer'></th>\
                    </FORM>\
                    <FORM action='/off'>\
                      <th><INPUT TYPE='submit' class='button' VALUE='Eteindre'></th>\
                    </FORM><tr/></table>\
          </div>\
            <h3>Param&eacute;trage</h3>\
          <div class='carre'>\
                    <h4>Nom DNS</h4>\
                     <FORM action='/record'>\
                       <p><input type='text' NAME='dnsName' VALUE=" + getMDNSname() + ">.local</a>\
                       <INPUT TYPE='submit' class='button' VALUE='Envoyer'></p>\
                     </FORM>\
             <table>\
                <tr><h4>Enregistrement du code</h4></tr>\
                     <FORM action='/param/protocole/codeON'>\
                        <th><INPUT TYPE='submit' class='button' VALUE=' ON '></th>\
                     </FORM>\
                     " + getFormCodeOFF() + "\
             </table>\
                    <h4>Protocole d'extinction</h4>\
                     " + getFormOnce() + "\
                     " + getFormTwice() + "\
          </div>";

  page += htmlFooter();

  return page;
}

// --------------------------------------------------------------
// CSS
// --------------------------------------------------------------
String css() {
  return  "<style>\
                body{font-family: Arial, sans-serif; font-size: 16px; height: 100%; margin: 0; padding: 0; color: #000; background: hsl(227, 10%, 10%);; color:#fff; margin-left: 1%; margin-top:5%;}\
                h1 {text-align: center; margin-top: -4%;}\
                .button {background: none; border: 2px solid #4CAF50; color: white; padding: 15px 32px; text-align: center; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; color:#4CAF50; transition-duration: 0.4s; transition-duration: 0.4s; cursor: pointer;border-radius: 3px;}\
                .button:hover {background-color: #4CAF50; color: white;}\
                .carre {min-height: 100px; width: 75%; padding: 10px; border: 2px solid #4CAF50; margin: 0;border-radius: 3px;}\
                .disabled {opacity: 0.7; background-color: #4CAF50; color: #ffffff;}\
                #Reinitialiser {padding: 20px 80px; margin-left: 75%; margin-top: 2%}\
            </style>";
}

// --------------------------------------------------------------
// Redirection client
// --------------------------------------------------------------
String clientRedirect() {

  return "<html>\
<head>\
  " + css() + "\
  <meta http-equiv='refresh' content='0; URL=http://" + getMDNSname() + ".local/\' />\
</head>\
<body>\
<h1>Redirection en cours</h1>\
</body>\
</html>";

}


// --------------------------------------------------------------
// Affichage formulaire de config "code extinction unique"
// --------------------------------------------------------------
String getFormOnce() {
  String disabled = "";

  if (EEPROM.read(CODE_OFF_ADDR) == 1)   {
    disabled = " disabled";
  }
  return "<FORM action='/param/protocole/extinction'>\
            <p><INPUT TYPE='submit' class='button " +  disabled + "' VALUE=\"Envoyer le code d'extinction une fois \"></p>\
          </FORM>";
}

// --------------------------------------------------------------
// Affichage formulaire de config "code extinction confirmé"
// --------------------------------------------------------------
String getFormTwice() {
  String msgDelais = "", disabled = "";

  if (EEPROM.read(CODE_OFF_ADDR) == 2) {
    msgDelais = afficherDelais();
    disabled = " disabled";
  }
  return "<FORM action='/param/protocole/extinction/confirmation'>\
        <p><INPUT TYPE='submit' class='button" + disabled  + "' VALUE=\"Envoyer le code d'extinction deux fois\"> " + msgDelais + "</p>\
    </FORM>";
}
// --------------------------------------------------------------
// Affichage formulaire de config Stockage de la valeur OFF 
// si différente de ON
// --------------------------------------------------------------
String getFormCodeOFF() {
  String msgDelais = "", disabled = "";

  if (EEPROM.read(CODE_OFF_ADDR) == 3) {
    msgDelais = afficherDelais();
    disabled = " disabled";
  }
  return "<FORM action='/param/protocole/codeOFF'>\
        <th><INPUT TYPE='submit' class='button" + disabled + "' VALUE='OFF'> " + msgDelais + "</th>\
    </FORM>";
}

// --------------------------------------------------------------
// Restart du MDNS du Feather
// --------------------------------------------------------------
void startMDNS(String NomDNS) {
  Serial.println(NomDNS);
  if (MDNS.begin(strToChar(NomDNS), WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
}

// --------------------------------------------------------------
// Affichage du délai
// --------------------------------------------------------------
String afficherDelais() {
  String s = "";
  int d = EEPROM.read(TIMEOUT_ADDR);
  if (d > 1) {
    s = "s";
  }
  return "Durée définie : " + String(d) +  " seconde" + s;
}

// --------------------------------------------------------------
// Fonction lecture d'EEPROM
// --------------------------------------------------------------
void EEPROMdump() {

  Serial.println();
  Serial.print("Nom MDNS du feather : ");
  Serial.println(getMDNSname() + ".local");

  Serial.print("Variable EEPROMoff : ");
  Serial.println(EEPROMReadlong(CODE_OFF_ADDR));

  Serial.print("Type de code : ");
  Serial.println(EEPROMReadlong(TYPE_ADDR));

  Serial.print("Code IR : ");
  Serial.println(EEPROMReadlong(CODE_ADDR));

  Serial.print("Code IR du signal d'extinction : ");
  Serial.println(EEPROMReadlong(CODE_ADDR2));

  Serial.print("Nb de bits du code : ");
  Serial.println(EEPROMReadlong(NB_BITS_ADDR));

  Serial.print("Délais avant confirmation : ");
  Serial.println((EEPROM.read(TIMEOUT_ADDR)) * 1000);
}

// --------------------------------------------------------------------------------------
// Nom du DNS
// --------------------------------------------------------------------------------------
String getMDNSname() {
  String mdnsName = "videoprojector"; // This is default mdns name
  String tmp = read_StringEE(MDNS_ADDR, 50);
  if (tmp != "") {
    mdnsName = tmp;
  }
  return mdnsName;
}

// --------------------------------------------------------------------------------------
// Bouton Envoyer du DNS
// --------------------------------------------------------------------------------------
void handleMDNSsetting() {
  String recordDNS = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "dnsName") {
      recordDNS = server.arg(i);
      Serial.println(recordDNS);
      break;
    }
  }

  write_StringEE(MDNS_ADDR, recordDNS); 
  EEPROM.write(DNS_LENGTH_ADDR, recordDNS.length() + 1);
  EEPROM.commit();

  // Restart mdns service
  startMDNS(getMDNSname());
  // The mdns has changed, so redirect to the new one !
  server.send ( 301, "text/html", clientRedirect());
}

// --------------------------------------------------------------------------------------
// Envoyer le code d'Allumage
// -------------------------------------------------------------------------------------
void handleON() {
  irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(NB_BITS_ADDR));
  LedBlink(2, 200);
  EEPROMdump();
  handleRoot();
}


// --------------------------------------------------------------------------------------
// Envoyer le code d'extinction 
// --------------------------------------------------------------------------------------
void handleOFF() {
  int delais = EEPROM.read(TIMEOUT_ADDR) * 1000;

  if (EEPROMReadlong(CODE_OFF_ADDR) == 1) {    //Si Eteindre à été choisi
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(NB_BITS_ADDR));
    //On envoie qu'une seul fois le code d'extinction
    LedBlink(4, 100);
  }

  else if (EEPROMReadlong(CODE_OFF_ADDR) == 2) { //Si Eteindre avec confirmation à été choisi
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(NB_BITS_ADDR));                   //On envoie deux fois le code d'extinction
    LedBlink(4, 100);

    delay(delais);                            //Avec un délai entre les deux pour la confirmation

    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(NB_BITS_ADDR));
    LedBlink(4, 100);
  }

  else if (EEPROMReadlong(CODE_OFF_ADDR) == 3) { //Si Code d'extinction different de l'allumage à été choisi
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR2), EEPROMReadlong(NB_BITS_ADDR));
    LedBlink(2, 100);

    delay(delais);

    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR2), EEPROMReadlong(NB_BITS_ADDR));                   //On envoie le code d'extinction correspondant au 2ème bouton
    LedBlink(2, 100);
  }

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinction() {

  EEPROMoff = 1;
  EEPROM.write(CODE_OFF_ADDR, EEPROMoff);
  EEPROM.commit();
  LedBlink(3, 150);

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre avec confirmation (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinctionConf() {

  EEPROMoff = 2;
  EEPROM.write(CODE_OFF_ADDR, EEPROMoff);
  EEPROM.commit();
  LedBlink(3, 150);
  //CHOISIR TEMPS
  String html;
  html = htmlHeader("Code d'extinction avec confirmation");
  html += "<p><h3>Enregistrez la durée du délai de confirmation :</h3></p>\
            <FORM action='/duree'>\
                <p><input type='radio' NAME='duree' value='1'> 1 seconde</p>\
                <p><input type='radio' NAME='duree' value='2'> 2 secondes</p>\
                <p><input type='radio' NAME='duree' value='3'> 3 secondes</p>\
                <p><input type='radio' NAME='duree' value='4'> 4 secondes</p>\
             <p><INPUT TYPE='submit' class='button' VALUE='Enregistrer'></p>\
           </FORM>";
  html += htmlFooter();

  server.send ( 200, "text/html", html );

}

// --------------------------------------------------------------------------------------
// Enregistrement du delai pour l'envoi
// --------------------------------------------------------------------------------------
void handleDuree() {

  int delai = 2000;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "duree") {
      delai = server.arg(i).toInt();
      break;
    }
  }
  EEPROM.write(TIMEOUT_ADDR, delai);    //On écrit dans l'EEPROM
  EEPROM.commit();

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Enregistrement code premier bouton
// --------------------------------------------------------------------------------------
void handleCodeON() {

  CodeRecep = 1;

  String html;
  html = htmlHeader("Recepteur");
  html += "<h3>Activation du recepteur</h3>\
                    <div id='carre'>\
                       <p>Passez le boitier en mode reception et enregistrer le code d'allumage</p>\
                       <FORM action='/'>\
                       <th><INPUT TYPE='submit' class='button' VALUE='Enregistrer'></th>\
                  </FORM></div>";

  html += htmlFooter();


  server.send ( 200, "text/html", html );
}

// --------------------------------------------------------------------------------------
// Enregistrement code deuxième bouton
// --------------------------------------------------------------------------------------
void handleCodeOFF() {

  CodeRecep = 2;
  EEPROMoff = 3;
  EEPROMWritelong(CODE_OFF_ADDR, EEPROMoff);
  EEPROM.commit();

  String html;
  html = htmlHeader("Recepteur");
  html += "<h3>Activation du recepteur</h3>\
                    <div id='carre'>\
                       <p>Passez le boitier en mode reception et enregistrer le code d'extinction</p>\
                       <p><h3>Enregistrez la durée du délai de confirmation :</h3></p>\
            <FORM action='/duree'>\
                <p><input type='radio' NAME='duree' value='1'> 1 seconde</p>\
                <p><input type='radio' NAME='duree' value='2'> 2 secondes</p>\
                <p><input type='radio' NAME='duree' value='3'> 3 secondes</p>\
                <p><input type='radio' NAME='duree' value='4'> 4 secondes</p>\
             <p><INPUT TYPE='submit' class='button' VALUE='Enregistrer'></p>\
           </FORM>";

  html += htmlFooter();


  server.send ( 200, "text/html", html );
}

// --------------------------------------------------------------------------------------
// Affichage de la page web de base
// --------------------------------------------------------------------------------------
void handleRoot() {

  server.send ( 200, "text/html", GetHTMLIndex());

}


// --------------------------------------------------------------------------------------
// Affichage de la page web non trouvée
// --------------------------------------------------------------------------------------
void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


// --------------------------------------------------------------------------------------------------------------
// Setup
// --------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);           //Activation du monitor
  Serial.println("----> Entering SETUP...");
  EEPROM.begin(512);

  
  irsend.begin();                 //On démarre les bilbliothèques
  irrecv.enableIRIn();

  pinMode(FEEDBACK_LED, OUTPUT);         //Déclaration des leds
  digitalWrite(FEEDBACK_LED, LOW);
  pinMode(SW_PIN, INPUT_PULLUP);

  WiFi.persistent(false);         //These 3 lines are a required work-around
  WiFi.mode(WIFI_OFF);            //otherwise the module will not reconnect
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
  WiFi.hostname(getMDNSname());
  Serial.println(WiFi.localIP());
  startMDNS(getMDNSname());

  server.on("/", handleRoot);
  server.on("/record", handleMDNSsetting);
  server.on("/on", handleON);
  server.on("/off", handleOFF);
  server.on("/param/protocole/codeON", handleCodeON);
  server.on("/param/protocole/codeOFF", handleCodeOFF);
  server.on("/param/protocole/extinction", handleExtinction);
  server.on("/param/protocole/extinction/confirmation", handleExtinctionConf);
  server.on("/duree", handleDuree);
  server.onNotFound(handleNotFound);  //Si on ne trouve pas la page
  server.begin();
  Serial.println("HTTP server started");

  MDNS.addService("http", "tcp", 80);
  Serial.println("MDNS service http on port 80 started");
  
  EEPROMdump();

  setupOTA(getMDNSname(), OTApassword);

  // Signaler visuellement la mise en fonction et la fin du setup
  Serial.println("----> Exiting SETUP !");
  LedBlink(10, 50);
  
}

// --------------------------------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------------------------------
void loop(void) {

  server.handleClient();      //On active le serveur web
  ArduinoOTA.handle();        // Activation de la mise à jour On The Air

  int learnMode = digitalRead(SW_PIN); //Lecture du switch

  if (learnMode == 1) {
    // Si le code d'extintinction est différent du code d'allumage, on l'écrit à CODE_ADDR2
    int addr_code = (CodeRecep == 2) ? CODE_ADDR2 : CODE_ADDR;
    if (irrecv.decode(&results)) {
      EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM
      EEPROMWritelong(addr_code, results.value);         //On écrit le code dans l'EEPROM
      EEPROMWritelong(NB_BITS_ADDR, results.bits);       //On écrit le nombre de bits dans l'EEPROM
      EEPROM.commit();
      // Un petit retour sur serial
      printRecordedCode();
      irrecv.resume();  
    }
  }
}

