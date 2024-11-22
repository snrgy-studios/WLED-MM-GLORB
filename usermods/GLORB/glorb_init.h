#pragma once

#include "wled.h"
#include "glorb_fx.h"

void initGlorbEffects() {
  // effects added from index MODE_COUNT
  strip.addEffect(254, &mode_static_glorb, _data_FX_MODE_STATIC_GLORB);
  strip.addEffect(254, &mode_rainbow_glorb, _data_FX_MODE_RAINBOW_GLORB);
  strip.addEffect(254, &mode_colorwaves_glorb, _data_FX_MODE_COLORWAVES_GLORB);
  strip.addEffect(254, &mode_running_glorb, _data_FX_MODE_RUNNING_GLORB);
  strip.addEffect(254, &mode_2DFrizzles_glorb, _data_FX_MODE_2DFRIZZLES_GLORB);
  strip.addEffect(254, &mode_2DBlackHole_glorb, _data_FX_MODE_2DBLACKHOLE_GLORB);
  strip.addEffect(254, &mode_2DHiphotic_glorb, _data_FX_MODE_2DHIPHOTIC_GLORB);
  strip.addEffect(254, &mode_2DSwirl_glorb, _data_FX_MODE_2DSWIRL_GLORB);
  strip.addEffect(254, &mode_2Dtartan_glorb, _data_FX_MODE_2DTARTAN_GLORB);
}

void initGlorbLedmapFile()
{
  if (WLED_FS.exists("/ledmap.json")) return;

  File f = WLED_FS.open("/ledmap.json", "w");
  if (!f) {
    errorFlag = ERR_FS_GENERAL;
    DEBUG_PRINTLN(F("Failed to create ledmap.json"));
    return;
  }

  // Write header with lineabreaks
  size_t written = f.print("{\"n\":\"GLORB\"\n,\"width\":20\n,\"height\":6\n,\"map\":[");
  if (written == 0) {
    errorFlag = ERR_FS_GENERAL;
    DEBUG_PRINTLN(F("Failed to write ledmap header"));
    f.close();
    return;
  }

  #ifdef GLORB_LEDMAP_81 // ledmap 81
  constexpr int8_t defaultMap[] = {
     0,-1,-1,-1, 1,-1,-1,-1, 2,-1,-1,-1, 3,-1,-1,-1, 4,-1,-1,-1,
     8, 9,-1,10,11,12,-1,13,14,15,-1,16,17,18,-1,19, 5, 6,-1, 7,
    35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,39,38,37,36,
    44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,40,41,42,43,
    -1,65,66,67,-1,68,69,70,-1,71,72,73,-1,74,75,61,-1,62,63,64,
    -1,-1,78,-1,-1,-1,79,-1,-1,-1,80,-1,-1,-1,76,-1,-1,-1,77,-1
  };
  #else // ledmap 83
  constexpr int8_t defaultMap[] = {
     1,-1,-1,-1, 2,-1,-1,-1, 3,-1,-1,-1, 4,-1,-1,-1, 5,-1,-1,-1,
     9,10,-1,11,12,13,-1,14,15,16,-1,17,18,19,-1,20, 6, 7,-1, 8,
    37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,41,40,39,38,
    46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,42,43,44,45,
    -1,67,68,69,-1,70,71,72,-1,73,74,75,-1,76,77,63,-1,64,65,66,
    -1,-1,80,-1,-1,-1,81,-1,-1,-1,82,-1,-1,-1,78,-1,-1,-1,79,-1
  };
  #endif

  // Write array elements with commas
  constexpr size_t mapSize = sizeof(defaultMap) / sizeof(defaultMap[0]);
  for (size_t i = 0; i < mapSize; i++) {
    f.print(defaultMap[i]);
    if (i < mapSize - 1) f.print(",");
  }

  // Close the JSON
  f.print("]}");
  f.close();

  // Verify the file was written
  f = WLED_FS.open("/ledmap.json", "r");
  if (!f || f.size() == 0) {
    errorFlag = ERR_FS_GENERAL;
    DEBUG_PRINTLN(F("Ledmap verification failed"));
    if (f) f.close();
    return;
  }
  f.close();

  updateFSInfo();
  DEBUG_PRINTLN(F("Ledmap created successfully"));
}

void initGlorbPanel() {
  if (strip.isMatrix && strip.panel.empty()) {
    strip.panel.clear();
    WS2812FX::Panel p;
    strip.panels = 1;
    p.width = 20;
    p.height = 6;
    p.xOffset = p.yOffset = 0;
    p.options = 0;
    strip.panel.push_back(p);
  }
}

void initGlorb2D() {
    strip.isMatrix = true;
    initGlorbPanel();
    initGlorbLedmapFile();
    strip.setUpMatrix();
    strip.makeAutoSegments(true);
    strip.deserializeMap();
}