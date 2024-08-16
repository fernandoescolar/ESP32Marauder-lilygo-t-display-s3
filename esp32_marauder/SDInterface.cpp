#include "SDInterface.h"
#include "lang_var.h"
//#ifdef USE_SD_MMC
#include "SD_MMC.h"
//#endif
#include "CSerial.h"

bool SDInterface::initSD() {
  #ifdef HAS_SD
    String display_string = "";

    #ifdef KIT
      pinMode(SD_DET, INPUT);
      if (digitalRead(SD_DET) == LOW) {
        CSerial.println(F("SD Card Detect Pin Detected"));
      }
      else {
        CSerial.println(F("SD Card Detect Pin Not Detected"));
        this->supported = false;
        return false;
      }
    #endif

    pinMode(SD_CS, OUTPUT);
    delay(10);
    
    #if defined(HAS_SD_SPI)
      this->spiExt = new SPIClass();
      this->spiExt->begin(SD_SPI_SCK, SD_SPI_MISO, SD_SPI_MOSI, SD_CS);
      if (!SD.begin(SD_CS, *(this->spiExt), 27000000)) {
    #elif defined(USE_SD_MMC)
      SD_MMC.setPins(SD_SPI_SCK, SD_SPI_MOSI, SD_SPI_MISO);
      if (!SD_MMC.begin("/sdcard", true, true)) {
    #else
      if (!SD.begin(SD_CS)) {
    #endif
      CSerial.println(F("Failed to mount SD Card"));
      this->supported = false;
      return false;
    }
    else {
      this->supported = true;
      #ifdef USE_SD_MMC
      this->cardType = SD_MMC.cardType();
      this->cardSizeMB = SD_MMC.cardSize() / (1024 * 1024);
      #else
      this->cardType = SD.cardType();
      this->cardSizeMB = SD.cardSize() / (1024 * 1024);
      #endif
      //if (cardType == CARD_MMC)
      //  CSerial.println(F("SD: MMC Mounted"));
      //else if(cardType == CARD_SD)
      //    CSerial.println(F("SD: SDSC Mounted"));
      //else if(cardType == CARD_SDHC)
      //    CSerial.println(F("SD: SDHC Mounted"));
      //else
      //    CSerial.println(F("SD: UNKNOWN Card Mounted"));

      CSerial.printf("SD Card Size: %lluMB\n", this->cardSizeMB);

      if (this->supported) {
        const int NUM_DIGITS = log10(this->cardSizeMB) + 1;

        char sz[NUM_DIGITS + 1];

        sz[NUM_DIGITS] =  0;
        for ( size_t i = NUM_DIGITS; i--; this->cardSizeMB /= 10)
        {
            sz[i] = '0' + (this->cardSizeMB % 10);
            display_string.concat((String)sz[i]);
        }

        this->card_sz = sz;
      }

      #ifdef USE_SD_MMC
      if (!SD_MMC.exists("/SCRIPTS")) {
        SD_MMC.mkdir("/SCRIPTS");
      #else
      if (!SD.exists("/SCRIPTS")) {
        SD.mkdir("/SCRIPTS");
      #endif
        CSerial.println("/SCRIPTS created");
      }

      this->sd_files = new LinkedList<String>();

      this->sd_files->add("Back");

      return true;
  }

  #else
    CSerial.println("SD support disabled, skipping init");
    return false;
  #endif
}

File SDInterface::getFile(String path) {
  if (this->supported) {
    #ifdef USE_SD_MMC
    File file = SD_MMC.open(path, FILE_READ);
    #else
    File file = SD.open(path, FILE_READ);
    #endif

    //if (file)
    return file;
  }
}

bool SDInterface::removeFile(String file_path) {
  #ifdef USE_SD_MMC
  if (SD_MMC.remove(file_path))
  #else
  if (SD.remove(file_path))
  #endif
    return true;
  else
    return false;
}

void SDInterface::listDirToLinkedList(LinkedList<String>* file_names, String str_dir, String ext) {
  if (this->supported) {
    #ifdef USE_SD_MMC
    File dir = SD_MMC.open(str_dir);
    #else
    File dir = SD.open(str_dir);
    #endif
    while (true)
    {
      File entry = dir.openNextFile();
      if (!entry)
      {
        break;
      }

      if (entry.isDirectory())
        continue;

      String file_name = entry.name();
      if (ext != "") {
        if (file_name.endsWith(ext)) {
          file_names->add(file_name);
        }
      }
      else
        file_names->add(file_name);
    }
  }
}

void SDInterface::listDir(String str_dir){
  if (this->supported) {
    #ifdef USE_SD_MMC
    File dir = SD_MMC.open(str_dir);
    #else
    File dir = SD.open(str_dir);
    #endif
    while (true)
    {
      File entry = dir.openNextFile();
      if (! entry)
      {
        break;
      }
      //for (uint8_t i = 0; i < numTabs; i++)
      //{
      //  CSerial.print('\t');
      //}
      CSerial.print(entry.name());
      CSerial.print("\t");
      CSerial.println(entry.size());
      entry.close();
    }
  }
}

void SDInterface::runUpdate() {
  #ifdef HAS_SCREEN
    display_obj.tft.setTextWrap(false);
    display_obj.tft.setFreeFont(NULL);
    display_obj.tft.setCursor(0, TFT_HEIGHT / 3);
    display_obj.tft.setTextSize(1);
    display_obj.tft.setTextColor(TFT_WHITE);

    display_obj.tft.println(F(text15));
  #endif
  #ifdef USE_SD_MMC
  File updateBin = SD_MMC.open("/update.bin");
  #else
  File updateBin = SD.open("/update.bin");
  #endif
  if (updateBin) {
    if(updateBin.isDirectory()){
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_RED);
        display_obj.tft.println(F(text_table2[0]));
      #endif
      CSerial.println(F("Error, could not find \"update.bin\""));
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_WHITE);
      #endif
      updateBin.close();
      return;
    }

    size_t updateSize = updateBin.size();

    if (updateSize > 0) {
      #ifdef HAS_SCREEN
        display_obj.tft.println(F(text_table2[1]));
      #endif
      CSerial.println(F("Starting update over SD. Please wait..."));
      this->performUpdate(updateBin, updateSize);
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_RED);
        display_obj.tft.println(F(text_table2[2]));
      #endif
      CSerial.println(F("Error, file is empty"));
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_WHITE);
      #endif
      return;
    }

    updateBin.close();

      // whe finished remove the binary from sd card to indicate end of the process
    #ifdef HAS_SCREEN
      display_obj.tft.println(F(text_table2[3]));
    #endif
    CSerial.println(F("rebooting..."));
    //SD.remove("/update.bin");
    delay(1000);
    ESP.restart();
  }
  else {
    #ifdef HAS_SCREEN
      display_obj.tft.setTextColor(TFT_RED);
      display_obj.tft.println(F(text_table2[4]));
    #endif
    CSerial.println(F("Could not load update.bin from sd root"));
    #ifdef HAS_SCREEN
      display_obj.tft.setTextColor(TFT_WHITE);
    #endif
  }
}

void SDInterface::performUpdate(Stream &updateSource, size_t updateSize) {
  if (Update.begin(updateSize)) {
    #ifdef HAS_SCREEN
      display_obj.tft.println(text_table2[5] + String(updateSize));
      display_obj.tft.println(F(text_table2[6]));
    #endif
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[7] + String(written) + text_table2[10]);
      #endif
      CSerial.println("Written : " + String(written) + " successfully");
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[8] + String(written) + "/" + String(updateSize) + text_table2[9]);
      #endif
      CSerial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    if (Update.end()) {
      CSerial.println("OTA done!");
      if (Update.isFinished()) {
        #ifdef HAS_SCREEN
          display_obj.tft.println(F(text_table2[11]));
        #endif
        CSerial.println(F("Update successfully completed. Rebooting."));
      }
      else {
        #ifdef HAS_SCREEN
          display_obj.tft.setTextColor(TFT_RED);
          display_obj.tft.println(text_table2[12]);
        #endif
        CSerial.println("Update not finished? Something went wrong!");
        #ifdef HAS_SCREEN
          display_obj.tft.setTextColor(TFT_WHITE);
        #endif
      }
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[13] + String(Update.getError()));
      #endif
      CSerial.println("Error Occurred. Error #: " + String(Update.getError()));
    }

  }
  else
  {
    #ifdef HAS_SCREEN
      display_obj.tft.println(text_table2[14]);
    #endif
    CSerial.println("Not enough space to begin OTA");
  }
}

bool SDInterface::checkDetectPin() {
  #ifdef KIT
    if (digitalRead(SD_DET) == LOW)
      return true;
    else
      return false;
  #endif

  return false;
}

void SDInterface::main() {
  if (!this->supported) {
    if (checkDetectPin()) {
      delay(100);
      this->initSD();
    }
  }
}
