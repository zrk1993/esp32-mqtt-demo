#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "a.h"

const int btnPin = 16;
const int ledPin = 14;

//iic驱动方式
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   

void setup(void) {
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  u8g2.begin();
  u8g2.enableUTF8Print();       // 使print支持UTF8字集
}

int light = 0;
int step = 1;

void loop(void) {
  int btnStatus = digitalRead(btnPin);
  if (light > 200) {
    step = -1;
  }
  if (light < 0) {
    step = 1;
  }
  light += step;
  delay(10);
  // u8g2.firstPage();
  // do {
  //   u8g2.setFont(u8g2_font_ncenB14_tr); //设置字体
  //   u8g2.setCursor(0, 35);    //设置光标处
  //   u8g2.print(btnStatus);
  //   Serial.println(btnStatus);
  // } while ( u8g2.nextPage() );
  // delay(1000);

  Serial.println(light);
  analogWrite(ledPin, light);
}