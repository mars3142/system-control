# MQTT Client Component for ESP-IDF

Diese Komponente stellt eine einfache MQTT-Client-Implementierung bereit, die Daten an einen TLS-gesicherten MQTT-Broker sendet.

## Dateien
- mqtt_client.c: Implementierung des MQTT-Clients
- mqtt_client.h: Header-Datei
- CMakeLists.txt: Build-Konfiguration
- Kconfig: Konfiguration für die Komponente

## Abhängigkeiten
- ESP-IDF (empfohlen: >= v4.0)
- Komponenten: esp-mqtt, esp-tls

## Nutzung
1. Füge die Komponente in dein Projekt ein.
2. Passe die Konfiguration in `Kconfig` an.
3. Binde die Komponente in deinem Code ein und nutze die API aus `mqtt_client.h`.
