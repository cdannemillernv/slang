//TEST(compute):COMPARE_COMPUTE:-vk -compute -profile cs_6_2 -render-features half
//Disable on Dx12 for now - because writing to structured buffer produces unexpected results
//TEST(compute):COMPARE_COMPUTE:-dx12 -compute -use-dxil -profile cs_6_2 -render-features half
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):dxbinding(0),glbinding(0),out

struct Thing
{
    uint pos;
    float radius;
    half4 color;
};

RWStructuredBuffer<Thing> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    
    float v = float(tid);
    
    float base = v* 4.0f;
    
    Thing thing;
    thing.pos = tid;
    thing.color = half4(base, base + 1.0f, base + 2.0f, base + 3.0f);    
    thing.radius = v;
    
    outputBuffer[tid] = thing;
}