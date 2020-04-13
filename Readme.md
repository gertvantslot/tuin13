Tuin 13
=======

Doel
----

1. Automatisch controleren van de tuinlamp
2. Automatisch water geven

### Secondaire doelen
* Voorkomen dat lamp/water te lang actief blijft
* Bedienen/controleren op afstand (via Internet)
* Simpele knopbediening (in meterkast)
* Tijden afstemmen op zon-op/onder

Hardware
--------

* ESP-32
* 5V - Voeding ESP-32
* Project box
* Knop met LED indicatoren
* Lamp
  * 220V Relay
  * Stopcontact
* ~~Water~~ Optioneel
  * Water ventiel
  * Water meter

Pins to use
===========

| Pin         | I/O      | Gebruik
|-------------|----------|--------------
| **GPIO ?**  | I-Pullup | _Gereserveerd:_ Knop - Water
| **GPIO ?**  | O        | _Gereserveerd:_ LED-water
| **GPIO 17** | I-Pullup | Knop - Licht
| **GPIO 16** | O        | LED - licht
| **GPIO 5**  | O        | Relay - licht

![Pins][pins]

[pins]: ./img/pinout.jpg
