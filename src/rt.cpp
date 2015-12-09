#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <float.h>
#include <math.h>

using namespace std;

#define WIDTH 500
#define HEIGHT 500

struct v3
{
  float x;
  float y;
  float z;
};

struct Ray
{
  v3 origin;
  v3 direction;
};

struct Sphere
{
  v3 center;
  float radius;
  v3 color;
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

v3 color = {255.0, 0.0, 0.0};

void printV3(v3 vector)
{
  cout << vector.x << ", " << vector.y << ", " << vector.z << endl;
}

float dotProduct(v3 vector1, v3 vector2)
{
  return (vector1.x * vector2.x +
          vector1.y * vector2.y +
          vector1.z * vector2.z);
}

v3 subtraction(v3 vector1, v3 vector2)
{
  v3 result;
  result = {vector1.x - vector2.x,
            vector1.y - vector2.y,
            vector1.z - vector2.z};
  return result;
}

v3 addition(v3 vector1, v3 vector2)
{
  v3 result;
  result = {vector1.x + vector2.x,
            vector1.y + vector2.y,
            vector1.z + vector2.z};
  return result;
}

v3 vectorByScalar(v3 vector, float scalar)
{
  v3 result;
  result ={vector.x * scalar,
           vector.y * scalar,
           vector.z * scalar};
  return result;
}

v3 normalize(v3 vector)
{
  float length = sqrt(vector.x * vector.x +
                      vector.y * vector.y +
                      vector.z * vector.z);

  v3 result = {vector.x / length, vector.y / length,
               vector.z / length};
  //printV3(result);
  return result;
}

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
        if (IMG_Init(0) < 0)
        {
          cout << "Problem loading image lib: " << IMG_GetError() << endl;
          success = false;
        }
        else
        {
          SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

          SDL_RenderClear(renderer);

          SDL_RenderPresent(renderer);
        }
      }
    }
  }

  return success;
}

void close()
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  window = NULL;

  IMG_Quit();
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
  bool success;
  float a = dotProduct(ray.direction, ray.direction);
  //cout << a << endl;
  float b = 2 * dotProduct(ray.direction, 
                           subtraction(ray.origin, sphere.center)); 
  //cout << b << endl;
  float c = dotProduct(subtraction(ray.origin, sphere.center),
                       subtraction(ray.origin, sphere.center)) - 
            sphere.radius * sphere.radius;

  //cout << c << endl;
  float discriminant = b * b - 4 * a * c;
  //cout << discriminant << endl;
  // If the discriminant is less than cero, we have complex roots
  if (discriminant < 0)
  {
    success = false;
  }
  else
  {
    float t, t1, t2;

    t1 = (-b + sqrt(discriminant)) / (2 * a); 
    t2 = (-b - sqrt(discriminant)) / (2 * a); 

    //cout << "t1: " << t1 << ", t2: " << t2 << endl;

    // If both roots are negative -> no solution
    if ((t1 < 0) && (t2 < 0))
    {
      success = false;
    }
    // Otherwise, we have solution. Which one?
    else
    {
      success = true;

      // Refactor this
      if ((t1 > 0) && (t2 < 0))
      {
        t = t1;
      }
      else if ((t1 < 0) && (t2 > 0))
      {
        t = t2;
      }
      else if (t1 <= t2)
      {
        t = t1;
      }
      else if (t1 >= t2)
      {
        t = t2;
      }

      *hitPoint = addition(ray.origin, vectorByScalar(ray.direction, t));
    }
  }

  return success;
}

void render()
{
  // Creates an image using the ray tracing method.
  // The first prototype will have only spheres and flat shading.
  // Then I will add diffuse shading (lights) and after that
  // shadows.

  // TODO(ivan): I need to define the camera and the plane
  // I'm projecting the scene on!!!
  // For the moment, the camera is at (0, 0, 0) and pointing 
  // to +Z and the plane is an XY plane at Z=1;

  // TODO(ivan): Decide the coordinate system. Right now I'm 
  // working with a left handed cs, but maybe I should change to
  // a right handed one
  
  vector<Sphere> scene;
  v3 eyePosition = {0.0f, 0.0f, 0.0f};

  v3 image[WIDTH][HEIGHT] = {};

  /*for (int i = 0; i < WIDTH; i++)
  {
    for (int j = 0; j < HEIGHT; j++)
    {
      cout << image[i][j].x << ", " << image[i][j].y 
           << ", " << image[i][j].z << endl;
    }
  }*/

  // prepare the scene
  Sphere testSphere = {};
  testSphere.center = {0.0f, 0.0f, 4.0f};
  testSphere.radius = 3.0f;
  testSphere.color = {255.0f, 255.0f, 0.0f};
  scene.push_back(testSphere);

  // In degrees
  float fov = 100.0f;
  float aspectRatio = (float)WIDTH / (float)HEIGHT;

  // trace rays
  for (int j = 0; j < HEIGHT; j++)
  {
    for (int i = 0; i < WIDTH; i++)
    {
      // create the ray based in eye position, i and j
      Ray ray;
      ray.origin = eyePosition;
      float pX = (2 * ((i + 0.5) / WIDTH) - 1) * tan(fov / 2 * M_PI / 180) * aspectRatio;
      float pY = (1 - 2 * ((j + 0.5) / HEIGHT)) * tan(fov / 2 * M_PI / 180);
      // As the origin is (0, 0, 0), I don't need to do the subtraction
      ray.direction = {pX, pY, 1.0f};
      ray.direction = normalize(ray.direction);
      //printV3(ray.direction);

      v3 hitPoint;

      float minDistance = FLT_MAX;
      
      Sphere *closerObject = NULL;

      for (int k = 0; k < scene.size(); k++)
      {
        if (intersect(scene[k], ray, &hitPoint))
        {
          //cout << "FOO" << endl;
          // Calculate the distance between hitPoint and eye
          // If it is less than the previous one, this is 
          // the closer object, and we have to store it in order
          // to take its color as the color for this pixel
          float dist = myDistance(eyePosition, hitPoint);
          
          if (dist < minDistance)
          {
            closerObject = &scene[k];
            minDistance = dist;
          }
        } 
        //exit(1);
      }

      // For the moment is going to be always NULL
      if (closerObject == NULL)
      {
        image[i][j] = color;
      }
      else
      {
        image[i][j] = closerObject->color;
      }
    }
  }

  /*for (int i = 0; i < WIDTH; i++)
  {
    for (int j = 0; j < HEIGHT; j++)
    {
      cout << image[i][j].x << ", " << image[i][j].y 
           << ", " << image[i][j].z << endl;
    }
  }*/

  std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
  ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
  for (unsigned i = 0; i < WIDTH; i++)
  {
    for (unsigned j = 0; j < HEIGHT; j++)
    {
      ofs << (unsigned char)(image[i][j].x) <<
             (unsigned char)(image[i][j].y) <<
             (unsigned char)(image[i][j].z);
    }
  } 

  ofs.close();
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
      else if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_1:
          {
            color = {255.0, 0.0, 0.0};
            break;
          }
          case SDLK_2:
          {
            color = {0.0, 255.0, 0.0};
            break;
          }
          case SDLK_3:
          {
            color = {0.0, 0.0, 255.0};
            break;
          }
          case SDLK_r:
          {
            render();
            SDL_Surface* surface = IMG_Load("untitled.ppm");
            if (!surface)
            {
              cout << "Problem loading the image: " << IMG_GetError() << endl;
            }
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, 0, 0);
            SDL_RenderPresent(renderer);
            break;
          }
        }
      }
    }
  }

  close();

  return (0);
}
