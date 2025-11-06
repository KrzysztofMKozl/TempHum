
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>



#define HOT   13
#define COLD  14
#define WET   15
#define DRY   16
#define DHTPIN 17
#define DHTTYPE DHT22

//requesty do serwera
const char* serverGet = "https://api.restful-api.dev:80/objects/7";

const char* serverPost = "https://api.restful-api.dev:80/";
//string na odpowiedź
String settings;

//setup LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
int dp = 0;


//setup WiFi
const char* ssid = "Kkoz";
const char* password = "12345678";
int status = WL_IDLE_STATUS;

//setup czujnika
DHT dht(DHTPIN, DHTTYPE);


float temp,tempMin,tempMax;
float hum,humMin,humMax;


JSONVar vege;

void setup()
{
    lcd.init();
    lcd.backlight();

    //ustawienie default temp i hum
    temp = 0;
    tempMin=10;
    tempMax=20;
    hum = 25;
    humMin = 40;
    humMax = 60;
    // ustawienie output dla led
    pinMode(HOT, OUTPUT);
    pinMode(COLD, OUTPUT);
    pinMode(WET, OUTPUT);
    pinMode(DRY, OUTPUT);
    
    //uruchom komunikacje seryjną
    Serial.begin(9600);
    Serial.println(F("DHTxx test!"));
  
    //uruchom czujnik
    dht.begin();

    //połącz z wifi
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
    //zczytaj temp i hum
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    if(WiFi.status()== WL_CONNECTED){
    //wysłanie GET request po ustawienia          
      settings = httpGETRequest(serverGet);

      JSONVar myObject = JSON.parse(settings);


      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      //wybranie danych z JSON
      JSONVar keys = myObject.keys();
    //vege = myObject[keys[0]];
      vege = "pomidor";
      humMax = 5 + atof(myObject[keys[0]]);
      tempMax = 2 + atof(myObject[keys[0]]);
      humMin = humMax - 10;
      tempMin = tempMax - 4;

    }
    //zapalenie diod
    outputLedsSet();
    
    //wyczytane dane na serial
    Serial.print(F("Humidity: "));
    Serial.print(hum);
    Serial.print(F("%  Temperature: "));
    Serial.print(temp);
    Serial.print(F("Humidity Max: "));
    Serial.print(humMax);
    Serial.print(F("%  Temperature Max: "));
    Serial.print(tempMax);
    Serial.println(F("°C "));

    //wyświetlanie danych na LCD
    display(dp);
    if(dp == 2)dp=0;
    else dp++;


    delay(10000);

}
//funkcja sterująca diodami
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

//funkcja sterująca LCD
void display(int x){
  lcd.clear();
  lcd.setCursor(0, 0);
  if(x == 0){
    lcd.print(vege);
  }
  else if(x == 1){
    lcd.print("temp");
    lcd.setCursor(0, 1);
    lcd.print(temp);
  }
  else if(x == 2){
    lcd.print("hum");
    lcd.setCursor(0, 1);
    lcd.print(hum);
  }
}
