/*
  esp32 text to speech
*/
 
#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>       // for http basic auth 
#include <driver/dac.h>   // Audio dac 

#include <BlynkSimpleEsp32.h> // Blynk for esp32

uint16_t data16;          // wav data 16bit(2 bytes)
uint8_t  left;            // Audio dac voltage

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "YOUR_WIFI_SSID";
const char* pass = "YOUR_WIFI_PASSWORD";

// #VoiceText Web API
// You should get apikey
// visit https://cloud.voicetext.jp/webapi
const String tts_url = "https://api.voicetext.jp/v1/tts";
const String tts_user = "YOUR_TSS_API_KEY"; // set your id
const String tts_pass = "";  // passwd is blank
uint16_t delayus = 60;  // depends on the sampling rate
uint8_t wavHeadersize = 44;  // depends on the wav format 
String tts_parms ="&speaker=show&volume=200&speed=120"; // he has natural(16kHz) wav voice

// #Blynk
// You should get Auth Token.
// visit https://docs.blynk.cc/
const char* auth = "YOUR_BLYNK_AUTH_TOKEN";

// play 16bit wav data 
void playWav16(uint8_t * buffPlay, int len){
    for( int i=0 ; i<len; i+=sizeof(data16)) {
      memcpy(&data16, (char*)buffPlay + i, sizeof(data16));    
      left = ((uint16_t) data16 + 32767) >> 8;  // convert 16bit to 8bit
      dac_output_voltage(DAC_CHANNEL_1, left);
      ets_delay_us(delayus);
    }
}

// text to speech
void text2speech(char * text){
    Serial.println("text to speech");
    
    if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
  
      HTTPClient http;  // Initialize the client library
      size_t size = 0; // available streaming data size
      
      http.begin(tts_url); //Specify the URL
      
      Serial.println();
      Serial.println("Starting connection to tts server...");
  
      //request header for VoiceText Web API
      String auth = base64::encode(tts_user + ":" + tts_pass);
      http.addHeader("Authorization", "Basic " + auth);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String request = String("text=") + URLEncode(text) + tts_parms;
      http.addHeader("Content-Length", String(request.length()));

      //Make the request
      int httpCode = http.POST(request);   
  
      if(httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] POST... code: %d\n", httpCode);
          
          // file found at server
          if(httpCode == HTTP_CODE_OK) {
              
              // get lenght of data (is -1 when Server sends no Content-Length header)
              int len = http.getSize();
              Serial.printf("lenght of data: %d\n", len);

              // create buffer for read
              uint8_t buff[128] = { 0 };
              int buffPoint = 0;

              // get tcp stream
              WiFiClient * stream = http.getStreamPtr();
              
              // read wav header from server
              while(size < wavHeadersize && http.connected() && (len > 0 || len == -1)) {
                  // get available data size
                  size = stream->available();
              }
              if(size >= wavHeadersize) {
                  int c = stream->readBytes(buff, wavHeadersize);
                  if (strncmp((char*)buff + wavHeadersize -8, "data", 4)) {
                     Serial.printf("Error: wav file\n");
                     return;
                  }
                  if (len >= wavHeadersize ) len -=wavHeadersize;
              } else { 
                  Serial.printf("Error: wav file\n");
                  return;
              }
              Serial.printf("wav header confirmed\n");

              // read streaming data from server
              while (http.connected() && (len > 0 || len == -1)) {
                // get available data size
                size = stream->available();
                if (size > 0 ) {
                    int buffLeft = sizeof(buff)-buffPoint;
                    int c = stream->readBytes(buff+buffPoint, ((size > buffLeft) ? buffLeft : size ));
                    //Serial.printf("read stream size: %d\n",c);    
                    buffPoint += c;
                    if (len >=0) len -= c;

                    if (buffPoint >= sizeof(buff)) {
                        playWav16(buff, buffPoint);
                        buff[0] = buff[buffPoint-1];
                        buffPoint = buffPoint % sizeof(data16);
                    }
                 }
              }
              if (buffPoint > sizeof(data16)) {
                playWav16(buff, buffPoint);
              }
              Serial.printf("len: %d  buffPoint: %d\n",len,buffPoint);    
          }
          Serial.println("finish play");
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();  //Free resources
    } else {
        Serial.println("Error in WiFi connection");
    }
    dac_output_voltage(DAC_CHANNEL_1, 0);
}

void setup() {
    Serial.begin(115200);
    dac_output_enable(DAC_CHANNEL_1); // use DAC_CHANNEL_1 (pin 25 fixed)
    dac_output_voltage(DAC_CHANNEL_1, 0);
    
    // WiFi setup
    WiFi.mode(WIFI_STA);  // Disable Access Point
    //WiFi.begin(ssid, pass);
    //while (WiFi.status() != WL_CONNECTED) {
    //  delay(1000);
    //  Serial.println("Connecting to WiFi..");
    //}

    Blynk.begin(auth, ssid, pass); 
 
    Serial.println("Connected to the WiFi network");

}

// Blynk 
// use 'ping blynk-cloud.com' to find ip address
// http://188.166.206.43/YOUR_BLYNK_AUTH_TOKEN/update/v1?value=hello%20world
BLYNK_WRITE(V1)
{
  String pinValue = param.asStr();
  char text[pinValue.length() + 1];
  pinValue.toCharArray(text, pinValue.length() + 1);
  
  Serial.print("BLYNK_WRITE(V1): ");
  Serial.println(pinValue);
  
  text2speech(text);

}

// Blynk 
// use 'ping blynk-cloud.com' to find ip address
// http://188.166.206.43/YOUR_BLYNK_AUTH_TOKEN/update/v2?value=hello%20world
BLYNK_WRITE(V2)
{
  String pinValue = param.asStr();
  char text[pinValue.length() + 1];
  pinValue.toCharArray(text, pinValue.length() + 1);
  
  Serial.print("BLYNK_WRITE(V2): ");
  Serial.println(pinValue);
  
  text2speech(text);
}

void loop() {
  
    Blynk.run();
    
}

// from http://hardwarefun.com/tutorials/url-encoding-in-arduino
// modified by chaeplin
String URLEncode(const char* msg) {
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9')
         || *msg  == '-' || *msg == '_' || *msg == '.' || *msg == '~' ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}

