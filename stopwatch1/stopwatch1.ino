
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

// Timer control
const long timer_delay_dht = 2000;
unsigned long timer_next_dht;


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

  // 清除顯示器並顯示初始訊息
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("DHT11 Sensor Test");
  display.display();

  // timer control
  timer_next_dht = millis();
}

void loop() {
  float h, t;
  int temp, humi;

  if(millis()>timer_next_dht) {
    timer_next_dht = millis() + timer_delay_dht;

    // 讀取 DHT11 的溫度與濕度
    h = dht.readHumidity();
    t = dht.readTemperature();
    humi = (int)h;
    temp = (int)t;

    // 檢查是否讀取失敗，並顯示錯誤訊息
    if (isnan(h) || isnan(t)) {
      Serial.println(F("無法從 DHT 讀取數據"));
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error reading DHT11");
      display.display();
      return;
    }

    // 在 OLED 上顯示數據
    display.clearDisplay();

    // 顯示標題
    display.setTextSize(1);
    display.setCursor(30, 4);
    display.print("Hello DHT11 ");

    // // 顯示溫度
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("Temperature ");
    display.setTextSize(2);
    display.setCursor(66, 24);
    display.print(temp);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");

    // // 顯示濕度 
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.print("Humidity ");
    display.setTextSize(2);
    display.setCursor(66, 48);
    display.print(humi);
    display.print(" %");

    // 更新顯示器
    display.display();
  }
}