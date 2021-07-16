#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "SPI.h"

#include "TFT_eSPI.h"

#define TFT_GREY 0x7BEF

TFT_eSPI myGLCD = TFT_eSPI(); // Invoke custom library

unsigned long runTime = 0;
void setup()
{
  randomSeed(analogRead(A0));
  // Setup the LCD
  myGLCD.init();
  myGLCD.setRotation(1);
}

typedef struct
{
  int16_t x, y;
} point_t;

point_t cubic_bezier_point(float t, const point_t *points)
{
  point_t ret;
  float cX = 3 * (points[1].x - points[0].x);
  float bX = 3 * (points[2].x - points[1].x) - cX;
  float aX = points[3].x - points[0].x - cX - bX;
  float cY = 3 * (points[1].y - points[0].y);
  float bY = 3 * (points[2].y - points[1].y) - cY;
  float aY = points[3].y - points[0].y - cY - bY;
  ret.x = (aX * pow(t, 3)) + (bX * pow(t, 2)) + (cX * t) + points[0].x;
  ret.y = (aY * pow(t, 3)) + (bY * pow(t, 2)) + (cY * t) + points[0].y;
  return ret;
}

point_t linear_move(float t, const point_t p0, const point_t p1)
{
  point_t ret;
  float dX = t * (p1.x - p0.x);
  float dY = t * (p1.y - p0.y);
  ret.x = dX + p0.x;
  ret.y = dY + p0.y;
  return ret;
}

#define BZACCURACY 100

void cubic_bezier(uint32_t color, const point_t *points)
{
  point_t ps, pe;
  ps.x = points[0].x;
  ps.y = points[0].y;
  for (int i = 0; i < BZACCURACY; i++)
  {
    pe = cubic_bezier_point((float)i / BZACCURACY, points);
    myGLCD.drawLine(ps.x, ps.y, pe.x, pe.y, color);
    ps = pe;
  }
}

point_t curve1[][4] = {
    {{10, 60},
     {20, 50},
     {30, 40},
     {50, 10}},
    {{50, 10},
     {50, 40},
     {50, 80},
     {50, 160}},
    {{0, 160},
     {30, 160},
     {50, 160},
     {100, 160}},
};

point_t curve2[][4] = {
    {{0, 60},
     {0, 10},
     {100, 10},
     {100, 60}},
    {{100, 60},
     {100, 100},
     {5, 110},
     {0, 160}},
    {{0, 160},
     {10, 160},
     {55, 160},
     {100, 160}},
};

void loop()
{
  myGLCD.fillScreen(TFT_BLACK);
  float t = 0, tinc = 0.05;
  while (true)
  {
    point_t curve[3][4];
    myGLCD.fillRect(0, 10, 100, 150, 0);
    for (int i = 0; i < sizeof(curve1) / sizeof(curve1[0]); ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        curve[i][j] = linear_move(t, curve1[i][j], curve2[i][j]);
      }
    }
    for (int i = 0; i < sizeof(curve) / sizeof(curve[0]); ++i)
      cubic_bezier(TFT_CYAN, curve[i]);
    delay(100);
    t += tinc;
    if (t > 1)
    {
      tinc = -tinc;
      t = 1 + tinc;
      delay(1000);
    }
    else if (t < 0)
    {
      tinc = -tinc;
      t = 0;
      delay(1000);
    }
  }
}
