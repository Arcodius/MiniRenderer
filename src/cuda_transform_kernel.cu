// // File: cuda_transform_kernel.cu

// #include <cuda_runtime.h>
// #include <device_launch_parameters.h>

// struct float3 { float x, y, z; };
// struct float2 { float x, y; };
// struct float4 { float x, y, z, w; };

// __device__ float4 make_float4(float3 v, float w) {
//     return { v.x, v.y, v.z, w };
// }

// __device__ float dot(float4 a, float4 b) {
//     return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
// }

// // Column-major 4x4 matrix
// struct float4x4 {
//     float m[4][4]; // m[column][row]
// };

// __device__ float4 mul(const float4x4& mat, const float4& vec) {
//     float4 res;
//     res.x = mat.m[0][0] * vec.x + mat.m[1][0] * vec.y + mat.m[2][0] * vec.z + mat.m[3][0] * vec.w;
//     res.y = mat.m[0][1] * vec.x + mat.m[1][1] * vec.y + mat.m[2][1] * vec.z + mat.m[3][1] * vec.w;
//     res.z = mat.m[0][2] * vec.x + mat.m[1][2] * vec.y + mat.m[2][2] * vec.z + mat.m[3][2] * vec.w;
//     res.w = mat.m[0][3] * vec.x + mat.m[1][3] * vec.y + mat.m[2][3] * vec.z + mat.m[3][3] * vec.w;
//     return res;
// }

// struct Vertex {
//     float3 worldPos;
//     float3 normal;
//     float2 uv;
// };

// struct ClippedVertex {
//     Vertex vertex;
//     float4 clipPos;
// };

// __global__ void transformKernel(
//     const Vertex* vertices,
//     const unsigned int* indices,
//     float4x4 mvp,
//     ClippedVertex* output,
//     int triangleCount
// ) {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;
//     if (idx >= triangleCount) return;

//     int i = idx * 3;
//     unsigned int idx0 = indices[i];
//     unsigned int idx1 = indices[i + 1];
//     unsigned int idx2 = indices[i + 2];

//     Vertex v0 = vertices[idx0];
//     Vertex v1 = vertices[idx1];
//     Vertex v2 = vertices[idx2];

//     float4 pos0 = make_float4(v0.worldPos, 1.0f);
//     float4 pos1 = make_float4(v1.worldPos, 1.0f);
//     float4 pos2 = make_float4(v2.worldPos, 1.0f);

//     output[i + 0] = { v0, mul(mvp, pos0) };
//     output[i + 1] = { v1, mul(mvp, pos1) };
//     output[i + 2] = { v2, mul(mvp, pos2) };
// }
