#include "Scene.h"

#include <algorithm>
#include <numeric> // For std::iota

#include "Intersection.h"


// bool Scene::intersect(const Ray& ray, Intersection& closestIsect) const {
//     bool hit = false;
//     for (const auto& obj : objects) {
//         Intersection temp;
//         if (obj->intersect(ray, temp)) {
//             if (temp.t < closestIsect.t) {
//                 closestIsect = temp;
//                 hit = true;
//             }
//         }
//     }
//     return hit;
// }

bool Scene::intersect(const Ray& ray, Intersection& closestIsect) const {
    if (rootNodeIdx == -1 || bvhNodes.empty()) { // 如果没有构建 BVH，则回退到原始遍历（或报错）
        // 原始遍历逻辑 (不建议保留，BVH是必须的)
        bool hit = false;
        // for (const auto& obj : objects) {
        //     Intersection temp;
        //     if (obj->intersect(ray, temp)) {
        //         if (temp.t < closestIsect.t) {
        //             closestIsect = temp;
        //             hit = true;
        //         }
        //     }
        // }
        return hit;
        printf("BVH is not built yet. Please call buildBVH() before intersecting.\n");
        return false;
    }

    // 使用 BVH 遍历
    closestIsect.t = std::numeric_limits<float>::max(); // 重置为最大值
    bool hit = false;

    // 创建一个栈来模拟递归遍历（非递归更适合性能）
    std::vector<int> nodeStack;
    nodeStack.push_back(rootNodeIdx); // 从根节点开始 (假设根节点索引是 0)

    while (!nodeStack.empty()) {
        int currentNodeIdx = nodeStack.back();
        nodeStack.pop_back();

        const BVHNode& node = bvhNodes[currentNodeIdx];

        float t_min_aabb = 0.0f, t_max_aabb = std::numeric_limits<float>::max();
        if (!node.bbox.intersect(ray, t_min_aabb, t_max_aabb)) {
            continue; // 光线不与当前节点的包围盒相交，跳过
        }
        
        // 优化: 如果当前已知最近交点比 AABB 的 t_max 还近，也跳过
        if (t_min_aabb >= closestIsect.t) {
            continue;
        }
        if (node.isLeaf()) {
            
            // 遍历叶节点中的所有图元
            for (int i = 0; i < node.numPrimitives; ++i) {
                // const Triangle& tri = flattenedTriangles[node.firstPrimitiveIdx + i];
                // 1. 从 primitiveIndices 获取真正的三角形索引
                int primIdx = primitiveIndices[node.firstPrimitiveIdx + i]; 
                // 2. 使用真正的索引访问 flattenedTriangles
                const Triangle& tri = flattenedTriangles[primIdx];
                Intersection tempIsect = closestIsect; // 传入当前最近交点的信息
                // 注意：这里 Triangle::intersect 应该返回找到的交点，如果它比 tempIsect.t 更近
                if (tri.intersect(ray, tempIsect)) { // Triangle 现在需要存储自己的 Material
                    if (tempIsect.t < closestIsect.t) {
                        closestIsect = tempIsect;
                        hit = true;
                    }
                }
            }
        } else {
            // 内部节点，按距离排序遍历子节点以提高效率
            // 总是先检查更近的子节点
            float t_left_aabb, t_right_aabb;
            bool leftHit = node.leftChildIdx != -1 && bvhNodes[node.leftChildIdx].bbox.intersect(ray, t_left_aabb, t_max_aabb);
            bool rightHit = node.rightChildIdx != -1 && bvhNodes[node.rightChildIdx].bbox.intersect(ray, t_right_aabb, t_max_aabb);

            if (leftHit && rightHit) {
                if (t_left_aabb < t_right_aabb) {
                    nodeStack.push_back(node.rightChildIdx);
                    nodeStack.push_back(node.leftChildIdx);
                } else {
                    nodeStack.push_back(node.leftChildIdx);
                    nodeStack.push_back(node.rightChildIdx);
                }
            } else if (leftHit) {
                nodeStack.push_back(node.leftChildIdx);
            } else if (rightHit) {
                nodeStack.push_back(node.rightChildIdx);
            }
        }
    }
    return hit;
}

bool Scene::hasIntersection(const Ray& ray) const {
    if (rootNodeIdx == -1) {
        return false;
    }

    std::vector<int> nodeStack;
    nodeStack.push_back(rootNodeIdx);

    while (!nodeStack.empty()) {
        int currentNodeIdx = nodeStack.back();
        nodeStack.pop_back();

        const BVHNode& node = bvhNodes[currentNodeIdx];

        // 1. 检查光线是否与节点的包围盒相交
        // 你的包围盒求交函数需要能处理 ray.t_min 和 ray.t_max
        if (!node.bbox.intersect(ray)) { // 假设 AABB::intersect(ray) 会考虑 ray.t_max
            continue;
        }

        // 2. 如果是叶子节点，检查其中的所有图元
        if (node.isLeaf()) {
            for (int i = 0; i < node.numPrimitives; ++i) {
                int primIdx = this->primitiveIndices[node.firstPrimitiveIdx + i];
                const Triangle& tri = this->flattenedTriangles[primIdx];
                
                // 只需要一个临时变量来接收求交结果，不需要完整的 Intersection
                Intersection tempIsect; 
                if (tri.intersect(ray, tempIsect)) {
                    // 只要找到任何一个在 t_max 范围内的交点，就说明有遮挡
                    // tri.intersect 应该保证返回的 tempIsect.t 在 [ray.t_min, ray.t_max] 之间
                    return true; // 立即返回，这是效率的关键
                }
            }
        } 
        // 3. 如果是内部节点，将子节点加入栈中
        else {
            // 对于遮挡函数，子节点的遍历顺序不影响最终结果，可以不排序以简化代码
            nodeStack.push_back(node.leftChildIdx);
            nodeStack.push_back(node.rightChildIdx);
        }
    }
    // 遍历完所有相关节点都没有找到交点
    return false;
}

void Scene::buildBVH() {
    bvhNodes.clear();
    flattenedTriangles.clear(); // 清空旧数据

    // 1. 扁平化所有 Mesh 的三角形
    for (const auto& objPtr : objects) {
        Mesh& mesh = objPtr->getMesh();
        if (typeid(mesh) == typeid(Mesh)) { // 检查是否是 Mesh
            mesh.firstTriangleIdx = flattenedTriangles.size();
            for (const auto& tri : mesh.triangles) {
                flattenedTriangles.push_back(tri);
            }
            // printf("mesh %s has %zu triangles.\n", mesh.getName().c_str(), mesh.triangles.size());
            mesh.numTriangles = flattenedTriangles.size() - mesh.firstTriangleIdx;
            // 重要：Mesh 内部的 triangles 成员现在可以清空或不再使用
            // 或者，你可以修改 Mesh 结构，让它只包含指向 Scene 中三角形数组的索引范围
        }
        // 如果有其他类型的 Object (如 Sphere)，它们也需要被 BVH 管理
        // 可以将它们也视为“图元”，或者构建一个混合 BVH
    }

    // 创建一个包含所有图元索引的列表
    primitiveIndices.resize(flattenedTriangles.size());
    std::iota(primitiveIndices.begin(), primitiveIndices.end(), 0); // 填充 0, 1, 2, ... num_triangles-1
    // printf("flattenedTriangles has %zu triangles.\n", flattenedTriangles.size());
    // 开始递归构建
    bvhNodes.reserve(flattenedTriangles.size() * 2);
    rootNodeIdx = buildBVHRecursive(0, primitiveIndices.size(), 0);
    printf("BVH built with %zu triangles.\n", flattenedTriangles.size());
}

AABB Scene::getPrimitiveAABB(int primitiveIdx) const {
    const Triangle& tri = flattenedTriangles[primitiveIdx];
    AABB bbox;
    bbox.extend(tri.vertices[0].worldPos);
    bbox.extend(tri.vertices[1].worldPos);
    bbox.extend(tri.vertices[2].worldPos);
    return bbox;
}

int Scene::buildBVHRecursive(int start, int end, int currentDepth) {
    int numPrims = end - start;
    int currentNodeIdx = bvhNodes.size();
    bvhNodes.emplace_back(); // 添加一个新节点

    // BVHNode& node = bvhNodes[currentNodeIdx];

    // 计算当前图元集合的 AABB
    AABB currentBbox;
    for (int i = start; i < end; ++i) {
        currentBbox.extend(getPrimitiveAABB(primitiveIndices[i]));
    }
    bvhNodes[currentNodeIdx].bbox = currentBbox;

    if (numPrims <= MAX_PRIMS_IN_LEAF) { // MAX_PRIMS_IN_LEAF 是一个阈值，例如 4-8
        // 创建叶节点
        bvhNodes[currentNodeIdx].firstPrimitiveIdx = start;
        bvhNodes[currentNodeIdx].numPrimitives = numPrims;
        bvhNodes[currentNodeIdx].leftChildIdx = -1; // 叶节点没有子节点
        bvhNodes[currentNodeIdx].rightChildIdx = -1; // 叶节点没有子节点
    } else {
        // 创建内部节点

        // 找到最长的轴进行分割
        glm::vec3 extent = currentBbox.maxBounds - currentBbox.minBounds;
        int axis = 0;
        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;

        // 按中点分割图元，或者使用 SAH (更优)
        // 简单中点分割:
        float midPoint = currentBbox.minBounds[axis] + extent[axis] / 2.0f;
        // std::partition 返回一个迭代器，指向第一个不满足谓词条件的元素
        // std::distance 计算这个迭代器相对于整个向量开头的偏移量
        auto partition_point = std::partition(primitiveIndices.begin() + start, primitiveIndices.begin() + end,
            [&](int primIdx) {
                return getPrimitiveAABB(primIdx).minBounds[axis] < midPoint;
            });

        int mid = std::distance(primitiveIndices.begin(), partition_point);

        // 处理分割导致一边为空的情况
        if (mid == start || mid == end) {
            // 如果无法有效分割，强制将图元分成两半
            mid = start + numPrims / 2;
        }

        // 递归构建左右子树
        bvhNodes[currentNodeIdx].leftChildIdx = buildBVHRecursive(start, mid, currentDepth + 1);
        bvhNodes[currentNodeIdx].rightChildIdx = buildBVHRecursive(mid, end, currentDepth + 1);
    
        
    }
    return currentNodeIdx;
}

void Scene::setup(){
    // 地面
    Material groundMaterial;
    groundMaterial.baseColor= glm::vec3(0.6f); // 灰色
    groundMaterial.roughness = 0.8f; // 粗糙
    groundMaterial.metallic = 0.05f; // 非金属

    // 顶面（白色）
    Material ceilingMaterial;
    ceilingMaterial.baseColor = glm::vec3(0.6f); // 白色
    ceilingMaterial.roughness = 0.8f; // 粗糙
    ceilingMaterial.metallic = 0.05f; // 非金属

    // 背墙
    Material backWallMaterial;
    backWallMaterial.loadBaseColorMap("Resources\\wall.jpg"); // 加载纹理
    backWallMaterial.loadRoughnessMap("Resources\\wall.jpg"); // 加载粗糙度纹理

    // 左墙（红色）
    Material leftWallMaterial;
    leftWallMaterial.baseColor= glm::vec3(1.0f, 0.0f, 0.0f); // 红色
    leftWallMaterial.roughness = 0.8f; // 粗糙
    leftWallMaterial.metallic = 0.05f; // 非金属

    // 右墙（绿色）
    Material rightWallMaterial;
    rightWallMaterial.baseColor = glm::vec3(0.0f, 1.0f, 0.0f); // 绿色
    rightWallMaterial.roughness = 0.8f; // 粗糙
    rightWallMaterial.metallic = 0.05f; // 非金属

    // 球体
    Material sphereMaterial;
    sphereMaterial.baseColor = glm::vec3(0.8f, 0.7f, 0.75f); // 蓝色
    sphereMaterial.roughness = 0.1f; // 光滑
    sphereMaterial.metallic = 0.2f; // 非金属

    // // 猴子
    // Material monkeyMaterial;
    // monkeyMaterial.baseColor = glm::vec3(0.8f, 0.5f, 0.2f); // 棕色
    // monkeyMaterial.roughness = 0.5f; // 中等粗糙
    // monkeyMaterial.metallic = 0.05f; // 非金属
    // monkeyMaterial.loadBaseColorMap("Resources\\liquid.jpg");

    Material complexMaterial;
    sphereMaterial.baseColor = glm::vec3(0.8f, 0.7f, 0.75f); // 蓝色
    sphereMaterial.roughness = 0.1f; // 光滑
    sphereMaterial.metallic = 0.2f; // 非金属
    complexMaterial.loadRoughnessMap("Resources\\complex_roughness.jpg"); // 复杂粗糙度纹理

    // 镜子
    Material mirrorMaterial;
    mirrorMaterial.baseColor = glm::vec3(1.0f); // 镜子颜色
    mirrorMaterial.roughness = 0.0f; // 完全光滑
    mirrorMaterial.metallic = 1.0f; // 金属材质
    mirrorMaterial.loadRoughnessMap("Resources\\complex_roughness.jpg"); // 镜子粗糙度纹理

    std::shared_ptr<Plane> ground = std::make_shared<Plane>(
        glm::vec3(0.0f, 0.0f, 0.0f), // 地面位置
        glm::vec3(0.0f, 1.0f, 0.0f), // 法线方向
        groundMaterial
    );
    ground->setScale(glm::vec3(1.0f));
    addObject(ground);

    std::shared_ptr<Plane> leftWall = std::make_shared<Plane>(
        glm::vec3(-1.0f, 1.0f, 0.0f), // 左墙位置
        glm::vec3(1.0f, 0.0f, 0.0f), // 法线方向
        leftWallMaterial
    );
    leftWall->setScale(glm::vec3(1.f));
    addObject(leftWall);

    std::shared_ptr<Plane> rightWall = std::make_shared<Plane>(
        glm::vec3(1.0f, 1.0f, 0.0f), // 右墙位置
        glm::vec3(-1.0f, 0.0f, 0.0f), // 法线方向
        rightWallMaterial
    );
    addObject(rightWall);

    std::shared_ptr<Plane> ceiling = std::make_shared<Plane>(
        glm::vec3(0.0f, 2.0f, 0.0f), // 顶面位置
        glm::vec3(0.0f, -1.0f, 0.0f), // 法线方向
        ceilingMaterial
    );
    addObject(ceiling);

    std::shared_ptr<Plane> backWall = std::make_shared<Plane>(
        glm::vec3(0.0f, 1.0f, -1.0f), // 后面位置
        glm::vec3(0.0f, 0.0f, 1.0f), // 法线方向
        backWallMaterial
    );
    addObject(backWall);

    std::shared_ptr<Plane> mirror = std::make_shared<Plane>(
        glm::vec3(-0.95f, 0.6f, 0.0f), // 镜子位置
        glm::vec3(1.0f, 0.0f, 0.0f), // 法线方向
        mirrorMaterial // 使用默认材质
    );
    mirror->setScale(glm::vec3(0.6f)); // 设置缩放
    addObject(mirror);

    std::shared_ptr<Sphere> sphereMirror = std::make_shared<Sphere>(
        glm::vec3(-0.5f, 0.4f, -0.2f), // 球体位置
        0.4f, // 半径
        mirrorMaterial
    );
    addObject(sphereMirror);

    std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(
        glm::vec3(0.5f, 0.4f, -0.2f), // 球体位置
        0.4f, // 半径
        complexMaterial
    );
    addObject(sphere);

    // std::shared_ptr<GenericObject> monkey = std::make_shared<GenericObject>(
    //     Mesh("Resources\\monkey.obj"), // 使用预先加载的猴子模型
    //     glm::vec3(-0.4f, 0.4f, 0.3f), // 位置
    //     glm::vec3(0.0f, 0.5f, 0.0f), // 旋转
    //     glm::vec3(0.4f), // 缩放
    //     std::make_shared<Material>(monkeyMaterial) // 使用之前定义的材质
    // );
    // addObject(monkey);

    // 灯光（顶光）
    std::shared_ptr<PointLight> topLight = std::make_shared<PointLight>(
        glm::vec3(1.0f, 1.0f, 1.0f), // 灯光颜色
        6.0f, // 强度
        glm::vec3(0.0f, 1.8f, 0.0f), // 灯光位置
        5.0f // 距离衰减
    );
    // std::shared_ptr<AreaLight> topLight = std::make_shared<AreaLight>(
    //     glm::vec3(1.0f, 1.0f, 1.0f), // 灯光颜色
    //     25.0f, // 强度
    //     glm::vec3(0.0f, 1.9f, 0.0f), // 灯光位置
    //     glm::vec3(1.0f, 0.0f, 0.0f), // u_dir
    //     glm::vec3(0.0f, 0.0f, 1.0f), // v_dir
    //     2.f, // width
    //     2.f // height
    // );
    addLight(topLight);

    // 设置相机
    camera.setPerspective(true);
    camera.setFovY(53.f);
    camera.setPosition(glm::vec3(0.0f, 1.f, 3.0f));
    camera.setTarget(glm::vec3(0.0f, 1.0f, 0.0f));
}

Scene::Scene (){
    camera = Camera();
    setup();
    buildBVH();
}