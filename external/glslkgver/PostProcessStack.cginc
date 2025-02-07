//PostProcessStack.cginc
// ODT_SAT => XYZ => D60_2_D65 => sRGB
const highp mat3 ACESOutputMatrix = mat3(
    1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const highp mat3 ACESInputMatrix = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);

highp vec3 RRTAndODT(highp vec3 v)
{
    highp vec3 a = v * (v + 0.0245786) - 0.000090537;
    highp vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 TonemapFilmic(vec3 color, float exposure)
{   
    color *= exposure;  
    color = clamp(color, 0.0, 30.0);           
    color = mul(ACESInputMatrix, color);
    color = RRTAndODT(color);
    color = mul(ACESOutputMatrix, color);
    color = saturate(color);
    return color;
}

vec3 TonemapACES(vec3 color, float exposure)
{
    color *= exposure;              
    const mediump float a = 2.51;
    const mediump float b = 0.03;
    const mediump float c = 2.43;
    const mediump float d = 0.59;
    const mediump float e = 0.14;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}   

// Brightness function
#define Brightness(c) max(max(c.r, c.g), c.b)
#define Median(a, b, c) (a + b + c - min(min(a, b), c) - max(max(a, b), c))

mediump vec2 Circle(float start, float count, float index) 
{
    float rad = (3.141592 * 2.0 * (1.0 / count)) * (index + start);
    return vec2(sin(rad), cos(rad));
}

//BloomSetupCommon
mediump vec4 BloomPrefilter(mediump vec4 sceneColor, mediump vec3 bloomTint, mediump float threshold, mediump float exposure)
{    
    // clamp to avoid artifacts from exceeding fp16 through framebuffer blending of multiple very bright lights
    sceneColor.rgb = min(vec3(20.0), sceneColor.rgb);

    // todo: make this adjustable (e.g. LUT)
    mediump float totalLuminance = Luminance(sceneColor);  //
    mediump float bloomLuminance = totalLuminance * exposure - threshold;
    // mask 0..1
    mediump float bloomAmount = saturate(bloomLuminance * 0.5);

    return vec4(sceneColor.rgb * bloomAmount, sceneColor.a);
}

mediump vec4 BloomDownSample(mediump sampler2D srcTex, mediump vec2 uv)
{    
    mediump ivec2 size = textureSize(srcTex, 0);
    mediump vec2 texelSize = vec2(0.5, 0.5) / vec2(float(size.x), float(size.y)); 
    mediump vec4 uv0 = texelSize.xyxy * vec4(1.0, 1.0, -1.0, -1.0);

    mediump vec4 s1 = textureLod(srcTex, uv + uv0.xy, 0.0);
    mediump vec4 s2 = textureLod(srcTex, uv + uv0.xw, 0.0);
    mediump vec4 s3 = textureLod(srcTex, uv + uv0.zy, 0.0);
    mediump vec4 s4 = textureLod(srcTex, uv + uv0.zw, 0.0);
     
    return 0.25 * (s1 + s2 + s3 + s4);
}

mediump vec3 BloomUpSample(mediump texture2D tex, sampler texSampler, mediump vec2 uv)
{
    // 9-tap bilinear upsampler (tent filter)
    mediump vec4 color = vec4(0.0);
    mediump ivec2 size = textureSize(sampler2D(tex, texSampler), 0);
    mediump vec2 texelSize = vec2(1.0) / vec2(float(size.x), float(size.y));
    mediump vec4 d = texelSize.xyxy * vec4(1.0, 1.0, -1.0, 0.0);

    mediump vec4 s = vec4(0);
    s =  textureLod(sampler2D(tex, texSampler), uv - d.xy, 0.0);
    s += textureLod(sampler2D(tex, texSampler), uv - d.wy, 0.0) * 2.0;
    s += textureLod(sampler2D(tex, texSampler), uv - d.zy, 0.0);

    s += textureLod(sampler2D(tex, texSampler), uv + d.zw, 0.0) * 2.0;
    s += textureLod(sampler2D(tex, texSampler), uv, 0.0)        * 4.0;
    s += textureLod(sampler2D(tex, texSampler), uv + d.xw, 0.0) * 2.0;

    s += textureLod(sampler2D(tex, texSampler), uv + d.zy, 0.0);
    s += textureLod(sampler2D(tex, texSampler), uv + d.wy, 0.0) * 2.0;
    s += textureLod(sampler2D(tex, texSampler), uv + d.xy, 0.0);

    return s.rgb / 16.0;
}

/*mediump vec3 BloomFinal(mediump sampler2D tex, mediump sampler2D lensDirtTex, mediump vec2 uv, mediump float scale)
{
    mediump vec3 dirty = texture(lensDirtTex, uv).rgb;
    mediump vec3 color = BloomUpSample(tex, uv);
    color *= scale;
    dirty *= dirtyIntenty;
    return color * dirty + color;
}*/

mediump float ComputeVignette(mediump vec2 pos, float intensity, float aspect)
{
    // Natural vignetting cosine-fourth law
    mediump vec2 d = (pos - vec2(0.5, 0.5)) * intensity;
    d.x *= aspect;//_ScreenParams.x / _ScreenParams.y;
    float tan2Angle = d.x * d.x + d.y * d.y; //can't use dot for vivo z70, for texSize int 
    float cos4Angle = pow2(1.0 / (tan2Angle + 1.0));
    return cos4Angle;           
}

mediump vec3 ColorGrade2DLut(mediump sampler2D LutTex, mediump vec3 color)
{
    mediump float chartDim = 16.0;
    mediump vec3 scale = vec3(chartDim - 1.0) / chartDim;
    mediump vec3 bias = vec3(0.5, 0.5, 0.0) / chartDim;
    mediump vec3 lookup = color * scale + bias;
    
    mediump float slice = lookup.z * chartDim;   
    mediump float sliceFrac = frac(slice);   
    mediump float sliceIdx = slice - sliceFrac;
    
    lookup.x = (lookup.x + sliceIdx) / chartDim;
    
    // lookup adjacent slices
    mediump vec3 col0 = textureLod(LutTex, lookup.xy, 0.0).rgb;     
    lookup.x += 1.0 / chartDim;
    mediump vec3 col1 = textureLod(LutTex, lookup.xy, 0.0).rgb;
    color = col0 + (col1 - col0) * ( sliceFrac);
    return color;
}

