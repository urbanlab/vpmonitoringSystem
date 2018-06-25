# Document technique

##Liste des composant :
- LED Infrarouge OPE5685 850nm (https://fr.rs-online.com/web/p/led-ir/4550802/)
- Recepteur Infrarouge TSOP32238 38KHZ (https://fr.rs-online.com/web/p/recepteurs-ir/9190008/)
- Adafruit Feather HUZZAH esp8266 (https://www.adafruit.com/product/2821)
- Grove Protoshield (https://www.seeedstudio.com/Grove-Protoshield-p-772.html)
- Interrupteur
- LED

##Branchements : 

![alt tag](https://user-images.githubusercontent.com/39366401/41089124-5f3587da-6a41-11e8-883c-0e6aca553fbe.jpg)

Signification des clignotements :
- Allumer : Clignote 2 fois avec 200ms de délai
- Éteindre : Clignote 4 fois avec 100ms de délai
- Enregistrer mode Éteindre dans EEPROM : Clignote 3 fois avec 150ms de délai
- Effacer : Clignote 10 fois avec 50ms de délai
- Enregistrer Code : Clignote 4 fois avec 50ms de délai

## Fonctionnement
2 modes de fonctionnements sont prévus : 
 - Mode émission de codes IR
 - Mode réception de codes IR, un mode d'apprentissage des codes.
Le micro switch permet de de basculer d'un mode à l'autre. Les codes appris sont stockés dans la ROM du micro-controleur.
