/*shadow.cginc*/
//#define PCF3x3

float GetAreaAboveFirstTexelUnderAIsocelesRectangleTriangle(float triangleHeight)
{
    return triangleHeight - 0.5;
}

/**
* Assuming a isoceles triangle of 1.5 texels height and 3 texels wide lying on 4 texels.
* This function return the area of the triangle above each of those texels.
*    |    <-- offset from -0.5 to 0.5, 0 meaning triangle is exactly in the center
*   / \   <-- 45 degree slop isosceles triangle (ie tent projected in 2D)
*  /   \
* _ _ _ _ <-- texels
* X Y Z W <-- result indices (in computedArea.xyzw and computedAreaUncut.xyzw)
*/
void GetAreaPerTexel_3TexelsWideTriangleFilter(float offset, out vec4 computedArea, out vec4 computedAreaUncut)
{
    //Compute the exterior areas
    float offset01SquaredHalved = (offset + 0.5) * (offset + 0.5) * 0.5;
    computedAreaUncut.x = computedArea.x = offset01SquaredHalved - offset;
    computedAreaUncut.w = computedArea.w = offset01SquaredHalved;

    //Compute the middle areas
    //For Y : We find the area in Y of as if the left section of the isoceles triangle would
    //intersect the axis between Y and Z (ie where offset = 0).
    computedAreaUncut.y = GetAreaAboveFirstTexelUnderAIsocelesRectangleTriangle(1.5 - offset);
    //This area is superior to the one we are looking for if (offset < 0) thus we need to
    //subtract the area of the triangle defined by (0,1.5-offset), (0,1.5+offset), (-offset,1.5).
    float clampedOffsetLeft = min(offset, 0.0);
    float areaOfSmallLeftTriangle = clampedOffsetLeft * clampedOffsetLeft;
    computedArea.y = computedAreaUncut.y - areaOfSmallLeftTriangle;

    //We do the same for the Z but with the right part of the isoceles triangle
    computedAreaUncut.z = GetAreaAboveFirstTexelUnderAIsocelesRectangleTriangle(1.5 + offset);
    float clampedOffsetRight = max(offset, 0.0);
    float areaOfSmallRightTriangle = clampedOffsetRight * clampedOffsetRight;
    computedArea.z = computedAreaUncut.z - areaOfSmallRightTriangle;
}

/**
 * Assuming a isoceles triangle of 1.5 texels height and 3 texels wide lying on 4 texels.
 * This function return the weight of each texels area relative to the full triangle area.
 */
void GetWeightPerTexel_3TexelsWideTriangleFilter(float offset, out vec4 computedWeight)
{
    vec4 dummy;
    GetAreaPerTexel_3TexelsWideTriangleFilter(offset, computedWeight, dummy);
    computedWeight *= 0.44444;//0.44 == 1/(the triangle area)
}


/**
* Assuming a isoceles triangle of 2.5 texel height and 5 texels wide lying on 6 texels.
* This function return the weight of each texels area relative to the full triangle area.
*  /       \
* _ _ _ _ _ _ <-- texels
* 0 1 2 3 4 5 <-- computed area indices (in texelsWeights[])
*/
void GetWeightPerTexel_5TexelsWideTriangleFilter(float offset, out vec3 texelsWeightsA, out vec3 texelsWeightsB)
{
    //See _UnityInternalGetAreaPerTexel_3TexelTriangleFilter for details.
    vec4 computedArea_From3texelTriangle;
    vec4 computedAreaUncut_From3texelTriangle;
    GetAreaPerTexel_3TexelsWideTriangleFilter(offset, computedArea_From3texelTriangle, computedAreaUncut_From3texelTriangle);

    //Triangle slop is 45 degree thus we can almost reuse the result of the 3 texel wide computation.
    //the 5 texel wide triangle can be seen as the 3 texel wide one but shifted up by one unit/texel.
    //0.16 is 1/(the triangle area)
    texelsWeightsA.x = 0.16 * (computedArea_From3texelTriangle.x);
    texelsWeightsA.y = 0.16 * (computedAreaUncut_From3texelTriangle.y);
    texelsWeightsA.z = 0.16 * (computedArea_From3texelTriangle.y + 1.0);
    texelsWeightsB.x = 0.16 * (computedArea_From3texelTriangle.z + 1.0);
    texelsWeightsB.y = 0.16 * (computedAreaUncut_From3texelTriangle.z);
    texelsWeightsB.z = 0.16 * (computedArea_From3texelTriangle.w);
}

half SampleShadowmap_PCF3x3Tent(texture2D shadowTex, samplerShadow shadowTexSampler, vec4 coord, float depthBias)
{
    float shadow = 1.0;

    // tent base is 3x3 base thus covering from 9 to 12 texels, thus we need 4 bilinear PCF fetches
    vec2 ShadowMapTexture_TexelSize = vec2(textureSize(sampler2D(shadowTex, shadowTexSampler), 0)); 
    vec2 tentCenterInTexelSpace = coord.xy * ShadowMapTexture_TexelSize;
    vec2 centerOfFetchesInTexelSpace = floor(tentCenterInTexelSpace + 0.5);
    vec2 offsetFromTentCenterToCenterOfFetches = tentCenterInTexelSpace - centerOfFetchesInTexelSpace;

    // find the weight of each texel based
    vec4 texelsWeightsU, texelsWeightsV;
    GetWeightPerTexel_3TexelsWideTriangleFilter(offsetFromTentCenterToCenterOfFetches.x, texelsWeightsU);
    GetWeightPerTexel_3TexelsWideTriangleFilter(offsetFromTentCenterToCenterOfFetches.y, texelsWeightsV);

    // each fetch will cover a group of 2x2 texels, the weight of each group is the sum of the weights of the texels
    vec2 fetchesWeightsU = texelsWeightsU.xz + texelsWeightsU.yw;
    vec2 fetchesWeightsV = texelsWeightsV.xz + texelsWeightsV.yw;

    // move the PCF bilinear fetches to respect texels weights
    vec2 fetchesOffsetsU = texelsWeightsU.yw / fetchesWeightsU.xy + vec2(-1.5,0.5);
    vec2 fetchesOffsetsV = texelsWeightsV.yw / fetchesWeightsV.xy + vec2(-1.5,0.5);
    fetchesOffsetsU /= ShadowMapTexture_TexelSize.xx;
    fetchesOffsetsV /= ShadowMapTexture_TexelSize.yy;

    // fetch !
    vec2 bilinearFetchOrigin = centerOfFetchesInTexelSpace / ShadowMapTexture_TexelSize;
    
    coord.z -= depthBias;
    shadow = fetchesWeightsU.x * fetchesWeightsV.x * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.x, fetchesOffsetsV.x), coord.zw), 0.0);
    shadow += fetchesWeightsU.y * fetchesWeightsV.x * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.y, fetchesOffsetsV.x), coord.zw), 0.0);
    shadow += fetchesWeightsU.x * fetchesWeightsV.y * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.x, fetchesOffsetsV.y), coord.zw), 0.0);
    shadow += fetchesWeightsU.y * fetchesWeightsV.y * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.y, fetchesOffsetsV.y), coord.zw), 0.0);

    return half_x(shadow);
}

half SampleShadowmap_PCF5x5Tent(texture2D shadowTex, samplerShadow shadowTexSampler, vec4 coord, float depthBias)
{
    float shadow = 1.0;

    // tent base is 5x5 base thus covering from 25 to 36 texels, thus we need 9 bilinear PCF fetches
    vec2 ShadowMapTexture_TexelSize = vec2(textureSize(sampler2D(shadowTex, shadowTexSampler), 0)); 
    vec2 tentCenterInTexelSpace = coord.xy * ShadowMapTexture_TexelSize;
    vec2 centerOfFetchesInTexelSpace = floor(tentCenterInTexelSpace + 0.5);
    vec2 offsetFromTentCenterToCenterOfFetches = tentCenterInTexelSpace - centerOfFetchesInTexelSpace;

    // find the weight of each texel based on the area of a 45 degree slop tent above each of them.
    vec3 texelsWeightsU_A, texelsWeightsU_B;
    vec3 texelsWeightsV_A, texelsWeightsV_B;
    GetWeightPerTexel_5TexelsWideTriangleFilter(offsetFromTentCenterToCenterOfFetches.x, texelsWeightsU_A, texelsWeightsU_B);
    GetWeightPerTexel_5TexelsWideTriangleFilter(offsetFromTentCenterToCenterOfFetches.y, texelsWeightsV_A, texelsWeightsV_B);

    // each fetch will cover a group of 2x2 texels, the weight of each group is the sum of the weights of the texels
    vec3 fetchesWeightsU = vec3(texelsWeightsU_A.xz, texelsWeightsU_B.y) + vec3(texelsWeightsU_A.y, texelsWeightsU_B.xz);
    vec3 fetchesWeightsV = vec3(texelsWeightsV_A.xz, texelsWeightsV_B.y) + vec3(texelsWeightsV_A.y, texelsWeightsV_B.xz);

    // move the PCF bilinear fetches to respect texels weights
    vec3 fetchesOffsetsU = vec3(texelsWeightsU_A.y, texelsWeightsU_B.xz) / fetchesWeightsU.xyz + vec3(-2.5,-0.5,1.5);
    vec3 fetchesOffsetsV = vec3(texelsWeightsV_A.y, texelsWeightsV_B.xz) / fetchesWeightsV.xyz + vec3(-2.5,-0.5,1.5);
    fetchesOffsetsU /= ShadowMapTexture_TexelSize.xxx;
    fetchesOffsetsV /= ShadowMapTexture_TexelSize.yyy;

    // fetch !
    vec2 bilinearFetchOrigin = centerOfFetchesInTexelSpace / ShadowMapTexture_TexelSize;
    coord.z -= depthBias;

    shadow  = fetchesWeightsU.x * fetchesWeightsV.x * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.x, fetchesOffsetsV.x), coord.zw), 0.0);
    shadow += fetchesWeightsU.y * fetchesWeightsV.x * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.y, fetchesOffsetsV.x), coord.zw), 0.0);
    shadow += fetchesWeightsU.z * fetchesWeightsV.x * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.z, fetchesOffsetsV.x), coord.zw), 0.0);
    shadow += fetchesWeightsU.x * fetchesWeightsV.y * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.x, fetchesOffsetsV.y), coord.zw), 0.0);
    shadow += fetchesWeightsU.y * fetchesWeightsV.y * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.y, fetchesOffsetsV.y), coord.zw), 0.0);
    shadow += fetchesWeightsU.z * fetchesWeightsV.y * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.z, fetchesOffsetsV.y), coord.zw), 0.0);
    shadow += fetchesWeightsU.x * fetchesWeightsV.z * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.x, fetchesOffsetsV.z), coord.zw), 0.0);
    shadow += fetchesWeightsU.y * fetchesWeightsV.z * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.y, fetchesOffsetsV.z), coord.zw), 0.0);
    shadow += fetchesWeightsU.z * fetchesWeightsV.z * textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), vec4(bilinearFetchOrigin + vec2(fetchesOffsetsU.z, fetchesOffsetsV.z), coord.zw), 0.0);

    return half_x(shadow);
}


//#define tex2DProj textureProjLod
half ComputeShadow(texture2D shadowTex, sampler shadowTexSampler, highp vec4 proj, highp float bias)
{
    highp float depth = proj.z - bias;		
    highp vec2 scale = proj.ww / vec2(textureSize(sampler2D(shadowTex, shadowTexSampler), 0));
    highp vec2 pos = proj.xy;    

    half s0 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2(-0.94201624, -0.39906216) * scale, 0.0).r));
    half s1 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2( 0.94558609, -0.76890725) * scale, 0.0).r));
    half s2 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2(-0.094184101,-0.92938870) * scale, 0.0).r));
    half s3 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2( 0.34495938,  0.29387760) * scale, 0.0).r));

#ifdef PCF3x3		
    half s4 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2(-0.91588581,  0.45771432) * scale, 0.0).r));
    half s5 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2(-0.81544232, -0.87912464) * scale, 0.0).r));
    half s6 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2(-0.38277543,  0.27676845) * scale, 0.0).r));
    half s7 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2( 0.97484398,  0.75648379) * scale, 0.0).r));
    half s8 = half_x(step(depth, textureLod(sampler2D(shadowTex, shadowTexSampler), pos + vec2( 0.44323325, -0.97511554) * scale, 0.0).r));
    half sm = (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / half_x(9.0);
#else
    half sm = (s0 + s1 + s2 + s3) * half_x(0.25);
#endif
    return sm;
}

half textureBilinear(in texture2D tex, samplerShadow shadowTexSampler, in vec2 coord, float depth)
{
    // Get texture size in pixels:
    vec2 size = vec2(textureSize(sampler2D(tex, shadowTexSampler), 0));

    // Convert UV coordinates to pixel coordinates and get pixel index of top left pixel (assuming UVs are relative to top left corner of texture)
    vec2 pixCoord = coord * size - 0.5;    // First pixel goes from -0.5 to +0.4999 (0.0 is center) last pixel goes from (size - 1.5) to (size - 0.5000001)
    vec2 originPixCoord = floor(pixCoord);              // Pixel index coordinates of bottom left pixel of set of 4 we will be blending

    // For Gather we want UV coordinates of bottom right corner of top left pixel
    vec2 gatherUV = (originPixCoord + 1.0) / size;

    // Gather from all surounding texels:
    half4 red = half4_x(textureGather(sampler2DShadow(tex, shadowTexSampler), gatherUV, depth));
 
    // Swizzle the gathered components to create four colours
    half c00 = red.w;
    half c01 = red.x;
    half c11 = red.y;
    half c10 = red.z;

    // Filter weight is fract(coord * size - 0.5) = (coord * size - 0.5) - floor(coord * size - 0.5)
    half2 filterWeight = half2_x(pixCoord - originPixCoord);
 
    // Bi-linear mixing:
    half temp0 = mix(c01, c11, filterWeight.x);
    half temp1 = mix(c00, c10, filterWeight.x);
    return mix(temp1, temp0, filterWeight.y);
}

half SimpleComputeShadow(texture2D shadowTex, samplerShadow shadowTexSampler, highp vec4 proj, highp float bias)
{         
#if 1
    proj.z -= bias * proj.w;          
    return half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj, 0.0));
#else
    return textureBilinear(shadowTex, shadowTexSampler, proj.xy, proj.z);
#endif    
}

half ComputeShadow(texture2D shadowTex, samplerShadow shadowTexSampler, highp vec4 proj, highp float bias)
{
#if 1
#if PLATFORM & PLATFORM_WIN
    return SampleShadowmap_PCF5x5Tent(shadowTex, shadowTexSampler, proj, bias);
#else 
    return SampleShadowmap_PCF3x3Tent(shadowTex, shadowTexSampler, proj, bias);
#endif    
#else    
    proj.z -= bias * proj.w;                  
    vec2 size = vec2(textureSize(sampler2D(shadowTex, shadowTexSampler), 0)); 
    highp vec4 scale = vec4(proj.ww, 0.0, 0.0) / vec4(size, 1.0, 1.0);

    half s0 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.94201624, -0.39906216, 0.0, 0.0) * scale, 0.0));
    half s1 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.94558609, -0.76890725, 0.0, 0.0) * scale, 0.0));
    half s2 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.094184101,-0.92938870, 0.0, 0.0) * scale, 0.0));
    half s3 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.34495938,  0.29387760, 0.0, 0.0) * scale, 0.0));

#ifdef PCF3x3       
    half s4 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.91588581,  0.45771432, 0.0, 0.0) * scale, 0.0));
    half s5 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.81544232, -0.87912464, 0.0, 0.0) * scale, 0.0));
    half s6 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.38277543,  0.27676845, 0.0, 0.0) * scale, 0.0));
    half s7 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.97484398,  0.75648379, 0.0, 0.0) * scale, 0.0));
    half s8 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.44323325, -0.97511554, 0.0, 0.0) * scale, 0.0));
    half sm = (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / half_x(9.0);
#else       
    half sm = (s0 + s1 + s2 + s3) * half_x(0.25);
#endif  
    return sm;
#endif    
}

half ComputeShadow(texture2D shadowTex, samplerShadow shadowTexSampler, highp vec4 proj, vec2 size, highp float bias)
{
#if 1
#if PLATFORM & PLATFORM_WIN    
    return SampleShadowmap_PCF5x5Tent(shadowTex, shadowTexSampler, proj, bias);
#else 
    return SampleShadowmap_PCF3x3Tent(shadowTex, shadowTexSampler, proj, bias);
#endif   
#else    
    proj.z -= bias * proj.w;                  
    highp vec4 scale = vec4(proj.ww, 0.0, 0.0) / vec4(size, 1.0, 1.0);
    
    half s0 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.94201624, -0.39906216, 0.0, 0.0) * scale, 0.0));
    half s1 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.94558609, -0.76890725, 0.0, 0.0) * scale, 0.0));
    half s2 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.094184101,-0.92938870, 0.0, 0.0) * scale, 0.0));
    half s3 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.34495938,  0.29387760, 0.0, 0.0) * scale, 0.0));

#ifdef PCF3x3       
    half s4 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.91588581,  0.45771432, 0.0, 0.0) * scale, 0.0));
    half s5 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.81544232, -0.87912464, 0.0, 0.0) * scale, 0.0));
    half s6 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4(-0.38277543,  0.27676845, 0.0, 0.0) * scale, 0.0));
    half s7 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.97484398,  0.75648379, 0.0, 0.0) * scale, 0.0));
    half s8 = half_x(textureProjLod(sampler2DShadow(shadowTex, shadowTexSampler), proj + vec4( 0.44323325, -0.97511554, 0.0, 0.0) * scale, 0.0));
    half sm = (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / half_x(9.0);
#else       
    half sm = (s0 + s1 + s2 + s3) * half_x(0.25);
#endif  
    return sm;
#endif    
}

half ComputeShadow(texture2DArray shadowTex, samplerShadow shadowTexSampler, highp vec4 proj, highp float index, highp float bias)
{
    proj.w = proj.z - bias;
    proj.z = index;
    highp vec4 scale = vec4(proj.ww, 0.0, 0.0) / vec4(textureSize(sampler2DArray(shadowTex, shadowTexSampler), 0).xy, 1.0, 1.0);

    half s0 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4(-0.94201624, -0.39906216, 0.0, 0.0) * scale));
    half s1 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4( 0.94558609, -0.76890725, 0.0, 0.0) * scale));
    half s2 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4(-0.094184101,-0.92938870, 0.0, 0.0) * scale));
    half s3 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4( 0.34495938,  0.29387760, 0.0, 0.0) * scale));
#ifdef PCF3x3       
    half s4 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4(-0.91588581,  0.45771432, 0.0, 0.0) * scale));
    half s5 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4(-0.81544232, -0.87912464, 0.0, 0.0) * scale));
    half s6 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4(-0.38277543,  0.27676845, 0.0, 0.0) * scale));
    half s7 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4( 0.97484398,  0.75648379, 0.0, 0.0) * scale));
    half s8 = half_x(texture(sampler2DArrayShadow(shadowTex, shadowTexSampler), proj + vec4( 0.44323325, -0.97511554, 0.0, 0.0) * scale));
    half sm = (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / half_x(9.0);
#else   
    half sm = (s0 + s1 + s2 + s3) * half_x(0.25);
#endif  
    return sm;
}

half ComputeShadow(texture2DArray shadowTex, sampler shadowTexSampler, highp vec4 proj, highp float index,highp float bias)
{
    highp vec3 scale = vec3(proj.ww, 0.0) / vec3(textureSize(sampler2DArray(shadowTex, shadowTexSampler), 0).xy, 1.0);
    highp float depth = proj.z-bias;
    proj.z = index;
    
    half sm = half_x(0.0);
    half s0 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3(-0.94201624, -0.39906216, 0.0) * scale).r));
    half s1 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3( 0.94558609, -0.76890725, 0.0) * scale).r));
    half s2 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3(-0.094184101,-0.92938870, 0.0) * scale).r));
    half s3 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3( 0.34495938,  0.29387760, 0.0) * scale).r));
#ifdef PCF3x3       
    half s4 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3(-0.91588581,  0.45771432, 0.0) * scale).r));
    half s5 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3(-0.81544232, -0.87912464, 0.0) * scale).r));
    half s6 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3(-0.38277543,  0.27676845, 0.0) * scale).r));
    half s7 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3( 0.97484398,  0.75648379, 0.0) * scale).r));
    half s8 = half_x(step(depth, texture(sampler2DArray(shadowTex, shadowTexSampler), proj.xyz + vec3( 0.44323325, -0.97511554, 0.0) * scale).r));
    sm = (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / half_x(9.0);
#else   
    sm = (s0 + s1 + s2 + s3) * half_x(0.25);
#endif  
    return sm;
}

highp vec4 CameraToShadowProj(highp vec4 viewPos, highp mat4 viewInv, highp mat4 lightView, highp mat4 lightProj)
{
    highp mat4 view = mul(viewInv, lightView);
    highp vec4 pos = mul(viewPos, view);
    pos /= pos.w;
    return mul(pos, lightProj);	
}

#define NORMAL_BAIS 1.0

highp vec3 ApplyLinearShadowBias(highp vec3 wPos, vec3 worldNormal, vec3 wLight)
{   
    highp float shadowCos = dot(worldNormal, wLight);
    highp float shadowSine = sqrt(1.0 - shadowCos * shadowCos);
    highp float normalBias = NORMAL_BAIS * shadowSine;
    wPos += worldNormal * normalBias;
    return wPos;
}

highp vec4 ApplyLinearShadowBias(highp vec4 wPos, vec3 worldNormal, vec3 wLight)
{	    	
    highp float shadowCos = dot(worldNormal, wLight);
    highp float shadowSine = sqrt(1.0 - shadowCos * shadowCos);
    highp float normalBias = NORMAL_BAIS * shadowSine;
    wPos.xyz += worldNormal * normalBias;
    return wPos;
}

highp vec3 ApplyLinearShadowBias(highp vec3 Pos, vec3 WorldNormal, float NoL)
{        
    float ShadowSine = sqrt(1.0 - NoL);
    float Bias = NORMAL_BAIS * ShadowSine;
    Pos += WorldNormal * Bias;
    return Pos;
}

highp float ComputeVarianceShadow(highp sampler2D shadowImage, highp mat4 lightvp, highp vec3 wPos)
{
    highp float light_vsm_epsilon = 0.00001;
    highp float light_shadow_epsilon = 0.00001;
    highp vec4 proj = mul(vec4(wPos.xyz, 1.0), lightvp);
    proj.xyz /= proj.w;
    proj.xy = proj.xy * 0.5 + vec2(0.5, 0.5);
    proj.z += light_shadow_epsilon;

    highp vec4 s4 = texture(shadowImage, proj.xy, 0.0);

    highp float lit_factor = 1.0;

    if (proj.z <= s4.x)
    {
        // Variance shadow mapping
        highp float E_x2 = s4.y;
        highp float Ex_2 = s4.x * s4.x;
        highp float variance = min(max(E_x2 - Ex_2, 0.0) + light_vsm_epsilon, 1.0);
        highp float m_d = (s4.x - proj.z);
        highp float p = variance / (variance + m_d * m_d);
        lit_factor = max(lit_factor, p);
    }

    return lit_factor;
}

/*
mediump int GetShadowLevel(mediump float vCurrentCascadeDepth,mediump vec4 vStaticPlane,mediump int nNumShadowMap)
{
    vec4 fComparison = vec4(vCurrentCascadeDepth > vStaticPlane.x,vCurrentCascadeDepth > vStaticPlane.y,vCurrentCascadeDepth > vStaticPlane.z,vCurrentCascadeDepth > vStaticPlane.w);
    vec4 temp = vec4(nNumShadowMap>0,nNumShadowMap>1,nNumShadowMap>2,nNumShadowMap>3);

    float fIndex = dot(fComparison, temp);
    mediump int iIndex = int(fIndex);
    return iIndex;
}
*/
