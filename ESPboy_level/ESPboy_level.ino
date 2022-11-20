/*
  Example program of small3dlib for Pokitto -- Quake-like level.

  author: Miloslav Ciz
  license: CC0 1.0
*/


#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"


#define SUBSAMPLE 6

#if 1   // This can switch between a textured and flat mode.
  #define S3L_Z_BUFFER 2
  #define S3L_SORT 0
  #define S3L_STENCIL_BUFFER 0
  #define S3L_FLAT 0
  #define S3L_PERSPECTIVE_CORRECTION 2
#else
  #define S3L_Z_BUFFER 2
  #define S3L_SORT 0
  #define S3L_STENCIL_BUFFER 0
  #define S3L_FLAT 1
  #define S3L_MAX_TRIANGES_DRAWN 400
#endif

#define S3L_PIXEL_FUNCTION pixelFunc

// Because we'll be writing pixels as 2x2, define the resolution one smaller.
#define BASE_W 128
#define BASE_H 128

#define S3L_RESOLUTION_X (BASE_W - BASE_W / SUBSAMPLE)
#define S3L_RESOLUTION_Y (BASE_H - BASE_H / SUBSAMPLE)

ESPboyInit myESPboy;
uint8_t screenBuffer[128*128];


#define S3L_STRICT_NEAR_CULLING 0

#define S3L_COMPUTE_DEPTH 1 // for fog

#define S3L_REDUCED_Z_BUFFER_GRANULARITY 6

#include "small3dlib.h"

#include "levelTexture1Pal.h"
#include "levelModel.h"


#if S3L_FLAT
uint8_t triangleColors[LEVEL_TRIANGLE_COUNT];
#endif


void clearLCD(){
  memset (screenBuffer, 0, sizeof(screenBuffer));
}


void updateLCD(){
  static uint16_t buffLine[128];
  uint16_t addr=0;
  myESPboy.tft.setWindow(0, 0, 127, 127);
  for(uint8_t i=0; i<128; i++){
    for(uint8_t j=0; j<128; j++){
      buffLine[j] = pgm_read_word(&level1Palette[screenBuffer[addr]]);
      addr++;
    }
    myESPboy.tft.pushPixels(buffLine, 128);
  }
}


static inline uint8_t texture(int32_t u, int32_t v)
{
  u = S3L_wrap(u,LEVEL1_TEXTURE_WIDTH);
  v = S3L_wrap(v,LEVEL1_TEXTURE_HEIGHT);

  uint32_t index = v * LEVEL1_TEXTURE_WIDTH + u;

  return level1Texture[index];
}

S3L_ScreenCoord subsampleMap[BASE_W + SUBSAMPLE];

uint32_t previousTriangle = 100;

static inline uint8_t addIntensity(uint8_t color, uint8_t intensity)
{
  uint8_t newValue = color + intensity; // value as in HSV

  if (color >> 4 == newValue >> 4)
    return newValue;

  return color | 0x0F;
}

static inline uint8_t substractIntensity(uint8_t color, uint8_t intensity)
{
  uint8_t newValue = color - intensity; // value as in HSV

  if (color >> 4 == newValue >> 4)
    return newValue;

  return 0;
}

uint8_t c = 0;

S3L_Vec4 uv0, uv1, uv2;

S3L_Index material = 0;

void pixelFunc(S3L_PixelInfo *p)
{
  uint8_t val;
  uint8_t *buf = screenBuffer;

#if S3L_FLAT
  val = triangleColors[p->triangleIndex];
#else
  if (p->triangleIndex != previousTriangle)
  {
    material = levelMaterials[p->triangleIndex];

    if (material == 1)
      c = 135;
    else if (material == 2)
      c = 213;
    else
      S3L_getIndexedTriangleValues(p->triangleIndex,levelUVIndices,levelUVs,2,&uv0,&uv1,&uv2);

    previousTriangle = p->triangleID;
  }

  S3L_Unit fog = p->depth >> 9;
  
  if (material == 0)
  {
    S3L_Unit uv[2];
    uv[0] = S3L_interpolateBarycentric(uv0.x,uv1.x,uv2.x,p->barycentric);
    uv[1] = S3L_interpolateBarycentric(uv0.y,uv1.y,uv2.y,p->barycentric);

    c = texture(uv[0] / 32,uv[1] / 32);
  }

  val = substractIntensity(c,fog);
#endif

  buf += subsampleMap[p->y] * 128;
  buf += subsampleMap[p->x];
  *buf = val;

  buf++;
  *buf = val;
  buf += 127;
  *buf = val;
  buf++;
  *buf = val;
}

S3L_Scene scene;


void draw()
{
  S3L_newFrame();
  S3L_drawScene(scene);
}



void setup()
{
  myESPboy.begin("small3dlib level DEMO");
  myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  
  for (uint16_t i = 0; i < BASE_W + SUBSAMPLE; ++i)
    subsampleMap[i] = i + i / SUBSAMPLE;

#if S3L_FLAT
  S3L_Vec4 toLight;
  S3L_setVec4(&toLight,10,5,7,0);
  S3L_normalizeVec3(&toLight);

  for (uint16_t i = 0; i < LEVEL_TRIANGLE_COUNT; ++i)
  {
    uint8_t c;

    S3L_Vec4 v0, v1, v2;

    S3L_getIndexedTriangleValues(
      i,
      levelTriangleIndices,
      levelVertices,3,&v0,&v1,&v2);

    material = levelMaterials[i];

    if (material == 1)
      c = 38;
    else if (material == 2)
      c = 53;
    else
      c = 24;

    S3L_Vec4 normal;

    S3L_triangleNormal(v0,v1,v2,&normal);

    triangleColors[i] = addIntensity(c,    
      S3L_max(0,(S3L_dotProductVec3(normal,toLight) + S3L_FRACTIONS_PER_UNIT) / 64));
  }
  #endif


  S3L_initCamera(&scene.camera);
  levelModelInit();
  S3L_initScene(&levelModel,1,&scene);

}




void loop(){
      static S3L_Vec4 camF, camR, camU;
      static int step = 300;
      static int step2 = 8;
      static uint8_t readedKey;

      readedKey = myESPboy.getKeys();
 
      S3L_rotationToDirections(
        scene.camera.transform.rotation,
        step,
        &camF,
        &camR,
        &camU);

      if (readedKey&PAD_ACT)
      {
        if (readedKey&PAD_UP)
          scene.camera.transform.rotation.x += 8;
        else if (readedKey&PAD_DOWN)
          scene.camera.transform.rotation.x -= 8;
        else if (readedKey&PAD_LEFT)
          scene.camera.transform.rotation.y += 8;
        else if (readedKey&PAD_RIGHT)
          scene.camera.transform.rotation.y -= 8;
      }
      else
      {
        if (readedKey&PAD_UP)
          S3L_vec3Add(&(scene.camera.transform.translation),camF);
        else if (readedKey&PAD_DOWN)
          S3L_vec3Sub(&scene.camera.transform.translation,camF);
        else if (readedKey&PAD_RIGHT)
          S3L_vec3Add(&scene.camera.transform.translation,camR);
        else if (readedKey&PAD_LEFT)
          S3L_vec3Sub(&scene.camera.transform.translation,camR);
      }

      clearLCD();
      draw();
      updateLCD();
}
