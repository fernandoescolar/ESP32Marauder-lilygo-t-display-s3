#include "settings.h"
#include "CSerial.h"

#ifdef USE_FFAT
  #define SETTINGS_FILE "/settings.jso"
#else 
  #define SETTINGS_FILE "/settings.json"
#endif
String Settings::getSettingsString() {
  return this->json_settings_string;
}

bool Settings::begin() {
  #ifndef USE_FFAT
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    CSerial.println("Settings SPIFFS Mount Failed");
    return false;
  }
  #else
  if(!FFat.begin(FORMAT_SPIFFS_IF_FAILED)){
      Serial.println("Settings FFAT Mount Failed");
        return false;
  }
  CSerial.println("Using FFat");

  File root = FFat.open("/");
  if(!root){
    CSerial.println("failed to open root directory - formatiing");
      FFat.format();
    }
  else {
    CSerial.println("FFat root good");
    root.close();
  }
  CSerial.println("Found default fs FFat");
  #endif

  File settingsFile;

  //SPIFFS.remove(SETTINGS_FILE); // NEED TO REMOVE THIS LINE

  #ifndef USE_FFAT
  if (SPIFFS.exists(SETTINGS_FILE)) {
    settingsFile = SPIFFS.open(SETTINGS_FILE, FILE_READ);
  }
  #else
  if(FFat.exists(SETTINGS_FILE)) {
    settingsFile = FFat.open(SETTINGS_FILE, FILE_READ);
  }
  #endif

  if (!settingsFile) {
      settingsFile.close();
      CSerial.println(F("Could not find settings file"));
      #ifndef USE_FFAT
      if (this->createDefaultSettings(SPIFFS))
        return true;
      #else
      if (this->createDefaultSettings(FFat))
        return true;
      #endif
      else
        return false;
  }

  CSerial.println("Created settings file");

  String json_string;
  DynamicJsonDocument jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, settingsFile);
  serializeJson(jsonBuffer, json_string);
  //CSerial.println("Settings: " + (String)json_string + "\n");
  //this->printJsonSettings(json_string);

  this->json_settings_string = json_string;

  return true;
}

template <typename T>
T Settings::loadSetting(String key) {}

// Get type int settings
template<>
int Settings::loadSetting<int>(String key) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key)
      return json["Settings"][i]["value"];
  }

  return 0;
}

// Get type string settings
template<>
String Settings::loadSetting<String>(String key) {
  //return this->json_settings_string;

  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key)
      return json["Settings"][i]["value"];
  }

  return "";
}

// Get type bool settings
template<>
bool Settings::loadSetting<bool>(String key) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key)
      return json["Settings"][i]["value"];
  }

  return false;
}

//Get type uint8_t settings
template<>
uint8_t Settings::loadSetting<uint8_t>(String key) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key)
      return json["Settings"][i]["value"];
  }

  return 0;
}

template <typename T>
T Settings::saveSetting(String key, bool value) {}

template<>
bool Settings::saveSetting<bool>(String key, bool value) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  String settings_string;
  File settingsFile;

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key) {
      json["Settings"][i]["value"] = value;

      CSerial.println("Saving setting...");

      #ifndef USE_FFAT
      settingsFile = SPIFFS.open(SETTINGS_FILE, FILE_WRITE);
      #else
      settingsFile = FFat.open(SETTINGS_FILE, FILE_WRITE);
      #endif

      if (!settingsFile) {
        CSerial.print(F("Failed to create settings file - "));
        CSerial.println(settingsFile, DEC);
        return false;
      }

      if (serializeJson(json, settingsFile) == 0) {
        CSerial.println(F("Failed to write to file"));
      }
      if (serializeJson(json, settings_string) == 0) {
        CSerial.println(F("Failed to write to string"));
      }

      // Close the file
      settingsFile.close();

      this->json_settings_string = settings_string;

      this->printJsonSettings(settings_string);

      return true;
    }
  }
  return false;
}

bool Settings::toggleSetting(String key) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key) {
      if (json["Settings"][i]["value"]) {
        saveSetting<bool>(key, false);
        CSerial.println("Setting value to false");
        return false;
      }
      else {
        saveSetting<bool>(key, true);
        CSerial.println("Setting value to true");
        return true;
      }

      return false;
    }
  }
}

String Settings::setting_index_to_name(int i) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  return json["Settings"][i]["name"];
}

int Settings::getNumberSettings() {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  return json["Settings"].size();
}

String Settings::getSettingType(String key) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, this->json_settings_string)) {
    CSerial.println("\nCould not parse json");
  }

  for (int i = 0; i < json["Settings"].size(); i++) {
    if (json["Settings"][i]["name"].as<String>() == key)
      return json["Settings"][i]["type"];
  }
}

void Settings::printJsonSettings(String json_string) {
  DynamicJsonDocument json(1024); // ArduinoJson v6

  if (deserializeJson(json, json_string)) {
    CSerial.println("\nCould not parse json");
  }

  CSerial.println("Settings\n----------------------------------------------");
  for (int i = 0; i < json["Settings"].size(); i++) {
    CSerial.println("Name: " + json["Settings"][i]["name"].as<String>());
    CSerial.println("Type: " + json["Settings"][i]["type"].as<String>());
    CSerial.println("Value: " + json["Settings"][i]["value"].as<String>());
    CSerial.println("----------------------------------------------");
  }
}

bool Settings::createDefaultSettings(fs::FS &fs) {
  CSerial.println("Creating default settings file");

  File settingsFile = fs.open(SETTINGS_FILE, FILE_WRITE);

  if (!settingsFile) {
    CSerial.print(F("Failed to create settings file - "));
    CSerial.println(settingsFile, DEC);
    return false;
  }

  DynamicJsonDocument jsonBuffer(1024);
  String settings_string;

  //jsonBuffer["Settings"][0]["name"] = "Channel";
  //jsonBuffer["Settings"][0]["type"] = "uint8_t";
  //jsonBuffer["Settings"][0]["value"] = 11;
  //jsonBuffer["Settings"][0]["range"]["min"] = 1;
  //jsonBuffer["Settings"][0]["range"]["max"] = 14;

  //jsonBuffer["Settings"][1]["name"] = "Channel Hop Delay";
  //jsonBuffer["Settings"][1]["type"] = "int";
  //jsonBuffer["Settings"][1]["value"] = 1;
  //jsonBuffer["Settings"][1]["range"]["min"] = 1;
  //jsonBuffer["Settings"][1]["range"]["max"] = 10;

  jsonBuffer["Settings"][0]["name"] = "ForcePMKID";
  jsonBuffer["Settings"][0]["type"] = "bool";
  jsonBuffer["Settings"][0]["value"] = true;
  jsonBuffer["Settings"][0]["range"]["min"] = false;
  jsonBuffer["Settings"][0]["range"]["max"] = true;

  jsonBuffer["Settings"][1]["name"] = "ForceProbe";
  jsonBuffer["Settings"][1]["type"] = "bool";
  jsonBuffer["Settings"][1]["value"] = true;
  jsonBuffer["Settings"][1]["range"]["min"] = false;
  jsonBuffer["Settings"][1]["range"]["max"] = true;

  jsonBuffer["Settings"][2]["name"] = "SavePCAP";
  jsonBuffer["Settings"][2]["type"] = "bool";
  jsonBuffer["Settings"][2]["value"] = true;
  jsonBuffer["Settings"][2]["range"]["min"] = false;
  jsonBuffer["Settings"][2]["range"]["max"] = true;

  jsonBuffer["Settings"][3]["name"] = "EnableLED";
  jsonBuffer["Settings"][3]["type"] = "bool";
  jsonBuffer["Settings"][3]["value"] = true;
  jsonBuffer["Settings"][3]["range"]["min"] = false;
  jsonBuffer["Settings"][3]["range"]["max"] = true;

  //jsonBuffer.printTo(settingsFile);
  if (serializeJson(jsonBuffer, settingsFile) == 0) {
    CSerial.println(F("Failed to write to file"));
  }
  if (serializeJson(jsonBuffer, settings_string) == 0) {
    CSerial.println(F("Failed to write to string"));
  }

  // Close the file
  settingsFile.close();

  this->json_settings_string = settings_string;

  this->printJsonSettings(settings_string);

  return true;
}

void Settings::main(uint32_t currentTime) {

}
