# Universal Remote
Universal remote controler with arduino

Library Download : https://github.com/markszabo/IRremoteESP8266

## Hardware : 

![alt tag](https://user-images.githubusercontent.com/39366401/40787534-2884d036-64ee-11e8-9586-2d007c10d1d0.jpg)

More information [here](https://github.com/AGoubs/Projet-Stage/blob/master/Doc/README.md)

## Description : 

This box allows you to save a code when the switch is activated (a remote control button code for example). When a code is detected, the              led flashes and the code is stored in the read-only memory of the card.
When the switch is desactivated, you can activate the box with internet and the bix will return the saved code. (To turn your TV on...)

The basic application of this box is to turn on projectors via a web interface:

![alt tag](https://user-images.githubusercontent.com/39366401/41723272-ebfc651a-756a-11e8-8db7-2e225c96b08d.png)

On this interface you can:
- **Allumer :** Send the code to turn on the videoprojector.
- **Eteindre :** Send the code to turn off the videoprojector.
- **Nom du DNS :** Allows to change the DNS name.
- **ON :** Allows to save the code to turn on the videoprojector.
- **OFF :** Allows to save the code to turn off the videoprojector.
- **Protocole d'extinction :** By default, some videoprojector needs to send the code once to turn it off but you can also said if you need to send it twice.

### API web
- http://nom-DNS.local/on : Switch on VP, TV or any IR stuff.
- http://nom-DNS.local/off : Switch off VP, TV or any IR stuff with the defined protocol.

### Restrictions
This IR remote controller works with multicast DNS. It is only reachable by IP or mDNS (name.local) on the local network on whoch it is connected.

/***********************************************************************************************************************************/
# Télécommande universelle

Télécommande universelle commandée par Arduino

Téléchargement de la librairie : https://github.com/markszabo/IRremoteESP8266

## Hardware : 

![alt tag](https://user-images.githubusercontent.com/39366401/40787534-2884d036-64ee-11e8-9586-2d007c10d1d0.jpg)

Plus d'information [ici](https://github.com/AGoubs/Projet-Stage/blob/master/Doc/README.md)

## Description:

Ce boîtier permet d'enregistrer un code lorsque le switch est activé (un code de bouton d'allumage de télécommande par exemple).
Lorsqu'un code est détécté, la led clignote et le code est enregistré dans la mémoire morte de la carte.

La deuxième application de ce boitier est qu'a l'activation sur internet, le boitier peut renvoyer le code enregistré (pour allumer un vidéoprojecteur ou une télé par exemple).


L'application de base de ce boîtier est d'allumer des vidéo-projecteurs via une interface web :

![alt tag](https://user-images.githubusercontent.com/39366401/41723272-ebfc651a-756a-11e8-8db7-2e225c96b08d.png)

Sur cette interface vous pouvez :
- **Allumer :** Envoyer le code d'allumage du projecteur.
- **Eteindre :** Envoyer le code d'extinction du projecteur (défini plus bas).
- **Nom DNS :** Permet de changer le nom DNS du micro-controleur.
- **ON :** Permet d'enregistrer le code d'allumage du vidéoprojecteur (N'enregistrer qu'un seul code si celui-ci permet l'allumage et l'extinction du vidéoprojecteur).
- **OFF :** Enregistrer le code d'extinction du vidéoprojecteur.
- **Protocole d'extinction :** Définit si par défaut, le projecteur nécessite d'envoyer le code une ou deux fois pour s'éteindre.

### API web
- http://nom-DNS.local/on : Allumer le VP, la TV ou tout autre appareil IR en fonction du paramétrage réalisé sur l'interface Web
- http://nom-DNS.local/off : Eteindre le VP, la TV ou tout autre appareil IR selon le protocole défini sur l'IHM web.

### Restrictions
Cette télécommande fonctionne en multicast DNS. Elle est accessible uniquement sur le réseau sur lequel elle vient se connecter (réseau wifi) soit par son IP son par son nom mDNS (nom.local).
