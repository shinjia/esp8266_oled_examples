
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <DHT.h>

// OLED 的尺寸
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 初始化 OLED
#define OLED_RESET    -1  // 不使用重置引腳
#define SCREEN_ADDRESS 0x3C  // 0x3C 是大多數 SSD1306 的 I2C 地址
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT11 設置
#define DHTPIN  4  // 將 DHT11 的訊號腳接到 ESP8266 的 D4 (GPIO 2)
#define DHTTYPE DHT11  // 使用 DHT11 感測器
DHT dht(DHTPIN, DHTTYPE);

// 儀表板數據範圍
const int temperatureMax = 50;  // 假設溫度最大值 50°C
const int humidityMax = 100;    // 濕度最大值 100%

// Timer control
const long timer_delay_dht = 2000;
unsigned long timer_next_dht;

float temperature = 0;
float humidity = 0;

void setup() {
  // 初始化序列埠
  Serial.begin(115200);

  // 初始化 DHT11
  dht.begin();

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
  
  // 設置定時器
  timer_next_dht = millis();
}

void loop() {
  if (millis() > timer_next_dht) {
    timer_next_dht = millis() + timer_delay_dht;

    // 讀取 DHT11 的溫度與濕度
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // 檢查是否讀取失敗，並顯示錯誤訊息
    if (isnan(h) || isnan(t)) {
      Serial.println(F("無法從 DHT 讀取數據"));
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error reading DHT11");
      display.display();
      return;
    }

    // 更新溫濕度值
    temperature = t;
    humidity = h;

    // 清除舊的指針和數值，保持標題和外圈不變
    display.clearDisplay();
    drawTitles();
    drawSemiCircleGaugeFrame(32, 48, 25);  // 左側溫度儀表
    drawSemiCircleGaugeFrame(96, 48, 25);  // 右側濕度儀表

    // 更新儀表板上的指針和數值
    drawSemiCircleGauge(32, 48, 25, temperature, temperatureMax);  // 更新溫度指針
    drawSemiCircleGauge(96, 48, 25, humidity, humidityMax);     // 更新濕度指針

    // 更新顯示器
    display.display();
  }
}

// 畫標題，並在右側顯示當前的溫度和濕度
void drawTitles() {
  // 顯示左上角的 "Temp" 標題
  display.setCursor(10, 6);
  display.print("Temp:");
  display.print(temperature, 0);  // 顯示實際的溫度數值
  
  // 顯示右上角的 "Humi" 標題
  display.setCursor(80, 6);
  display.print("Humi:");
  display.print(humidity, 0);  // 顯示實際的濕度數值
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