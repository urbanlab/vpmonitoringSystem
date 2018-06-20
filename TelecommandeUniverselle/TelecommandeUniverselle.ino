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
#include "DataEEPROM.h"

// --------------------------------------------------------------------------------------
// Déclaration des variables
// --------------------------------------------------------------------------------------
#define TYPE_ADDR 0   //On définit les adresses de départ pour l'EEPROM
#define BITS_ADDR 20
#define DUREE_ADDR 40
#define OFF_ADDR 50    
#define CODE_ADDR 60
#define CODE_ADDR2 160
#define DNS_ADDR 300

const int inter = 15;       //Interrupteur sur la PIN 15
uint16_t RECV_PIN = 14;     //Recepteur sur la PIN 14
const int IR_LED = 13;      //LED infrarouge sur la PIN 13
const int A_LED = 12;       //LED sur la PIN 12
int lecture;                //Variable du switch
int CodeRecep;             //Variable du deuxième bouton
int EEPROMoff;                 //Variable du deuxième bouton dans l'EEPROM
String nomduDNS;  //Variable du nom du DNS (de base "erasmevp")
int delai=2000;

int affichage;
// --------------------------------------------------------------------------------------
// Identifiant de connexion au wifi
// --------------------------------------------------------------------------------------
const char* ssid = "erasme-guests";         //SSID du wifi
const char* password = "guests@erasme";     //Mot de passe du wifi


// --------------------------------------------------------------------------------------
// "Activation" des librairies
// --------------------------------------------------------------------------------------
ESP8266WebServer server(80);

IRrecv irrecv(RECV_PIN);
IRsend irsend(IR_LED);

decode_results results;


// --------------------------------------------------------------------------------------
// Fonction pour afficher l'adresse IP en String
// --------------------------------------------------------------------------------------
String humanReadableIp(IPAddress ip) {
  return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
}

// --------------------------------------------------------------------------------------
// Fonction pour faire clignoter la LED
// --------------------------------------------------------------------------------------
void clignote(int a, int b) {

  for (int i = 0; i < 2 * a; i++) {

    digitalWrite(A_LED, !digitalRead(A_LED));
    delay(b);
  }
}

// --------------------------------------------------------------------------------------
// On affiche le détail du code enregistrer par le récepteur
// --------------------------------------------------------------------------------------
void afficherDetails() {
  Serial.print("Numero du type d'encodage : ");
  Serial.println(results.decode_type);

  Serial.print("Code hexadecimal : ");
  serialPrintUint64(results.value, 16);
  Serial.println();

  Serial.print("Nombre de bits : ");
  Serial.println(uint64ToString(results.bits));

  clignote(4, 50);
}

// --------------------------------------------------------------------------------------
// Affichage de la page web de base
// --------------------------------------------------------------------------------------
String htmlHeader(String title) {
  return "<html>\
          <head>\
          <meta charset='utf-8' />\
            <title>" + nomduDNS + "</title>\
            "+getStyle()+"\
          </head>\
          <body>\
          <h1>" + title + "</h1>";
}

String htmlFooter() {
  return "</body>\
        </html>";
}


// --------------------------------------------------------------
// Index html
// --------------------------------------------------------------
String GetIndex() {
  CodeRecep=0;                        //On désactive le récepteur
  
  String page;

  String messages = "Nom DNS : ";
  messages += nomduDNS;
  messages += " / Adresse IP : ";
  messages += humanReadableIp(WiFi.localIP());

  page = htmlHeader(messages);
  page += "<h3>Allumage et extinction</h3>\
         <div class='carre'>\
             <table>\
                <tr><h4>Code d'allumage</h4>\
                    <FORM action='/on'>\
                      <th><INPUT TYPE='submit' class='button' VALUE='Allumer'></th>\
                    </FORM>\
                    <FORM action='/off'>\
                      <th><INPUT TYPE='submit' class='button' VALUE='Eteindre'></th>\
                    </FORM><tr/></table>\
          </div>\
            <h3>Parametrage</h3>\
          <div class='carre'>\
                    <h4>Nom du DNS</h4>\
                     <FORM action='/record'>\
                       <p><input type='text' NAME='dnsName' VALUE=" + nomduDNS + ">.local</a>\
                       <INPUT TYPE='submit' class='button' VALUE='Envoyer'></p>\
                     </FORM>\
             <table>\
                <tr><h4>Enregistrement du code</h4></tr>\
                     <FORM action='/param/protocole/codeON'>\
                        <th><INPUT TYPE='submit' class='button' VALUE=' ON '></th>\
                     </FORM>\
                     "+getAffichage3()+"\
             </table>\
                    <h4>Protocole d'extinction</h4>\
                     "+getAffichage()+"\
                     "+getAffichage2()+"\
          </div>\
              <h3>Réinitialisation</h3>\
          <div class='carre'>\
                    <FORM action='/erase'>\
                       <INPUT TYPE='submit' class='button' id='Reinitialiser' onclick=\"return confirm('Effacer tous les parametres ?')\" VALUE='Réinitialiser'>\
                    </FORM>\
          </div>";

  page += htmlFooter();

  return page;
}

// --------------------------------------------------------------
// CSS
// --------------------------------------------------------------
String getStyle() {
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
  "+getStyle()+"\
  <meta http-equiv='refresh' content='0; URL=http://" + nomduDNS + ".local/\' />\
</head>\
<body>\
<h1>Redirection en cours</h1>\
</body>\
</html>";

}

// --------------------------------------------------------------
// Affichage config
// --------------------------------------------------------------
String getAffichage(){
  if (EEPROMoff==1)   {
    return "<FORM action='/param/protocole/extinction'>\
              <p><INPUT TYPE='submit' class='button disabled' VALUE=\"Envoyer le code d'extinction une fois  \"></p>\
            </FORM>";
   }
  else    {
    return "<FORM action='/param/protocole/extinction'>\
              <p><INPUT TYPE='submit' class='button' VALUE=\"Envoyer le code d'extinction une fois \"></p>\
            </FORM>";
    }
}
// --------------------------------------------------------------
// Affichage config
// --------------------------------------------------------------
String getAffichage2(){    
    if (EEPROMoff==2) {
      return "<FORM action='/param/protocole/extinction/confirmation'>\
          <p><INPUT TYPE='submit' class='button disabled' VALUE=\"Envoyer le code d'extinction deux fois\">   "+ afficherDelai() +"</p>\
      </FORM>";
    }
    else {
      return "<FORM action='/param/protocole/extinction/confirmation'>\
          <p><INPUT TYPE='submit' class='button' VALUE=\"Envoyer le code d'extinction deux fois\"></p>\
      </FORM>";
    }
}
// --------------------------------------------------------------
// Affichage config
// --------------------------------------------------------------
String getAffichage3(){    
    if (EEPROMoff==3) {
      return "<FORM action='/param/protocole/codeOFF'>\
          <th><INPUT TYPE='submit' class='button disabled' VALUE='OFF'>   "+ afficherDelai() +"</th>\
      </FORM>";
    }
    else {
      return "<FORM action='/param/protocole/codeOFF'>\
          <th><INPUT TYPE='submit' class='button' VALUE='OFF'></th>\
      </FORM>";
    }
}

// --------------------------------------------------------------
// Config du DNS
// --------------------------------------------------------------
void configDNS() {

  if (MDNS.begin(strToChar(nomduDNS))) {             //Définition du DNS
    Serial.println("MDNS responder started");
  }
}

// --------------------------------------------------------------
// Affichage du délai
// --------------------------------------------------------------
String afficherDelai() {
  String printDelai="";
  int delais = EEPROM.read(DUREE_ADDR)*1000;

  if (delais == 1000){
    printDelai="Durée définie : 1 seconde";
  }
  else if (delais == 2000){
    printDelai="Durée définie : 2 secondes";
  }
  else if (delais == 3000){
    printDelai="Durée définie : 3 secondes";
  }
  else if (delais == 4000){
    printDelai="Durée définie : 4 secondes";
  }

  return printDelai;
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
// Bouton Envoyer du DNS
// --------------------------------------------------------------------------------------
void handleRecord() {
  String recordDNS = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "dnsName") {
      recordDNS = server.arg(i);
      Serial.println(recordDNS);
      break;
    }
  }

   for (int i = DNS_ADDR; i < 512; i++) {
    EEPROM.write(i, 0);
   }
   
  write_StringEE(DNS_ADDR, recordDNS);    //On écrit en String dans l'EEPROM
  nomduDNS = (read_StringEE(DNS_ADDR, (recordDNS.length() + 1)));
  EEPROM.commit(); //On met la valeur écrite dans l'EEPROM dans une variable
  Serial.println(strToChar(nomduDNS));

  configDNS();      //On actualise le DNS

  server.send ( 310, "text/html", clientRedirect());



}

// --------------------------------------------------------------------------------------
// Allumer
// -------------------------------------------------------------------------------------
void handleON() {

  irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));

  clignote(2, 200);

  Serial.println(EEPROMReadlong(TYPE_ADDR));
  Serial.println(EEPROMReadlong(CODE_ADDR));
  Serial.println(EEPROMReadlong(BITS_ADDR));

  handleRoot();
}


// --------------------------------------------------------------------------------------
// Eteindre avec l'EEPROM
// --------------------------------------------------------------------------------------
void handleOFF() {

  int delais = EEPROM.read(DUREE_ADDR)*1000;

  if (EEPROMReadlong(OFF_ADDR) == 1) {    //Si Eteindre à été choisi
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
                  //On envoie qu'une seul fois le code d'extinction
    clignote(4, 100);
  }

  else if (EEPROMReadlong(OFF_ADDR) == 2) { //Si Eteindre avec confirmation à été choisi
    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));                   //On envoie deux fois le code d'extinction
    clignote(4, 100);

    delay(delais);                            //Avec un délai entre les deux pour la confirmation

    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR), EEPROMReadlong(BITS_ADDR));
    clignote(4, 100);
  }

  else if (EEPROMReadlong(OFF_ADDR) == 3) { //Si Code d'extinction different de l'allumage à été choisi
   irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR2), EEPROMReadlong(BITS_ADDR));
    clignote(2, 100);

    delay(delais);

    irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CODE_ADDR2), EEPROMReadlong(BITS_ADDR));                   //On envoie le code d'extinction correspondant au 2ème bouton
    clignote(2, 100);
  }

  Serial.println(EEPROMReadlong(TYPE_ADDR));
  Serial.println(EEPROMReadlong(CODE_ADDR2));
  Serial.println(EEPROMReadlong(BITS_ADDR));

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinction() {

  for (int i = 128; i < 160; i++) {
    EEPROM.write(i, 0);
  }

  EEPROMoff = 1;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);
  EEPROM.commit();
  clignote(3, 150);

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre avec confirmation (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinctionConf() {


  for (int i = 128; i < 160; i++) {
    EEPROM.write(i, 0);
  }

  EEPROMoff = 2;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);
  EEPROM.commit();
  clignote(3, 150);
//CHOISIR TEMPS
  String page2;
  page2 = htmlHeader("Code d'extinction avec confirmation");
  page2 += "<p><h3>Enregistrez la durée du délai de confirmation :</h3></p>\
            <FORM action='/duree'>\  
                <p><input type='radio' NAME='duree' value='1'> 1 seconde</p>\
                <p><input type='radio' NAME='duree' value='2'> 2 secondes</p>\
                <p><input type='radio' NAME='duree' value='3'> 3 secondes</p>\
                <p><input type='radio' NAME='duree' value='4'> 4 secondes</p>\  
             <p><INPUT TYPE='submit' class='button' VALUE='Enregistrer'></p>\
           </FORM>";
  page2 += htmlFooter();

  server.send ( 200, "text/html", page2 );

}

// --------------------------------------------------------------------------------------
// Enregistrement du delai pour l'envoi
// --------------------------------------------------------------------------------------
void handleDuree() {
  
  int delai=2000;
  
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "duree") {
     delai = server.arg(i).toInt();
      break;
    }
 }
   EEPROM.write(DUREE_ADDR, delai);    //On écrit dans l'EEPROM
   EEPROM.commit();
 
  handleRoot();
}

// --------------------------------------------------------------------------------------
// Enregistrement code premier bouton
// --------------------------------------------------------------------------------------
void handleCodeON() {

  CodeRecep = 1;

  String page2;
  page2 = htmlHeader("Recepteur");
  page2 += "<h3>Activation du recepteur</h3>\
                    <div id='carre'>\
                       <p>Passez le boitier en mode reception et enregistrer le code d'allumage</p>\
                       <FORM action='/'>\
                       <th><INPUT TYPE='submit' class='button' VALUE='Enregistrer'></th>\
                  </FORM></div>";

  page2 += htmlFooter();


  server.send ( 200, "text/html", page2 );
}

// --------------------------------------------------------------------------------------
// Enregistrement code deuxième bouton
// --------------------------------------------------------------------------------------
void handleCodeOFF() {

  CodeRecep = 2;
  EEPROMoff = 3;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);
  EEPROM.commit();

  String page2;
  page2 = htmlHeader("Recepteur");
  page2 += "<h3>Activation du recepteur</h3>\
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

  page2 += htmlFooter();


  server.send ( 200, "text/html", page2 );
}

// --------------------------------------------------------------------------------------
// Erase
// --------------------------------------------------------------------------------------
void handleErase() {

    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
      EEPROM.commit();
    }
  
    clignote(10, 50);
  
    EEPROMoff=0;
  
    configDNS();
  
    server.send ( 310, "text/html", clientRedirect());
    
    handleRoot();
  }





// --------------------------------------------------------------------------------------
// Affichage de la page web de base
// --------------------------------------------------------------------------------------
void handleRoot() {

  server.send ( 200, "text/html", GetIndex());

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

  irsend.begin();                 //On démarre les bilbliothèques
  irrecv.enableIRIn();

  pinMode(A_LED, OUTPUT);         //Déclaration des leds
  digitalWrite(A_LED, LOW);
  pinMode(inter, INPUT_PULLUP);

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


  String dnsName="erasme-vp";
  String tmp = read_StringEE(DNS_ADDR, 50);
  if (tmp != ""){
    dnsName=tmp;
  }

  configDNS();

  server.on("/", handleRoot);
  server.on("/record", handleRecord);
  server.on("/on", handleON);
  server.on("/off", handleOFF);
  server.on("/param/protocole/codeON", handleCodeON);
  server.on("/param/protocole/codeOFF", handleCodeOFF);
  server.on("/param/protocole/extinction", handleExtinction);
  server.on("/param/protocole/extinction/confirmation", handleExtinctionConf);
  server.on("/duree", handleDuree);
  server.on("/erase", handleErase);

  server.onNotFound(handleNotFound);  //Si on ne trouve pas la page

  server.begin();
  Serial.println("HTTP server started");

}

// --------------------------------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------------------------------
void loop(void) {

  server.handleClient();      //On active le serveur
  lecture = digitalRead(inter); //Lecture du switch

  if (lecture == 1 && CodeRecep == 1) {

    if (irrecv.decode(&results)) {

      EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM
      EEPROMWritelong(CODE_ADDR, results.value);         //On écrit le code dans l'EEPROM
      EEPROMWritelong(BITS_ADDR, results.bits);          //On écrit le nombre de bits dans l'EEPROM
      EEPROM.commit();

      afficherDetails();


      irrecv.resume();  // Reçoit la prochaine valeur
    }
  }
  if (lecture == 1 && CodeRecep == 2) {

    if (irrecv.decode(&results)) {

      EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM
      EEPROMWritelong(CODE_ADDR2, results.value);         //On écrit le code dans l'EEPROM
      EEPROMWritelong(BITS_ADDR, results.bits);          //On écrit le nombre de bits dans l'EEPROM
      EEPROM.commit();

      afficherDetails();

      irrecv.resume();  // Reçoit la prochaine valeur
    }
  }
}

