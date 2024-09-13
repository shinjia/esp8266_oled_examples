#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// OLED 顯示器參數
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 按鍵和 LED 引腳
#define BUTTON_PIN 5       // 按鍵引腳：GPIO5
#define LED_PIN BUILTIN_LED // 內建 LED 引腳：GPIO2

// 計時器相關變量
unsigned long startTime = 0;      // 記錄開始時間
unsigned long elapsedTime = 0;    // 經過的時間
bool running = false;             // 是否在運行

// 去抖相關變量
bool lastButtonState = HIGH;      // 儲存上一次按鍵狀態
unsigned long lastDebounceTime = 0; // 上一次去抖的時間
unsigned long debounceDelay = 200;   // 去抖延遲（200 毫秒）

void setup() {
  // 初始化序列埠
  Serial.begin(115200);

  // 初始化 I2C 腳位，SDA 設為 GPIO14，SCL 設為 GPIO12
  Wire.begin(14, 12);

  // 初始化按鍵和內建 LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED 預設為熄滅（反相，LOW 開啟，HIGH 關閉）

  // 初始化 OLED 顯示器
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED 初始化失敗"));
    while (true);
  }

  // 清除顯示並顯示初始訊息
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("00:00:00");
  display.display();
}

void loop() {
  // 檢查按鍵狀態
  checkButton();

  // 如果秒表正在運行，更新經過的時間
  if (running) {
    elapsedTime = millis() - startTime;
  }

  // 顯示更新的時間
  displayTime(elapsedTime);
}

// 檢查按鍵的狀態，實現開始/停止功能
void checkButton() {
  // 讀取按鍵狀態
  bool buttonState = digitalRead(BUTTON_PIN);

  // 如果按鍵狀態發生改變，記錄下時間
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  // 當經過去抖延遲後，確認按鍵狀態改變
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == LOW) {
      // 按鍵從高變低時觸發
      if (!running) {
        // 開始秒表
        startTime = millis() - elapsedTime;  // 調整開始時間，保持已經計時的時間
        running = true;
        digitalWrite(LED_PIN, LOW);  // 開啟內建 LED
        Serial.println("秒表開始");
      } else {
        // 暫停秒表
        running = false;
        digitalWrite(LED_PIN, HIGH);  // 關閉內建 LED
        Serial.println("秒表暫停");
      }
    }
  }

  // 記錄按鍵的最後狀態
  lastButtonState = buttonState;
}

// 顯示秒表時間
void displayTime(unsigned long time) {
  int minutes = (time / 60000) % 60;
  int seconds = (time / 1000) % 60;
  int milliseconds = (time % 1000) / 10;

  // 清除顯示器並顯示當前的時間
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.printf("%02d:%02d:%02d", minutes, seconds, milliseconds);
  display.display();
}
