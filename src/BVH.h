#pragma once

#include "MyMath.h"
#include "Ray.h"


struct AABB {
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    AABB() : minBounds(glm::vec3(std::numeric_limits<float>::max())),
             maxBounds(glm::vec3(std::numeric_limits<float>::lowest())) {}

    AABB(const glm::vec3& pMin, const glm::vec3& pMax) : minBounds(pMin), maxBounds(pMax) {}

    // 扩展包围盒以包含另一个点
    void extend(const glm::vec3& p) {
        minBounds = glm::min(minBounds, p);
        maxBounds = glm::max(maxBounds, p);
    }

    // 扩展包围盒以包含另一个 AABB
    void extend(const AABB& other) {
        minBounds = glm::min(minBounds, other.minBounds);
        maxBounds = glm::max(maxBounds, other.maxBounds);
    }

    // 判断光线是否与 AABB 相交
    bool intersect(const Ray& ray, float& t_min_out, float& t_max_out) const {
        float t_min = 0.0f;
        float t_max = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) {
            float invDir = 1.0f / ray.direction[i];
            float t0 = (minBounds[i] - ray.origin[i]) * invDir;
            float t1 = (maxBounds[i] - ray.origin[i]) * invDir;

            if (invDir < 0.0f) std::swap(t0, t1);

            t_min = std::max(t_min, t0);
            t_max = std::min(t_max, t1);

            if (t_min > t_max) {
                return false;
            }
        }

        t_min_out = t_min;
        t_max_out = t_max;
        return true;
    }
};

// 定义 BVH 节点结构
struct BVHNode {
    AABB bbox; // 该节点的包围盒
    int leftChildIdx = -1; // 左子节点索引，-1 表示没有
    int rightChildIdx = -1; // 右子节点索引，-1 表示没有
    int firstPrimitiveIdx = -1; // 如果是叶节点，这是第一个图元的索引
    int numPrimitives = 0; // 如果是叶节点，这是包含的图元数量
    // 注意：如果不是叶节点，则 firstPrimitiveIdx 和 numPrimitives 无意义
    // 如果是叶节点，leftChildIdx 和 rightChildIdx 无意义
    
    bool isLeaf() const { return numPrimitives > 0; }
};