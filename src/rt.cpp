#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <float.h>
#include <math.h>

#include <glm/glm.hpp>

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
  glm::vec3 origin;
  glm::vec3 direction;
};

struct Sphere
{
  glm::vec3 center;
  float radius;
  glm::vec3 color;
  glm::vec3 diffuse = {128.0f, 128.0f, 128.0f};
  glm::vec3 specular= {255.0f, 255.0f, 255.0f};
  float shininess = 100.0f;
  glm::vec3 hitPoint;
  glm::vec3 hitNormal;
};

struct Plane
{
  glm::vec3 normal;
  glm::vec3 p0;
  glm::vec3 color;
};

struct Light
{
  glm::vec3 position;
  glm::vec3 color;
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

glm::vec3 color = {255.0, 0.0, 0.0};
glm::vec3 black = {0.0, 0.0, 0.0};

void printV3(glm::vec3 vector)
{
  cout << vector.x << ", " << vector.y << ", " << vector.z << endl;
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

bool intersectSphere(Sphere sphere, Ray ray, glm::vec3 *hitPoint, glm::vec3 *hitNormal)
{
  // first version for an intersect method
  // we just need the hit point for the intersection
  bool success;
  glm::vec3 originCenter = ray.origin - sphere.center;
  float a = glm::dot(ray.direction, ray.direction);
  //cout << a << endl;
  float b = 2 * glm::dot(ray.direction, originCenter); 
  //cout << b << endl;
  float c = glm::dot(originCenter, originCenter) - 
            (sphere.radius * sphere.radius);

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

      *hitPoint = ray.origin + ray.direction * t;
      //*hitPoint = ray.origin + ray.direction * (t - 0.1f);
      *hitNormal = glm::normalize(*hitPoint - sphere.center);
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
  // NOTE(ivan): For the moment, the camera is at (0, 0, 0) and pointing 
  // to -Z and the plane is an XY plane at Z=-1;

  // NOTE(ivan): I'm working with a right handed one coordinate
  // system
  
  vector<Sphere> scene;
  glm::vec3 eyePosition = {0.0f, 0.0f, 0.0f};
  glm::vec3 ambient = {51.0f, 51.0f, 51.0f};

  Light testLight = {};
  testLight.position = {0.0f, 30.0f, -10.0f};
  testLight.color = {255.0f, 255.0f, 255.0f};


  glm::vec3 image[WIDTH][HEIGHT] = {};

  // prepare the scene
  Sphere testSphere = {};
  testSphere.center = {0.0f, -4.0f, -10.0f};
  testSphere.radius = 3.0f;
  testSphere.color = {0.0f, 0.0f, 255.0f};
  scene.push_back(testSphere);
  testSphere.center = {0.0f, 1.5f, -10.0f};
  testSphere.radius = 2.0f;
  testSphere.color = {0.0f, 255.0f, 0.0f};
  scene.push_back(testSphere);
  /*testSphere.center = {0.0f, 5.0f, -7.0f};
  testSphere.radius = 2.0f;
  testSphere.color = {0.0f, 255.0f, 0.0f};
  scene.push_back(testSphere);*/

  // In degrees
  float fov = 45.0f;
  float aspectRatio = (float)WIDTH / (float)HEIGHT;

  // trace rays
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      // create the ray based in eye position, x and y
      Ray ray;
      ray.origin = eyePosition;
      float pX = (2 * ((x + 0.5) / WIDTH) - 1) * tan(fov / 2 * M_PI / 180) * aspectRatio;
      float pY = (1 - 2 * ((y + 0.5) / HEIGHT)) * tan(fov / 2 * M_PI / 180);
      // As the origin is (0, 0, 0), I don't need to do the subtraction
      ray.direction = {pX, pY, -1.0f};
      ray.direction = glm::normalize(ray.direction);
      //printV3(ray.direction);

      //glm::vec3 hitPoint;
      //glm::vec3 hitNormal;

      float minDistance = FLT_MAX;
      
      Sphere *closerObject = NULL;

      for (int k = 0; k < scene.size(); k++)
      {
        if (intersectSphere(scene[k], ray, &scene[k].hitPoint, &scene[k].hitNormal))
        {
          //cout << "FOO" << endl;
          // Calculate the distance between hitPoint and eye
          // If it is less than the previous one, this is 
          // the closer object, and we have to store it in order
          // to take its color as the color for this pixel
          float dist = glm::distance(eyePosition, scene[k].hitPoint);
          
          if (dist < minDistance)
          {
            closerObject = &scene[k];
            minDistance = dist;
          }
        } 
      }

      // For the moment is going to be always NULL
      if (closerObject == NULL)
      {
        image[x][y] = color;
      }
      else
      {
        Ray shadowRay;
        glm::vec3 hitPointS;
        glm::vec3 hitNormalS;
        shadowRay.direction = glm::normalize(testLight.position - closerObject->hitPoint);
        bool inShadow = false;
        for (int k = 0; k < scene.size(); k++)
        {
          if (intersectSphere(scene[k], shadowRay, &hitPointS, &hitNormalS))
          {
            inShadow = true;
            break;
          } 
        }

        if (!inShadow)
        {
          //image[x][y] = testLight.color * (closerObject->color * max(glm::dot(hitNormal, shadowRay.direction), 0.0f));
          image[x][y] = closerObject->color * max(glm::dot(closerObject->hitNormal, shadowRay.direction), 0.0f) ;
        }
        else
        {
          cout << "IN SHADOW" << endl;
          image[x][y] = black + ambient;
        }
      }
    }
  }

  std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
  ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
  for (unsigned y = 0; y < HEIGHT; y++)
  {
    for (unsigned x = 0; x < WIDTH; x++)
    {
      ofs << (unsigned char)(image[x][y].x) <<
             (unsigned char)(image[x][y].y) <<
             (unsigned char)(image[x][y].z);
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
