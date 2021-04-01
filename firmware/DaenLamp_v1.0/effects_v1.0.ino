
// ****************************** ОГОНЁК ******************************
int16_t position;
boolean direction;

void lighter() {
  FastLED.clear();
  if (direction) {
    position++;
    if (position > NUM_LEDS - 2) {
      direction = false;
    }
  } else {
    position--;
    if (position < 1) {
      direction = true;
    }
  }
  leds[position] = CRGB::White;
}

// ****************************** СВЕТЛЯЧКИ ******************************
#define MAX_SPEED 30
#define BUGS_AMOUNT 5
int16_t speed[BUGS_AMOUNT];
int16_t pos[BUGS_AMOUNT];
CRGB bugColors[BUGS_AMOUNT];

void lightBugs() {
  if (loadingFlag) {
    loadingFlag = false;
    for (int i = 0; i < BUGS_AMOUNT; i++) {
      bugColors[i] = CHSV(random(0, 9) * 28, 255, 255);
      pos[i] = random(0, NUM_LEDS);
      speed[i] += random(-5, 6);
    }
  }
  FastLED.clear();
  for (int i = 0; i < BUGS_AMOUNT; i++) {
    speed[i] += random(-5, 6);
    if (speed[i] == 0) speed[i] += (-5, 6);

    if (abs(speed[i]) > MAX_SPEED) speed[i] = 0;
    pos[i] += speed[i] / 10;
    if (pos[i] < 0) {
      pos[i] = 29;

    }
    if (pos[i] > NUM_LEDS - 1) {
      pos[i] = 0;

    }
    leds[pos[i]] = bugColors[i];
  }
}

// ****************************** ЦВЕТА ******************************
byte hue;
void colors() {
  hue += 2;
  CRGB thisColor = CHSV(hue, 255, 255);
  fillAll(CHSV(hue, 255, 255));
}

// ****************************** РАДУГА ******************************
void rainbow() {
  hue += 2;
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV((byte)(hue + i * float(255 / NUM_LEDS)), 255, 255);
}

// ****************************** КОНФЕТТИ ******************************
void sparkles() {
  byte thisNum = random(0, NUM_LEDS);
  if (getPixColor(thisNum) == 0)
    leds[thisNum] = CHSV(random(0, 255), 255, 255);
  fade();
}

// ****************************** ОГОНЬ ******************************
#define COOLING  115 // 30-100
#define SPARKING 115 // 50-170

void fire() {
  random16_add_entropy( random());
  Fire2012WithPalette(); // run simulation frame, using palette colors
}

void Fire2012WithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heatleft[15];
  static byte heatright[15];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < 15; i++) {
    heatleft[i] = qsub8 ( heatleft[i],  random8(0, COOLING * 10 / 15));
    heatright[i] = qsub8 ( heatright[i],  random8(0, COOLING * 10 / 15));
  }
  if ( heatleft[0] < 120) heatleft[0] = 120;
  if ( heatright[0] < 120) heatright[0] = 120;
  if ( heatleft[1] < 150) heatleft[0] = 150;
  if ( heatright[1] < 150) heatright[0] = 150;

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = 14; k >= 2; k--) {
    heatleft[k] = (heatleft[k - 1] + heatleft[k - 2] + heatleft[k - 2] ) / 3;
    heatright[k] = (heatright[k - 1] + heatright[k - 2] + heatright[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(2);
    heatleft[y] = qadd8( heatleft[y], random8(160, 200) );
  }
  if ( random8() < SPARKING ) {
    int z = random8(2);
    heatright[z] = qadd8( heatright[z], random8(160, 200) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < 15; j++) {
    byte colorindexleft = scale8( heatleft[j], 230);
    CRGB colorleft = ColorFromPalette( gPal, colorindexleft);
    byte colorindexright = scale8( heatright[j], 230);
    CRGB colorright = ColorFromPalette( gPal, colorindexright);
    leds[j] = colorleft;
    leds[29 - j] = colorright;
  }
}

void clockled() {
  DateTime now = rtc.now();          // получение времени с часов
  byte now_sec = now.second();       // создание переменных с текущими значениями секунд, минут и часов
  byte now_min = now.minute();       
  byte now_hour = now.hour(); 
  static byte counter = 0;           // создание счетчика итераций за 1 секунду
  static byte prev_led_sec = 0;      
  int led_sec, sec_prev_led, sec_next_led, min_prev_led, min_next_led, hour_prev_led, hour_next_led;
  int sec_prev_color, sec_next_color, min_prev_color, min_next_color, hour_prev_color, hour_next_color;
  int sec_val, min_val, hour_val;

  if (now_sec < 30)                  // расчет номера первого секундного светодиода
    led_sec = (now_sec / 2) + 15;
  else
    led_sec = (now_sec / 2) - 15;
  if (now_min < 30)                  // расчет номера первого минутного светодиода
    min_prev_led = (now_min / 2) + 15;
  else
    min_prev_led = (now_min / 2) - 15;
  if (now_hour < 12) {               // расчет номера первого часового светодиода
    if ((now_hour * 60 + now_min) < 360)
      hour_prev_led = ((now_hour * 60 + now_min) / 24) + 15;
    else
      hour_prev_led = ((now_hour * 60 + now_min) / 24) - 15;
  }
  else {
    if (((now_hour - 12) * 60 + now_min) < 360)
      hour_prev_led = (((now_hour - 12) * 60 + now_min) / 24) + 15;
    else
      hour_prev_led = (((now_hour - 12) * 60 + now_min) / 24) - 15;
  }

  if (counter == 0)
    sec_prev_led = led_sec;          // расчет номера второго секундного светодиода
  if (sec_prev_led != 29)
    sec_next_led = sec_prev_led  + 1;
  else
    sec_next_led = 0;
  if (min_prev_led != 29)            // расчет номера второго минутного светодиода
    min_next_led = min_prev_led + 1;
  else
    min_next_led = 0;
  if (hour_prev_led != 29)           // расчет номера второго часового светодиода
    hour_next_led = hour_prev_led + 1;
  else
    hour_next_led = 0;

  sec_val = (2000 / timer);          // расчет яркости светодиодов
  sec_prev_color = map (counter, sec_val , 0 , MIN_BRIGHTNESS , brightness);
  sec_prev_color = constrain (sec_prev_color , MIN_BRIGHTNESS , brightness);
  sec_next_color = map (counter, 0 , sec_val , MIN_BRIGHTNESS , brightness);
  sec_next_color = constrain (sec_next_color , MIN_BRIGHTNESS , brightness);

  min_val = ((now_min * 60 + now_sec) % 120);
  min_prev_color = map (min_val, 119 , 0 , MIN_BRIGHTNESS , brightness);
  min_prev_color = constrain (min_prev_color , MIN_BRIGHTNESS , brightness);
  min_next_color = map (min_val, 0 , 119 , MIN_BRIGHTNESS , brightness);
  min_next_color = constrain (min_next_color , MIN_BRIGHTNESS , brightness);

  hour_val = ((now_hour * 60 + now_min) % 24);
  hour_prev_color = map (hour_val, 23 , 0 , MIN_BRIGHTNESS , brightness);
  hour_prev_color = constrain (hour_prev_color , MIN_BRIGHTNESS , brightness);
  hour_next_color = map (hour_val, 0 , 23 , MIN_BRIGHTNESS , brightness);
  hour_next_color = constrain (hour_next_color , MIN_BRIGHTNESS , brightness);

  for (byte i = 0; i < NUM_LEDS; i++) {
    byte c_R, c_G, c_B;
    if (i == sec_prev_led) c_R = sec_prev_color;
    else if (i == sec_next_led) c_R = sec_next_color;
    else c_R = 0;
    if (i == min_prev_led) c_G = min_prev_color;
    else if (i == min_next_led) c_G = min_next_color;
    else c_G = 0;
    if (i == hour_prev_led) c_B = hour_prev_color;
    else if (i == hour_next_led) c_B = hour_next_color;
    else c_B = 0;
    leds[i] = CRGB(c_R, c_G, c_B);
  }

  counter++; // увеличение счетчика итераций
  if (sec_prev_led < led_sec || sec_prev_led - led_sec > 2)
    counter = 0; // сброс счетчика итераций на следующей секунде
}

// ****************** СЛУЖЕБНЫЕ ФУНКЦИИ *******************
#define TRACK_STEP 50

// залить все
void fillAll(CRGB newcolor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = newcolor;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(int thisPixel) {
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}

//затухание
void fade() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if ((uint32_t)getPixColor(i) == 0) continue;
    leds[i].fadeToBlackBy(TRACK_STEP);
  }
}