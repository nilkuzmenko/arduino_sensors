#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <Vector.h>
#include "heltec.h" // alias for #include "SSD1306Wire.h"
#include "images.h"

BME280I2C bme;

#define DHTPIN 12 //D6
#define MQ2PIN A0
#define EXTPIN 0 //D7

/*************************** Calibration and Tweaks************************************/
String  dev_loc = "OTK"; //device location
String  dev_num = "4"; //device number

float t_add = 0; //temperature calibration value as addendum
float t_mult = 1; //temperature calibration value as factor

float et_add = 0; //external temperature calibration value as addendum
float et_mult = 1.019; //external temperature calibration value as factor (1.019 as default)

float h_add = 0; //humidity calibration value as addendum
float h_mult = 1; //humidity calibration value as factor

int heating = 60; //MQ2 heating time (sec)
int refresh = 4000;  //display and sending refresh time
//int sensor_refresh = 3000;  //sensor refresh time

/************************* WiFi Access Point *********************************/

#define wifi_ssid "netis"
#define wifi_password "12341488"

/************************* MQTT Setup *********************************/

#define mqtt_server "192.168.100.2"
#define mqtt_user "bananagas"      // if exist
#define mqtt_password "123456Aa"

String display_loc_num_str = dev_loc + " " + dev_num;
String temperature_topic_str = dev_loc + "/" + dev_num + "/" + "t"; //Topic temperature
String humidity_topic_str = dev_loc + "/" + dev_num + "/" + "h";
String pressure_topic_str = dev_loc + "/" + dev_num + "/" + "p";
String etemperature_topic_str = dev_loc + "/" + dev_num + "/" + "et";
String etcon_topic_str = dev_loc + "/" + dev_num + "/" + "ec";
String gas_topic_str = dev_loc + "/" + dev_num + "/" + "g";
const char *temperature_topic = temperature_topic_str.c_str();
const char *humidity_topic = humidity_topic_str.c_str();
const char *pressure_topic = pressure_topic_str.c_str();
const char *etemperature_topic = etemperature_topic_str.c_str();
const char *etcon_topic = etcon_topic_str.c_str();
const char *gas_topic = gas_topic_str.c_str();
const char *display_loc_num = display_loc_num_str.c_str();

//Definition main vars
float t; //temperature
float h; //humidity
float p; //pressure
float et; //external temperature
float g; // gas
float g_per; //gas in percents
float temp, hum; //sensor reading vars
bool et_on; // external temperature connection cache vars

//Output to display template vars
char buf_temp[4];
char buf_humi[4];
char buf_gas[4];
char buf_etemp[4];
//Output to MQTT  template vars
char buf_mqtt_temp[4];
char buf_mqtt_humi[4];
char buf_mqtt_pres[4];
char buf_mqtt_etemp[4];
char buf_mqtt_gas[4];

//Arrays
std::vector <float> t_arr;
std::vector <int> h_arr;
std::vector <float> p_arr;
std::vector <float> et_arr;
std::vector <int> g_arr;

unsigned long timing; //timer
unsigned long wifi_timing; //timer

WiFiClient espClient;
PubSubClient client(espClient);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(EXTPIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  bme.begin();
  sensors.begin();
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
  pinMode(EXTPIN, INPUT);
  // setup_wifi();           //Connect to Wifi network at start
  client.setServer(mqtt_server, 1883);
  delay(1000);
}

void loop() {
  Heltec.display->clear();
  if (millis() - timing > (refresh))
  {
    timing = millis();
    client.loop();

    bme.read(p, temp, hum);
    t = (temp * et_mult) + et_add;
    h = (hum * et_mult) + et_add;
    g = analogRead(MQ2PIN);
    sensors.requestTemperatures();
    et = (sensors.getTempCByIndex(0) * et_mult) + et_add;

    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(93, 22, display_loc_num);
    Heltec.display->setFont(ArialMT_Plain_16);

    //WI-FI
    if (WiFi.status() != WL_CONNECTED)
    {
      //  Serial.println("Trying to connect");
      WiFi.begin(wifi_ssid, wifi_password);
      delay(5000);
    }
    else
    {
      Heltec.display->drawXbm(60, 25, wifi_width, wifi_height, wifi_bits); // Wi-Fi icon
    }

    //MQTT
    if (!client.connected())
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        client.connect(display_loc_num, mqtt_user, mqtt_password);
        delay(5000);
      }
    }
    else
    {
      Heltec.display->drawXbm(77, 25, server_width, server_height, server_bits); //MQTT icon
    }

    // OLED BLOCK

    // header pics
    Heltec.display->drawXbm(8, 0, tem_w, tem_h, tem);
    Heltec.display->drawXbm(48, 0, shup_w, shup_h, shup);
    Heltec.display->drawXbm(82, 0, vla_w, vla_h, vla);
    Heltec.display->drawXbm(111, 0, gaz_w, gaz_h, gaz);

    // splitters
    Heltec.display->drawVerticalLine(36, 0, 21);
    Heltec.display->drawVerticalLine(76, 0, 21);
    Heltec.display->drawVerticalLine(105, 0, 21);
    Heltec.display->drawHorizontalLine(0, 22, 128);

    //DATA BLOCK
    //temperature
    if (isnan(t))
    {
      Heltec.display->drawString(6, 5, "Err");
    }
    else
    {
      dtostrf(t, 4, 1, buf_temp);
      Heltec.display->drawString(0, 5, buf_temp);
      /************************* TEMPERATURE MEDIAN SEND *********************************/
      t_arr.push_back (t);
      if (t_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (t_arr[r] < t_arr[r + 1])
            {
              float temp = t_arr[r];
              t_arr[r] = t_arr[r + 1];
              t_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(t_arr[2], 0, 1, buf_mqtt_temp);  
        client.publish(temperature_topic, String(buf_mqtt_temp).c_str(), true);
        t_arr.clear();   // array content remove
      }
    }

    //external temperature
    if (et < -100 || et > 80)
    {
      Heltec.display->drawXbm(45, 8, dis_w, dis_h, dis);
      if (et_on = true);
      {
        client.publish(etcon_topic, String("offline").c_str(), true);
        et_on = false;
      }
    }
    else
    {
      dtostrf(et, 4, 1, buf_etemp);
      Heltec.display->drawString(40, 5, buf_etemp);
      if (et_on == false);
      {
        client.publish(etcon_topic, String("online").c_str(), true);
        et_on = true;
      }
      /************************* ETEMPERATURE MEDIAN SEND *********************************/
      et_arr.push_back (et);
      if (et_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (et_arr[r] < et_arr[r + 1])
            {
              float temp = et_arr[r];
              et_arr[r] = et_arr[r + 1];
              et_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(et_arr[2], 0, 1, buf_mqtt_etemp);
        client.publish(etemperature_topic, String(buf_mqtt_etemp).c_str(), true);
        et_arr.clear();
      }
    }

    //humidity
    if (isnan(h))
    {
      Heltec.display->drawString(78, 5, "Err");
    }
    else
    {
      dtostrf(h, 3, 0, buf_humi);
      Heltec.display->drawString(78, 5, buf_humi);
      /************************* HUMIDITY MEDIAN SEND *********************************/
      h_arr.push_back (h);
      if (h_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (h_arr[r] < h_arr[r + 1])
            {
              float temp = h_arr[r];
              h_arr[r] = h_arr[r + 1];
              h_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(h_arr[2], 0, 0, buf_mqtt_humi);
        client.publish(humidity_topic, String(buf_mqtt_humi).c_str(), true);
        h_arr.clear();
      }
    }

    //gas
    if (millis() < (heating * 1000))
    {
      int progress = (100 * millis()) / (heating * 1000);
      Heltec.display->drawProgressBar(108, 9, 19, 8, progress);
    }
    else
    {
      g_per = (g * 100) / 1023;
      dtostrf(g_per, 3, 0, buf_gas);
      Heltec.display->drawString(107, 5, buf_gas);
      /************************* GAS MEDIAN SEND *********************************/
      g_arr.push_back (g_per);
      if (g_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (g_arr[r] < g_arr[r + 1])
            {
              float temp = g_arr[r];
              g_arr[r] = g_arr[r + 1];
              g_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(g_arr[2], 0, 0, buf_mqtt_gas);
        client.publish(gas_topic, String(buf_mqtt_gas).c_str(), true);
        g_arr.clear();
      }
    }

    /************************* PRESSURE MEDIAN SEND *********************************/
    p_arr.push_back (p);
    if (p_arr.size () >= 5)
    {
      for (int i = 1; i < 5; ++i)
      {
        for (int r = 0; r < 5 - i; r++)
        {
          if (p_arr[r] < p_arr[r + 1])
          {
            float temp = p_arr[r];
            p_arr[r] = p_arr[r + 1];
            p_arr[r + 1] = temp;
          }
        }
      }
      dtostrf(p_arr[2], 0, 0, buf_mqtt_pres);
      client.publish(pressure_topic, String(buf_mqtt_pres).c_str(), true);
      p_arr.clear();
    }

    //display all above
    Heltec.display->display();

    //SERIAL PRINT BLOCK
    Serial.print(t);
    Serial.print("  ");

    Serial.print(h);
    Serial.print("  ");

    Serial.print(p);
    Serial.print("  ");

    Serial.print(et);
    Serial.print("  ");

    Serial.print(g);
    Serial.print("  ");

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print(WiFi.localIP());
    }
    else
    {
      Serial.print("Wi-fi is DOWN!");
    }
    Serial.println("");
  }
}
