#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

const char* ssid = "Keenetic-7719";
const char* password = "43jRtKAb";

const char* HomeServer = "94.19.250.166";//"192.168.1.35";
const char* HomeServer1 = "192.168.1.35";
String apiKey = "2PUS370UGTVHO600";
const char* server = "api.thingspeak.com";

WiFiClient client;

Adafruit_BMP085 bmp;

//boolean start = false;

int delay_s = 0;
int delay_sBackup = 0;
//int delay_sE = 0;
void WiFiInit()
{
  WiFi.begin(ssid, password);
  //Serial.begin(9600);
  //Serial.println();
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("Hello");
  //Serial.println("");
  //Serial.println("WiFi connected");
}

void send(float T, float P, int ADC)
{
    String postStr;
    postStr = "GET /";
    postStr += "temp.php?T=";
    postStr += T;//String(t);
    postStr += "&&P=";
    postStr += P;
    postStr += "&&ADC=";
    postStr += (ADC+1);
    postStr += "\r\n\r\n";
    //Serial.println(postStr);
    //client.print("GET / HTTP/1.0\r\n\r\n");
    //delay(1000);
    client.print(postStr);
    delay(1000);
    boolean Stop = false;
    while (client.available()&& (Stop == false))
    {
      //delay_s = 10;
      String line = client.readStringUntil('\r');
      //Serial.print(line);
      delay_s = atoi(line.c_str());
      delay_sBackup = delay_s;
      if(delay_s !=0)
         {
           Stop = true;
           //Serial.println("delay= " + delay_s);
         }  
    }
}

void setup() {
  
  EEPROM.begin(4);
  int delay_sE = EEPROM.read(0);
  int delay_m  = EEPROM.read(1);
  if(delay_m != 0)
  {
    delay_m--;
    EEPROM.write(1, (uint8_t)delay_m);
    EEPROM.commit();
    ESP.deepSleep(60 * 1000000);
  }

  if(delay_sE !=0)
  {
    EEPROM.write(0, 0);
    EEPROM.commit();
    ESP.deepSleep(delay_sE * 1000000);
  }
  /*if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }*/
  bmp.begin();
  delay(10);
  //pinMode(15, OUTPUT);
  //Serial.begin(9600);
}

void loop() {
  WiFiInit();
  //digitalWrite(15, HIGH);
  float T = bmp.readTemperature();
  float P = bmp.readPressure() * 0.0075;
  int ADC = analogRead(A0);

  
  if (client.connect(HomeServer,8080)) { // "184.106.153.149" or api.thingspeak.com
    send(T, P, ADC);
  } else if(client.connect(HomeServer1,8080)){
    send(T, P, ADC);
  } else{
  
      delay_s = delay_sBackup;
      if(delay_s == 0)
        delay_s = 600;
    //Serial.println(delay_s);
  }
  client.stop();

  if (client.connect(server, 80)) { // "184.106.153.149" or api.thingspeak.com
    //Serial.println("thingspeak connect");
    String postStr = apiKey;
    postStr = "GET /update?key=";
    postStr += apiKey;
    postStr += "&field1=";
    postStr += T;
    postStr += "&field2=";
    postStr += P;
    postStr += "&field3=";
    postStr += ADC;
    postStr += "\r\n\r\n";
    client.print(postStr);
  }
  //else
  //  Serial.println("Error");
  client.stop();
  WiFi.disconnect();
  delay(1000);
  //digitalWrite(15, LOW);

  //Serial.println("Waiting…");
  
  EEPROM.write(0, (uint8_t)(delay_s%60));
  EEPROM.write(1, (uint8_t)(delay_s/60));
       
  EEPROM.write(2, 0);
  EEPROM.write(3, 0);
  EEPROM.commit();
  ESP.reset();
  //ESP.deepSleep(delay_s * 1000000);
}

