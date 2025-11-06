
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>


#define HOT   13
#define COLD  14
#define WET   15
#define DRY   16
#define DHTPIN 17
#define DHTTYPE DHT22

const char* serverGet = "https://api.restful-api.dev:80/objects/7";

const char* serverPost = "https://api.restful-api.dev:80/";
String settings;


int status = WL_IDLE_STATUS;

const char* ssid = "Kkoz";
const char* password = "12345678";

DHT dht(DHTPIN, DHTTYPE);


float temp,tempMin,tempMax;
float hum,humMin,humMax;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup()
{
    temp = 0;
    tempMin=10;
    tempMax=20;
    hum = 25;
    humMin = 40;
    humMax = 60;
    // Set LED as output
    pinMode(HOT, OUTPUT);
    pinMode(COLD, OUTPUT);
    pinMode(WET, OUTPUT);
    pinMode(DRY, OUTPUT);
    
    Serial.begin(9600);
    Serial.println(F("DHTxx test!"));
    dht.begin();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
        while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    
}



void loop()
{   
    
    hum = dht.readHumidity();
    temp= dht.readTemperature();

    if(WiFi.status()== WL_CONNECTED){
              
      settings = httpGETRequest(serverGet);
      Serial.println(settings);
      JSONVar myObject = JSON.parse(settings);
  
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      JSONVar keys = myObject.keys();
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = myObject[keys[i]];
        Serial.print(keys[i]);
        Serial.print(" = ");
        Serial.println(value);
      }
      humMax = 5 + atof(myObject[keys[0]]);
      tempMax = 2 + atof(myObject[keys[0]]);
      humMin = humMax - 10;
      tempMin = tempMax - 4;

    }
    
    outputLedsSet();
    Serial.print(F("Humidity: "));
    Serial.print(hum);
    Serial.print(F("%  Temperature: "));
    Serial.print(temp);
    Serial.print(F("Humidity Max: "));
    Serial.print(humMax);
    Serial.print(F("%  Temperature Max: "));
    Serial.print(tempMax);
    Serial.println(F("°C "));
    delay(10000);
}

void outputLedsSet(){
  if(temp < tempMin){
    digitalWrite(COLD,HIGH);
  }else{
    digitalWrite(COLD,LOW);
  }

  if(temp > tempMax){
    digitalWrite(HOT,HIGH);
  }else{
    digitalWrite(HOT,LOW);
  }

  if(hum < humMin){
    digitalWrite(DRY,HIGH);
  }else{
    digitalWrite(DRY,LOW);
  }

  if(hum > humMax){
    digitalWrite(WET,HIGH);
  }else{
    digitalWrite(WET,LOW);
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);

  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}
void httpPOSTRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  JSONVar myObject;
    
  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");

  myObject["temp"] = temp;
  myObject["hum"] = hum;
  
  int httpResponseCode = http.POST(myObject);
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  http.end();

}
