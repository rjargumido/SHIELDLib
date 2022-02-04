#include "SHIELDLib.h"

RTC_DS3231 rtc;
WiFiUDP ntpUDP;

Clock _clk;
Wifi _wf;

NTPClient timeClient(ntpUDP, NTP_SERVER_ADDRESS, UTC_OFFSET_IN_SECONDS);

#pragma region Device
/**
 * @brief Initializes the device type which automatically configures the device based on its type. 
 * 
 * @param _deviceAs Set the device type either as BEACON or as NEURON.
 */
Device::Device(DeviceType _setdeviceAs) {
    currentType = _setdeviceAs;
}

/**
 * @brief Starts the device based on the set device type. 
 */
void Device::startDevice() {
    _clk.beginClock();    
    _clk.syncClock();
    FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);

    Serial.print("SHIELD Device ");
    Serial.print(getDeviceType());
    Serial.println(" now running.");
}

/**
 * @brief Returns the SHIELD device type.
 * 
 * @return const char* 
 */
const char* Device::getDeviceType() {
    const char* type = (currentType == BEACON) ? "Beacon" : "Neuron";
    return type;
}

/**
 * @brief Returns device timestamp based on the specifie format.
 * 
 * @param _dtFormat 
 * @return char* 
 */
char* Device::getDeviceTime(DeviceTimeFormat _dtFormat) {
    char* _dt = (char*)malloc(50);
    DateTime now = rtc.now();
    unsigned long _unixtime = now.unixtime();

    switch(_dtFormat){
        case inUnix:            
            ltoa(_unixtime, _dt, 10);
            break;
        
        case inHumanReadableFormat:
            sprintf(_dt, "%02d.%02d.%02d %02d:%02d:%02d", day(_unixtime), month(_unixtime), year(_unixtime), hour(_unixtime), minute(_unixtime), second(_unixtime));
            break;
    }
    return _dt;
}

/**
 * @brief This lights up the builtin LED 
 */
void Device::lightupLED() {
    int _status = 1;    //Get the HealthStatus of the device

    switch(_status) {
        case 1:     //U0 - Healthy or Normal            
            leds[0] = CRGB(0, 255, 0);  //Green
            break;

        case 2:     //U1 - Suspected
            leds[0] = CRGB(255, 165, 0);  //Orange
            break;

        case 3:     //U2 - Positive
            leds[0] = CRGB(255, 0, 0);  //Red
            break;

        case 4:     //U3 - Recovered
            leds[0] = CRGB(0, 0, 255);  //Blue
            break;

        case 5:     //U5 and U6 - System Flag
            leds[0] = CRGB(128,0,128);  //Purple
            break;
    }

    FastLED.show();
}

/**
 * @brief This generates the tag.
 * 
 * @return char* 
 */
char* Device::generateTag() {
    //Performs the protocol PREPROCESSING BLOCK
    StaticJsonDocument<384> doc;
    char* _rawpayload;

    doc["HS"] = "U3";
    doc["CV"] = 1351824120;
    doc["MA"] = "506583791D47";
    doc["CN"] = "9971432991";
    doc["CT"] = "1642088234";
    doc["ET"] = "1642089134";
    doc["IV"] = "y$B&E)H@McQfThWmZq4t7w!z%C*F-JaN";
    doc["IK"] = "3s6v9y$B&E)H@McQfTjWnZr4u7w!z%C*";

    serializeJson(doc, _rawpayload);
    //End Preprocessing Block
    return _rawpayload;
}
#pragma endregion

#pragma region CLOCK
//=============================== Clock Functions ===============================

/**
 * @brief Initializes SHIELD RTC Module. 
 */
void Clock::beginClock() {
    while(!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    delay(10);
  }
  Serial.println("\nClock started.");
}

/**
 * @brief Synchronizes device clock with NTP Server.
 */
void Clock::syncClock() {
    Serial.println("Synchronizing date and time with servers...");
    
    _wf.connect("ARGUMIDO", "odimugra023026");
    timeClient.begin();

    int _timer = 0;
    unsigned long _t;
    do {
        _t = _getNTPUnixTime(); //Connects to the NTP Server and gets Unix time
        _timer += 1;
    }while(_timer < 2);

    rtc.adjust(DateTime(year(_t), month(_t), day(_t), hour(_t), minute(_t), second(_t)));
    Serial.println("Clock successfully synchronized!");
}

/**
 * @brief Connects to the defined NTP Server and retrieves the Unix time.
 */
unsigned long Clock::_getNTPUnixTime() {    
    timeClient.update();
    return timeClient.getEpochTime();
}
//===============================================================================
#pragma endregion

#pragma region Wi-Fi
//=============================== Wi-Fi Functions ===============================

/**
 * @brief Connects to a saved Wi-Fi.
 */
void Wifi::connect(char* wifi_ssid, char* wifi_password) {
    this -> _ssid = wifi_ssid;
    this -> _password = wifi_password;

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
    
    Serial.print("Connecting to ");
    Serial.print(_ssid);
    Serial.println("...");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected.");
}
//===============================================================================
#pragma endregion

#pragma region 
//=============================== Health Status Functions ===============================
HealthStatus Tag::getHealthStatus() {
    return _currentHealthStatus;
}

char* Tag::getContactNumber() {
    return _cnumber;
}

/**
 * @brief Sets the Contact number
 * 
 * @param _contactNumber 
 */
void Tag::_setContactNumber(char* _contactNumber) {
    _cnumber = _contactNumber;
}

/**
 * @brief Sets the current health status
 * 
 * @param _hstatus 
 */
void Tag::_setHealthStatus (HealthStatus _hstatus){
    currentHealthStatus = _hstatus;
}
#pragma endregion