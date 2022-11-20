/*
  Example program for small3dlib, showing a Quake-like level.

  author: Miloslav Ciz
  license: CC0 1.0
*/

#define TEXTURES 1 // whether to use textures for the level
#define FOG 1

#include <stdio.h>
#include <time.h>

#define S3L_RESOLUTION_X 640
#define S3L_RESOLUTION_Y 480

#define S3L_NEAR_CROSS_STRATEGY 3

#if TEXTURES
  #define S3L_PERSPECTIVE_CORRECTION 2
#else
  #define S3L_PERSPECTIVE_CORRECTION 0
#endif

#define S3L_NEAR (S3L_F / 5)

#define S3L_USE_WIDER_TYPES 0
#define S3L_FLAT 0
#define S3L_SORT 0
#define S3L_Z_BUFFER 1
#define S3L_MAX_TRIANGES_DRAWN 512

#define S3L_PIXEL_FUNCTION drawPixel

#include "../small3dlib.h"

#define TEXTURE_W 64
#define TEXTURE_H 64

#include "sdl_helper.h"

#include "levelModel.h"
#include "levelTextures.h"

S3L_Scene scene;

S3L_Vec4 teleportPoint;

uint32_t frame = 0;
const uint8_t *texture = 0;
uint32_t previousTriangle = 1000;
S3L_Vec4 uv0, uv1, uv2;

void drawTeleport(int16_t x, int16_t y, S3L_ScreenCoord size)
{
  int16_t halfSize = size / 2;

  S3L_ScreenCoord x0 = S3L_max(0,x - halfSize);
  S3L_ScreenCoord x1 = S3L_min(S3L_RESOLUTION_X,x + halfSize);
  S3L_ScreenCoord y0 = S3L_max(0,y - halfSize);
  S3L_ScreenCoord y1 = S3L_min(S3L_RESOLUTION_Y,y + halfSize);

  S3L_ScreenCoord row = y0 - (y - halfSize);

  for (S3L_ScreenCoord j = y0; j < y1; ++j)
  {
    S3L_ScreenCoord i0, i1;

    if (row <= halfSize)
    {
      i0 = S3L_max(x0,x - row); 
      i1 = S3L_min(x1,x + row); 
    }
    else
    {
      i0 = S3L_max(x0,x - size + row); 
      i1 = S3L_min(x1,x + size - row); 
    }

    for (S3L_ScreenCoord i = i0; i < i1; ++i)
      if (rand() % 8 == 0)
        setPixel(i,j,255,0,0);

    row++;
  }
}

void drawPixel(S3L_PixelInfo *p)
{
  uint8_t r, g, b;

#if TEXTURES
  if (p->triangleID != previousTriangle)
  {
    uint8_t material = levelMaterials[p->triangleIndex];

    switch (material)
    {
      case 0:
        texture = level1Texture;
        break;

      case 1:
        texture = level2Texture;
        break;

      case 2:
      default:
        texture = level3Texture;
        break;
    }

    S3L_getIndexedTriangleValues(p->triangleIndex,levelUVIndices,levelUVs,2,&uv0,&uv1,&uv2);
    previousTriangle = p->triangleID;
  }

  S3L_Unit uv[2];

  uv[0] = S3L_interpolateBarycentric(uv0.x,uv1.x,uv2.x,p->barycentric) / 16;
  uv[1] = S3L_interpolateBarycentric(uv0.y,uv1.y,uv2.y,p->barycentric) / 16;

  sampleTexture(texture,uv[0],uv[1],&r,&g,&b);
#else
  switch (p->modelIndex)
  {
    case 0: r = 255; g = 0; b = 0; break;
    case 1: r = 0; g = 255; b = 0; break;
    case 2:
    default: r = 0; g = 0; b = 255; break;
  }
#endif

#if FOG
  S3L_Unit fog = (p->depth * 
#if TEXTURES
    8
#else
    16
#endif
  ) / S3L_F;

  r = S3L_clamp(((S3L_Unit) r) - fog,0,255);
  g = S3L_clamp(((S3L_Unit) g) - fog,0,255);
  b = S3L_clamp(((S3L_Unit) b) - fog,0,255);
#endif

  setPixel(p->x,p->y,r,g,b); 
}

S3L_Transform3D modelTransform;
S3L_DrawConfig conf;

clock_t nextT;

int fps = 0;

void draw(void)
{
  S3L_newFrame();

  clearScreen();

  S3L_drawScene(scene);

  S3L_Vec4 screenPoint;

  S3L_project3DPointToScreen(teleportPoint,scene.camera,&screenPoint);

  if (screenPoint.w > 0 && 
      screenPoint.x >= 0 && screenPoint.x < S3L_RESOLUTION_X &&
      screenPoint.y >= 0 && screenPoint.y < S3L_RESOLUTION_Y &&
      screenPoint.z < S3L_zBufferRead(screenPoint.x,screenPoint.y)) 
    drawTeleport(screenPoint.x,screenPoint.y,screenPoint.w);

  clock_t nowT = clock();

  double timeDiff = ((double) (nowT - nextT)) / CLOCKS_PER_SEC;

  fps++;

  if (timeDiff >= 1.0)
  {
    nextT = nowT;
    printf("FPS: %d\n",fps);

    printf("camera: ");
    S3L_logTransform3D(scene.camera.transform);
    fps = 0;
  }
}

int main(void)
{
  sdlInit();

  teleportPoint.x = 6 * S3L_F;
  teleportPoint.y = -3 * S3L_F;
  teleportPoint.z = 3 * S3L_F / 2;
  teleportPoint.w = S3L_F;

  nextT = clock();

  levelModelInit();

  S3L_sceneInit(&levelModel,1,&scene);

  int running = 1;

  while (running) // main loop
  {
    draw();

    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
        running = 0;

    S3L_Vec4 camF, camR;
 
    S3L_rotationToDirections(scene.camera.transform.rotation,20,&camF,&camR,0);

    const uint8_t *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_ESCAPE])
      running = 0;

    if (state[SDL_SCANCODE_A])
      scene.camera.transform.rotation.y += 1;
    else if (state[SDL_SCANCODE_D])
      scene.camera.transform.rotation.y -= 1;
    else if (state[SDL_SCANCODE_W])
      scene.camera.transform.rotation.x += 1;
    else if (state[SDL_SCANCODE_S])
      scene.camera.transform.rotation.x -= 1;

    if (state[SDL_SCANCODE_UP])
      S3L_vec3Add(&scene.camera.transform.translation,camF);
    else if (state[SDL_SCANCODE_DOWN])
      S3L_vec3Sub(&scene.camera.transform.translation,camF);
    else if (state[SDL_SCANCODE_LEFT])
      S3L_vec3Sub(&scene.camera.transform.translation,camR);
    else if (state[SDL_SCANCODE_RIGHT])
      S3L_vec3Add(&scene.camera.transform.translation,camR);

    sdlUpdate();

    frame++;
  }

  sdlEnd();

  return 0;
}
