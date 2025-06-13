#include "Scene.h"

#include "Intersection.h"

bool Scene::intersect(const Ray& ray, Intersection& closestIsect) const {
    bool hit = false;
    for (const auto& obj : objects) {
        Intersection temp;
        if (obj->intersect(ray, temp)) {
            if (temp.t < closestIsect.t) {
                closestIsect = temp;
                hit = true;
            }
        }
    }
    return hit;
}