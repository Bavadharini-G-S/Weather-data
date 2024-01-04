#include <WiFi.h> //to connect with wifi
#include <HTTPClient.h> //to make http get request to open weather(server)
#include <Arduino_JSON.h> //to parse the json data
#include "ThingSpeak.h" //to send data to Thingspeak
WiFiClient client;

unsigned long myChannelNumber = 2388675;
const char * myWriteAPIKey = "94IYZ3SCTZ4MIW3A";

const char* ssid = "Sundar2"; //Network SSID
const char* password = "97283xx*"; //Network password

//Domain name with URL path or IP address with path
String openWeatherMapApiKey = "65687c5ac605b6689ac295b77ac2b34b";
String city = "Delhi";
String countryCode = "IN";

//Default timedelay is set as 10sec for testing
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
String jsonBuffer;

int tempThings = 0;
int pressureThings = 0;
int humidityThings = 0;
int windspeedThings = 0;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.println("Connecting...");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to Wifi Network: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client); //Initializing ThingSpeak
  Serial.println();
}

void loop()
{
  //Send HTTP GET request
  if ((millis() - lastTime) > timerDelay)
  {
    // Wifi conncetion status
    if (WiFi.status() == WL_CONNECTED)
    {
      String serverPath = "https://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&appid=" + openWeatherMapApiKey;
      Serial.println(serverPath);

      //getting data from openweather
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);

      //parsing data
      JSONVar myObject = JSON.parse(jsonBuffer);
      if (JSON.typeof(myObject) == "undefined")
      {
        Serial.println("Parsing input failed");
        return;
      }
      JSONVar temperature = myObject["main"]["temp"];
      JSONVar pressure = myObject["main"]["pressure"];
      JSONVar humidity = myObject["main"]["humidity"];
      JSONVar windspeed = myObject["wind"]["speed"];

      int tempThings = temperature;
      tempThings = tempThings - 273; //converting kelvin to celsius
      int pressureThings = pressure;
      int humidityThings = humidity;
      int windspeedThings = windspeed;

      ThingSpeak.setField(1,tempThings);
      ThingSpeak.setField(2,pressureThings);
      ThingSpeak.setField(3,humidityThings);
      ThingSpeak.setField(4,windspeedThings);

      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if (x==200)
      {
        Serial.println("Channel update successful.");
      }
      else
      {
        Serial.println("Problem in updating channel." );
      }
      Serial.print("Temperature: ");
      Serial.println(tempThings);
      Serial.print("Pressure: ");
      Serial.println(myObject["main"]["pressure"]);
      Serial.print("Humidity: ");
      Serial.println(myObject["main"]["humidity"]);
      Serial.print("Wind Speed: ");
      Serial.println(myObject["wind"]["speed"]);
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime=millis();
  }
}

String httpGETRequest (const char* serverName)
{
  WiFiClient client;
  HTTPClient http;
  // Your domain name with URL path or IP address with path
  http.begin(serverName);
  // Send HTTP POST Request
  int httpResponseCode = http.GET();
  String payload = "{ }";
  if (httpResponseCode>0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.println("Error code: " + httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}