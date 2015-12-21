// TODO(ralntdir): add multiple light support

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
// In degrees
#define FOV 80.0f
#define ASPECT_RATIO (float)WIDTH / (float)HEIGHT


enum meshType
{
  sphere,
  plane
};

typedef enum meshType meshType;

struct Ray
{
  glm::vec3 origin;
  glm::vec3 direction;
};

struct Mesh
{
  glm::vec3 color;
  glm::vec3 hitPoint;
  glm::vec3 hitNormal;
  meshType type;
  union
  {
    glm::vec3 center;
    glm::vec3 p0;
  };
  union
  {
    glm::vec3 normal;
    float radius;
  };
  float dist;
  bool reflective;
};

// TODO(ralntdir): Check if this type of light is correct
struct Light
{
  glm::vec3 position;
  glm::vec3 color;
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

glm::vec3 color = {200.0, 0.0, 0.0};
glm::vec3 black = {0.0, 0.0, 0.0};
glm::vec3 white = {255.0f, 255.0f, 255.0f};

vector<Mesh> scene;
glm::vec3 eyePosition = {0.0f, 0.0f, 0.0f};
glm::vec3 ambient = {20.5f, 20.5f, 20.5f};

Light testLight = {};

void printV3(glm::vec3 vector)
{
  cout << "Vec3: " << vector.x << ", " << vector.y << ", " << vector.z << endl;
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

bool intersectSphere(glm::vec3 center, float radius, Ray ray, glm::vec3 *hitPoint, glm::vec3 *hitNormal)
{
  // first version for an intersect method
  // we just need the hit point for the intersection
  bool success;
  glm::vec3 originCenter = ray.origin - center;
  float a = glm::dot(ray.direction, ray.direction);
  //cout << a << endl;
  float b = 2 * glm::dot(ray.direction, originCenter); 
  //cout << b << endl;
  float c = glm::dot(originCenter, originCenter) - 
            (radius * radius);

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

      //*hitPoint = ray.origin + ray.direction * t;
      *hitPoint = ray.origin + ray.direction * (t - 0.001f);
      *hitNormal = glm::normalize(*hitPoint - center);
    }
  }

  return success;
}

bool intersectPlane(glm::vec3 p0, glm::vec3 normal, float dist, Ray ray, glm::vec3 *hitPoint, glm::vec3 *hitNormal)
{
  // t = (p_0 - rayOrigin).normal / rayDirection.n
  bool success = false;
  float denom = glm::dot(ray.direction, normal);
  //cout << denom << endl;
  if (abs(denom) > 1e-6)
  {
    glm::vec3 p0rayOrigin = p0 - ray.origin;
    //printV3(p0rayOrigin);
    float t = glm::dot(p0rayOrigin, normal) / denom;
    //cout << t << endl;
    *hitPoint = ray.origin + ray.direction * (t - 0.001f);
    // normal for a point in the plane is the normal of the plane...? I think so.
    *hitNormal = normal;
    if (t >= 0)
    {
      if ((abs(p0.x - hitPoint->x) < dist) &&
          (abs(p0.y - hitPoint->y) < dist))
      {
        success = true;
      }
      else
      {
        success = false;
      }
    }
    else
    {
      success = false;
    }

  }
  return success;
}

bool intersect(Mesh *mesh, Ray ray)
{
  if (mesh->type == sphere)
  {
    return intersectSphere(mesh->center, mesh->radius, ray, &mesh->hitPoint, &mesh->hitNormal);
  }
  else if (mesh->type == plane)
  {
    return intersectPlane(mesh->p0, mesh->normal, mesh->dist, ray, &mesh->hitPoint, &mesh->hitNormal);
  }
}

glm::vec3 castRay(Ray ray, int depth)
{
  glm::vec3 resultColor = {};
  if (depth < 5)
  {
    float minDistance= FLT_MAX;

    Mesh *closerObject= NULL;

    for (int k = 0; k < scene.size(); k++)
    {
      if (intersect(&scene[k], ray))
      {
        // Calculate the distance between hitPoint and eye
        // If it is less than the previous one, this is
        // the closer object, and we have to store it in order
        // to take its color as the color for this pixel
        float dist = glm::distance(eyePosition, scene[k].hitPoint);

        if (dist < minDistance)
        {
          closerObject= &scene[k];
          minDistance= dist;
        }
      }
    }
    // For the moment is going to be always NULL
    if (closerObject == NULL)
    {
      resultColor = color;
    }
    else
    {
      Ray shadowRay;
      glm::vec3 hitPointS;
      glm::vec3 hitNormalS;
      shadowRay.direction = glm::normalize(testLight.position - closerObject->hitPoint);
      shadowRay.origin = closerObject->hitPoint;
      bool inShadow = false;
      for (int k = 0; k < scene.size(); k++)
      {
        if (intersect(&scene[k], shadowRay))
        {
          inShadow = false;
          break;
        }
      }
      if (!inShadow)
      {
        resultColor = glm::clamp(closerObject->color * max(glm::dot(closerObject->hitNormal, shadowRay.direction), 0.0f) + ambient, 0.0f, 255.0f);
        //resultColor = closerObject->color * max(glm::dot(closerObject->hitNormal, shadowRay.direction), 0.0f);
      }
      else
      {
        resultColor = glm::clamp(black + ambient, 0.0f, 255.0f);
      }
      if (depth > 1) printV3(resultColor);
    }

    if (closerObject != NULL && closerObject->reflective)
    {
      cout << "Reflective" << endl;
      Ray reflectedRay;
      reflectedRay.origin = closerObject->hitPoint;
      reflectedRay.direction = glm::normalize(glm::reflect(ray.direction, closerObject->hitNormal));
      //printV3(closerObject->hitNormal);
      //printV3(ray.direction);
      //printV3(reflectedRay.direction);
      //exit(1);
      glm::vec3 reflectedColor = castRay(reflectedRay, depth+1);
      resultColor = glm::clamp(resultColor + 0.8f * reflectedColor, 0.0f, 255.0f);
    }
  }
  return resultColor;
}

void render()
{
  // Creates an image using the ray tracing method.
  // The first prototype will have only spheres and flat shading.
  // Then I will add diffuse shading (lights) and after that
  // shadows.

  // TODO(ralntdir): I need to define the camera and the plane
  // I'm projecting the scene on!!!
  // NOTE(ralntdir): For the moment, the camera is at (0, 0, 0) and pointing 
  // to -Z and the plane is an XY plane at Z=-1;

  // NOTE(ralntdir): I'm working with a right handed one coordinate
  // system
  
  /*vector<Mesh> scene;
  glm::vec3 eyePosition = {0.0f, 0.0f, 0.0f};
  glm::vec3 ambient = {20.5f, 20.5f, 20.5f};

  Light testLight = {};*/
  //testLight.position = {0.0f, 10.0f, -10.0f};
  testLight.position = {0.0f, -2.0f, -5.0f};
  testLight.color = {255.0f, 255.0f, 255.0f};


  glm::vec3 image[WIDTH][HEIGHT] = {};

  // prepare the scene
  Mesh testMesh = {};
  //testMesh.center = {0.0f, -2.0f, -10.0f};
  testMesh.center = {0.0f, 3.0f, -15.0f};
  testMesh.radius = 3.0f;
  testMesh.color = {0.0f, 255.0f, 255.0f};
  testMesh.type = sphere;
  testMesh.reflective = false;
  scene.push_back(testMesh);
  /*testMesh.center = {1.0f, 3.5f, -10.0f};
  testMesh.radius = 2.0f;
  testMesh.color = {0.0f, 255.0f, 0.0f};
  testMesh.type = sphere;
  testMesh.reflective = false;
  scene.push_back(testMesh);*/
  /*testMesh.p0 = {0.0f, -00.0f, -15.0f};
  testMesh.normal = {0.0f, -0.0f, 1.0f};
  testMesh.color = {255.0f, 255.0f, 0.0f};
  testMesh.dist = 10.0f;
  testMesh.type = plane;
  testMesh.reflective = true;
  scene.push_back(testMesh);*/
  testMesh.p0 = {0.0f, -1.0f, -20.0f};
  testMesh.normal = {0.0f, 1.0f, 0.0f};
  testMesh.color = {0.0f, 0.0f, 0.0f};
  testMesh.dist = 10.0f;
  testMesh.type = plane;
  testMesh.reflective = true;
  scene.push_back(testMesh);
  /*testMesh.center = {0.0f, 5.0f, -7.0f};
  testMesh.radius = 2.0f;
  testMesh.color = {0.0f, 255.0f, 0.0f};
  sceneSpheres.push_back(testMesh);*/

  // trace rays
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      // create the ray based in eye position, x and y
      Ray ray;
      ray.origin = eyePosition;
      float pX = (2 * ((x + 0.5) / WIDTH) - 1) * tan(FOV / 2 * M_PI / 180) * ASPECT_RATIO;
      float pY = (1 - 2 * ((y + 0.5) / HEIGHT)) * tan(FOV / 2 * M_PI / 180);
      // As the origin is (0, 0, 0), I don't need to do the subtraction
      ray.direction = {pX, pY, -1.0f};
      ray.direction = glm::normalize(ray.direction);
      //printV3(ray.direction);

      image[x][y] = castRay(ray, 1);
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
