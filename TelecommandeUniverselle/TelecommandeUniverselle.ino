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
#define DNS_ADDR 160

const int inter = 15;       //Interrupteur sur la PIN 15
uint16_t RECV_PIN = 14;     //Recepteur sur la PIN 14
const int IR_LED = 13;      //LED infrarouge sur la PIN 13
const int A_LED = 12;       //LED sur la PIN 12
int lecture;                //Variable du switch
int CodeRecep = 0;             //Variable du deuxième bouton
int EEPROMoff;                 //Variable du deuxième bouton dans l'EEPROM
String nomduDNS = "erasmeVP";  //Variable du nom du DNS (de base "erasmevp")

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
// Fonction d'envoi du code IR
// --------------------------------------------------------------------------------------
void sendCode(int CodeAdressage) {
  irsend.send(EEPROMReadlong(TYPE_ADDR), EEPROMReadlong(CodeAdressage), EEPROMReadlong(BITS_ADDR));
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
            <title>Erasme VP</title>\
            <style>\
              body { font-family: Arial, Helvetica, Sans-Serif; Color: #4CAF50;}\
               #carre  {box-sizing: content-box; width: auto;height: auto; padding: 10px; border: 2px solid #4CAF50};\
            </style>\
          </head>\
          <body>\
          <h1>" + title + "</h1>";
}

String htmlFooter() {
  return "</body>\
        </html>";
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
      break;
    }
  }
  write_StringEE(DNS_ADDR, recordDNS);    //On écrit en String dans l'EEPROM

  // Redirection
  handleRoot();

  nomduDNS = (read_StringEE(DNS_ADDR, (recordDNS.length() + 1))); //On met la valeur écrite dans l'EEPROM dans une variable

  configDNS();      //On actualise le DNS

}

// --------------------------------------------------------------------------------------
// Allumer
// -------------------------------------------------------------------------------------
void handleON() {

  sendCode(CODE_ADDR);  //On envoie le code qui correspond au code de la télécommande

  clignote(2, 200);

  handleRoot();
}


// --------------------------------------------------------------------------------------
// Eteindre avec l'EEPROM
// --------------------------------------------------------------------------------------
void handleOFF() {

  if (EEPROMReadlong(OFF_ADDR) == 1) {    //Si Eteindre à été choisi
    sendCode(CODE_ADDR);                  //On envoie qu'une seul fois le code d'extinction
    clignote(4, 100);
  }

  else if (EEPROMReadlong(OFF_ADDR) == 2) { //Si Eteindre avec confirmation à été choisi
    sendCode(CODE_ADDR);                    //On envoie deux fois le code d'extinction
    clignote(4, 100);

    delay(1500);                            //Avec un délai entre les deux pour la confirmation

    sendCode(CODE_ADDR);
    clignote(4, 100);
  }

  else if (EEPROMReadlong(OFF_ADDR) == 3) { //Si Code d'extinction different de l'allumage à été choisi
    sendCode(CODE_ADDR2);                   //On envoie le code d'extinction correspondant au 2ème bouton
    clignote(2, 100);
  }

  handleRoot();
}


// --------------------------------------------------------------------------------------
// Eteindre
// --------------------------------------------------------------------------------------
void handleOFF1() {

  sendCode(CODE_ADDR);
  clignote(4, 100);

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre avec confirmation
// --------------------------------------------------------------------------------------
void handleOFF2() {

  sendCode(CODE_ADDR);
  clignote(4, 100);

  delay(1500);

  sendCode(CODE_ADDR);
  clignote(4, 100);

  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre (2ème bouton)
// --------------------------------------------------------------------------------------
void handleOFF3() {

  sendCode(CODE_ADDR2);
  clignote(4, 100);
  
  handleRoot();
}

// --------------------------------------------------------------------------------------
// Eteindre (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinction() {

  for (int i = 128; i < 160; i++)
    EEPROM.write(i, 0);

  EEPROMoff = 1;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);

  clignote(3, 150);

  String page2;
  page2 = htmlHeader("Code d'extinction sans confirmation");
  page2 += "<a input type href='/'>Retour</a>";
  page2 += htmlFooter();

  server.send ( 200, "text/html", page2 );
}

// --------------------------------------------------------------------------------------
// Eteindre avec confirmation (EEPROM)
// --------------------------------------------------------------------------------------
void handleExtinctionConf() {

  for (int i = 128; i < 160; i++)
    EEPROM.write(i, 0);

  EEPROMoff = 2;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);

  clignote(3, 150);

  String page2;
  page2 = htmlHeader("Code d'extinction avec confirmation");
  page2 += "<a input type href='/'>Retour</a>";
  page2 += htmlFooter();

  server.send ( 200, "text/html", page2 );
}

// --------------------------------------------------------------------------------------
// Code d'extinction different de l'allumage
// --------------------------------------------------------------------------------------
void handleCodediff() {

  CodeRecep = 1;
  EEPROMoff = 3;
  EEPROMWritelong(OFF_ADDR, EEPROMoff);

  String page2;
  page2 = htmlHeader("Recepteur");
  page2 += "<h3>Activation du recepteur</h3>\
                    <div id='carre'>\
                       <p>Passez le boitier en mode reception et enregistrer le code d'extinction</p>\
                       <p><a input type href='/'>Enregistrer</a></p></div>";

  page2 += htmlFooter();


  server.send ( 200, "text/html", page2 );
}

// --------------------------------------------------------------------------------------
// Erase
// --------------------------------------------------------------------------------------
void handleErase() {

  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);

  clignote(10, 50);

  handleRoot();
}


// --------------------------------------------------------------------------------------
// Affichage de la page web de base
// --------------------------------------------------------------------------------------
void handleRoot() {

  CodeRecep = 0;
  String page;

  String messages = "Nom du DNS : ";
  messages += nomduDNS;
  messages += " / Adresse IP : ";
  messages += humanReadableIp(WiFi.localIP());

  page = htmlHeader(messages);
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
               <FORM action='/record'>\
                  <p><input type='text' NAME='dnsName'>.local</a></p>\
                  <INPUT TYPE='submit' VALUE='Envoyer'>\
                </FORM>\
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

  configDNS();

  server.on("/", handleRoot);
  server.on("/record", handleRecord);
  server.on("/on", handleON);
  server.on("/off", handleOFF);
  server.on("/off1", handleOFF1);
  server.on("/off2", handleOFF2);
  server.on("/off3", handleOFF3);
  server.on("/param/protocole/extinction", handleExtinction);
  server.on("/param/protocole/extinction/confirmation", handleExtinctionConf);
  server.on("/param/protocole/codediff", handleCodediff);
  server.on("/erase", handleErase);

  server.onNotFound(handleNotFound);  //Si on ne trouve pas la page

  server.begin();
  Serial.println("HTTP server started");

}


// --------------------------------------------------------------------------------------
// On écrit en string dans l'EEPROM  (Lien de la page Web : http://mario.mtechcreations.com/programing/write-string-to-arduino-eeprom/)
// --------------------------------------------------------------------------------------
bool write_StringEE(int Addr, String input)
{
  char cbuff[input.length() + 1]; //Finds length of string to make a buffer
  input.toCharArray(cbuff, input.length() + 1); //Converts String into character array
  return eeprom_write_string(Addr, cbuff); //Saves String
}

//Updated 4/10/16 cast type replaces concat

//Given the starting address, and the length, this function will return
//a String and not a Char array
String read_StringEE(int Addr, int length)
{

  char cbuff[length + 1];
  eeprom_read_string(Addr, cbuff, length + 1);

  String stemp(cbuff);
  return stemp;

}

boolean eeprom_write_bytes(int startAddr, const byte * array, int numBytes) {
  // counter
  int i;


  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }

  return true;
}


// Writes a string starting at the specified address.
// Returns true if the whole string is successfully written.
// Returns false if the address of one or more bytes fall outside the allowed range.
// If false is returned, nothing gets written to the eeprom.
boolean eeprom_write_string(int addr, const char* string) {

  int numBytes; // actual number of bytes to be written

  //write the string contents plus the string terminator byte (0x00)
  numBytes = strlen(string) + 1;

  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
  byte ch; // byte read from eeprom
  int bytesRead; // number of bytes read so far

  if (bufSize == 0) { // how can we store bytes in an empty buffer ?
    return false;
  }

  // is there is room for the string terminator only, no reason to go further
  if (bufSize == 1) {
    buffer[0] = 0;
    return true;
  }

  bytesRead = 0; // initialize byte counter
  ch = EEPROM.read(addr + bytesRead); // read next byte from eeprom
  buffer[bytesRead] = ch; // store it into the user buffer
  bytesRead++; // increment byte counter

  // stop conditions:
  // - the character just read is the string terminator one (0x00)
  // - we have filled the user buffer
  // - we have reached the last eeprom address
  while ( (ch != 0x00) && (bytesRead < bufSize)) {
    // if no stop condition is met, read the next byte from eeprom
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch; // store it into the user buffer
    bytesRead++; // increment byte counter
  }

  // make sure the user buffer has a string terminator, (0x00) as its last byte
  if ((ch != 0x00) && (bytesRead >= 1)) {
    buffer[bytesRead - 1] = 0;
  }

  return true;
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
void loop(void) {

  server.handleClient();      //On active le serveur
  lecture = digitalRead(inter); //Lecture du switch

  if (lecture == 1 && CodeRecep == 0) {

    if (irrecv.decode(&results)) {

      EEPROMWritelong(TYPE_ADDR, results.decode_type);   //On ecrit le type de codage dans l'EEPROM
      EEPROMWritelong(CODE_ADDR, results.value);         //On écrit le code dans l'EEPROM
      EEPROMWritelong(BITS_ADDR, results.bits);          //On écrit le nombre de bits dans l'EEPROM
      EEPROM.commit();

      afficherDetails();
      

      irrecv.resume();  // Reçoit la prochaine valeur
    }
  }
  if (lecture == 1 && CodeRecep == 1) {

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

