//Bibliotecas utilizadas
#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

//Pinos de conexão do ESP32 e o módulo de cartão SD
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

//Pinos de conexão do ESP32-I2S e o módulo I2S/DAC CJMCU 1334
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

const int button = 34;
int buttonState = 0;

//Cria o objeto que representará o áudio
Audio audio;

//Cria o objeto que representará o Wi-Fi
WiFiMulti wifiMulti;

// Definições da Rede Wi-Fi
String ssid = "yoshikawa2g";
String password = "yyl6333o";

void setup()
{
  //Inicia o Serial para debug
  Serial.begin(115200);

  //Configura e inicia o SPI para conexão com o cartão SD
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  pinMode(button, INPUT);
  
  //Configura e inicia o Wi-Fi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect(true);
    wifiMulti.run();
  }

  //Ajusta os pinos de conexão I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  //Ajusta o volume de saída
  audio.setVolume(18); // 0...21

  //Para executar um arquivo MP3 no cartão SD, descomente esta linha
  //audio.connecttoSD("nome_do_arquivo.mp3");

  //Para executar uma síntese de voz, descomente esta linha
  //audio.connecttospeech("Este é um exemplo de síntese de voz usando esp 32, o protocolo i2s e um módulo d.a.c.", "pt");

  //Para executar um streaming, descomente esta linha
 // audio.connecttohost("http://192.168.0.166:9000/audio"); //  128k mp3
}

void loop()
{
 //Serial.println(buttonState);
  if(digitalRead(button) == HIGH){
  audio.connecttohost("http://192.168.0.166:9000/audio");
  unsigned long starttime = millis();
  unsigned long endtime = starttime;
  int loopcount;
  while ((endtime - starttime) <=6000) // do this loop for up to 6000mS
  {
  //Executa o loop interno da biblioteca audio
    audio.loop();
  loopcount = loopcount+1;
  endtime = millis();
  }
  Serial.print (loopcount,DEC);
 }       
}

// As seguintes funções são opcionais e retornam informações sobre a execução
void audio_info(const char *info)
{
  Serial.print("info        "); Serial.println(info);
}

void audio_id3data(const char *info)
{ //id3 metadata
  Serial.print("id3data     "); Serial.println(info);
}

void audio_eof_mp3(const char *info)
{ //end of file
  Serial.print("eof_mp3     "); Serial.println(info);
}

void audio_showstation(const char *info)
{
  Serial.print("station     "); Serial.println(info);
}

void audio_showstreaminfo(const char *info)
{
  Serial.print("streaminfo  "); Serial.println(info);
}

void audio_showstreamtitle(const char *info)
{
  Serial.print("streamtitle "); Serial.println(info);
}

void audio_bitrate(const char *info)
{
  Serial.print("bitrate     "); Serial.println(info);
}

void audio_commercial(const char *info)
{ //duração
  Serial.print("commercial  "); Serial.println(info);
}

void audio_icyurl(const char *info)
{ //homepage
  Serial.print("icyurl      "); Serial.println(info);
}

void audio_lasthost(const char *info)
{ //stream URL played
  Serial.print("lasthost    "); Serial.println(info);
}

void audio_eof_speech(const char *info)
{
  Serial.print("eof_speech  "); Serial.println(info);
}
