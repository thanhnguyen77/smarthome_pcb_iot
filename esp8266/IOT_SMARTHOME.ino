#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif
#include "DHT.h"
#define DHTPIN 12 
#define DHTTYPE DHT11 // DHT 11
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
int Relay = 14;
DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebasedata;
FirebaseAuth auth;
FirebaseConfig config;
WiFiClientSecure client;
ESP8266WebServer sv(80);
#define API_KEY "AIzaSyDhwiP8gbvQnlR_PBQ89owvlK8nOgumKlM"
#define DATABASE_URL "bd-22c59-default-rtdb.firebaseio.com" //
String path="/";
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbwQTgzDOc6oo8pgXFM8kKdxcHMX41XZg94WvijrN6ln-Zx5zdOm0UI7NkHztJnU_s8";
unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
unsigned long t1=0;
unsigned long t2=0;
unsigned long t3=0;
String x="";
 
void setup() {
  
  Serial.begin(115200);
  
   WiFi.mode(WIFI_STA);
  WiFi.softAP("IOTHOME","12345678");

  Serial.println("kjabfckjsdbvjksdbvkjsdbvkjsdbvkjsdbkvjhbskjvsdvsdbvsđb");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
    sv.on("/",[]{
    Serial.println(String("co ng truy cap:"));
     sv.send(200,"text/html",
     "<meta charset='utf-8'>"
     "<html>"
     "<head>"
     "<title>"
     "IOT HOME"
     "</title>"
     "</head>"
     "<body>"
     "iot home"
     "<form action='caidat'>"
     "<br><br>"
     "TEN WF"
     "<input name='tenwifi' type='text'/>"
     "<br><br>"
     "MK WF"
     "<input name='matkhau' type='text'/>"
     "<br><br>"
     "TEN DN"
     "<input name='tenDN' type='text'/>"
     "<br><br>"
     "MK DN"
     "<input name='matkhauDN' type='text'/>"
     "<br><br>"
     "<input type='submit'/>"
     "</form>"
     "<body>"
     "</html>"
     );
    });
     sv.on("/caidat",ketnoi);
  sv.begin(); 
  dht.begin();

  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, HIGH);
}

void ketnoi(){
  String tenwf=sv.arg("tenwifi");
  String mkwf=sv.arg("matkhau");
  String tentk=sv.arg("tenDN");
  String mktk=sv.arg("matkhauDN");
  Serial.print("tenwf");
   Serial.println(tenwf);
   Serial.print("mkwf");
   Serial.println(mkwf);
   Serial.print("tentb");
   Serial.println(tentk);
   Serial.print("mktb");
   Serial.println(mktk);
   sv.send(200,"text/plain","connected");
   WiFi.begin(tenwf, mkwf);
  Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  client.setInsecure();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);


config.api_key = API_KEY;
   auth.user.email = tentk;
  auth.user.password = mktk;
   config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
}


void sendDataGGSheets(float tem, float hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum); 
   Serial.println("string_humidity" +string_humidity);
  String url = "/macros/s/" + GAS_ID + "/exec?value1="+string_temperature+ "&value2="+ string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}
void readMQ2(){
  int value = analogRead(A0);  
  Firebase.setString(firebasedata,"/gas",value);      
  if(value >= 150){
    Firebase.setString(firebasedata,"/gascaution","1");
  } else Firebase.setString(firebasedata,"/gascaution","0");
  Serial.println(value);
}

void setRelay(){
  if(Firebase.getString(firebasedata,path+"/led"))  x=firebasedata.stringData();
  if(x=="0"){
     digitalWrite(Relay, HIGH);
  }else digitalWrite(Relay, LOW);
  
}

void readDHT(){
  
   float h = dht.readHumidity();
   float t = dht.readTemperature();

   if (isnan(h) || isnan(t)) {
   Serial.println("Failed to read from DHT sensor!");
   return;
   }


   Serial.print("Humidity: ");
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperature: ");
   Serial.print(t);
   Serial.print(" *C ");
   Serial.println("");
   Firebase.setString(firebasedata,"/doam",h);
   Firebase.setString(firebasedata,"/nhietdo",t);
   sendDataGGSheets(t,h);
}


void loop() {
  sv.handleClient();
  
  if(millis()- t1>100){
    setRelay();
    t1=millis();
  }

  if(millis()- t2>100){
    readMQ2();
    t2=millis();
  }

  if(millis()- t3>=60000){
    readDHT();
    t3=millis();
  }
  
}
