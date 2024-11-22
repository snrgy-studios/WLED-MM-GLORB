#pragma once

#include "wled.h"

// paletteBlend: 0 - wrap when moving, 1 - always wrap, 2 - never wrap, 3 - none (undefined)
#define PALETTE_SOLID_WRAP   (strip.paletteBlend == 1 || strip.paletteBlend == 3)
#define PALETTE_MOVING_WRAP !(strip.paletteBlend == 2 || (strip.paletteBlend == 0 && SEGMENT.speed == 0))

inline uint16_t tri16(uint16_t in) {
  if (in < 0x8000) return in *2;
  return 0xFFFF - (in - 0x8000)*2;
}

// ported wled effects with customization

/////////////////////////////
//          SOLID          //
/////////////////////////////
/*
 * No blinking. Just plain old static light.
 */
uint16_t mode_static_glorb(void) {
  SEGMENT.fill(SEGCOLOR(0));
  return strip.isOffRefreshRequired() ? FRAMETIME : 350;
}
static const char _data_FX_MODE_STATIC_GLORB[] PROGMEM = "Solid - GLORB@;!";

/////////////////////////////////
//          COLORLOOP          //
/////////////////////////////////
/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t mode_rainbow_glorb(void) {
  uint16_t duration = 10 + (SEGMENT.speed >> 1);
  uint16_t time = SEGENV.step + duration;
  uint8_t hue8 = tri16(time) >> 8;

  if (SEGMENT.intensity < 128){
    SEGMENT.fill(color_blend(SEGMENT.color_from_palette(hue8, false, false, 255), WHITE, 128-SEGMENT.intensity));
  } else {
    SEGMENT.fill(SEGMENT.color_from_palette(hue8, false, false, 255));
  }

  SEGENV.step = time;

  return FRAMETIME;
}
static const char _data_FX_MODE_RAINBOW_GLORB[] PROGMEM = "Colorloop - GLORB@Speed,Intensity;!;!";

//////////////////////////////////
//          COLORWAVES          //
//////////////////////////////////
// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
uint16_t mode_colorwaves_glorb() {
  // EDIT: add 2D
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up
  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  uint16_t duration = 10 + (SEGMENT.speed >> 4); // EDIT: reduce speed
  uint16_t sPseudotime = SEGENV.step;
  uint16_t sHue16 = SEGENV.aux0;

  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;
  uint16_t hueinc16 = beatsin88(113, 60, 300)*SEGMENT.intensity*10/510;  // EDIT: reduce intensity

  if (SEGMENT.check1) { // EDIT: add sr support
    um_data_t *um_data;
    if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
      // add support for no audio
      um_data = simulateSound(SEGMENT.soundSim);
    }
    float   volumeSmth  = *(float*)   um_data->u_data[0]; 
    duration += (uint16_t)volumeSmth / (32 - (SEGMENT.intensity>>3));
    hueinc16 += (uint16_t)volumeSmth / (32 - (SEGMENT.intensity>>3));
  }
  
  sPseudotime += duration * msmultiplier;
  sHue16 += duration * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (int y = 0; y < rows; y++) {
    // EDIT: hueinc only vertically
    hue16 += hueinc16;
    uint8_t hue8 = tri16(hue16) >> 8; // EDIT: use triwave

    for (int x = 0; x < cols; x++) {
      brightnesstheta16 += brightnessthetainc16;
      uint16_t b16 = sin16(brightnesstheta16) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);

      SEGMENT.blendPixelColorXY(x, y, SEGMENT.color_from_palette(hue8, false, PALETTE_SOLID_WRAP, 0, bri8), 128);
    }
  }
  SEGENV.step = sPseudotime;
  SEGENV.aux0 = sHue16;

  return FRAMETIME;
}
static const char _data_FX_MODE_COLORWAVES_GLORB[] PROGMEM = "Colorwaves - GLORB@Speed,Hue variation,,,,Sound Reactive;!;!;2v";

///////////////////////////////
//          RUNNING          //
///////////////////////////////
/*
 * Running lights effect with smooth sine transition.
 */
uint16_t mode_running_glorb() {
  uint16_t hueinc = 10 + (SEGMENT.speed >> 1); // (10, 137)
  uint16_t timeinc = 1 + (SEGMENT.speed >> 6); // (1, 4)
  uint8_t brithetainc16 = 12 + (SEGMENT.intensity >> 2); // (12, 75)

  uint16_t time = SEGENV.step;
  uint16_t hue16 = SEGENV.aux0;

  time += timeinc;
  hue16 += hueinc;
  
  uint16_t britheta16 = time;
  uint8_t hue8 = tri16(hue16) >> 8;

  for (int i = 0; i < SEGLEN; i++) {
    britheta16 += brithetainc16;
    uint8_t bri8 = sin8(britheta16);

    SEGMENT.setPixelColor(i, color_blend(BLACK, SEGMENT.color_from_palette(hue8, false, PALETTE_SOLID_WRAP, 0), bri8));
  }

  SEGENV.step = time;
  SEGENV.aux0 = hue16;

  return FRAMETIME;
}
static const char _data_FX_MODE_RUNNING_GLORB[] PROGMEM = "Running - GLORB@Speed,Wave width;!;!";

//////////////////////////////
//     2D Frizzles          //
//////////////////////////////
uint16_t mode_2DFrizzles_glorb(void) {                 // By: Stepko https://editor.soulmatelights.com/gallery/640-color-frizzles , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();
  uint8_t x, y;

  SEGMENT.fadeToBlackBy(16);
  for (size_t i = 1 + (SEGMENT.custom2 >> 5); i > 0; i--) { // EDIT: use intensity for number of lines
    x = beatsin8(SEGMENT.speed/16 + i, cols/2, 2*cols + cols/2 - 1); // EDIT: double range
    y = beatsin8(8 + SEGMENT.intensity/32 - i, 1, rows - 2); // EDIT: remove first and last rows
    SEGMENT.addPixelColorXY(x%cols, y, ColorFromPalette(SEGPALETTE, beatsin8(12, 0, 255), 255, LINEARBLEND));
  }
  SEGMENT.blur(8 + (SEGMENT.custom1 >> 4));

  return FRAMETIME;
} // mode_2DFrizzles()
static const char _data_FX_MODE_2DFRIZZLES_GLORB[] PROGMEM = "Frizzles - GLORB@X scale,Y scale,Blur,Intensity;!;!;2";

///////////////////////////////////
//          2D Hiphotic          //
///////////////////////////////////
uint16_t mode_2DHiphotic_glorb() {                        //  By: ldirko  https://editor.soulmatelights.com/gallery/810 , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  uint16_t a = SEGENV.step;
  a += 1 + (SEGMENT.speed >> 6);
  // const uint32_t a = strip.now / (20 - (SEGMENT.speed>>4)); // EDIT: reverse slider

  uint8_t x_scale = 8 + (SEGMENT.custom1 >> 5);
  uint8_t y_scale = 8 + (SEGMENT.custom2 >> 5);

  uint8_t hue_variation = 4 - (SEGMENT.intensity >> 6);
  uint8_t hue_offset = beatsin8(5, 0, 255 - (255/hue_variation));
  
  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      uint8_t hue = sin8(cos8(x * x_scale + a / 3) + sin8(y * y_scale + a / 4) + a) / hue_variation + hue_offset;
      SEGMENT.setPixelColorXY(x, y, color_blend(SEGMENT.getPixelColorXY(x, y), SEGMENT.color_from_palette(hue, false, PALETTE_SOLID_WRAP, 0), 64)); // EDIT: blend to reduce flickering
    }
  }

  SEGENV.step = a;

  return FRAMETIME;
} // mode_2DHiphotic()
static const char _data_FX_MODE_2DHIPHOTIC_GLORB[] PROGMEM = "Hiphotic - GLORB@Speed,Hue variation,X scale,Y scale;!;!;2";

//////////////////////////////////
//          BLACK HOLE          //
//////////////////////////////////
uint16_t mode_2DBlackHole_glorb(void) {            // By: Stepko https://editor.soulmatelights.com/gallery/1012 , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();
  uint16_t x, y;

  SEGMENT.fadeToBlackBy(SEGMENT.custom2>>3); // create fading trails
  unsigned long t = millis()/510; // timebase, 256 instead of 128

  for (size_t i = 0; i < 2 + (SEGMENT.custom1 >> 6); i++) {
    x = beatsin8(SEGMENT.speed>>4, cols/2, 2*cols + cols/2 - 1, 0, ((i % 2) ? 128 : 0) + t * i);
    y = beatsin8(SEGMENT.intensity>>3, 1, rows - 2, 0, ((i % 2) ? 192 : 64) + t * i);
    SEGMENT.addPixelColorXY(x % cols, y, SEGMENT.color_from_palette(i*63, false, PALETTE_SOLID_WRAP, 0));
  }

  // blur everything a bit
  SEGMENT.blur(32);

  return FRAMETIME;
} // mode_2DBlackHole()
static const char _data_FX_MODE_2DBLACKHOLE_GLORB[] PROGMEM = "Black Hole - GLORB@X scale,Y scale,Intensity,Fade rate;!;!;2"; // EDIT: remove default palette

//////////////////////////////////////////////////////////////
//                    BEGIN FFT ROUTINES                    //
//////////////////////////////////////////////////////////////

/////////////////////////
//    * 2D Swirl       //
/////////////////////////
// By: Mark Kriegsman https://gist.github.com/kriegsman/5adca44e14ad025e6d3b , modified by Andrew Tuline
uint16_t mode_2DSwirl_glorb(void) {
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.fill(BLACK);
  }

  const uint8_t borderWidth = 2;

  SEGMENT.blur(32 + (SEGMENT.custom1 >> 1));

  uint8_t  i = beatsin8( 27*SEGMENT.speed/255, borderWidth, 2*(cols - borderWidth));
  uint8_t  j = beatsin8( 41*SEGMENT.speed/255, borderWidth, rows - borderWidth);
  uint8_t ni = 2*(cols - 1) - i;
  uint8_t nj = 2*(cols - 1) - j;
  uint16_t ms = millis();

  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  float   volumeSmth  = *(float*)   um_data->u_data[0]; //ewowi: use instead of sampleAvg???
  int16_t volumeRaw   = *(int16_t*) um_data->u_data[1];

  SEGMENT.addPixelColorXY( i%cols, j, ColorFromPalette(SEGPALETTE, (ms / 11 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 11, 200, 255);
  SEGMENT.addPixelColorXY( j%cols, i, ColorFromPalette(SEGPALETTE, (ms / 13 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 13, 200, 255);
  SEGMENT.addPixelColorXY(ni%cols,nj, ColorFromPalette(SEGPALETTE, (ms / 17 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 17, 200, 255);
  SEGMENT.addPixelColorXY(nj%cols,ni, ColorFromPalette(SEGPALETTE, (ms / 29 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 29, 200, 255);
  SEGMENT.addPixelColorXY( i%cols,nj, ColorFromPalette(SEGPALETTE, (ms / 37 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 37, 200, 255);
  SEGMENT.addPixelColorXY(ni%cols, j, ColorFromPalette(SEGPALETTE, (ms / 41 + volumeSmth*4), volumeRaw * SEGMENT.intensity / 64, LINEARBLEND)); //CHSV( ms / 41, 200, 255);

  return FRAMETIME;
} // mode_2DSwirl()
static const char _data_FX_MODE_2DSWIRL_GLORB[] PROGMEM = "Swirl - GLORB@Speed,Sensitivity,Blur;!;!;2v";

/////////////////////////
//     2D Tartan       //
/////////////////////////
uint16_t mode_2Dtartan_glorb(void) {          // By: Elliott Kember  https://editor.soulmatelights.com/gallery/3-tartan , Modified by: Andrew Tuline
  if (!strip.isMatrix) return mode_static_glorb(); // not a 2D set-up

  const uint16_t cols = SEGMENT.virtualWidth();
  const uint16_t rows = SEGMENT.virtualHeight();

  if (SEGENV.call == 0) {
    SEGMENT.fill(BLACK);
  }

  uint8_t hue, bri;
  size_t intensity;
  int offsetX = beatsin16(3, -360, 360);
  int offsetY = beatsin16(2, -360, 360);
  int sharpness = SEGMENT.custom1 / 64; // 0-3

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      // hue = x * beatsin16(10, 1, 10) + offsetY;
      hue = y * beatsin16(10, 1, 10) + offsetY; // EDIT: hue variation only in y
      intensity = bri = sin8(x * SEGMENT.speed/2 + offsetX);
      for (int i=0; i<sharpness; i++) intensity *= bri;
      intensity >>= 8*sharpness;
      SEGMENT.setPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, hue, intensity, LINEARBLEND));
      // hue = y * 3 + offsetX;
      intensity = bri = sin8(y * SEGMENT.intensity/2 + offsetY);
      for (int i=0; i<sharpness; i++) intensity *= bri;
      intensity >>= 8*sharpness;
      SEGMENT.addPixelColorXY(x, y, ColorFromPalette(SEGPALETTE, hue, intensity, LINEARBLEND));
    }
  }

  return FRAMETIME;
} // mode_2DTartan()
static const char _data_FX_MODE_2DTARTAN_GLORB[] PROGMEM = "Tartan - GLORB@X scale,Y scale,Sharpness;!;!;2";
