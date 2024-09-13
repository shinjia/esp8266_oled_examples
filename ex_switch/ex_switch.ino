#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

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

// WiFi SSID 和 密碼
#define WLAN_SSID "Your_Wifi_SSID"
#define WLAN_PASS "Your_Wifi_Password"

// 按鍵引腳
#define BUTTON_PIN 5

// 模式控制變量
int displayMode = 0;
const int totalModes = 3;

// Timer control
const long timer_delay_dht = 2000;
unsigned long timer_next_dht;

// 溫濕度變量
float temperature = 0;
float humidity = 0;

// WiFi 訊號強度變量
int32_t rssi = 0;

void setup() {
  // 初始化序列埠
  Serial.begin(115200);

  // 初始化 DHT11
  dht.begin();

  // 初始化按鍵
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 初始化 WiFi 連接
  connectToWiFi();

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

  // 清除顯示器並顯示初始訊息
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("System Ready");
  display.display();

  // 設置定時器
  timer_next_dht = millis();
}

void loop() {
  // 檢查按鍵按下狀況並切換顯示模式
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(200);  // 去除按鍵抖動
    displayMode = (displayMode + 1) % totalModes;  // 切換模式
    display.clearDisplay();  // 清除 OLED 顯示器
  }

  // 根據當前模式顯示
  switch (displayMode) {
    case 0:
      showGraphicalDashboard();
      break;
    case 1:
      showTextTemperatureHumidity();
      break;
    case 2:
      showWiFiSignalStrength();
      break;
  }
}

// 顯示模式 1: 圖形儀表板顯示
void showGraphicalDashboard() {
  if (millis() > timer_next_dht) {
    timer_next_dht = millis() + timer_delay_dht;

    // 讀取 DHT11 的溫度與濕度
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // 檢查是否讀取失敗
    if (isnan(humidity) || isnan(temperature)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error reading DHT11");
      display.display();
      return;
    }

    // 清除顯示器
    display.clearDisplay();
    drawGraphicalDashboard(temperature, humidity);
    display.display();
  }
}

void drawGraphicalDashboard(float temp, float humi) {
  // 畫標題
  display.setCursor(10, 0);
  display.print("Temp:");
  display.print(temp, 0);
  display.setCursor(80, 0);
  display.print("Humi:");
  display.print(humi, 0);

  // 繪製半圓形儀表板
  drawSemiCircleGaugeFrame(32, 48, 25);  // 左側溫度儀表
  drawSemiCircleGaugeFrame(96, 48, 25);  // 右側濕度儀表

  // 更新儀表板上的指針
  drawSemiCircleGauge(32, 48, 25, temp, 50);  // 更新溫度指針
  drawSemiCircleGauge(96, 48, 25, humi, 100);  // 更新濕度指針
}

// 顯示模式 2: 文字顯示溫濕度
void showTextTemperatureHumidity() {
  if (millis() > timer_next_dht) {
    timer_next_dht = millis() + timer_delay_dht;

    // 讀取 DHT11 的溫度與濕度
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // 檢查是否讀取失敗
    if (isnan(humidity) || isnan(temperature)) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error reading DHT11");
      display.display();
      return;
    }

    // 清除顯示器
    display.clearDisplay();
    display.setCursor(30, 4);
    display.print("Hello DHT11 ");

    // 顯示溫度
    display.setCursor(0, 16);
    display.print("Temperature: ");
    display.print(temperature, 1);
    display.println(" C");

    // 顯示濕度
    display.setCursor(0, 32);
    display.print("Humidity: ");
    display.print(humidity, 1);
    display.println(" %");

    display.display();
  }
}

// 顯示模式 3: WiFi 訊號強度顯示
void showWiFiSignalStrength() {
  // 取得 Wi-Fi 訊號強度（RSSI）
  rssi = WiFi.RSSI();

  // 清除顯示器
  display.clearDisplay();

  // 顯示標題
  display.setCursor(0, 0);
  display.print("WiFi Signal Strength");

  // 顯示訊號強度數值
  display.setCursor(20, 24);
  display.setTextSize(2);
  display.print(rssi);
  display.println(" dBm");
  display.setTextSize(1);

  // 更新顯示器
  display.display();
}

// 畫半圓形儀表板
void drawSemiCircleGaugeFrame(int16_t x, int16_t y, int16_t r) {
  drawArc(x, y, r, 180, 360, SSD1306_WHITE);
  for (int angle = 180; angle <= 360; angle += 30) {
    float radian = angle * PI / 180;
    int x0 = x + r * cos(radian);
    int y0 = y + r * sin(radian);
    int x1 = x + (r - 5) * cos(radian);
    int y1 = y + (r - 5) * sin(radian);
    display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
  }
}

void drawSemiCircleGauge(int16_t x, int16_t y, int16_t r, float value, float max_value) {
  float angle = map(value, 0, max_value, 180, 360);
  float radian = angle * PI / 180;
  int x1 = x + (r - 5) * cos(radian);
  int y1 = y + (r - 5) * sin(radian);
  display.drawLine(x, y, x1, y1, SSD1306_WHITE);
}

// 自定義函數，用於繪製弧形
void drawArc(int16_t x, int16_t y, int16_t r, int startAngle, int endAngle, int color) {
  for (int angle = startAngle; angle <= endAngle; angle++) {
    float radian = angle * PI / 180;
    int x1 = x + r * cos(radian);
    int y1 = y + r * sin(radian);
    display.drawPixel(x1, y1, color);
  }
}

// 連接到 WiFi
void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
}