
# Installation

## Arduino IDE

### Setup

Make sure you use a version of the **Arduino IDE** <br>
which is supported by the **ESP8266** platform.

Please follow the **[Tutorial]** on our website.

<br>

### Libraries

Please install the following libraries:

- `ESP8266 Weather Station`
- `JSON Streaming Parser by Daniel Eichhorn`
- `ESP8266 OLED Driver for SSD1306 display by Daniel Eichhorn`

    *Please use `v3.0.0+`*

##### How To

<kbd>**Sketch**</kbd> <br>
 › <kbd>**Include Library**</kbd> <br>
 › <kbd>**Manage Libraries**</kbd>

<br>

### Preparation

1. *[Create An API Key][API Key]* for **OpenWeatherMap**.

2. Open the demo.

    <kbd>**File**</kbd> <br>
     › <kbd>**Examples**</kbd> <br>
     › <kbd>**ESP8266 Weather Station**</kbd> <br>
     › <kbd>**Weather Station Demo**</kbd>

3. Insert the **OpenWeatherMap API Key**.

4. Insert your **WiFi Credentials**.

5. Adjust the location according to **OpenWeatherMap**.

    ↳ `Zurich, CH`
    
6. Adjust the **UTC** offset.


<br>

---

<br>

## PlatformIO

### Setup

1. Choose one of the available IDE integration or the Atom based IDE.

2. Install libraries:
    - `561`
    - `562`
    - `563`
    
    With:
    
    ```sh
    platformio lib install
    ```
    
3. Adapt the [`WeatherStationDemo.ino`][Example] file to your<br>
   needs as show in the **Arduino** preparations.


<!----------------------------------------------------------------------------->

[Tutorial]: https://docs.thingpulse.com/how-tos/Arduino-IDE-for-ESP8266/
[API Key]:  https://docs.thingpulse.com/how-tos/openweathermap-key/
[Example]:  examples/WeatherStationDemo/WeatherStationDemo.ino

