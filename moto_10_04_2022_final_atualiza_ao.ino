
#include <Arduino.h>
#include <otadrive_esp.h>
/// AKI E UPDATE
 #include <WiFiManager.h> 

#include <EEPROM.h>
//#include <TimeLib.h>
 int8_t    fusoHorario;  // -12 a 12 (inteiro)    - Ex.: -3
 int8_t    autoconect;  // -12 a 12 (inteiro)     - Ex.: -3
 int8_t    look;  // -12 a 12 (inteiro)     - Ex.: -3
 int8_t    pane;
 const   byte  CFG_LUMINOSIDADE  =  0;
 const   byte  CFG_FUSOHORARIO   =  1 + CFG_LUMINOSIDADE;
 const   byte  CFG_UMIDADE       =  1 + CFG_FUSOHORARIO;
 const   byte  CFG_PANE          =  1 + CFG_UMIDADE;
 const   byte  CFG_TOTAL         =  20 ;//+ CFG_USUARIO;

 int8_t getEEPROMInt8(byte offset) {
  // Le dado Int8 da EEPROM (–128 a 127)
  return int8(EEPROM.read(offset));
  }
 void setEEPROMInt8(byte offset, int8_t v) {
  // Grava dado Int8 na EEPROM (–128 a 127)
  EEPROM.write(offset, v);
  }

 unsigned long previousMillis_2 = 0;
 unsigned long previousMillis = 0;        // will store last time LED was updated
 const long interval = 60000;
 const long mini_interval = 1000;

#include <Firebase_ESP_Client.h>
 #include <addons/RTDBHelper.h>
 #define DATABASE_URL "casa-a0f99-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
 #define DATABASE_SECRET "nBOML3NGVw51C7YGdXdHmm4IceW7k12c2Xxe6F1T"

 FirebaseData fbdo;
 FirebaseAuth auth;
 FirebaseConfig config;
 
 String  valor ;

  #define TRIGGER_PIN 13 // 12 ou 13 somente placa do toem  
 unsigned long depoisMillis = 0;
 unsigned long LOOKMillis = 0;
 unsigned long paneMillis = 0;
 #define chave 5  ///5 PELO 4 troquei por boba minha placa 
 #define partida 15 // 15 POR 12 PRA MINHA PLACA
 #define bomba 4   // 4 PELO 5 troquei por  chave minha placa 
 #define D4 2
 int seg = 150;
 int conta = 0;


 char Count = 0;
 boolean   flag  = 0;
 #define tempoDebounce 1500
 unsigned long delayBotao;

 const char*   SSID      = "automacao";
 const char*   PASSWORD  = "356524dj ";


 WiFiServer server(80);
//AsyncWebServer server(80);

 WiFiManager wm; // global wm instance
 WiFiManagerParameter custom_field; // global param ( for non blocking w params )
 #define FPM_SLEEP_MAX_TIME           0xFFFFFFF

 extern "C" {
 #include "user_interface.h"
 }

#define APIKEY "1299fb6e-54cf-4867-9c20-1b7337fb4370"
#define LED 2

void update();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void onUpdateProgress(int progress, int totalt);
uint32_t loopCounter = 0;

void ajuste    ();
void trava     ();
void desconect ();
void CHAVE     ();
void desl      ();
void desliga   ();
void atualiza  ();

#ifdef ESP8266
#define FILESYS LittleFS
#elif defined(ESP32)
#define FILESYS SPIFFS
#endif

void setup()
{
  
 WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  //delay(3000);
  Serial.println("\n Starting");

  EEPROM.begin(CFG_TOTAL);
  fusoHorario   = -3;
  autoconect = -3;// -12 a 12 (inteiro)    Ex.: -3
  look   = -3;
  pane =   -3;
  // Grava EEPROM
  EEPROM.commit();
  fusoHorario   = getEEPROMInt8(CFG_FUSOHORARIO);
  autoconect = getEEPROMInt8(CFG_LUMINOSIDADE);
  look = getEEPROMInt8(CFG_UMIDADE);
  pane = getEEPROMInt8(CFG_PANE);
  
  pinMode(D4,          OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(chave,       OUTPUT);
  pinMode(partida,     OUTPUT);
  pinMode(bomba,       OUTPUT);
 
  digitalWrite(chave,   LOW);
  digitalWrite(partida, LOW);
  digitalWrite(bomba,   LOW);
  digitalWrite(D4,      HIGH);
  // wm.resetSettings(); // wipe settings

  if (look == 1) {
    digitalWrite(bomba, HIGH);
  }

    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;
    Firebase.reconnectWiFi(true);
    Firebase.begin(&config, &auth);
    //////

  ////aki e inicio de config de rede 
//if (digitalRead(TRIGGER_PIN) == LOW){
     WiFiManager wm;
      bool res;

  int customFieldLength = 40;
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

 if ((digitalRead(TRIGGER_PIN) == HIGH) && digitalRead(D5) == LOW){ 
    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
    wm.setMenu(menu);
    wm.setConfigPortalTimeout(120); 
    }
 else {
      std::vector<const char *> menu = {"info","param","sep","restart","exit"};
      wm.setMenu(menu);
      wm.setConfigPortalTimeout(10);
      wm.setCaptivePortalEnable(false);
    }
 
  wm.setClass("invert");
  wm.setScanDispPerc(true);  

 // bool res; 
  res = wm.autoConnect("minhamoto", "356524dj"); // password protected ap
     ///fim de comfiguraçaoes de rede 
   
   server.begin();
  
  ajuste ();
}

String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback() {
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}


void callback() {
  //Serial.println("Callback");
  Serial.flush();

}

#define LIGHT_WAKE_PIN D5

void loop()
{
  
  setEEPROMInt8(CFG_FUSOHORARIO, fusoHorario);
  setEEPROMInt8(CFG_LUMINOSIDADE, autoconect);
  setEEPROMInt8(CFG_UMIDADE, look);
  setEEPROMInt8(CFG_PANE, pane);
  EEPROM.commit();
  valor = WiFi.localIP().toString();
  
  unsigned long currentMillis = millis();
  
if ((millis() - delayBotao) > tempoDebounce){
    if(digitalRead(D5)) flag = 1;
        delayBotao = millis();
     }

  if (WiFi.status() == WL_CONNECTED) {
         Serial.println("conectado");
   if (autoconect == 1) {
         digitalWrite(chave, HIGH);
        }
        
    if (digitalRead(TRIGGER_PIN) == LOW){
         desl();
       }
       
    if (Count <= 5){
          digitalWrite(D4, HIGH);
          digitalWrite(bomba, LOW);
           look = 0;
      if (fusoHorario == 1){  
        Serial.printf("Set String... %s\n", Firebase.RTDB.setString(&fbdo, "/test/String", valor) ? "ok" : fbdo.errorReason().c_str());
            }
       Count++;
          }
          depoisMillis = currentMillis;
      }
///// desconectado 
  else  {
   // Count = 0; 
    Serial.println("desconectado");
    Serial.println(fusoHorario);
    Serial.println(conta);
    Serial.println(look);
    desconect();
  }

  WiFiClient client = server.available();

  if (!client) {
    client.println(WiFi.localIP());
    return;
  }

  
  String req = client.readStringUntil('\r');
     req = req.substring(req.indexOf("/") + 1, req.indexOf("HTTP") - 1);
       Serial.println(req);
      client.flush();
 
  if (req.indexOf("B12030") != -1)// 192.168.4.1/600
  {
    digitalWrite(partida, LOW);
    digitalWrite(chave, HIGH);
   // Serial.println("chave ligada");
  }


  

  else if (req.indexOf("ver") != -1)
  {if ((digitalRead(TRIGGER_PIN) == HIGH) && digitalRead(D5) == LOW){
           client.print("codico de segurança");
           client.print("  == ");
           client.println("2030");
        }
           client.println("ABRA O JAMP");
    }

  else if (req.indexOf("A12030") != -1)
  {
    digitalWrite(chave, LOW);
    autoconect = 0;
   // Serial.println("chave desligada");

  }
  
  else if (req.indexOf("A22030") != -1 )  //&& digitalRead(chave)) ==  HIGH)
  {
    digitalWrite(partida, HIGH);
   // Serial.println("start");
  }
  else if (req.indexOf("A32030") != -1 ) {
    digitalWrite(partida, LOW);
  //  Serial.println("desl. start");
  }

  else if (req.indexOf("eco") != -1)
  {
    fusoHorario = 0;
  }

  else if (req.indexOf("sim") != -1)
  {
    fusoHorario = 1;
  }

  else if (req.indexOf("A42030") != -1)
  {
    autoconect = 1;
  }

  else if (req.indexOf("A52030") != -1)
  {
    autoconect = 0;
    digitalWrite(chave, LOW);
  }

  else if (req.indexOf("atualiza") != -1)
  {
   atualiza();
  }

else if (req.indexOf("ok") != -1)
  {
   client.println("OK");
   //client.println(look);
  }
  else if (req.indexOf("pa1") != -1)
  {
   pane = 1;
  // Serial.println ("habilitou a chave");
  }
  else if (req.indexOf("pa0") != -1)
  {
   pane = 0;
  // Serial.println ("desabilitou a chave");
  }
  else if (req.indexOf("A62030") != -1)
  {
    digitalWrite(chave, LOW);
    digitalWrite(partida, LOW);
    WiFi.mode (WIFI_OFF);
    WiFi.forceSleepBegin ();  //deliga o wifi
    Serial.println ("O WiFi está inoperante");
    delay(2000);
    desliga();
  }

  else if (req.indexOf("A92030") != -1)
  {
   ESP.deepSleep(0);
  }

   else if (req.indexOf("A72030") != -1)
  {
     autoconect = 0;
    digitalWrite(chave, LOW);
       delay(100);
    digitalWrite(bomba, HIGH);
     Serial.println("aciona bomba");
    }

   else if (req.indexOf("A82030") != -1)
    {
    digitalWrite(bomba, LOW);
     Serial.println("desliga bomba");
  }
  
}

void desconect(){
    unsigned long currentMillis = millis();
     Count = 0; 
    // Serial.println("void desconect");
     if (currentMillis - depoisMillis >= 45000) {
         digitalWrite(chave, LOW);
         digitalWrite(bomba, HIGH);
            }
     if (currentMillis - depoisMillis >= 70000) {
           trava();
         }
     if (fusoHorario == 1){ 
           CHAVE();
        }
       }

  void CHAVE () {
    unsigned long currentMillis = millis();
              digitalWrite(D4, LOW);
    if (currentMillis - paneMillis >= 3000){
       if (conta == 5){
        digitalWrite(chave, LOW);
        digitalWrite(bomba, LOW);
           digitalWrite(D4, HIGH);
           setEEPROMInt8(CFG_UMIDADE, look);
           EEPROM.commit();
                delay(1000);
                 desliga();
          }
          
    if ((millis() - delayBotao) > tempoDebounce){
          conta = 0;
         delayBotao = millis();
        }
    }
 
  if(digitalRead(D5)) flag = 1;
  if(!digitalRead(D5) && flag){
       paneMillis = currentMillis;
       flag = 0;
       conta+=1; 
         delay(1000);
       }
     } 
     
       

  void trava (){
    look = 1;
    Serial.println("void travar");
    digitalWrite(chave, LOW);
    digitalWrite(bomba, LOW);
    digitalWrite(D4,    LOW);
       delay(500);
      desliga();
   }
   
  void desl (){
     if (digitalRead(TRIGGER_PIN) == LOW){
              delay(300);
        }
     if (digitalRead(TRIGGER_PIN) == LOW){  
              digitalWrite(chave, LOW);
      if (autoconect == 1) {
                desliga();
          }
      }
  }
  
  void desliga() {
     if (fusoHorario == 1 ){
           delay(4000);
       }
  //gpio_pin_wakeup_enable(GPIO_ID_PIN, GPIO_PIN_INTR_LOLEVEL);
  gpio_pin_wakeup_enable(GPIO_ID_PIN(LIGHT_WAKE_PIN), GPIO_PIN_INTR_LOLEVEL);
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE); // set WiFi mode to null mode
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); // set modem sleep
  wifi_fpm_open(); // enable force sleep
  //wifi_fpm_set_wakeup_cb(wakeup_cb);
  wifi_fpm_set_wakeup_cb(callback);
  wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
    //delay(1000);
  ESP.restart();
}


void ajuste () {
  // initialize FileSystem
  OTADRIVE.setFileSystem(&FILESYS);
#ifdef ESP8266
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
    LittleFS.format();
    return;
  }
#elif defined(ESP32)
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
#endif
  Serial.println("File system Mounted");

  OTADRIVE.setInfo(APIKEY, "2.5.5");

  OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);
}

void atualiza () {
   if (WiFi.status() == WL_CONNECTED)
     {
   
      // retrive firmware info from OTAdrive server
      updateInfo inf = OTADRIVE.updateFirmwareInfo();
      Serial.printf("\nfirmware info: %s ,%dBytes\n%s\n",
    inf.version.c_str(), inf.size, inf.available ? "New version available" : "No newer version");
      // update firmware if newer available
      if (inf.available)
        OTADRIVE.updateFirmware();

      // sync local files with OTAdrive server
      OTADRIVE.syncResources();
      // list local files to serial port
      listDir(FILESYS, "/", 0);

      // get configuration of device
      String c = OTADRIVE.getConfigs();
      Serial.printf("\nconfiguration: %s\n", c.c_str());
    
  }
}

void onUpdateProgress(int progress, int totalt)
{
  static int last = 0;
  int progressPercent = (100 * progress) / totalt;
  Serial.print("*");
  if (last != progressPercent && progressPercent % 10 == 0)
  {
    //print every 10%
    Serial.printf("%d", progressPercent);
  }
  last = progressPercent;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname, "r");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
