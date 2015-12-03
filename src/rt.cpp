#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

#include <float.h>


using namespace std;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define WIDTH 100
#define HEIGHT 100

struct v3
{
  float x;
  float y;
  float z;
};

typedef int* Sphere;
typedef int Ray;

bool init()
{
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    cout << "Error initializing SDL: " << SDL_GetError() << endl;
    success = false;
  }
  else
  {
    window = SDL_CreateWindow("Devember", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
      cout << "Problem creating the window: " << SDL_GetError() << endl;
      success = false;
    }
    else
    {
      renderer = SDL_CreateRenderer(window, -1, 0);

      if (renderer == NULL)
      {
        cout << "Problem creating the renderer: " << SDL_GetError() << endl;
      }
      else
      {
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
      }
    }
  }

  return success;
}

void close()
{
  SDL_DestroyWindow(window);
  window = NULL;

  SDL_Quit();
}

float myDistance(v3 point1, v3 point2)
{
  return 0.0f;
}

bool intersect(Sphere sphere, Ray ray, v3 *hitPoint)
{
  // first version for an intersect method
  // we just need the hit point for the intersection
  return false;
}

void render()
{
  // Creates an image using the ray tracing method.
  // The first prototype will have only spheres and flat shading.
  // Then I will add diffuse shading (lights) and after that
  // shadows.
  vector<Sphere> scene;
  v3 eyePosition;

  v3 image[WIDTH][HEIGHT] = {};

  // prepare the scene

  // trace rays
  for (int i = 0; i < WIDTH; i++)
  {
    for (int j = 0; j < HEIGHT; j++)
    {
      // create the ray based in eye position, i and j
      Ray ray;
      v3 hitPoint;
      float minDistance = FLT_MAX;
      
      Sphere closerObject = NULL;

      for (int k = 0; k < scene.size(); k++)
      {
        if (intersect(scene[k], ray, &hitPoint))
        {
          // Calculate the distance between hitPoint and eye
          // If it is less than the previous one, this is 
          // the closer object, and we have to store it in order
          // to take its color as the color for this pixel
          float dist = myDistance(eyePosition, hitPoint);
          
          if (dist < minDistance)
          {
            closerObject = scene[k];
            minDistance = dist;
          }
        } 
      }

      // For the moment is going to be always NULL
      if (closerObject == NULL)
      {
        image[i][j] = {255.0, 0.0, 0.0};
      }
    }
  }
}

int main(int argc, char* argv[])
{
  if (!init())
  {
    return (1);
  }

  bool quit = false;
  SDL_Event event;

  while (!quit)
  {
    while (SDL_PollEvent(&event) != 0)
    {
      if (event.type == SDL_QUIT)
      {
        quit = true;
      }
    }
    render();
  }

  close();

  return (0);
}
