# Plug Firmware

## MQTT Topics:
We follow the best practices described here https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf

### Command:

{ "desired": { "doors": "unlocked" } }
{ "desired": { "doors": "locked" } }
{ "desired": { "vehicle": "immoblized" } }
{ "desired": { "vehicle": "unimmoblized" } }

### Telemetry:

