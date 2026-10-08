#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
    float4 position [[position]];
    float3 color;
};

vertex VertexOut vertexMain(uint vertexId [[vertex_id]])
{
    const float2 positions[3] = { float2(0.0, 0.6), float2(-0.6, -0.6), float2(0.6, -0.6) };
    const float3 colors[3] = { float3(1.0, 0.3, 0.3), float3(0.3, 1.0, 0.3), float3(0.3, 0.3, 1.0) };

    VertexOut out;
    out.position = float4(positions[vertexId], 0.0, 1.0);
    out.color = colors[vertexId];
    return out;
}

fragment float4 fragmentMain(VertexOut in [[stage_in]])
{
    return float4(in.color, 1.0);
}

kernel void doubleValues(device float* values [[buffer(0)]],
                         uint index [[thread_position_in_grid]])
{
    values[index] *= 2.0;
}
