
struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR; 
};


cbuffer GlobalConstants : register(b0)
{
    float4x4 ViewProjectionMatrix;
    float3 CameraPos;
};

cbuffer MeshConstants : register(b1)
{
    float4x4 LocalToWorld; 
}

PSInput VSMain(float4 position: POSITION, float4 color : COLOR)
{
    PSInput result;
    
    position = float4(position.xyz, 1.0); 
    
    float3 worldPos = mul(LocalToWorld, position).xyz;
    
    result.position = mul(ViewProjectionMatrix, float4(worldPos, 1.0));
    result.position = result.position;
    result.color = color;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

float4 main() : SV_TARGET
{
    return 0.0.xxxx; //stub
}