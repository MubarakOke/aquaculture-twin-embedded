#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>

WiFiServer server(80);

const char ssid[]="Mubbie-connect-X";
const char password[]="123456789101112";
const char AWS_IOT_ENDPOINT[]="a1xx3xfwt5w5b0-ats.iot.eu-west-1.amazonaws.com";

// MQTT topics for the device
#define THINGNAME "es32_aquaculture"
#define AWS_IOT_PUBLISH_TOPIC   "digitwin/facility/1/aquaculture/1"
#define AWS_IOT_SUBSCRIBE_TOPIC "digitwin/facility/1/aquaculture/1/control"

WiFiClientSecure wifi_client = WiFiClientSecure();
MQTTClient mqtt_client = MQTTClient(256);

static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUeXYRDmN0scN89PyeI/Gv+tbX2m0wDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMTIwMzExMzIx
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANHe08bGMc9zMh5aqx5V
lFQBMZAgLjO7bNxBpcm4LEXzZ7sfjqYXhbvmRZgXiozc4Vsq/Fy8d6KHZG3X3Y+h
vHFwSaOhSQXhru61Ls6/rjPFhK7Mdmwj/kIaTYeNV9j75OpNcDyt4EbB8GFpROLp
5na9sKrk0fCtbr9oyBVIIeINqWAtmjzdRb8y/0PSr+iF26xZWTjlsP82NoOqN1TL
GVm7WqkHKoftcukyQFDWnbCo6j3wZRjOuvJMlwHJHXqQqe6Aua9BXiIF9HAAkIBK
6qpVPrAWXOLyKpZaHle8ncC6JnwWemdf+2BZxohkmIcRO2B6bVFIJHiWZRFKyNfm
wo0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUxujpnwCedhtuhBDvMD0Is/xjX0cwHQYD
VR0OBBYEFB6NOcECOnIGO9ovZ8d2ud5y5+YcMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAIY4IHdWJmL/ApkpNTFNOTX50P
vAe+9NPGU56Fc4P6/H3fHz5t/0L/ywR4B/457PwK/0gsanzNSTFLaXbzfdnVKx4L
em+GMaK2C2kID0z7JBFhfP9QPOuULuF39dsHzv7giW+WVZsA63A/HunS6iVVJY1z
g6nmfPl0mzrgspfTXAk3pZJ0gvlowAZkqfSn4x9q9l43KTRVIdrveJY6pgD8teQl
/gZVNbQrrc6VIJ4gfb9aS9GFTzWF+zT0m1HRSAEtUlSaeMlIW+mw2ipLRIwcpPZB
US+51o9NTmXPNtZBwIuiwwAck8ZSu3QZ/SeCjXluV07lbipisEc2pBAxEoii
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA0d7TxsYxz3MyHlqrHlWUVAExkCAuM7ts3EGlybgsRfNnux+O
pheFu+ZFmBeKjNzhWyr8XLx3oodkbdfdj6G8cXBJo6FJBeGu7rUuzr+uM8WErsx2
bCP+QhpNh41X2Pvk6k1wPK3gRsHwYWlE4unmdr2wquTR8K1uv2jIFUgh4g2pYC2a
PN1FvzL/Q9Kv6IXbrFlZOOWw/zY2g6o3VMsZWbtaqQcqh+1y6TJAUNadsKjqPfBl
GM668kyXAckdepCp7oC5r0FeIgX0cACQgErqqlU+sBZc4vIqlloeV7ydwLomfBZ6
Z1/7YFnGiGSYhxE7YHptUUgkeJZlEUrI1+bCjQIDAQABAoIBAFYv7kS5bAdHxZNV
pOSGPK3q95G6xX6VP/WMOCuJKRVpCnZ8VTa8fj1WKcp2EH4cz6eDFbR36aGjfIjn
l6O+xgbIGFXMjJKPxild2uUpLr8wJHcsnI14kphO+Pvr/eGsQKxANRGWTn6Actlu
Q830RKMK75ye7+CpOMv+mfyzbMbKGuMiGIW086s4/S/c/pSEB4xJ8GT4KXUOfKNV
VyWnNvaEt0Lp68/WXJCWN514ZNU4at2LzAMGm54iXbnpQbwJNW7kjUFL2aGb0/Ux
wtFZOrahaMBGwekRAbbSRBxrHOVDDM9GEYzAhAasJbQb8a/Jp+zPURlCES/o+YZI
KuJr+wECgYEA82RCFTRfiQmxaD3s3HUywb6f5ZkoiutgYtjBzBquKQQUV20e42cX
HSa+AP4HIsiQFiUD2CCPcQtcZekbgGBDtOQXPCU5J/KwLgaPXsX/zAbqbHr+nBRq
kE/ms1U54azJdrd/w0TcKeIPZlib6Bb+Vo/2GHuR4kHnaLfEQt2mRiECgYEA3L4G
6rdI24tmnFKPm2SJmmkilp5iBZe54lYjnZeIxwl3UpqTij7kj5Aq2kEZdbeEmJxt
dV9n9HXTB63yWWK23cr82G9oWQ9rLkBy9pxoL+TgTCbA8mcgwP1hrd8ARxRXFve3
OAL5JfJVI7VVKT1rmndNvBH3rSoM5DJlntrRFu0CgYEAtrr3zhmy/ByV3/BWCzHG
OKVYQLVbNej/RVb9MLJ7PPn+F6EqRopYVR+IGRCe5ON1IdsKZtITP0jLRwvbBOfB
DWcNdq9vmOlgkj2EmB/JKzGqAGXbD+1YEB6c0tjSivcBjiq4Ni9Vp/A7pZgy/O1+
aBjPJ5Rmc1sNVJZbN+RW+GECgYAKRsJ1rOC0b9HwFxveZRjO2nQkxOzVS9H4ioAP
9x0D/xnR+6ZAulrToCeOj5dK1qdSn32tCCsDw3R2mA9/P8w5mRiaTt1xv/kCZzey
6WckS6hxgyBaaJiqw2EHwij2JilDxXZ/IgxNSvHRvsBfMh/PShyRU9jO8/UzrnDy
q0cupQKBgQDXrovThSV5shmysf4LMkR3oPwJvE1Mm2eV24W1ugjKHReA+yYrCkud
mEODaaIARrMQuIEleQ+11QqcQUwrXj0G4Q6UcKUqeBT/q+dhzw3kx/npxxdfL+DV
VFWbGNfNiEWn8sGCyzwIfpjEMgGb53nV9sg2iHp6hcT6GkWgnlH+0w==
-----END RSA PRIVATE KEY-----
)EOF";

String header;

String temperatureF = "";
String temperatureC = "";

unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

String statePin1 = "off";
String statePin2 = "off";


unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

LiquidCrystal lcd(13,12,14,27,26,25);
OneWire oneWire(15);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress; 




#define TEMPERATURE_PRECISION 9 
#define Tank        do{digitalWrite(trigPin1,HIGH); delayMicroseconds(20);     digitalWrite(trigPin1,LOW);  delayMicroseconds(20);} while(0);
#define pond        do{digitalWrite(trigPin2,HIGH); delayMicroseconds(20);     digitalWrite(trigPin2,LOW);  delayMicroseconds(20);} while(0);
#define onPump1     do{digitalWrite(Pump1,LOW);}       while(0);
#define offPump1    do{digitalWrite(Pump1,HIGH);}      while(0);
#define onPump2     do{digitalWrite(Pump2,LOW);}       while(0);
#define offPump2    do{digitalWrite(Pump2,HIGH);}      while(0);
#define echoPin1 2 
#define trigPin1 4
#define echoPin2 5 
#define trigPin2 18
#define turbity  34
#define Pump1    22
#define Pump2    23
#define pH       35

float ph_act;
float tempC;
int numberOfDevices; 
float calibration_value = 7.00; int phval = 0;  
unsigned long int avgval; int buffer_arr[10],temp;
long  duration1 ,duration2; 
int distance1, distance2,TankLevel,pondLevel; 
int turbityRead;
byte degree [8]={
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
};
void tempSet(void)
{
  numberOfDevices = sensors.getDeviceCount();
  sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
}


void Welcome(void){
  lcd.clear(); 
  lcd.setCursor(6,0); lcd.print("IoT"); 
  lcd.setCursor(5,1); lcd.print("Based"); 
  delay(2000);
  lcd.clear();
  lcd.setCursor(3,0); lcd.print("Automatic");
  lcd.setCursor(3,1); lcd.print("Monitoring");
  delay(2000);
  lcd.clear();
  lcd.setCursor(6,0); lcd.print("And");
  lcd.setCursor(3,1); lcd.print("Control of");
  delay(2000);
  lcd.clear();
  lcd.setCursor(2,0); lcd.print("Aquaculture");
  lcd.setCursor(5,1); lcd.print("System");
  delay(2000);

  lcd.print("Calibrating ");   lcd.setCursor(0,1);   lcd.print("System.");   delay(2000);  lcd.print("."); delay(500);  lcd.print("."); delay(500);
 lcd.print("."); delay(500);
 lcd.clear();   lcd.print("System Ready"); delay(1000);    lcd.print("!"); delay(500);  lcd.print("!"); delay(500);    lcd.print("!"); delay(500);  
  
}


// SetUp Function 
void setup() {
  offPump1; offPump2;
  Serial.begin(4800);
  lcd.begin(16,2);
  Welcome();
  connectAWS();
  sensors.begin();
  tempSet();     
  pinMode(Pump1,OUTPUT);  pinMode(Pump2,OUTPUT);  
  pinMode(trigPin1, OUTPUT); pinMode(trigPin2, OUTPUT); 
  pinMode(echoPin1, INPUT);  pinMode(echoPin2, INPUT);  
  pinMode(turbity,INPUT);
  lcd.createChar(0,degree);
  server.begin();
}
//End Function



/////Loop function
void loop() {
 waterLevel();
 PH_Sensor();
 Turbid();
 printTemp();
 Display();
 control();
 Webpage(); 
 publishMessage(tempC,turbityRead, pondLevel, ph_act, determinepumpStateInteger(statePin1), determinepumpStateInteger(statePin2));
 mqtt_client.loop();
 delay(2000);
}



void Display(void)
{
  lcd.clear();
  lcd.setCursor(0,0);   lcd.print("Tank Level:");    lcd.print(TankLevel);   lcd.print("%");
  lcd.setCursor(1,1);   lcd.print("Pond Level:");    lcd.print(pondLevel);   lcd.print("%");
  delay(1000);
  lcd.clear();
  lcd.print("Temp: ");   lcd.print(tempC);           lcd.write(byte(0));  lcd.print("C");
  lcd.setCursor(0,1);    lcd.print("pH Value: ");    lcd.print(ph_act);
  delay(1000);
  lcd.clear();
  lcd.setCursor(3,0);    lcd.print("Turbidity");     lcd.setCursor(4,1);   lcd.print(turbityRead); lcd.print(" NTU");
  delay(1000);
}


void control(void)
{
  if ((pondLevel<= 70) && (TankLevel >=5))
  {
    onPump2; 
  }
  else
  {
   offPump2; 
  }
   if (pondLevel >= 70)
 {
  offPump2;        
 }
 else if ((pondLevel >= 70) && (TankLevel >=5))
 {
 if (tempC >= 34)
 {
  onPump1; delay(3000); onPump2;
 }
 else
 {
  offPump1; offPump2;
 }
 }
 
}
/// Level Function 
void waterLevel(void){
 pond;
 duration2 = pulseIn(echoPin2,HIGH); distance2 = duration2 * 0.034 / 2;
 Tank;
 duration1 = pulseIn(echoPin1, HIGH); distance1 = duration1 * 0.034 / 2; 
 TankLevel =  (23 - distance1)* 4.348;  pondLevel =  (23 - distance2)* 4.348;
 Serial.print("pond");
 Serial.println(distance2);
 Serial.print("Tank");
 Serial.println(distance1);
  }
//End function


//Turbity Function 
void Turbid(void)
{
  int tur = analogRead(turbity)/4;
  turbityRead = tur - 200;
  Serial.println(turbityRead); 
}

///// pH Functuion 
void PH_Sensor(void){
 for(int i=0;i<10;i++) { 
      buffer_arr[i]=analogRead(pH);
      delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j]){
 temp=buffer_arr[i]; buffer_arr[i]=buffer_arr[j]; buffer_arr[j]=temp;
 }}}
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i]; 
 float volt = (float)avgval * 5.0/4096/6; 
 ph_act = -5.70 * volt + calibration_value;
}
//////End function 


void printTemp(void)
{
  sensors.requestTemperatures(); 
  for(int i=0;i<numberOfDevices; i++)
  {
    if(sensors.getAddress(tempDeviceAddress, i))
  {
    printTemperature(tempDeviceAddress);
  } 
  }
}
//Temperature function 
void printTemperature(DeviceAddress deviceAddress)
{
   tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC); // Converts tempC to Fahrenheit
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    //Serial.print(deviceAddress[i], HEX);
  }
}
//End function 

void Webpage(){
  WiFiClient client = server.available(); 

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /22/on") >= 0) {
              statePin1 = "on";
              onPump1;              // turns the LED on
            } else if (header.indexOf("GET /22/off") >= 0) {
              statePin1 = "off";
              offPump1;               //turns the LED off
            }
            
            if (header.indexOf("GET /23/on") >= 0) {
              statePin2 = "on";
              onPump2;              // turns the LED on
            } else if (header.indexOf("GET /23/off") >= 0) {
              statePin2 = "off";
              offPump2;              //turns the LED off
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: gray;}</style></head>");

            client.println("<body><h1>IOT-base automatic monitoring and control of aquaculture system</h1>");
           
            client.println("<p>Temperature:</p>"); client.print(tempC);  client.println("<p>Turbidity:</p>"); client.print(turbityRead);
            client.println("<p>pH Value:</p>"); client.print(ph_act);   client.println("<p>Pond Level:</p>"); client.print(pondLevel);
            
            client.println("<p>Control of pumps</p>");
            if (statePin1 == "off") {
              client.println("<p><a href=\"/22/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/22/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            if(statePin2 == "off") {
              client.println("<p><a href=\"/23/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/23/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            client.println();
            // Break out of the while loop
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
   // client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void connectAWS()
{
  //Begin WiFi in station mode
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi");

  //Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure wifi_client with the correct certificates and keys
  wifi_client.setCACert(AWS_CERT_CA);
  wifi_client.setCertificate(AWS_CERT_CRT);
  wifi_client.setPrivateKey(AWS_CERT_PRIVATE);

  //Connect to AWS IOT Broker. 8883 is the port used for MQTT
  mqtt_client.begin(AWS_IOT_ENDPOINT, 8883, wifi_client);

  //Set action to be taken on incoming messages
  mqtt_client.onMessage(incomingMessageHandler);

  Serial.print("Connecting to AWS IOT");
  mqtt_client.connect(THINGNAME);

  //Wait for connection to AWS IoT
  if (mqtt_client.connected()) {
    Serial.print(".connected");
    delay(100);
  }
  else{
    Serial.print(mqtt_client.lastError());
    Serial.print(mqtt_client.returnCode());
    connectAWS();
  }
  Serial.println();

  if(!mqtt_client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  //Subscribe to a topic
  mqtt_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(float tempC,int turbityRead, int pondLevel, float ph_act, int drain_power_status, int pump_power_status)
{
  //Create a JSON document of size 200 bytes, and populate it
  //See https://arduinojson.org/
  StaticJsonDocument<200> doc;
  doc["ph_value"] = ph_act;
  doc["turbidity"] = turbityRead;
  doc["temperature"] = tempC;
  doc["water_level"] = pondLevel;
  doc["drain_power_status"] = drain_power_status;
  doc["pump_power_status"] = pump_power_status;
  doc["alarm_message"] = evaluatePondStatus(tempC, ph_act, turbityRead, pondLevel);
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to mqtt_client

  //Publish to the topic
  mqtt_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("Sent a message");
}

void incomingMessageHandler(String &topic, String &payload) {
  Serial.println("Message received!");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);

   // Parse the JSON payload
  DynamicJsonDocument doc(200); // Create a JSON document
  deserializeJson(doc, payload); // Deserialize the JSON

  if (doc["drain_power_status"] == 1) {
    statePin1 = "on";
  }
  if (doc["drain_power_status"] == 0) {
    statePin1 = "off";
  }
  if (doc["pump_power_status"] == 1) {
    statePin2 = "on";
  }
  if (doc["drain_power_status"] == 0) {
    statePin2 = "off";
  }
}

String evaluatePondStatus(float temperature, float ph, float turbidity, float waterLevel) {
    String temperatureStatus = (temperature >= 10 && temperature <= 28) ? "normal" : "abnormal";
    String phStatus = (ph >= 6.5 && ph <= 8.5) ? "normal" : "abnormal";
    String turbidityStatus = (turbidity <= 10) ? "normal" : "abnormal";
    String waterLevelStatus = (waterLevel >= 25) ? "normal" : "abnormal";
    
    String statusList[4] = {temperatureStatus, phStatus, turbidityStatus, waterLevelStatus};
    String risk = calculateRisk(statusList, 4); // Assuming calculateRisk() function is available
    
    return risk;
}

String calculateRisk(String statusList[], int listSize) {
    int abnormalCount = 0;
    for (int i = 0; i < listSize; i++) {
        if (statusList[i] == "abnormal") {
            abnormalCount++;
        }
    }

    String risk;
    if (abnormalCount == 0) {
        risk = "Normal";
    } else if (abnormalCount == 1) {
        risk = "Low";
    } else if (abnormalCount == 2) {
        risk = "Medium";
    } else {
        risk = "High";
    }

    return risk;
}

int determinepumpStateInteger(String statePin) {
    if (statePin == "off") {
        return 0;
    } else {
        return 1;
    }
}