#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define GFXFF 1
#define FSB9 &FreeSerifBold9pt7b

const char* ssid = "yoshikawa2g";
const char* password = "yyl6333o";
const unsigned long timeout = 30000; // 30 seconds

const int buttonPin = 2;    // the number of the pushbutton pin
const int audio_trigger = 14;
int buttonState;             
int lastButtonState = LOW;   
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 25;    // the debounce time; increase if the output flickers
bool isNormalMode = true;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  pinMode(buttonPin, INPUT);
  pinMode(audio_trigger, OUTPUT);
  
  Serial.println("INIT CAMERA");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // 320x240
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

bool wifiConnect(){
  unsigned long startingTime = millis();
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    if((millis() - startingTime) > timeout){
      return false;
    }
  }
  return true;
}

void buttonEvent(){
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == HIGH) {
        isNormalMode = !isNormalMode;
        Serial.println("--> Button Click");

        //if(!isNormalMode)
          sendingImage();
        //   
      }
    }
  }
  lastButtonState = reading;
}

void sendingImage(){
  camera_fb_t *fb = capture();
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }else{
    Serial.println("--> Image Captured");
    Serial.println("--> Wifi Connecting!");
    
    if(wifiConnect()){
      Serial.println("--> Wifi Connected!");
      postingImage(fb);
    }else{
      Serial.println("Check Wifi credential!");
    }
    esp_camera_fb_return(fb);
  }
}

camera_fb_t* capture(){
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  return fb;
}

void postingImage(camera_fb_t *fb){
  HTTPClient client;
  client.begin("http://192.168.0.166:9000/imageUpdate");
  client.addHeader("Content-Type", "image/jpeg");
  int httpResponseCode = client.POST(fb->buf, fb->len);
  Serial.println("--> Posting Image");
  if(httpResponseCode == 200){
    String response = client.getString();
    parsingResult(response);
  }else{
    //Error
    Serial.println("Check Your Server!!!");
  }
  client.end();
  WiFi.disconnect();
}

void parsingResult(String response){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);
  JsonArray array = doc.as<JsonArray>();
  int yPos = 4;
  Serial.println("--> Objects Detected");
  for(JsonVariant v : array){
    JsonObject object = v.as<JsonObject>();
    const char* description = object["description"];
    float score = object["score"];
    String label = "";
    label += description;
    label += ":";
    label += score;
    
    Serial.printf("        ");
    Serial.println(label);
    yPos += 16;
  }
Serial.println("--------------------------------");
audioTrigger();
}

void audioTrigger(){
  delay(2000);
  digitalWrite(audio_trigger, HIGH);
  delay(200);
  digitalWrite(audio_trigger, LOW);
  }

void showingImage(){
  camera_fb_t *fb = capture();
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }else{
    esp_camera_fb_return(fb);
  }
}

void loop() {
  buttonEvent();
  
  if(isNormalMode)
    showingImage();
  
}
