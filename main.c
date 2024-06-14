#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "RTClib.h"

// RTC对象
RTC_DS3231 rtc;
DateTime now;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// 定义引脚
const int DS = D1;
const int SHCP = D2;
const int STCP = D3;
const int state = D10;

float divergence;
int pinState;
uint8_t numbers[8];
uint16_t hc595Buf[8];
uint8_t num = 0;
uint8_t clockFlag = 0;

// AP模式的SSID和密码
const char* ap_ssid = "divergencemeter";
const char* ap_password = "12345678";

// 创建Web服务器实例
WebServer server(80);

// HTML内容
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Divergence Meter</title>
<style>
    body {
        background-color: black;
        color: #00FF00; /* 霓虹绿色 */
        font-family: 'Courier New', Courier, monospace;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        margin: 0;
    }
    #container {
        text-align: center;
        border: 2px solid #00FF00; /* 霓虹绿色边框 */
        padding: 20px;
        border-radius: 10px;
        background: rgba(0, 0, 0, 0.8); /* 半透明黑色背景 */
        box-shadow: 0 0 20px #00FF00; /* 霓虹光晕 */
    }
    #title {
        font-size: 50px;
        color: #FF4500; /* 辉光管橙色 */
        text-shadow: 0 0 10px #FF4500, 0 0 20px #FF4500, 0 0 30px #FF4500;
        margin-bottom: 10px;
        text-transform: uppercase; /* 全部大写 */
        letter-spacing: 2px; /* 增加字间距 */
    }
    #description {
        font-size: 20px;
        color: #00FF00;
        margin-bottom: 30px;
        padding: 0 20px;
    }
    #hint {
        font-size: 40px;
        color: #FF00FF; /* 霓虹粉色 */
        text-shadow: 0 0 10px #FF00FF, 0 0 20px #FF00FF, 0 0 30px #FF00FF;
        margin-bottom: 20px;
        text-transform: uppercase; /* 全部大写 */
        letter-spacing: 2px; /* 增加字间距 */
    }
    .divider {
        height: 2px;
        background: #FF00FF; /* 霓虹粉色 */
        margin: 20px 0;
        box-shadow: 0 0 10px #FF00FF, 0 0 20px #FF00FF, 0 0 30px #FF00FF;
    }
    form {
        display: flex;
        flex-direction: column;
        align-items: center;
        font-size: 30px;
        line-height: 50px;
    }
    .form-group {
        display: flex;
        justify-content: space-between;
        width: 400px;
        margin-bottom: 20px;
    }
    label {
        flex: 1;
        text-align: right;
        margin-right: 10px;
        color: #FF00FF; /* 霓虹粉色 */
        text-shadow: 0 0 10px #FF00FF, 0 0 20px #FF00FF, 0 0 30px #FF00FF;
    }
    input[type=text] {
        flex: 2;
        padding: 10px;
        color: #FF4500; /* 辉光管橙色 */
        background-color: black;
        border: 2px solid #00FF00;
        border-radius: 5px;
        font-size: 20px;
        text-align: center;
        font-family: 'Courier New', Courier, monospace;
        text-shadow: 0 0 5px #FF4500, 0 0 10px #FF4500, 0 0 15px #FF4500; /* 辉光管效果 */
    }
    input[type=text]::placeholder {
        color: #555555;
        text-align: center;
    }
    input[type=submit] {
        color: black;
        background-color: #00FF00; /* 霓虹绿色 */
        border: none;
        padding: 10px 20px;
        font-size: 30px;
        cursor: pointer;
        margin-top: 20px;
        border-radius: 5px;
        transition: background-color 0.3s, transform 0.3s;
    }
    input[type=submit]:hover {
        background-color: #FF00FF; /* 霓虹粉色 */
        transform: scale(1.1); /* 放大 */
    }
</style>
</head>
<body>
    <div id="container">
        <h1 id="title">Divergence Meter</h1>
        <p id="description">
            Welcome to the Divergence Meter settings page. Here you can adjust the time settings for the divergence meter. 
            Please enter the desired day, hour, minute, and second values, and then click 'Confirm' to save your changes.
        </p>
        
        <div class="divider"></div>

        <h2 id="hint" title="设置">SETTINGS</h2>

        <form action="/submit" method="post">
            <div class="form-group">
                <label for="day">DAY</label>
                <input type="text" id="day" name="day" placeholder="DD">
            </div>
            <div class="form-group">
                <label for="hour">HOUR</label>
                <input type="text" id="hour" name="hour" placeholder="HH">
            </div>
            <div class="form-group">
                <label for="minute">MINUTE</label>
                <input type="text" id="minute" name="minute" placeholder="MM">
            </div>
            <div class="form-group">
                <label for="second">SECOND</label>
                <input type="text" id="second" name="second" placeholder="SS">
            </div>
            <input type="submit" value="CONFIRM">
        </form>
    </div>
</body>
</html>
)rawliteral";

// 处理主页请求
void handleRoot() {
    server.send(200, "text/html", htmlContent);
}

// 处理表单提交请求
void handleFormSubmit() {
    if (server.method() == HTTP_POST) {
        // 获取表单数据
        String dayStr = server.arg("day");
        String hourStr = server.arg("hour");
        String minuteStr = server.arg("minute");
        String secondStr = server.arg("second");

        // 将表单数据转换为8位无符号整型
        uint8_t day = dayStr.toInt();
        uint8_t hour = hourStr.toInt();
        uint8_t minute = minuteStr.toInt();
        uint8_t second = secondStr.toInt();

        // 打印表单数据
        Serial.println("Form Submitted:");
        Serial.print("Day: "); Serial.println(day);
        Serial.print("Hour: "); Serial.println(hour);
        Serial.print("Minute: "); Serial.println(minute);
        Serial.print("Second: "); Serial.println(second);

        // 获取当前时间
        now = rtc.now();
        // 如果日为0，则使用当前日
        if (day == 0) day = now.day();

        // 调整RTC时间
        rtc.adjust(DateTime(now.year(), now.month(), day, hour, minute, second));

        // 重定向到主页
        server.sendHeader("Location", String("/"), true);
        server.send(302, "text/plain", "");
    }
}

// 发送HC595缓冲区
void hc595WriteBuf() {
    // 前4位
    for (int i = 0; i < 4; i++) {
        digitalWrite(SHCP, LOW);
        usleep(10);
        digitalWrite(SHCP, HIGH);
        usleep(10);
        digitalWrite(SHCP, LOW);
    }
    for (int i = 0; i < 4; i++) {
        hc595WriteDigit(hc595Buf[i]);
    }
    // 后4位
    for (int i = 4; i < 8; i++) {
        digitalWrite(SHCP, LOW);
        usleep(10);
        digitalWrite(SHCP, HIGH);
        usleep(10);
        digitalWrite(SHCP, LOW);
    }
    for (int i = 4; i < 8; i++) {
        hc595WriteDigit(hc595Buf[i]);
    }
    digitalWrite(STCP, LOW);
    usleep(10);
    digitalWrite(STCP, HIGH);
    usleep(10);
    digitalWrite(STCP, LOW);
}

// 发送单个数字到HC595
void hc595WriteDigit(uint16_t byte) {
    for (int i = 0; i < 11; i++) {
        digitalWrite(DS, byte >> 15);
        byte <<= 1;
        digitalWrite(SHCP, LOW);
        usleep(10);
        digitalWrite(SHCP, HIGH);
        usleep(10);
        digitalWrite(SHCP, LOW);
    }
}

// 设置特定位置的数字
void hc595SetDigit(uint8_t digit, uint8_t number) {
    switch (number) {
        case 0:
            hc595Buf[digit] = 0b0000000000100000;
            break;
        case 9:
            hc595Buf[digit] = 0b0000000001000000;
            break;
        case 8:
            hc595Buf[digit] = 0b0000000010000000;
            break;
        case 7:
            hc595Buf[digit] = 0b0000000100000000;
            break;
        case 6:
            hc595Buf[digit] = 0b0000001000000000;
            break;
        case 5:
            hc595Buf[digit] = 0b0000010000000000;
            break;
        case 4:
            hc595Buf[digit] = 0b0000100000000000;
            break;
        case 3:
            hc595Buf[digit] = 0b0001000000000000;
            break;
        case 2:
            hc595Buf[digit] = 0b0010000000000000;
            break;
        case 1:
            hc595Buf[digit] = 0b0100000000000000;
            break;
        case 10:
            hc595Buf[digit] = 0b1000000000000000;
            break;
        case 11:
            hc595Buf[digit] = 0b0000000000000000;
            break;
    }
}

// 将浮点数拆分为整数
void splitFloatToInts(float number) {
    numbers[0] = (int)number % 10;
    float decimalPart = number - int(number);
    long long decimalNumber = (decimalPart * 10000000);

    for (int i = 6; i >= 0; i--) {
        numbers[i + 1] = decimalNumber % 10;
        decimalNumber /= 10;
    }
}

// 显示分歧率
void nixieDivergence(float number) {
    splitFloatToInts(number);
    hc595SetDigit(7, numbers[0]);
    hc595SetDigit(6, 10);
    for (int i = 0; i < 6; i++) {
        hc595SetDigit(5 - i, numbers[i + 1]);
    }
    hc595WriteBuf();
}

// 打印当前日期时间到串口
void displayDate() {
    now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    hc595SetDigit(7, now.day() / 10);
    hc595SetDigit(6, now.day() % 10);
    hc595SetDigit(5, now.hour() / 10);
    hc595SetDigit(4, now.hour() % 10);
    hc595SetDigit(3, now.minute() / 10);
    hc595SetDigit(2, now.minute() % 10);
    hc595SetDigit(1, now.second() / 10);
    hc595SetDigit(0, now.second() % 10);
    hc595WriteBuf();

    if ((now.minute() == 0 || now.minute() == 30) && now.second() == 0) {
        displayDivergence();
    }
}

// 测试数码管显示
void nixieTest() {
    for (int i = 0; i < 8; i++) hc595SetDigit(i, num);
    hc595WriteBuf();
    num++;
    if (num == 11) num = 0;
}

// 处理服务器请求的延迟
void delayServer(int ms) {
    for (int i = 0; i < ms; i++) server.handleClient();
}

// 显示分歧率动画
void displayDivergence() {
    for (int i = 0; i < 50; i++) {
        float randomFloat = 2.0 * (float(random(1000000)) / 1000000.0);
        nixieDivergence(randomFloat);
        delay(50);
    }
    float randomFloat = 2.0 * (float(random(1000000)) / 10000000.0) - 0.1;
    divergence += randomFloat;
    if (divergence < 0) divergence = 0;
    nixieDivergence(divergence);
    delay(5000);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(state, INPUT_PULLUP);
    pinMode(DS, OUTPUT);
    pinMode(STCP, OUTPUT);
    pinMode(SHCP, OUTPUT);

    digitalWrite(STCP, LOW);
    digitalWrite(SHCP, LOW);
    randomSeed(analogRead(0));
    float randomFloat = 2.0 * (float(random(1000000)) / 1000000.0);
    divergence = randomFloat;

    // 初始化RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
    }

    // 如果RTC电力丢失，则设置当前时间
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // 设置ESP32为AP模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // 设置mDNS
    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("MDNS responder started");

    // 处理服务器请求
    server.on("/", handleRoot);
    server.on("/submit", handleFormSubmit);  // 设置表单提交处理器

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // 处理按钮按下暂停
    if (pinState == 0 && digitalRead(state) == 1) {
        Serial.println("pause");
        for (int i = 0; i < 8; i++) hc595SetDigit(i, 11);
        hc595WriteBuf();
        pinState = 1;
        while (digitalRead(state))server.handleClient();
    }

    // 处理按钮按下开始
    if (pinState == 1 && digitalRead(state) == 0) {
        Serial.println("start");
        displayDivergence();
        pinState = 0;
        Serial.printf("DIVERGENCE : %f\n", divergence);
    }

    displayDate();
    Serial.println("Hello");
    delayServer(500);
}
