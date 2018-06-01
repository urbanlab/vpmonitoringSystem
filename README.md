# Projet-Stage
Universal remote controler with arduino

Library Download : https://github.com/markszabo/IRremoteESP8266

Hardware : 

    ![alt tag](https://user-images.githubusercontent.com/39366401/40787534-2884d036-64ee-11e8-9586-2d007c10d1d0.jpg)

    List of component :
    - IR LED OPE5685 850nm
    - LED
    - IR Reciever TSOP32238 38KHZ
    - Adafruit Feather HUZZAH esp8266
    - Grove Protoshield
    - Switch

Description : 

This box allows you to save a code when the switch is activated (a remote control button code for example). When a code is detected, the              led flashes and the code is stored in the read-only memory of the card.
When the switch is desactivated, you can activate the box with internet and the bix will return the saved code. (To turn your TV on...)


/******************************************************************************************************************************************/
Télécommande universelle commandée par Arduino

Téléchargement de la librairie : https://github.com/markszabo/IRremoteESP8266

Hardware : 

    ![alt tag](https://user-images.githubusercontent.com/39366401/40787534-2884d036-64ee-11e8-9586-2d007c10d1d0.jpg)

    Liste des composant :
    - LED Infrarouge OPE5685 850nm
    - LED
    - Recepteur Infrarouge TSOP32238 38KHZ
    - Adafruit Feather HUZZAH esp8266
    - Grove Protoshield
    - Interrupteur

Description:

Ce boîtier permet d'enregistrer un code lorsque le switch est activé (un code de bouton d'allumage de télécommande par exemple).
Lorsqu'un code est détécté, la led clignote et le code est enregistré dans la mémoire morte de la carte.

La deuxième application de ce boitier est qu'a l'activation sur internet, le boitier peut renvoyer le code enregistré (pour allumer un vidéoprojecteur ou une télé par exemple).


