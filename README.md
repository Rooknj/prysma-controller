# Prysma-Controller

<p align="center">
  <img alt="prysmalight-esp8266" src="./images/esp8266.png" width="480">
  <br/>
  <b>An arduino sketch to control addressable RGB led strips</b>
</p>

## IDE Installation/Setup Steps

1. (mac only) Setup serial recognition - https://apple.stackexchange.com/questions/334311/connection-of-esp32-esp8266-not-recognized-using-macbook-pro
2. Install the Arduino IDE (v1.8.9)
3. Install ESP8266 Board Manager: https://github.com/esp8266/Arduino#installing-with-boards-manager
4. Select the correct board: Tools -> Board -> NodeMCU 1.0
5. Set a fast upload speed: Upload Speed -> 460800

## Install Libraries

- WiFiManager by Tzapu: Version 0.14.0 (or latest)
- PubSubClient by Nick O'Leary: Version 2.7.0 (or latest)
  - Go to ~/Documents/Arduino/libraries/PubSubClient/src/PubSubClient.h and change MQTT_MAX_PACKET_SIZE to 512 instead of 128. This is because the messages sent by this app are greater than 128 bytes and will be ignored by the pubsubclient unless increased.
- ArduinoJson by Benoit Blanchon: Version 6.11.1
- FastLED by Daniel Garcia: Version 3.2.6 (or latest)

## SPIFFS File Uploader Setup

1. https://github.com/esp8266/arduino-esp8266fs-plugin#installation
2. Take templates/config.json and put it in the data folder after filling it out with the appropriate information

## Features

- The builtin LED will be on until the wifi is connected
- It will automatically discover and connect to any MQTT brokers being advertized over MDNS with priority going to prysma.local hostnames
- Arduino OTA sketch and data uploads
  - All config info such as number of leds and mqtt password are loaded from a config file stored in SPIFFS

## MQTT API

### Command Topic: `prysma/<id>/command`

- Fields:
  - mutationId `<Number> (optional)`: Unique id of command
  - id `<String>`: id of the light
  - on `<boolean>`: light is on or off
  - color `<Object {r, g, b}>`: RGB color of light from 0-255
  - brightness `<Number 0-100>`: Brightness of light
  - effect `<String>`: Name of the current effect or "None" for no effect
  - speed `<Number 1-7>`: Effect speed
- Example Command:

```
{
  "mutationId": 10ba038e-48da-487b-96e8-8d3b99b6d18a,
  "id": "Prysma-84F3EBB45500",
  "on": true,
  "brightness": 50,
  "color": {
    "r": 127,
    "g": 255,
    "b": 0
  },
  "effect": "Flash",
  "speed": 4
}
```

### Discovery Topic: `prysma/<id>/discovery`

- Fields:
  - \*: This can be anything
- Example Command:

```
Discover
```

### Identify Topic: `prysma/<id>/identify`

- Fields:
  - \*: This can be anything
- Example Command:

```
Identify
```

### State Topic: `prysma/<id>/state`

- Fields:
  - mutationId `<String (UUIDv4)> (optional)`: Unique id of command that triggered change in state
  - id `<String>`: id of the light
  - on `<boolean>`: light is on or off
  - color `<Object {r, g, b}>`: RGB color of light from 0-255
  - brightness `<Number 0-100>`: Brightness of light
  - effect `<String>`: Name of the current effect or "None" for no effect
  - speed `<Number 1-7>`: Effect speed
- Example Response:

```
{
  "mutationId": 10ba038e-48da-487b-96e8-8d3b99b6d18a,
  "id": "Prysma-84F3EBB45500",
  "on": true,
  "color": {
    "r": 255,
    "g": 255,
    "b": 255
  },
  "brightness": 50,
  "effect": "Flash",
  "speed": 4
}
```

### Connection Topic: `prysma/<id>/connected`

- Fields:
  - id `<String>`: id of the light
  - connected `<boolean>`: whether the light is connected to MQTT or not
- Example Response:

```
{
  "id": "Prysma-84F3EBB45500",
  "connected": true
}
```

### Effect List Topic: `prysma/<id>/effectList`

- Fields:
  - id `<String>`: id of the light
  - effectList `<Array>`: list of supported effects
- Example Response:

```
{
  "id": "Prysma-84F3EBB45500",
  "effectList": [
    "Flash",
    "Fade",
    "Rainbow",
    "Cylon",
    "Confetti",
    "Juggle",
    "Fire",
    "Blue Noise"
    "Visualize",
  ]
}
```

### Configuration Topic: `prysma/<id>/config`

- Fields:
  - id `<String>`: id of the light
  - version `<String>`: Semantic Versioning version of light firmware
  - hardware `<String>`: Type of hardware running the light strip
  - colorOrder `<String>`: order of RGB colors in the light strip
  - stripType `<String>`: Type of LED strip being used
  - ipAddress `<String>`: IP address of the light strip
  - macAddress `<String>`: Mac address of the light strip
  - numLeds `<int>`: number of addressable leds the light strip has
  - udpPort `<int>`: udp port the strip is listening on for visualization packets
- Example Response:

```
{
  "id": "Prysma-84F3EBB45500",
  "version": "1.0.0",
  "hardware": "8266",
  "colorOrder": "GRB",
  "stripType": "WS2812B",
  "ipAddress": "10.0.0.114",
  "macAddress": "84:F3:EB:B4:55:00",
  "numLeds": 60,
  "udpPort": 7778
}
```

### Discovery Response Topic: `prysma/<id>/discoveryResponse`

- Fields:
  - id `<String>`: id of the light
  - version `<String>`: Semantic Versioning version of light firmware
  - hardware `<String>`: Type of hardware running the light strip
  - colorOrder `<String>`: order of RGB colors in the light strip
  - stripType `<String>`: Type of LED strip being used
  - ipAddress `<String>`: IP address of the light strip
  - macAddress `<String>`: Mac address of the light strip
  - numLeds `<int>`: number of addressable leds the light strip has
  - udpPort `<int>`: udp port the strip is listening on for visualization packets
- Example Response:

```
{
  "id": "Prysma-84F3EBB45500",
  "version": "1.0.0",
  "hardware": "8266",
  "colorOrder": "GRB",
  "stripType": "WS2812B",
  "ipAddress": "10.0.0.114",
  "macAddress": "84:F3:EB:B4:55:00",
  "numLeds": 60,
  "udpPort": 7778
}
```

## License

This project is licensed under the terms of the
[MIT license](/LICENSE).
