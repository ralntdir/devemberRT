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
  //printV3(result);
  return result;
}

v3 addition(v3 vector1, v3 vector2)
{
  v3 result;
  result = {vector1.x + vector2.x,
            vector1.y + vector2.y,
            vector1.z + vector2.z};
  //printV3(result);
  return result;
}

v3 vectorByScalar(v3 vector, float scalar)
{
  v3 result;
  result ={vector.x * scalar,
           vector.y * scalar,
           vector.z * scalar};
  //printV3(result);
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

    cout << "t1: " << t1 << ", t2: " << t2 << endl;

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

int main(int argc, char* argv[])
{
  Ray rayo = {};
  rayo.origin = {0.0f, 0.0f, 0.0f};
  rayo.direction = {0.0f, 0.0f, 1.0f};

  Sphere esfera = {};
  esfera.center = {0.0f, 0.0f, 4.0f};
  esfera.radius = 2.0f;

  v3 hitPoint = {};

  //printV3(hitPoint);

  if (intersect(esfera, rayo, &hitPoint))
  {
    printV3(hitPoint);
  }
  else
  {
    cout << "There is no intersection point" << endl;
  }

  return (0);
}
