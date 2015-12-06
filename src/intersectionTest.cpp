#include <iostream>
#include <math.h>

using namespace std;

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
};

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

bool intersect(Sphere sphere, Ray ray, v3 *hitPoint)
{
  bool success;
  float a = dotProduct(ray.direction, ray.direction);
  float b = 2 * dotProduct(ray.direction, 
                           subtraction(ray.origin, sphere.center)); 
  float c = dotProduct(subtraction(ray.origin, sphere.center),
                       subtraction(ray.origin, sphere.center)) - 
            sphere.radius * sphere.radius;

  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0)
  {
    success = false;
  }
  else
  {
    float t1, t2;

    t1 = (b + sqrt(discriminant)) / (2 * a); 
    t2 = (b - sqrt(discriminant)) / (2 * a); 

    if (t1 == t2)
    {
      success = false;
    }
    else if ((t1 > 0) && (t1 < t2))
    {
      success = true;
      //hitPoint = ray.origin + ray.direction * t1;
    }
  }

  return success;
}

int main(int argc, char* argv[])
{
  Ray rayo = {};
  rayo.origin = {0.0f, 0.0f, 0.0f};
  rayo.direction = {0.0f, 0.0f, 1.0f};

  Sphere esfera = {};
  esfera.center = {0.0f, 0.0f, 4.0f};
  esfera.radius = 2.0f;

  v3 hitPoint = {};

  intersect(esfera, rayo, &hitPoint);

  return (0);
}
