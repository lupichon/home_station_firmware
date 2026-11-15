# Changelog

## [1.1.0] - 2026-11-15
### Added
- État de la porte (ouverte/fermée) récupéré via Bluetooth.
- Délai de 30 minutes avant le premier envoi LoRaWAN après le démarrage, pour laisser les capteurs se stabiliser.

### Changed
- Détection sonore du MAX9814 basée sur une fenêtre glissante de 60 ms, avec seuil recalibré.
- Pilotes FC-51, HC-SR501 et SW-420 factorisés autour d'une classe commune de capteur à interruption.
- Détection des capteurs à interruption adaptée à leur comportement (état actif ou événement ponctuel).
- Architecture de l'écran SSD1306 séparée entre pilote, interface et mise en page.
- Architecture de la gestion de la LED de statut revue.

### Fixed
- Fonctions d'initialisation réorganisées pour éviter un déclenchement de watchdog lorsque l'utilisateur est demandé à entrer un paramètre via la connexion série.

## [1.0.1] - 2026-11-04
### Added
- BMP280 : pression ramenée au niveau de la mer.
- Outil `tools/rgb_led_tuner` (environnement `rgb_led_tuner`) pour régler les couleurs de la LED via le port série.

### Changed
- Couleurs de la LED de statut recalibrées.

### Fixed
- Wi-Fi : Timeout de mise en veille corrigé.
 
## [1.0.0] - 2026-11-01
Première version officielle du firmware.