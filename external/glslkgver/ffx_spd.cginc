// [FFX SPD] Single Pass Downsampler 2.0

//GroupMemoryBarrierWithGroupSync();
#define SpdWorkgroupShuffleBarrier() barrier()

spdType SpdReduceLoadSourceImage4H(ivec2 i0, ivec2 i1, ivec2 i2, ivec2 i3, int slice)
{
    spdType v0 = SpdLoadSourceImageH(i0, slice);
    spdType v1 = SpdLoadSourceImageH(i1, slice);
    spdType v2 = SpdLoadSourceImageH(i2, slice);
    spdType v3 = SpdLoadSourceImageH(i3, slice);
    return SpdReduce4H(v0, v1, v2, v3);
}

#define SpdReduceLoadSourceImageH(base, slice) SpdReduceLoadSourceImage4H(base + ivec2(0, 0), base + ivec2(0, 1), base + ivec2(1, 0), base + ivec2(1, 1), slice)

spdType SpdReduceIntermediateH(ivec2 i0, ivec2 i1, ivec2 i2, ivec2 i3)
{
    spdType v0 = SpdLoadIntermediateH(i0.x, i0.y);
    spdType v1 = SpdLoadIntermediateH(i1.x, i1.y);
    spdType v2 = SpdLoadIntermediateH(i2.x, i2.y);
    spdType v3 = SpdLoadIntermediateH(i3.x, i3.y);
    return SpdReduce4H(v0, v1, v2, v3);
}

void SpdDownsampleMips_0_1_LDSH(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mips, int slice) 
{
    spdType v[4];

    ivec2 tex = workGroupID * 64 + ivec2(x * 2, y * 2);
    ivec2 pix = workGroupID * 32 + ivec2(x, y);
    v[0] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[0], 0, slice);

    tex = workGroupID * 64 + ivec2(x * 2 + 32, y * 2);
    pix = workGroupID * 32 + ivec2(x + 16, y);
    v[1] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[1], 0, slice);

    tex = workGroupID * 64 + ivec2(x * 2, y * 2 + 32);
    pix = workGroupID * 32 + ivec2(x, y + 16);
    v[2] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[2], 0, slice);

    tex = workGroupID * 64 + ivec2(x * 2 + 32, y * 2 + 32);
    pix = workGroupID * 32 + ivec2(x + 16, y + 16);
    v[3] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[3], 0, slice);

    //if (mips <= 1)
        //return;

    for (int i = 0; i < 4; i++)
    {
        SpdStoreIntermediateH(x, y, v[i]);
        SpdWorkgroupShuffleBarrier();
        if (localInvocationIndex < 64)
        {
            v[i] = SpdReduceIntermediateH(ivec2(x * 2, y * 2 + 0), ivec2(x * 2 + 1, y * 2),
                                          ivec2(x * 2, y * 2 + 1), ivec2(x * 2 + 1, y * 2 + 1));
            SpdStoreH(workGroupID * 16 + ASU2(x + (i % 2) * 8, y + (i / 2) * 8), v[i], 1, slice);
        }
        SpdWorkgroupShuffleBarrier();
    }

    if (localInvocationIndex < 64)
    {
        SpdStoreIntermediateH(x + 0, y + 0, v[0]);
        SpdStoreIntermediateH(x + 8, y + 0, v[1]);
        SpdStoreIntermediateH(x + 0, y + 8, v[2]);
        SpdStoreIntermediateH(x + 8, y + 8, v[3]);
    }
}

#ifdef A_WAVE 

spdType SpdReduceQuadH(spdType_t v)
{    
    spdType v0 = v;
#if defined(A_HALF)
    #if SHADER_FEATURES & SHADER_SUBGROUPF16
        spdType v1 = subgroupQuadSwapHorizontal(v);
        spdType v2 = subgroupQuadSwapVertical(v);
        spdType v3 = subgroupQuadSwapDiagonal(v);
    #else
        vec4 v4 = vec4(v);
        spdType v1 = half4_t(subgroupQuadSwapHorizontal(v4));
        spdType v2 = half4_t(subgroupQuadSwapVertical(v4));
        spdType v3 = half4_t(subgroupQuadSwapDiagonal(v4));
    #endif   
#else
    spdType v1 = subgroupQuadSwapHorizontal(v);
    spdType v2 = subgroupQuadSwapVertical(v);
    spdType v3 = subgroupQuadSwapDiagonal(v);         
#endif    
    return SpdReduce4H(v0, v1, v2, v3);
}

void SpdDownsampleMips_0_1_IntrinsicsH(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mips, int slice)
{
    spdType v[4];

    ivec2 tex = workGroupID.xy * 64 + ivec2(x * 2, y * 2);
    ivec2 pix = workGroupID.xy * 32 + ivec2(x, y);
    v[0] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[0], 0, slice);

    tex = workGroupID.xy * 64 + ivec2(x * 2 + 32, y * 2);
    pix = workGroupID.xy * 32 + ivec2(x + 16, y);
    v[1] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[1], 0, slice);

    tex = workGroupID.xy * 64 + ivec2(x * 2, y * 2 + 32);
    pix = workGroupID.xy * 32 + ivec2(x, y + 16);
    v[2] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[2], 0, slice);

    tex = workGroupID.xy * 64 + ivec2(x * 2 + 32, y * 2 + 32);
    pix = workGroupID.xy * 32 + ivec2(x + 16, y + 16);
    v[3] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[3], 0, slice);

    //if (mips <= 1)
    //    return;

    v[0] = SpdReduceQuadH(v[0]);
    v[1] = SpdReduceQuadH(v[1]);
    v[2] = SpdReduceQuadH(v[2]);
    v[3] = SpdReduceQuadH(v[3]);

    if ((localInvocationIndex % 4) == 0)
    {
        SpdStoreH(workGroupID.xy * 16 + ivec2(x/2, y/2), v[0], 1, slice);
        SpdStoreIntermediateH(x/2, y/2, v[0]);

        SpdStoreH(workGroupID.xy * 16 + ivec2(x/2 + 8, y/2), v[1], 1, slice);
        SpdStoreIntermediateH(x/2 + 8, y/2, v[1]);

        SpdStoreH(workGroupID.xy * 16 + ivec2(x/2, y/2 + 8), v[2], 1, slice);
        SpdStoreIntermediateH(x/2, y/2 + 8, v[2]);

        SpdStoreH(workGroupID.xy * 16 + ivec2(x/2 + 8, y/2 + 8), v[3], 1, slice);
        SpdStoreIntermediateH(x/2 + 8, y/2 + 8, v[3]);
    }
}
#endif

void SpdDownsampleMips_0_1H(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mips, int slice) 
{
#ifdef SPD_NO_WAVE_OPERATIONS    
    SpdDownsampleMips_0_1_LDSH(x, y, workGroupID, localInvocationIndex, mips, slice);  
#else
    SpdDownsampleMips_0_1_IntrinsicsH(x, y, workGroupID, localInvocationIndex, mips, slice);
#endif    
}

void SpdDownsampleMip_2H(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mip, int slice)
{
#ifdef SPD_NO_WAVE_OPERATIONS    
    if (localInvocationIndex < 64)
    {
        spdType v = SpdReduceIntermediateH(ivec2(x * 2, y * 2 + 0), ivec2(x * 2 + 1, y * 2 + 0),
                                         ivec2(x * 2, y * 2 + 1), ivec2(x * 2 + 1, y * 2 + 1));
        SpdStoreH(ASU2(workGroupID.xy * 8) + ASU2(x, y), v, mip, slice);
        // store to LDS, try to reduce bank conflicts
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0 x
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // ...
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        SpdStoreIntermediateH(x * 2 + y % 2, y * 2, v);
    }
#else
    spdType v = SpdLoadIntermediateH(x, y);
    v = SpdReduceQuadH(v);
    // quad index 0 stores result
    if (localInvocationIndex % 4 == 0)
    {   
        SpdStoreH(ASU2(workGroupID.xy * 8) + ASU2(x/2, y/2), v, mip, slice);
        SpdStoreIntermediateH(x + (y/2) % 2, y, v);
    }
#endif    
}

void SpdDownsampleMip_3H(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mip, int slice)
{
#ifdef SPD_NO_WAVE_OPERATIONS    
    if (localInvocationIndex < 16)
    {
        // x 0 x 0
        // 0 0 0 0
        // 0 x 0 x
        // 0 0 0 0
        spdType v = SpdReduceIntermediateH(ivec2(x * 4 + 0, y * 4 + 0), ivec2(x * 4 + 2 + 0, y * 4 + 0),
                                         ivec2(x * 4 + 1, y * 4 + 2), ivec2(x * 4 + 2 + 1, y * 4 + 2));
        SpdStoreH(ASU2(workGroupID.xy * 4) + ASU2(x, y), v, mip, slice);
        // store to LDS
        // x 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0 0
        // ...
        // 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0
        // ...
        // 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x
        // ...
        SpdStoreIntermediateH(x * 4 + y, y * 4, v);
    }
#else
    if (localInvocationIndex < 64)
    {
        spdType v = SpdLoadIntermediateH(x * 2 + y % 2,y * 2);
        v = SpdReduceQuadH(v);
        // quad index 0 stores result
        if (localInvocationIndex % 4 == 0)
        {   
            SpdStoreH(ASU2(workGroupID.xy * 4) + ASU2(x/2, y/2), v, mip, slice);
            SpdStoreIntermediateH(x * 2 + y/2, y * 2, v);
        }
    }
#endif    
}

void SpdDownsampleMip_4H(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mip, int slice)
{
#ifdef SPD_NO_WAVE_OPERATIONS    
    if (localInvocationIndex < 4)
    {
        // x 0 0 0 x 0 0 0
        // ...
        // 0 x 0 0 0 x 0 0
        spdType v = SpdReduceIntermediateH(ivec2(x * 8 + 0 + y * 2, y * 8 + 0), ivec2(x * 8 + 4 + 0 + y * 2, y * 8 + 0),
                                         ivec2(x * 8 + 1 + y * 2, y * 8 + 4), ivec2(x * 8 + 4 + 1 + y * 2, y * 8 + 4));
        SpdStoreH(ASU2(workGroupID.xy * 2) + ASU2(x, y), v, mip, slice);
        // store to LDS
        // x x x x 0 ...
        SpdStoreIntermediateH(x + y * 2, 0, v);
    }
#else
    if (localInvocationIndex < 16)
    {
        spdType v = SpdLoadIntermediateH(x * 4 + y,y * 4);
        v = SpdReduceQuadH(v);
        // quad index 0 stores result
        if (localInvocationIndex % 4 == 0)
        {   
            SpdStoreH(ASU2(workGroupID.xy * 2) + ASU2(x/2, y/2), v, mip, slice);
            SpdStoreIntermediateH(x / 2 + y, 0, v);
        }
    }
#endif    
}

void SpdDownsampleMip_5H(ivec2 workGroupID, AU1 localInvocationIndex, AU1 mip, int slice)
{
#ifdef SPD_NO_WAVE_OPERATIONS    
    if (localInvocationIndex < 1)
    {
        // x x x x 0 ...        
        spdType v = SpdReduceIntermediateH(ivec2(0, 0), ivec2(1, 0), ivec2(2, 0),ivec2(3, 0));
        //SpdStoreMip5H(ASU2(workGroupID.xy), v, slice);
        SpdStoreH(ASU2(workGroupID.xy), v, mip, slice);
    }
#else
    if (localInvocationIndex < 4)
    {
        spdType v = SpdLoadIntermediateH(localInvocationIndex,0);
        v = SpdReduceQuadH(v);
        // quad index 0 stores result
        if (localInvocationIndex % 4 == 0)
        {   
            SpdStoreH(ASU2(workGroupID.xy), v, mip, slice);
        }
    }
#endif    
}

void SpdDownsampleNextFourH(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 mips, int slice)
{     
    if (mips > 2)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_2H(x, y, workGroupID, localInvocationIndex, 2, slice);
    }

    if (mips > 3)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_3H(x, y, workGroupID, localInvocationIndex, 3, slice);
    }

    if (mips > 4)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_4H(x, y, workGroupID, localInvocationIndex, 4, slice);
    }

    if (mips > 5)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_5H(workGroupID, localInvocationIndex, 5, slice);
    }
}

#ifndef MAX_MIP_6
void SpdDownsampleNextFourH(int x, int y, ivec2 workGroupID, AU1 localInvocationIndex, AU1 baseMip, AU1 mips, int slice)
{
    if (mips > baseMip)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_2H(x, y, workGroupID, localInvocationIndex, baseMip, slice);
    }

    if (mips > baseMip + 1)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_3H(x, y, workGroupID, localInvocationIndex, baseMip + 1, slice);
    }

    if (mips > baseMip + 2)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_4H(x, y, workGroupID, localInvocationIndex, baseMip + 2, slice);
    }

    if (mips > baseMip + 3)
    {
        SpdWorkgroupShuffleBarrier();
        SpdDownsampleMip_5H(workGroupID, localInvocationIndex, baseMip + 3, slice);
    }  
}

spdType SpdReduceLoad4H(ivec2 base, int slice)
{
    ivec2 i0 = base + ivec2(0, 0);
    ivec2 i1 = base + ivec2(0, 1);
    ivec2 i2 = base + ivec2(1, 0);
    ivec2 i3 = base + ivec2(1, 1);

    spdType v0 = SpdLoadH(i0, slice);
    spdType v1 = SpdLoadH(i1, slice);
    spdType v2 = SpdLoadH(i2, slice);
    spdType v3 = SpdLoadH(i3, slice);
    return SpdReduce4H(v0, v1, v2, v3);
}

void SpdDownsampleMips_6_7H(int x, int y, AU1 mips, int slice)
{
    ivec2 tex = ivec2(x * 4 + 0, y * 4 + 0);
    ivec2 pix = ivec2(x * 2 + 0, y * 2 + 0);
    spdType v0 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v0, 6, slice);

    tex = ivec2(x * 4 + 2, y * 4 + 0);
    pix = ivec2(x * 2 + 1, y * 2 + 0);
    spdType v1 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v1, 6, slice);

    tex = ivec2(x * 4 + 0, y * 4 + 2);
    pix = ivec2(x * 2 + 0, y * 2 + 1);
    spdType v2 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v2, 6, slice);

    tex = ivec2(x * 4 + 2, y * 4 + 2);
    pix = ivec2(x * 2 + 1, y * 2 + 1);
    spdType v3 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v3, 6, slice);

    if (mips >= 8)
    {
        // no barrier needed, working on values only from the same thread
        spdType v = SpdReduce4H(v0, v1, v2, v3);
        SpdStoreH(ivec2(x, y), v, 7, slice);
        SpdStoreIntermediateH(x, y, v);
    }
}

// Only last active workgroup should proceed
bool SpdExitWorkgroup(AU1 numWorkGroups, AU1 localInvocationIndex, int slice) 
{
    // global atomic counter
    if (localInvocationIndex == 0)
    {
        SpdIncreaseAtomicCounter(slice);
    }
    SpdWorkgroupShuffleBarrier();
    return (SpdGetAtomicCounter() != (numWorkGroups - 1));
}
#endif


void SpdDownsampleH(ivec2 workGroupID, AU1 localInvocationIndex, AU1 mips, AU1 numWorkGroups, int slice)
 {
    AU2 sub_xy = ARmpRed8x8(localInvocationIndex % 64);
    int x = int(sub_xy.x + 8 * ((localInvocationIndex >> 6) % 2));
    int y = int(sub_xy.y + 8 * ((localInvocationIndex >> 7)));

    SpdDownsampleMips_0_1H(x, y, workGroupID, localInvocationIndex, mips, slice);
    SpdDownsampleNextFourH(x, y, workGroupID, localInvocationIndex, mips, slice);

#ifndef MAX_MIP_6
    if (mips >= 7) 
    {
        if (!SpdExitWorkgroup(numWorkGroups, localInvocationIndex, slice)) 
        {
            SpdResetAtomicCounter(slice);    
            // After mip 6 there is only a single workgroup left that downsamples the remaining up to 64x64 texels.
            SpdDownsampleMips_6_7H(x, y, mips, slice);
            SpdDownsampleNextFourH(x, y, ivec2(0,0), localInvocationIndex, 8, mips, slice);
        }
    }
#endif    
}

