#include <SD.h>
#include <SPI.h>
#include "TMRpcm.h"

TMRpcm audio;
File root;

void setup()
{

  if (!SD.begin()) {
    while(true);  // stay here.
  }
 
  audio.speakerPin = 9;  // set speaker output to pin 9
 
  root = SD.open("/");      // open SD card main root
 
  audio.setVolume(5);    //   0 to 7. Set volume level
  audio.quality(1);      //  Set 1 for 2x oversampling Set 0 for normal
}

void loop()
{
  // if no audio is playing
  if (!audio.isPlaying()) {
    // no audio file is playing
    File entry =  root.openNextFile();  // open next file
    if (!entry) {
      // no more files
      root.rewindDirectory();  // go to start of the folder
      return;
    }

    uint8_t nameSize = String(entry.name()).length();  // get file name size
    String str1 = String(entry.name()).substring( nameSize - 4 );  // save the last 4 characters (file extension)

    if ( str1.equalsIgnoreCase(".wav") ) {
      // the opened file has '.wav' extension
      audio.play( entry.name() );      // play the audio file
    }

    else {
      // not '.wav' format file
      entry.close();
      return;
    }
  }
}