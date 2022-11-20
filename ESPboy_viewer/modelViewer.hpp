/*
  Gamebuino META example demo for small3dlib -- model viewer.

  author: Miloslav Ciz
  license: CC0 1.0
*/

#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"


/* Before including small3dlib, we need to define some values for it, such as
  the resolution, the name of the pixel drawing function etc.: */

#define S3L_PIXEL_FUNCTION pixelFunc

#define S3L_RESOLUTION_X 128
#define S3L_RESOLUTION_Y 128

ESPboyInit myESPboy;
uint8_t screenBuffer[S3L_RESOLUTION_X*S3L_RESOLUTION_Y];

#define S3L_Z_BUFFER 2   // this sets up a reduced precision z-buffer
#define S3L_SORT 0       // no sorting of triangles
#define S3L_STENCIL_BUFFER 0   // no stencil buffer
#define S3L_PERSPECTIVE_CORRECTION 0   /* perspective correction expensive and
                                          dosn't improve quality significantly
                                          with far away models, so turn it off,
                                          but you can try setting it it 2
                                          (approximation) */
#include "small3dlib.h"     // now we can include the library

// include the resources (converted using provided Python scripts):

#include "palette.h"
#include "houseModel.h"
#include "houseTexture.h"
#include "chestModel.h"
#include "chestTexture.h"
#include "earthModel.h"
#include "earthTexture.h"
#include "gbmetaModel.h"
#include "gbmetaTexture.h"

uint16_t paletteGB[256];

#define MIN_ZOOM (-6 * S3L_FRACTIONS_PER_UNIT)
#define MAX_ZOOM (-16 * S3L_FRACTIONS_PER_UNIT)

#define TEXTURE_W 64
#define TEXTURE_H 64

// helper global variabls and pointers:

int16_t previousTriangle = -1;
S3L_Vec4 uv0, uv1, uv2;
const uint8_t *texture = houseTexture;
const S3L_Index *uvIndices = houseUVIndices;
const S3L_Unit *uvs = houseUVs;

void clearLCD(){
  memset (screenBuffer, 0, sizeof(screenBuffer));
}

void updateLCD(){
  static uint16_t buffLine[S3L_RESOLUTION_X];
  uint16_t addr;
  myESPboy.tft.setWindow(0, 0, S3L_RESOLUTION_X-1, S3L_RESOLUTION_Y-1);
  for(uint8_t i=0; i<S3L_RESOLUTION_Y; i++){
    addr = i*S3L_RESOLUTION_X;
    for(uint8_t j=0; j<S3L_RESOLUTION_X; j++){
      buffLine[j] = pgm_read_word(&paletteGB[screenBuffer[addr]]);
      addr++;
    }
    myESPboy.tft.pushPixels(buffLine, S3L_RESOLUTION_X);
  }
}



static inline unsigned short sampleTexure(int32_t u, int32_t v)
{
  int index = v * TEXTURE_W + u;

  return texture[index];
}

/* This function will be called by the library to render individual pixels --
   remember, this is the bottleneck, it should be as fast as possible! */
void pixelFunc(S3L_PixelInfo *p)
{
  S3L_Unit u, v;   // texturing coordinates

  if (p->triangleIndex != previousTriangle)
  {
    /* This is a per-triangle cache, it prevents computing per-triangle values
       for each pixel. We only recompute these when the triangleID changes. */

    S3L_getIndexedTriangleValues(p->triangleIndex,uvIndices,uvs,2,&uv0,&uv1,&uv2);
    /* ^ This is a helper funtion that retrieves the 3 UV coordinates of the
       triangle (one for each triangle vertex). */

    previousTriangle = p->triangleIndex;
  }
  
  u = S3L_interpolateBarycentric(uv0.x,uv1.x,uv2.x,p->barycentric);
  v = S3L_interpolateBarycentric(uv0.y,uv1.y,uv2.y,p->barycentric);

  uint8_t c = sampleTexure(u >> 3,v >> 3);
              /* ^ Shifting by 3 gets the value from 0 to 512
                 (S3L_FRACTIONS_PER_UNIT) to 0 to 32 (texture resoltion). */

  // now write the pixel:

  //uint16_t col = paletteGB[c];
  screenBuffer[p->x+p->y*S3L_RESOLUTION_X] = c;
  //tft.drawPixel(p->x,p->y,col);
}

S3L_Scene scene; // our 3D scene, it will only hold one model at a time

void draw()
{
  S3L_newFrame();       // needs to be done before rendering a new frame
  S3L_drawScene(scene); // renders the 3D scene
}

void setModel(uint8_t index)
{
  #define modelCase(n)\
    scene.models = &(n##Model);\
    texture = n##Texture;\
    uvIndices = n##UVIndices;\
    uvs = n##UVs;

  switch (index)
  {
    case 1:
      modelCase(chest)
      break;
    
    case 2:
      modelCase(earth)
      break;

    case 3:
      modelCase(gbmeta)
      break;

    default:
      modelCase(house)
      break;
  }

  #undef modelCase
}

uint8_t modelIndex = 0;

void setup()
{
  myESPboy.begin("small3dlib DEMO");
  myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  
  for (uint16_t i = 0; i < 256; ++i)
  {
    uint8_t h,s,v,r,g,b;

    r = palette[i * 3];
    g = palette[i * 3 + 1];
    b = palette[i * 3 + 2];

    paletteGB[i] = myESPboy.tft.color565(r,g,b);
  }

  houseModelInit();
  chestModelInit();
  earthModelInit();
  gbmetaModelInit();

  S3L_initScene(&houseModel,1,&scene);

  setModel(0);

  scene.camera.transform.translation.z = -8 * S3L_FRACTIONS_PER_UNIT;
  // ^ place the camera a little bit to the front so that the model is seen
}

void loop(){
 static uint16_t keysReaded=0;
 static uint32_t lastCount=0;
 static uint16_t fpsToPrint;
 
 fpsToPrint = (1000/(millis()-lastCount));
 lastCount=millis();

  S3L_Unit rotationStep = 16;
  S3L_Unit zoomStep = 140;

  keysReaded = myESPboy.getKeys();
  
  if (keysReaded&PAD_ACT)
  {
    if (keysReaded&PAD_DOWN)
      scene.camera.transform.translation.z =
        S3L_max(MAX_ZOOM,scene.camera.transform.translation.z - zoomStep);
    else if (keysReaded&PAD_UP)
      scene.camera.transform.translation.z =
        S3L_min(MIN_ZOOM,scene.camera.transform.translation.z + zoomStep);
  }
  else
  {
    if (keysReaded&PAD_UP)
      scene.models[0].transform.rotation.x += rotationStep;
    else if (keysReaded&PAD_DOWN)
      scene.models[0].transform.rotation.x -= rotationStep;
        
    if (keysReaded&PAD_RIGHT)
      scene.models[0].transform.rotation.y += rotationStep;
    else if (keysReaded&PAD_LEFT)
      scene.models[0].transform.rotation.y -= rotationStep;
  }

  if (keysReaded&PAD_ESC)
  {
    modelIndex = (modelIndex + 1) % 4;
    setModel(modelIndex);
    while(myESPboy.getKeys())delay(0);
    delay(100);
  }

  clearLCD();
  draw();
  updateLCD();
  myESPboy.tft.setCursor(0,10);
  myESPboy.tft.print(fpsToPrint);
}
