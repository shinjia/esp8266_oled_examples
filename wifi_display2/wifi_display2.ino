
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

/****** WiFi Access Point ******/
#define WLAN_SSID       "Your_Wifi_SSID"
#define WLAN_PASS       "Your_Wifi_Password"

// OLED 的尺寸
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 初始化 OLED
#define OLED_RESET    -1  // 不使用重置引腳
#define SCREEN_ADDRESS 0x3C  // 0x3C 是大多數 SSD1306 的 I2C 地址
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 模擬溫度和濕度數據
float temperature = 0;
float humidity = 0;

// 儀表板數據範圍
const int temperatureMax = 100; // 假設溫度最大值 100
const int humidityMax = 100; // 假設濕度最大值 100

// Timer control
const long timer_delay_wifi = 1000;
unsigned long timer_next_wifi;

void wifi_connect() {  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
}

void setup() {
  // 初始化序列埠
  Serial.begin(115200);

  // 連接 Wi-Fi
  wifi_connect();

  // 設定 I2C 腳位，SDA 設為 GPIO14，SCL 設為 GPIO12
  Wire.begin(14, 12);

  // 初始化 I2C 並啟動 OLED 顯示器
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("OLED 初始化失敗"));
    for (;;);
  }

  // 設定字型
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // 畫標題
  drawTitles();
  
  // 初次畫兩個半圓形儀表板框架（外圈和刻度）
  drawSemiCircleGaugeFrame(32, 48, 25);  // 左側溫度儀表
  drawSemiCircleGaugeFrame(96, 48, 25);  // 右側濕度儀表
  
  // timer control
  timer_next_wifi = millis();
}

void loop() {
  if(millis() > timer_next_wifi) {
    timer_next_wifi = millis() + timer_delay_wifi;

    // 模擬數據更新 (實際應該從感測器讀取)
    temperature = random(0, 100);  // 模擬溫度變化
    humidity = random(0, 100);     // 模擬濕度變化

    // 清除舊的指針和數值，保持標題和外圈不變
    display.clearDisplay();
    drawTitles();
    drawSemiCircleGaugeFrame(32, 48, 25);
    drawSemiCircleGaugeFrame(96, 48, 25);

    // 更新指針和數值
    drawSemiCircleGauge(32, 48, 25, temperature, temperatureMax);
    drawSemiCircleGauge(96, 48, 25, humidity, humidityMax);

    // 更新顯示器
    display.display();
  }
}

// 畫標題
void drawTitles() {
  display.clearDisplay();
  display.setCursor(20, 6);   // 左上角標題
  display.print("Temp");
  display.setCursor(90, 6);   // 右上角標題
  display.print("Humi");
}

// 畫半圓形儀表板的框架，顯示在 (x, y) 中心點，半徑 r，不包括指針
void drawSemiCircleGaugeFrame(int16_t x, int16_t y, int16_t r) {
  // 畫半圓形
  drawArc(x, y, r, 180, 360, SSD1306_WHITE);  // 顛倒顯示的儀表，從180度到360度

  // 畫刻度（從 180 到 360 度）
  for (int angle = 180; angle <= 360; angle += 30) {
    float radian = angle * PI / 180;
    int x0 = x + r * cos(radian);
    int y0 = y + r * sin(radian);
    int x1 = x + (r - 5) * cos(radian);
    int y1 = y + (r - 5) * sin(radian);
    display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  }
}

// 畫半圓形儀表板，顯示在 (x, y) 中心點，半徑 r，數值 value
void drawSemiCircleGauge(int16_t x, int16_t y, int16_t r, float value, float max_value) {
  // 計算指針角度（從 180 度到 360 度，顛倒）
  float angle = map(value, 0, max_value, 180, 360);
  float radian = angle * PI / 180;

  // 計算指針的終點
  int x1 = x + (r - 5) * cos(radian);
  int y1 = y + (r - 5) * sin(radian);

  // 畫指針
  display.drawLine(x, y, x1, y1, SSD1306_WHITE);

  // 顯示數值
  display.setCursor(x - 8, y + r + 10);
  display.setTextSize(1);
  display.print(value, 0); // 數值無小數點
}

// 自定義函數，用於繪製弧形（逐點畫弧形）
void drawArc(int16_t x, int16_t y, int16_t r, int startAngle, int endAngle, int color) {
  for (int angle = startAngle; angle <= endAngle; angle++) {
    float radian = angle * PI / 180;
    int x1 = x + r * cos(radian);
    int y1 = y + r * sin(radian);
    display.drawPixel(x1, y1, color);
  }
}
