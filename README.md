# Wordclock

Firmware für die WordClock (in der Art von https://www.amazon.de/dp/B01N5TKXP3/) vom c3e. Als Rahmen bietet sich ["RIBBA" von IKEA](https://www.ikea.com/de/de/catalog/products/40378401/) an.
Außerdem wird benötigt:
* Ein ESP8266 (z.B. [WeMos D1 Mini](https://www.amazon.de/dp/B076F53B6S/))
* Ein LED-Strip WS8212 mit 60 LED/m, 1m reicht (also 60 LED) z.B. von eBay
* Kabel in 3 Farben, am besten Draht, Litze 3-adrig flach geht notfalls auch
* Ein USB-Netzteil mit genügend Leistung z.B. https://www.amazon.de/dp/B00WLI5E3M
* Ein USB-Kabel
* Etwa 2.5 m² MDF (Mitteldichte Faserplatte) 2,5mm die gelasert wird
* Karton zum Lasern der Worte
* Viel Geduld beim Löten

## Hardware

Die MDF-Platte zuschneiden und mit den CAD-Dateien lasern (Makerspace in der Umgebung ansteuern, z.B. [Chaosdorf Düsseldorf](https://wiki.chaosdorf.de/Lasercutter), [Das Labor in Bochum](https://wiki.das-labor.org/w/LABOR_Wiki)). Die einzelnen LEDs vom Strip schneiden und auf die Trägerplatte an den vorgezeichneten Stellen aufkleben (z.B. mit UHU Flüssigkleber). Dabei darauf achten, dass die LEDs innerhalb einer Zeile immer die gleiche Ausrichtung (+5V, Din/DO, GND) haben und die Ausrichtungen von Zeile zu Zeile entgegengesetzt sind.
Die LEDs mit Drahtstücken verlöten, dabei darauf achten, dass alle Leitungen jeweils durchgängig verbunden sind (also alle +5V miteinander, alle GND, alle D) und nicht untereinander.
Am Anfang den ESP anlöten:
* 5V->+5V
* G->GND
* D5->Din

## Flashen
1. Dieses Repo clonen (`git clone https://github.com/c3e/wordclock.git`)
2. Die Arduino IDE installieren (https://www.arduino.cc/en/Main/Software)
3. Die Datei `wordclock/wordclock.ino` in der Arduino IDE öffnen
4. In der IDE unter Datei->Voreinstellungen->Zusätzliche Boardverwalter-URLs http://arduino.esp8266.com/stable/package_esp8266com_index.json hinzufügen
5. Unter Werkzeuge->Boards->Boardverwalter nach "esp" suchen, esp8266 installieren
6. Das richtige Board unter Werkzeuge->Boards aus wählen (z.B. WeMos D1 R1)
7. Sketcch->Bibliothek einbinden->Bibliotheken verwalten
8. Folgende installieren:
  * FastLED
  * WiFiManager
9. Folgende Zip-Dateien herunterladen:
  * https://github.com/PaulStoffregen/Time/archive/master.zip
  * https://github.com/JChristensen/Timezone/archive/master.zip
10. In der Arduino IDE unter Sketch->Bibliothek einbinden->.ZIP-Bibliothek hinzufügen die beiden heruntergeladenen .zip-Dateien auswählen
11. Den "Kompilieren"-Haken oben links anklicken (die Warnungen bzgl. Architektur und bitbanging können ignoriert werden)
12. Den ESP per USB anschließen
13. Den "Hochladen"-Pfeil klicken, darauf warten, dass der Upload vollständig ist (wird in Konsole und Statuszeile ausgegeben)

## Inbetriebnahme

1. Strom anschließen, 5V, USB-Netzeil
2. Ins WLAN `WordClock` verbinden (Der Name kann im Quelltext verändert werden)
3. Im Browser die IP `192.168.4.1` aufrufen
4. WLAN auswählen und ggf. Passwort eingeben
5. Nach dem Speichern der Einstellungen verschwindet das WLAN
6. Uhr zeigt die Uhrzeit an

Wenn das WLAN (wird für den Empfang der Uhrzeit benötigt) nicht mehr auffindbar ist, wird das Konfigurations-WLAN unter `192.168.4.1` automatisch wieder aufgebaut. Um die Einstellungen von Hand zurückzusetzen:
In der Arduino-IDE: Werkzeuge->Erase Flash->All Flash contents auswählen, dann den Programmcode erneut hochladen

## Design

Die Idee für das Design wurden vom FabLab Nürnberg übernommen. Die Firmware ist eine Eigenentwicklung.

http://wiki.fablab-nuernberg.de/w/Ding:Wortuhr_von_Udo

## Lizenz

MIT License
