/*
  Скетч к проекту "DaenLamp"
  Версия 1.0 
  Группа Вконтакте: https://vk.com/daenshow
  Исходники на GitHub: https://github.com/DaenShow/DaenLamp
  Нравится проект? Поддержи автора! https://vk.me/moneysend/to/1fxiW
  Автор: DaenShow, 2021
*/

/*
 За основу взят скетч, написанный AlexGyver, который мной доработан
 Исходник был взят тут https://github.com/AlexGyver/gyverlight
*/

/*
   Управление левой кнопкой/сенсором
  - Удержание - яркость
  - 1х тап - вкл/выкл ленты
  - 2х тап - предыдущий режим
  - 3х тап - вкл/выкл белый свет
  - 4х тап - старт/стоп авто смены режимов

   Управление правой кнопкой/сенсором
  - Удержание - скорость эффектов
  - 1х тап - вкл/выкл ленты
  - 2х тап - следующий режим
  - 3х тап - вкл/выкл белый свет
  - 4х тап - старт/стоп авто смены режимов
*/

// ************************** НАСТРОЙКИ ***********************
#define AUTOPLAY_TIME 60    // время между сменой режимов в секундах
#define LED_PIN 8           // пин ленты
#define BTN_PIN_LEFT 2      // пин левой кнопки/сенсора
#define BTN_PIN_RIGHT 3     // пин правой кнопки/сенсора
#define MIN_BRIGHTNESS 5    // минимальная яркость при ручной настройке
#define MIN_TIMER 25        // максимальная скорость эффектов - рекомендованное значение 25 (50 кадров в секунду)
#define MAX_TIMER 67        // минамальная скорость эффектов - рекомендованное значение 67 (15 кадров в секунду)
#define BRIGHTNESS 200      // начальная яркость
#define FIRE_PALETTE 3      // разные типы огня (0 - 3)

// ************************** ДЛЯ РАЗРАБОТЧИКОВ ***********************
#define MODES_AMOUNT 7
#define NUM_LEDS 30         
#include "GyverButton.h"
GButton touch_left (BTN_PIN_LEFT, LOW_PULL, NORM_OPEN);
GButton touch_right (BTN_PIN_RIGHT, LOW_PULL, NORM_OPEN);
#include <FastLED.h>
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;
#include "GyverTimer.h"
int timer = 50;
GTimer_ms effectTimer(timer);
GTimer_ms autoplayTimer((long)AUTOPLAY_TIME * 1000);
GTimer_ms brightTimer(20);
#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;

int brightness = BRIGHTNESS;
int tempBrightness;
int thisMode;
bool gReverseDirection = false;
boolean loadingFlag = true;
boolean autoplay = true;
boolean powerDirection = true;
boolean powerActive = false;
boolean powerState = true;
boolean whiteMode = false;
boolean brightDirection = true;
boolean timerDirection = true;
boolean wasLeftStep = false;
boolean wasRightStep = false;

// ************************** УСТАНОВКА ***********************
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);
  FastLED.show();
  randomSeed(analogRead(0));

  touch_left.setTimeout(300);
  touch_left.setStepTimeout(50);
  touch_right.setTimeout(300);
  touch_right.setStepTimeout(50);

  if (FIRE_PALETTE == 0) gPal = HeatColors_p;
  else if (FIRE_PALETTE == 1) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  else if (FIRE_PALETTE == 2) gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  else if (FIRE_PALETTE == 3) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::DarkOrange) ;

  if (rtc.lostPower())
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

}
// ************************** ОСНОВНОЙ ЦИКЛ ПРОГРАММЫ ***********************
void loop() {
  touch_left.tick();        // опрос левой кнопки
  touch_right.tick();       // опрос правой кнопки

// ************************** НАСТРОЙКА ЯРКОСТИ ***********************
  if (touch_left.isStep()) {    // 
    if (!powerActive) {
      wasLeftStep = true;
      if (brightDirection) {
        brightness += 5;
      } else {
        brightness -= 5;
      }
      brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
    }
  }
  if (touch_left.isRelease()) {
    if (wasLeftStep) {
      wasLeftStep = false;
      brightDirection = !brightDirection;
    }
  }
// ************************** НАСТРОЙКА СКОРОСТИ ЭФФЕКТОВ ***********************
  if (touch_right.isStep()) {
    if (!powerActive) {
      wasRightStep = true;
      if (timerDirection) {
        timer += 1;
      } else {
        timer -= 1;
      }
      timer = constrain(timer, MIN_TIMER, MAX_TIMER);
    }
  }
  if (touch_right.isRelease()) {
    if (wasRightStep) {
      wasRightStep = false;
      timerDirection = !timerDirection;
      effectTimer.setInterval(timer);
    }
  }
// ********************** НАСТРОЙКИ КОЛИЧЕСТВА НАЖАТИЙ НА ЛЕВУЮ КНОПКУ/СЕНСОР ***********************
  if (touch_left.hasClicks()) {
    byte clicks_left = touch_left.getClicks();
    switch (clicks_left) {
      case 1:
        powerDirection = !powerDirection;
        powerActive = true;
        tempBrightness = brightness * !powerDirection;
        break;
      case 2: if (!whiteMode && !powerActive) {
          prevMode();
        }
        break;
      case 3: if (!powerActive) {
          whiteMode = !whiteMode;
          if (whiteMode) {
            effectTimer.stop();
            fillAll(CRGB::White);
            FastLED.setBrightness(150);
            FastLED.show();
          } else {
            effectTimer.start();
          }
        }
        break;
      case 4: if (!whiteMode && !powerActive) autoplay = !autoplay;
        break;
      default:
        break;
    }
  }
// ********************** НАСТРОЙКИ КОЛИЧЕСТВА НАЖАТИЙ НА ПРАВУЮ КНОПКУ/СЕНСОР ***********************
  if (touch_right.hasClicks()) {
    byte clicks_right = touch_right.getClicks();
    switch (clicks_right) {
      case 1:
        powerDirection = !powerDirection;
        powerActive = true;
        tempBrightness = brightness * !powerDirection;
        break;
      case 2: if (!whiteMode && !powerActive) {
          nextMode();
        }
        break;
      case 3: if (!powerActive) {
          whiteMode = !whiteMode;
          if (whiteMode) {
            effectTimer.stop();
            fillAll(CRGB::White);
            FastLED.show();
          } else {
            effectTimer.start();
          }
        }
        break;
      case 4: if (!whiteMode && !powerActive) autoplay = !autoplay;
        break;
      default:
        break;
    }
  }
// ********************** ЭФФЕКТЫ ***********************
  if (effectTimer.isReady() && powerState) {
    switch (thisMode) {
      case 0: clockled();
        break;
      case 1: fire();
        break;
      case 2: colors();
        break;
      case 3: rainbow();
        break;
      case 4: sparkles();
        break;
      case 5: lighter();
        break;
      case 6: lightBugs();
        break;
    }
    FastLED.show();
  }
// ********************** АВТОСМЕНА РЕЖИМА ***********************
  if (autoplayTimer.isReady() && autoplay) {    // таймер смены режима
    nextMode();
  }
  brightnessTick();
}
// ********************** ФУНКЦИИ ***********************
void nextMode() {
  thisMode++;
  if (thisMode >= MODES_AMOUNT) thisMode = 0;
  loadingFlag = true;
  FastLED.clear();
}
void prevMode() {
  thisMode--;
  if (thisMode < 0) thisMode = MODES_AMOUNT-1;
  loadingFlag = true;
  FastLED.clear();
}
void brightnessTick() {
  if (powerActive) {
    if (brightTimer.isReady()) {
      if (powerDirection) {
        powerState = true;
        tempBrightness += 10;
        if (tempBrightness > brightness) {
          tempBrightness = brightness;
          powerActive = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      } else {
        tempBrightness -= 10;
        if (tempBrightness < 0) {
          tempBrightness = 0;
          powerActive = false;
          powerState = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      }
    }
  }
}
