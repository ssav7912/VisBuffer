
struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR; 
};

PSInput VSMain(float4 position: POSITION, float4 color : COLOR)
{
    PSInput result;
    
    result.position = position;
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