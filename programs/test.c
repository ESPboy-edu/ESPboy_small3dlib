/**
  Some basic tests for small3dlib.

  author: Miloslav Ciz
  license: CC0 1.0
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#define S3L_PIXEL_FUNCTION pixelFunc
#define S3L_RESOLUTION_X 64
#define S3L_RESOLUTION_Y 40

#define S3L_SORT 1

#include "../small3dlib.h"

#define TEST_BUFFER_W 16
#define TEST_BUFFER_H 16

const char expectedRender[S3L_RESOLUTION_X * S3L_RESOLUTION_Y + 1] =
  "...................54433221100.................................."
  "...................55443221100.................................."
  "...................6554432211..................................."
  "...................65544332....................................."
  "...................665544......................................."
  "..........AAA......7655........................................."
  "..........BAAACE...76..........................................."
  "..........CABAABDEG..........................................ddd"
  ".........DBABAAABCDFGH...............................eeeeeeeeddd"
  ".........DBABBAAABCDEGH.....................fffffffeeeeeeeeeeddd"
  ".........EDBACBAAABCDEFG............gggggffffffffffeeeeeeeeeeddd"
  "........FECBACCBBAAABDEG....hhhggggggggggffffffffffeeeeeeeeeeddd"
  "........GFDCBADCBBAihhhhhhhhhhggggggggggffffffffffeeeeeeeeeeeddd"
  "........HFECBADCCBBahhhhhhhhhhggggggggggffffffffffeeeeeeeeeeeddd"
  ".......IGFECBADDCCBaahhhhhhhhhggggggggggffffffffffeeeeeeeeeeeddd"
  "........HGEDCBAEDCCaaahhhhhhhhggggggggggfffffffffffeeeeeeeeeeddd"
  "........AGFDCBAEDDCaaaahhhhhhhgggggggggggffffffffffeeeeeeeeeeddd"
  "........BAFECBAEEDDaaaaahhhhhhhggggggggggffffffffffeeeeeeeeeeddd"
  "........BAAFDCBAFEEaaaaaahhhhhhggggggggggffffffffffeeeeeeeeeeddd"
  ".........BAAECBAFFEbaaaaahhhhhggggggggggffffffffffeeeeeeeeeeeddd"
  ".........CBAADCBAGFbbaaaaahhhhggggggggggffffffffffeeeeeeeeeeeddd"
  ".........CCBAACBAGFbbbaaaaahhhggggggggggfffffffffffeeeeeeeeeeddd"
  ".........DCBBAACAGGbbbbaaaaahhgggggggggggffffffffffeeeeeeeeeeddd"
  "..........DCCBAABAHbbbbbaaaaahhggggggggggffffffffffeeeeeeeeeeddd"
  "..........DDCBBAAAHbbbbbaaaaaahggggggggggffffffffffeeeeeeeeeeddd"
  "..........EDDCBBAAIcbbbbbaaaaaaggggggggggffffffffffeeeeeeeeeeddd"
  "..........EEDDCCBAHccbbbbbaaaaagggggggggffffffffffeeeeeeeeeeeddd"
  "...........FEDDCBGFcccbbbbbaaaaaggggggggfffffffffffeeeeeeeeeeddd"
  "...........FEEDCFEDccccbbbbbaaaaaggggggggffffffffffeeeeeeeeeeddd"
  "...........GFEEDEDCcccccbbbbbaaaaagggggggffffffffffeeeeeeeeeeddd"
  "...........GFFEDCBAcccccbbbbbaaaaaaggggggffffffffffeeeeeeeeeeddd"
  "............GFCBAACdcccccbbbbbaaaaaagggggffffffffffeeeeeeeeeeddd"
  "............HGBABAAddcccccbbbbbaaaaaggggffffffffffeeeeeeeeeeeddd"
  "............HAAA...dddcccccbbbbbaaaaagggfffffffffffeeeeeeeeeeddd"
  "...................ddddcccccbbbbbaaaaagggffffffffffeeeeeeeeeeddd"
  "...................dddddcccccbbbbbaaaaaggffffffffffeeeeeeeeeeddd"
  "...................dddddcccccbbbbbaaaaaagffffffffffeeeeeeeeeeddd"
  "...................edddddcccccbbbbbaaaaaaffffffffffeeeeeeeeeeddd"
  "...................eedddddcccccbbbbbaaaaaafffffffffeeeeeeeeeeddd"
  "...................eeedddddcccccbbbbbaaaaafffffffffeeeeeeeeeeddd";

uint8_t testRaster[TEST_BUFFER_W * TEST_BUFFER_H];
uint8_t testScreen[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];
uint8_t renderingMode = 0;

void pixelFunc(S3L_PixelInfo *p)
{
  if (renderingMode == 0)
    testRaster[p->y * TEST_BUFFER_W + p->x] += 1;
    else
  {
    char c = 'x';
 
    switch (p->modelIndex)
    {
      case 0: c = 'a'; break;
      case 1: c = 'A'; break;
      case 2: c = '0'; break;
      default: break;
    }

    c += ((p->barycentric[0] * 8) / S3L_FRACTIONS_PER_UNIT);

    testScreen[p->y * S3L_RESOLUTION_X + p->x] = c;
  }
}

int testTriangleRasterization(
  S3L_ScreenCoord x0,
  S3L_ScreenCoord y0,
  S3L_ScreenCoord x1,
  S3L_ScreenCoord y1,
  S3L_ScreenCoord x2,
  S3L_ScreenCoord y2,
  uint8_t *expectedPixels
  )
{
  printf("  --- testing tringle rasterization [%d,%d] [%d,%d] [%d,%d] (|: expected, -: rasterized) ----\n",x0,y0,x1,y1,x2,y2);

  memset(testRaster,0,TEST_BUFFER_W * TEST_BUFFER_H);

  S3L_Vec4 p0, p1, p2;

  S3L_vec4Set(&p0,x0,y0,1000,0);
  S3L_vec4Set(&p1,x1,y1,1000,0);
  S3L_vec4Set(&p2,x2,y2,1000,0);

  S3L_drawTriangle(p0,p1,p2,0,0);
  
  printf("     0123456789ABCDEF\n");

  uint16_t numErrors = 0;

  for (uint8_t y = 0; y < TEST_BUFFER_H; ++y)
  {
    printf("  %d",y);
    
    if (y < 10)
      printf(" ");

    for (uint8_t x = 0; x < TEST_BUFFER_W; ++x)
    {
      uint8_t expected = expectedPixels[y * TEST_BUFFER_W + x];
      uint8_t rasterized = testRaster[y * TEST_BUFFER_W + x];

      char c =
        expected ?
          (rasterized ? '+' : '|')
          :
          (rasterized ? '-' : ' ');

      if (c == '-' || c == '|')
        numErrors++;

      printf("%c",c);
    }
  
    printf("\n");
  }
    
  printf("  errors: %d\n\n",numErrors);

  return numErrors;
}

int testRasterization(void)
{
  printf("\n=== TESTING RASTERIZATION ===\n");
  
  uint16_t numErrors = 0;
  
  uint8_t pixelsEmpty[TEST_BUFFER_W * TEST_BUFFER_H];
  memset(pixelsEmpty,0,TEST_BUFFER_W * TEST_BUFFER_H);

  numErrors += testTriangleRasterization(5,3, 3,3, 9,3, pixelsEmpty);
  numErrors += testTriangleRasterization(9,4, 9,0, 9,9, pixelsEmpty);
  numErrors += testTriangleRasterization(9,9, 6,6, 3,3, pixelsEmpty);
  numErrors += testTriangleRasterization(0,6, 3,3, 6,0, pixelsEmpty);
  numErrors += testTriangleRasterization(7,7, 7,7, 7,7, pixelsEmpty);

  uint8_t pixels1[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 1
     0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // 2
     0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0, // 4
     0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0, // 5
     0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(4,1, 1,6, 9,7, pixels1);

  uint8_t pixels2[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 1
     0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, // 2
     0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0, // 4
     0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // 5
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(7,1, 1,2, 4,6, pixels2);

  uint8_t pixels3[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 1
     0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // 2
     0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // 4
     0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, // 5
     0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(2,1, 1,3, 6,9, pixels3);

  uint8_t pixels4[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 1
     0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // 2
     0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // 4
     0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, // 5
     0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(4,8, 4,2, 0,0, pixels4);

  uint8_t pixels5[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0, // 1
     0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, // 2
     0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 4
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 5
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(6,2, 2,4, 0,0, pixels5);

  uint8_t pixels6[TEST_BUFFER_W * TEST_BUFFER_H] =
  {
  // 0 1 2 3 4 5 6 7 8 9 A B C D E F
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0
     0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0, // 1
     1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // 2
     0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0, // 3
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 4
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 5
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 6
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 7
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 8
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 9
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // A
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // B
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // C
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // D
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // E
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // F
  }; 

  numErrors += testTriangleRasterization(0,2, 6,0, 4,4, pixels6);

  printf("cover test (each pixel should be covered exactly once):\n\n");

  S3L_ScreenCoord coords[] =
  {
    0,0,
    6,0,       
    13,0,     
    15,0,
    14,1,
    11,2,
    3,3,
    11,4,
    14,5,
    0,6,
    6,6,
    13,8,
    8,9,
    3,12,       
    9,12,
    11,13,
    9,14,
    0,15,                          
    15,15
  };

  memset(testRaster,0,TEST_BUFFER_W * TEST_BUFFER_H);

  #define dt(i1,i2,i3)\
  {\
    S3L_Vec4 p0, p1, p2;\
    S3L_vec4Set(&p0,coords[2*i1],coords[2*i1 + 1],1000,0);\
    S3L_vec4Set(&p1,coords[2*i2],coords[2*i2+1],1000,0);\
    S3L_vec4Set(&p2,coords[2*i3],coords[2*i3+1],1000,0);\
    S3L_drawTriangle(p0,p1,p2,0,0);\
  }

  dt(0,1,6)    // 0
  dt(1,2,5)    // 1
  dt(2,4,5)    // 2
  dt(2,3,4)    // 3
  dt(0,6,9)    // 4
  dt(1,10,6)   // 5
  dt(1,5,10)   // 6
  dt(5,4,8)    // 7
  dt(4,3,8)    // 8
  dt(9,6,10)   // 9
  dt(10,5,12)  // 10
  dt(5,7,12)   // 11
  dt(5,7,11)   // 12
  dt(5,8,11)   // 13
  dt(8,3,18)   // 14
  dt(9,10,13)  // 15
  dt(10,12,13) // 16
  dt(12,7,11)  // 17
  dt(11,8,18)  // 18
  dt(9,13,17)  // 19
  dt(13,12,14) // 20
  dt(12,11,14) // 21
  dt(11,14,15) // 22
  dt(15,11,18) // 23
  dt(13,14,16) // 24
  dt(14,15,16) // 25
  dt(17,13,16) // 26
  dt(16,15,18) // 27
  dt(16,17,18) // 28

  // extra empty triangles
  dt(12,12,12);
  dt(9,10,10);
  dt(1,10,10);
  dt(9,6,1);
  dt(0,6,10);

  #undef dt

  uint16_t numErrors2 = 0;
 
  for (uint8_t y = 0; y < TEST_BUFFER_H - 1; ++y)
  {                                    // ^ complete left and bottom aren't 
    printf("  ");                      // supposed to be rasterized

    for (uint8_t x = 0; x < TEST_BUFFER_W - 1; ++x)
    {
      uint8_t count = testRaster[y * TEST_BUFFER_W + x];
     
      printf("%d",count);
      
      if (count != 1)
        numErrors2++;
    }
 
    printf("\n");
  }

  printf("  errors: %d\n",numErrors2);

  numErrors += numErrors2;

  printf("total rasterization errors: %d\n",numErrors);

  return numErrors;
}

static inline double absVal(double a)
{
  return a >= 0.0 ? a : (-1 * a);
}

double vec3Len(S3L_Vec4 v)
{
  return sqrt(
    ((double) v.x) * ((double) v.x) +
    ((double) v.y) * ((double) v.y) +
    ((double) v.z) * ((double) v.z));
}

int testGeneral(void)
{
  printf("\n=== TESTING GENERAL ===\n");

  printf("testing vector normalization precision...\n");

  S3L_Unit m = 100 * S3L_FRACTIONS_PER_UNIT;
  S3L_Unit tolerance = 0.1 * S3L_FRACTIONS_PER_UNIT;

  uint32_t errors0 = 0;
  uint32_t errors1 = 0;

  for (S3L_Unit x = -1 * m; x < m; x += 3 * (absVal(x) / 64 + 1))
    for (S3L_Unit y = -1 * m; y < m; y += 3 * (absVal(y) / 32 + 1))
      for (S3L_Unit z = -1 * m; z < m; z += 5 * (absVal(z) / 64 + 1))
      {
        S3L_Vec4 v;

        S3L_vec4Set(&v,x,y,z,0);
        S3L_vec3NormalizeFast(&v);

        double l0 = vec3Len(v);
        double e0 = absVal(l0 - S3L_FRACTIONS_PER_UNIT);

        S3L_vec4Set(&v,x,y,z,0);
        S3L_vec3Normalize(&v);

        double l1 = vec3Len(v);
        double e1 = absVal(l1 - S3L_FRACTIONS_PER_UNIT);

        if (e0 > tolerance)
          errors0++;

        if (e1 > tolerance)
        {
          errors1++;

          printf("%f\n",l1);
          S3L_logVec4(v);
        }
      }

  printf("wrong normalization with unsafe function: %d\nwrong normalizations with safe function: %d\n",errors0,errors1);

  return errors1;
}

S3L_Unit cubeVertices[] = { S3L_CUBE_VERTICES(S3L_FRACTIONS_PER_UNIT) };
S3L_Index cubeTriangles[] = { S3L_CUBE_TRIANGLES };
S3L_Unit triangleVertices[] = { -512, 0, 512, 402, 0, 200, 0, 600, 0 };
S3L_Index triangleTriangles[] = { 0, 1, 2 };

S3L_Model3D cubeModel;
S3L_Model3D triangleModel;
S3L_Model3D models[4];
S3L_Scene scene;

int testRender(void) 
{
  printf("\n=== TESTING RENDER ===\n");

  memset(testScreen,'.',S3L_RESOLUTION_X * S3L_RESOLUTION_Y);

  S3L_model3DInit(cubeVertices,S3L_CUBE_VERTEX_COUNT,cubeTriangles,S3L_CUBE_TRIANGLE_COUNT,&cubeModel); 
  S3L_model3DInit(triangleVertices,3,triangleTriangles,1,&triangleModel); 

  models[0] = cubeModel;
  models[0].transform.translation.z -= S3L_FRACTIONS_PER_UNIT;

  models[1] = cubeModel;
  models[1].transform.translation.x -= S3L_FRACTIONS_PER_UNIT * 2;
  models[1].transform.translation.y = S3L_FRACTIONS_PER_UNIT / 2;
  models[1].transform.scale.y = S3L_FRACTIONS_PER_UNIT * 2;
  models[1].transform.rotation.x = 200;
  models[1].transform.rotation.y = 100;

  models[2] = triangleModel;
  models[2].transform.translation.x = -1000;
  models[2].transform.translation.y = 1000;

  models[3] = triangleModel;
  models[3].transform.translation.x = -1500;
  models[3].transform.translation.y = 1200;
  models[3].transform.rotation.x = S3L_FRACTIONS_PER_UNIT / 2; // turn away, test BF culling

  S3L_sceneInit(models,4,&scene);

  scene.camera.transform.translation.z = -2 * S3L_FRACTIONS_PER_UNIT;
  scene.camera.transform.translation.y = S3L_FRACTIONS_PER_UNIT / 3;
  scene.camera.transform.rotation.y = 30;

  renderingMode = 1;

  S3L_newFrame(); 
  S3L_drawScene(scene);

  int errors = 0;

  for (uint32_t i = 0; i < (S3L_RESOLUTION_X * S3L_RESOLUTION_Y); ++i)
  {
    if ((i % S3L_RESOLUTION_X) == 0)
      printf("  \n");

    printf("%c",testScreen[i]);

    if (testScreen[i] != expectedRender[i])
      errors += 1;
  } 

  return errors;
}

int main(void)
{
  printf("testing small3dlib\n\n");

  S3L_Mat4 m, m2;
  S3L_Vec4 v;

  S3L_mat4Init(&m);
  S3L_logMat4(m);

  S3L_vec4Init(&v);

  S3L_logVec4(v);
 
  S3L_vec4Xmat4(&v,&m); 
  S3L_logVec4(v);

  S3L_makeTranslationMat(100,200,300,&m2);
  S3L_logMat4(m2);

  S3L_mat4Xmat4(&m,&m2);
  S3L_logMat4(m);

  uint32_t totalErrors = 0;

  totalErrors += testGeneral();
  totalErrors += testRasterization();
  totalErrors += testRender();

  printf("\n\n===== DONE =====\ntotal errors: %d\n",totalErrors);

  return 0;
}
