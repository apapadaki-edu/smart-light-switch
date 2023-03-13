#define BLYNK_TEMPLATE_ID "tempid"
#define BLYNK_DEVICE_NAME "testnode"
#define BLYNK_AUTH_TOKEN "authtoken"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <PubSubClient.h>

#define WIFI_SSID "wifyname"
#define WIFI_PASSWORD "password"
#define servoPin 2
#define lightInputPin A0

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;
const char* mqtt_server = "broker.hivemq.com";
int mqtt_port =1883;
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

//initialize real time clock and servo motor objects
Servo servo;
ThreeWire clkWire(5,4,16);
RtcDS1302<ThreeWire> Rtc(clkWire);

//light measurement variables
int lightVal;
const unsigned long lightReadInterval= 4000; // Time interval for light sensor
unsigned long previousTime = 0;
char lightString[50];
char servoString[20];

//blynk connection read params
int appState;
BLYNK_WRITE(V0)
{
  appState = param.asInt();
}
BLYNK_CONNECTED(){
  Blynk.syncVirtual(V0);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  client.setServer(mqtt_server, mqtt_port);
  
  pinMode(servoPin,INPUT);
  servo.attach(servoPin);
  servo.write(0);

  Rtc.Begin();
  RtcDateTime now = Rtc.GetDateTime();
  setTimeClock(now);
}

void loop() {
  Blynk.run();

  // checks connection with the mqtt broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  /*read arduino clock time*/
  unsigned long currentTime = millis();
  /*real time clock module time value*/
  RtcDateTime now = Rtc.GetDateTime();


  // Important piece that controlls the sensor
  if(!Blynk.connected()){
    //Turn on switch when light is low
    if (lightVal <= 150 && (int) now.Hour() < 17 ) {  //layter add condition of time;
        servo.write(130); // open switch
    }else {
        servo.write(0);// close switch
    }
  }else {
    if (appState == 1){
      servo.write(130);
    }else{
      servo.write(0);
    }
  }

    /*read the light measurement every 10 minutes*/
  if (currentTime-previousTime >= lightReadInterval){
      lightVal = readLightVal(lightInputPin);
      Serial.println(servo.read());
      formatNumber(lightVal, servo.read());
      client.publish("luminocity/SwitchPj3/dat", lightString);
      previousTime = currentTime;
  } 
}

void formatNumber(int light, int servo){
  char servoMeas[20];
  char tempLight[20];
  char del[2]= ":";
  itoa(light, tempLight, 10);
  itoa(servo, servoMeas, 10);
  strcpy(lightString, tempLight);
  strcpy(servoString, servoMeas);
  strcat(lightString, del);
  strcat(lightString, servoString);
}

int readLightVal(int lightInputPin){
    int lightVal = analogRead(lightInputPin);
    Serial.print("Current light val: ");
    Serial.println(lightVal);
    return lightVal;
}

void setTimeClock (RtcDateTime now){
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if(!Rtc.IsDateTimeValid()){
    Rtc.SetDateTime(compiled);
  }  
  if (Rtc.GetIsWriteProtected()) {
      Rtc.SetIsWriteProtected(false);
  }
  if(!Rtc.GetIsRunning()){
      Rtc.SetIsRunning(true);
  }

  if (now < compiled) {
    Rtc.SetDateTime(compiled);
  } 
}
  /* in setup function
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("."); 
  }
  Serial.println("ok");
  */

void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP8266Client - MyClient";
    // Attempt to connect
    // Insert your password
    if (client.connect(clientId.c_str(), "smartSwithchOEO", "your_password")) {
      Serial.println("connected");
      // Once connected, publish an announcement…
      client.publish("testTopic", "hello world");
      // … and resubscribe
      client.subscribe("testTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
