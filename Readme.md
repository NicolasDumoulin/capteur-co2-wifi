# Capteur CO² avec serveur web en wifi

Ce capteur permet de faire des mesures de concentration de CO² dans l'air, de stocker ces mesures et les diffuser sur une page web en se connectant en Wifi au controlleur.

## Matériel utilisé
 - Arduino Nano RP2040 Connect
 - Sensirion SCD30

### Branchement

On utilise le bus I2C. Dans la documentation du RP2040, on trouve page 10 que le port SDA est en A4 et que le port SCL est en A5.
On branche donc les 4 premières broches du capteur SDC30 sur le RP2040 comme suit :
 - VDD -> 3.3V
 - GND -> GND
 - TX/SCL -> A5
 - RX/SDA -> A4

