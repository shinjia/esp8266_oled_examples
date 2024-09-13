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

// Timer control
const long timer_delay_wifi = 100;
unsigned long timer_next_wifi;

// 儲存訊號強度的歷史數據
const int numBars = 128; // 每個像素點都作為一個條形圖單位
int rssiHistory[numBars]; // 儲存歷次訊號強度

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

  // timer control
  timer_next_wifi = millis();
}

void loop() {
  if(millis()>timer_next_wifi) {
    timer_next_wifi = millis() + timer_delay_wifi;

    // 取得 Wi-Fi 訊號強度（RSSI）
    int32_t rssi = WiFi.RSSI();

    // 更新歷史數據
    updateRssiHistory(rssi);

    // 清除顯示器畫面，並保留標題
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Wi-Fi Signal");

    // 顯示訊號強度數值
    display.setCursor(20, 16);
    display.setTextSize(2);
    display.print(rssi);
    display.println(" dBm");

    // 繪製訊號強度長條圖
    drawSignalStrengthBars();

    // 更新顯示器
    display.display();
  }
}

// 更新歷史訊號強度數據
void updateRssiHistory(int32_t rssi) {
  // 將歷史訊號數據向左移動
  for (int i = 1; i < numBars; i++) {
    rssiHistory[i - 1] = rssiHistory[i];
  }
  // 將最新的訊號強度保存到最後一個位置
  rssiHistory[numBars - 1] = rssi;
}
// 畫訊號強度長條圖

void drawSignalStrengthBars() {
  // 訊號強度的最大和最小範圍
  int32_t rssiMin = -70;  // 最弱訊號
  int32_t rssiMax = -20;  // 最強訊號
  int32_t lengthMin = 1;  // 長度最短
  int32_t lengthMax = 30;  // 長度最長

  // 每個像素畫一個條形圖
  for (int i = 0; i < numBars; i++) {
    // 計算條形高度（相對於訊號強度）
    int barHeight = map(rssiHistory[i], rssiMin, rssiMax, lengthMin, lengthMax);

    // 畫出每個條形圖，x 軸是時間，y 軸是訊號強度
    display.drawLine(i, SCREEN_HEIGHT-1, i, SCREEN_HEIGHT-1 - barHeight, SSD1306_WHITE);
  }
}