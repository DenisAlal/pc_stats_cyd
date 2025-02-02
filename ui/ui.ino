#include "functions_time.h"
#include "images.h"
#include "keys.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <GyverNTP.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include <screens.h>
#include <ui.h>
#include <vars.h>

const char *ssid = ssidWifi;
const char *password = passwordWifi;
const String q = city;
const String apiKey = apiWeatherKey;
const String url =
    "http://dataservice.accuweather.com/forecasts/v1/hourly/1hour/" + q +
    "?apikey=" + apiKey + "&language=ru&details=true&metric=true";

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3860,
         touchScreenMinimumY = 240, touchScreenMaximumY = 3860;
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
SPIClass touchscreenSpi = SPIClass(VSPI);
uint8_t *draw_buf;
lv_indev_t *indev;
unsigned long lastRequestTime = 0;
unsigned long lastRequestTimeWeather = 0;

XPT2046_Touchscreen touchscreen(33, 36);
#define TFT_HOR_RES TFT_WIDTH
#define TFT_VER_RES TFT_HEIGHT
#define LED_R 4
#define LED_G 16
#define LED_B 17

struct HardwareInfo
{
  String cpuTemp;
  String cpuLoad;
  String cpuPower;

  String gpuTemp;
  String gpuLoad;
  String gpuPower;

  String ramUsed;
  String ramFree;
} hwInfo;

struct weather_structure
{
  String city;
  String descriprion;
  String temp;
  String perceivedTemp;
  String uvIndex;
  String humidity;
  String wind;

} weather;

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    // Some very basic auto calibration so it doesn't go out of range
    if (p.x < touchScreenMinimumX)
      touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX)
      touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY)
      touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY)
      touchScreenMaximumY = p.y;
    // Map this to the pixel position
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1,
                        TFT_HOR_RES); /* Touchscreen X calibration */
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1,
                        TFT_VER_RES); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
String getSubstring(const char *input, char delimiter)
{
  String strInput = String(input);
  int index = strInput.indexOf(delimiter);
  return (index != -1) ? strInput.substring(0, index) : strInput;
}

void hardwareMonitor()
{
  HTTPClient httpMonitor;
  httpMonitor.setConnectTimeout(1000);
  httpMonitor.begin("http://192.168.31.184:8085/data.json");
  unsigned long startTime = millis();
  int httpCode = httpMonitor.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = httpMonitor.getString();
      const size_t capacity = JSON_OBJECT_SIZE(10) + 4000;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);
      if (error)
      {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        httpMonitor.end();
        // ledBlink(LED_R, 300);
        return;
      }

      JsonArray children = doc["Children"];
      if (children.isNull())
      {
        Serial.println("Error: 'Children' is null.");
        httpMonitor.end();
        // ledBlink(LED_R, 300);
        return;
      }

      for (JsonObject system : children)
      {
        if (system["Text"] == "DENIS")
        {
          JsonArray components = system["Children"];
          if (components.isNull())
          {
            Serial.println("Error: 'DENIS' has no children.");
            // ledBlink(LED_R, 300);
            httpMonitor.end();
            return;
          }

          for (JsonObject component : components)
          {
            String componentName = component["Text"].as<String>();

            // CPU Данные
            if (componentName == "AMD Ryzen 5 5600X")
            {
              JsonArray cpuStats = component["Children"];
              if (cpuStats.isNull())
                continue;

              for (JsonObject stat : cpuStats)
              {
                String statType = stat["Text"].as<String>();

                if (statType == "Temperatures")
                {
                  JsonArray tempChildren = stat["Children"];
                  if (tempChildren.isNull())
                    continue;

                  for (JsonObject temp : tempChildren)
                  {
                    if (temp["Text"] == "CPU Package")
                    {
                      hwInfo.cpuTemp = temp["Value"].as<String>();
                    }
                  }
                }
                if (statType == "Load")
                {
                  JsonArray loadChildren = stat["Children"];
                  if (loadChildren.isNull())
                    continue;

                  for (JsonObject load : loadChildren)
                  {
                    if (load["Text"] == "CPU Total")
                    {
                      hwInfo.cpuLoad = load["Value"].as<String>();
                    }
                  }
                }
                if (statType == "Powers")
                {
                  JsonArray powerChildren = stat["Children"];
                  if (powerChildren.isNull())
                    continue;

                  for (JsonObject power : powerChildren)
                  {
                    if (power["Text"] == "CPU Package")
                    {
                      String cpuPowerStr = power["Value"].as<String>();
                      cpuPowerStr.replace(",", ".");
                      cpuPowerStr.replace(" %", "");
                      hwInfo.cpuPower = cpuPowerStr;
                    }
                  }
                }
              }
            }

            // GPU Данные
            if (componentName == "NVIDIA NVIDIA GeForce RTX 3060")
            {
              JsonArray gpuStats = component["Children"];
              if (gpuStats.isNull())
                continue;

              for (JsonObject stat : gpuStats)
              {
                String statType = stat["Text"].as<String>();

                if (statType == "Temperatures")
                {
                  JsonArray tempChildren = stat["Children"];
                  if (tempChildren.isNull())
                    continue;

                  for (JsonObject temp : tempChildren)
                  {
                    if (temp["Text"] == "GPU Core")
                    {
                      hwInfo.gpuTemp = temp["Value"].as<String>();
                    }
                  }
                }
                if (statType == "Load")
                {
                  JsonArray loadChildren = stat["Children"];
                  if (loadChildren.isNull())
                    continue;

                  for (JsonObject load : loadChildren)
                  {
                    if (load["Text"] == "GPU Core")
                    {
                      hwInfo.gpuLoad = load["Value"].as<String>();
                    }
                  }
                }
                if (statType == "Powers")
                {
                  JsonArray powerChildren = stat["Children"];
                  if (powerChildren.isNull())
                    continue;

                  for (JsonObject power : powerChildren)
                  {
                    if (power["Text"] == "GPU Power")
                    {
                      String gpuPowerStr = power["Value"].as<String>();
                      gpuPowerStr.replace(",", ".");
                      gpuPowerStr.replace(" %", "");
                      hwInfo.gpuPower = gpuPowerStr;
                    }
                  }
                }
              }
            }

            // Оперативная память
            if (componentName == "Generic Memory")
            {
              JsonArray memoryStats = component["Children"];
              if (memoryStats.isNull())
                continue;

              for (JsonObject stat : memoryStats)
              {
                String statType = stat["Text"].as<String>();

                if (statType == "Data")
                {
                  JsonArray memChildren = stat["Children"];
                  if (memChildren.isNull())
                    continue;

                  for (JsonObject mem : memChildren)
                  {
                    if (mem["Text"] == "Used Memory")
                    {
                      String ramUsedStr = mem["Value"].as<String>();
                      ramUsedStr.replace(",", ".");
                      ramUsedStr.replace(" %", "");
                      hwInfo.ramUsed = ramUsedStr;
                    }
                    if (mem["Text"] == "Available Memory")
                    {
                      String ramFreeStr = mem["Value"].as<String>();
                      ramFreeStr.replace(",", ".");
                      ramFreeStr.replace(" %", "");
                      hwInfo.ramFree = ramFreeStr;
                    }
                  }
                }
              }
            }

            // CPU DATA
            lv_label_set_text_fmt(objects.cpu_temp, "TEMP %s",
                                  hwInfo.cpuTemp.c_str());
            lv_label_set_text_fmt(objects.cpu_using, "USE %s",
                                  hwInfo.cpuLoad.c_str());

            // GPU DATA
            lv_label_set_text_fmt(objects.gpu_temp, "TEMP %s",
                                  hwInfo.gpuTemp.c_str());
            lv_label_set_text_fmt(objects.gpu_using, "USE %s",
                                  hwInfo.gpuLoad.c_str());

            // BARS
            int32_t ru = getPercent(hwInfo.ramUsed.toInt(), 32);
            int32_t rf = getPercent(hwInfo.ramFree.toInt(), 32);
            int32_t cp = getPercent(hwInfo.cpuPower.toInt(), 180);
            int32_t gp = getPercent(hwInfo.gpuPower.toInt(), 157);

            lv_bar_set_value(objects.ram_used_bar, ru, LV_ANIM_ON);
            lv_bar_set_value(objects.ram_free_bar, rf, LV_ANIM_ON);
            lv_bar_set_value(objects.cpu_power_bar, cp, LV_ANIM_ON);
            lv_bar_set_value(objects.gpu_power_bar, gp, LV_ANIM_ON);

            // TEXT INSIDE BARS
            lv_label_set_text_fmt(objects.ram_free_text, "%s",
                                  hwInfo.ramFree.c_str());
            lv_label_set_text_fmt(objects.ram_used_text, "%s",
                                  hwInfo.ramUsed.c_str());
            lv_label_set_text_fmt(objects.cpu_power_text, "%s",
                                  hwInfo.cpuPower.c_str());
            lv_label_set_text_fmt(objects.gpu_power_text, "%s",
                                  hwInfo.gpuPower.c_str());
          }
        }
      }
    }
    else
    {
      // ledBlink(LED_R, 300);
      Serial.printf("HTTP request failed, error: %s\n",
                    httpMonitor.errorToString(httpCode).c_str());
    }
  }
  else
  {
    // ledBlink(LED_R, 300);
    Serial.printf("HTTP request failed, error: %s\n",
                  httpMonitor.errorToString(httpCode).c_str());
  }

  httpMonitor.end();
}

void getWeatherData()
{
  HTTPClient httpWeather;
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(url);
    httpWeather.begin(url);
    int httpCode = httpWeather.GET();
    if (httpCode > 0)
    {
      // Check for the response
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = httpWeather.getString();
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
          Serial.println("Json parsing failed!");
          httpWeather.end();
          return;
        }

        float temperature = doc[0]["Temperature"]["Value"];
        int humidity = doc[0]["RelativeHumidity"];
        float windSpeed = doc[0]["Wind"]["Speed"]["Value"];
        const char *descriptionWeather = doc[0]["IconPhrase"];
        float realFeelTemperature = doc[0]["RealFeelTemperature"]["Value"];
        int uvIndex = doc[0]["UVIndex"];
        windSpeed = windSpeed * 1000 / 3600;
        // Формирование строк для вывода
        String outputHumidity = "Влажность: " + String(humidity) + "%";
        String outputTemperature = "Температура: " + String(temperature) + "°C";
        String outputWind = "Ветер: " + String(windSpeed) + " м/с";
        String outputIconPhrase = String(descriptionWeather);
        String outputRealFeel =
            "Ощущается как: " + String(realFeelTemperature) + "°C";
        String outputUVIndex = "УФ индекс: " + String(uvIndex);

        weather.descriprion = outputIconPhrase;
        weather.temp = outputTemperature;
        weather.perceivedTemp = outputRealFeel;
        weather.uvIndex = outputUVIndex;
        weather.humidity = outputHumidity;
        weather.wind = outputWind;

        lv_label_set_text_fmt(objects.city_weather, "%s", weather.city.c_str());
        lv_label_set_text_fmt(objects.temp_weather, "%s", weather.temp.c_str());
        lv_label_set_text_fmt(objects.feel_temp_weather, "%s",
                              weather.perceivedTemp.c_str());
        lv_label_set_text_fmt(objects.wind_weather, "%s", weather.wind.c_str());
        lv_label_set_text_fmt(objects.hum_weather, "%s",
                              weather.humidity.c_str());
        lv_label_set_text_fmt(objects.uv_weather, "%s",
                              weather.uvIndex.c_str());
        lv_label_set_text_fmt(objects.description_weather, "%s",
                              weather.descriprion.c_str());
      }
      else
      {
        Serial.println("Failed");
      }
    }
    else
    {
      // ledBlink(LED_R, 300);
      Serial.printf("GET request failed, error: %s\n",
                    httpWeather.errorToString(httpCode).c_str());
    }
    httpWeather.end();
  }
  else
  {
    Serial.println("Not connected to Wi-Fi");
  }
}
uint32_t lastTick = 0;
void setup()
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_R, LOW);
  pinMode(LED_G, LOW);
  pinMode(LED_B, LOW);
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() +
                        "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
  }
  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());
  touchscreenSpi.begin(25, 39, 32, 36);
  touchscreen.begin(touchscreenSpi);
  touchscreen.setRotation(2);
  lv_init();

  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t *disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  // Initialize the XPT2046 input device driver
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();

  // RTC Update
  NTP.begin(7);
  weather.city = "Новосибирск";
  getWeatherData();
}

void ledBlink(uint8_t id, unsigned long time)
{
  pinMode(id, HIGH);
  delay(time);
  pinMode(id, LOW);
}
int32_t getPercent(int32_t data, int32_t value)
{
  float percent = ((float)data / value) * 100;
  int32_t roundedPercent = round(percent);
  return roundedPercent;
}

void loop()
{
  unsigned long currentMillis = millis();
  lv_tick_inc(currentMillis - lastTick);
  lastTick = currentMillis;

  // TIME
  if (NTP.tick())
  {
    Datime dt = NTP;
    lv_label_set_text_fmt(objects.menu_time, "%02d:%02d", dt.hour, dt.minute);
    lv_label_set_text_fmt(objects.menu_date, "%02d.%02d.%04i", dt.day, dt.month,
                          dt.year);
    lv_label_set_text_fmt(objects.weather_time, "%02d:%02d", dt.hour,
                          dt.minute);
    lv_label_set_text_fmt(objects.weather_date, "%02d.%02d.%04i", dt.day,
                          dt.month, dt.year);
  }

  if (currentMillis - lastRequestTime >= 2000)
  {
    hardwareMonitor();
    lastRequestTime = currentMillis;
  }

  if (currentMillis - lastRequestTimeWeather >= 7800000)
  {
    getWeatherData();
    lastRequestTimeWeather = currentMillis;
  }

  ui_tick();
  delay(lv_timer_handler());
}