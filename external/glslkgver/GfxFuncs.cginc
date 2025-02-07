/*GfxFuncs.cginc*/
//Using stripped down, 'pure log', formula. Parameterized by grey points and dynamic range covered.
#define LogToLin(LogColor) exp2((LogColor - 444.0 / 1023.0) * 14.0) * 0.18
//Generic log lin transforms
#define LinToLog(LinearColor) saturate(log2(LinearColor) / 14.0 - log2(0.18) / 14.0 + 444.0 / 1023.0)

#ifndef A_HALF
#define RGBToYCoCg(c) vec3(0.25 * c.r + 0.5 * c.g + 0.25 * c.b, 0.5 * c.r - 0.5 * c.b ,-0.25 * c.r + 0.5 * c.g - 0.25 * c.b)
#define YCoCgToRGB(yCoCg) vec3(yCoCg.x + yCoCg.y - yCoCg.z, yCoCg.x + yCoCg.z, yCoCg.x - yCoCg.y - yCoCg.z)
//Simple Reinhard Tonemapping
#if 1
#define Reinhard(hdr) (hdr / (hdr + 1.0))
#define ReinhardInverse(sdr) (sdr / max(1.0 - sdr, 0.04762))
#else
#define Reinhard(hdr) (hdr)
#define ReinhardInverse(sdr) (sdr)
#endif

#define RGBToYCoCgX(c) (c.r * 0.25 + c.g * 0.5 + c.b * 0.25)
#define Min3Color(a, b, c) min(a, min(b, c))
#define Max3Color(a, b, c) max(a, max(b, c))
#define LinearToSrgb(lin) min(max(lin, HALF_MIN) * 12.92, pow(max(lin, 0.00313067), vec3(1.0/2.4)) * 1.055 - 0.055)
#define SRGBToLinear(rgb) lerp(rgb / 12.92, pow((rgb + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), rgb))
#define DecodeRenderType(mask) (uint(mask * 255.0 + 0.5) >> 4u)
#define DecodeMetal(mask) float(uint(mask * 255.0 + 0.5) & 0xF) / 15.0

//v = l < r ? v0 : v1 或 >=
#define selectLess(l, r, v0, v1) mix(v1, v0, step(l, r))

half LinearToSrgbBranchless(half lin)
{    
    //minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
    lin = max(lin, HALF_MIN);
    return lin < 0.00313067 ? lin * 12.92 : pow(lin, 1.0 / 2.4) * 1.055 - 0.055;
}

half4 Texture2DSampleBicubic(texture2D tex, sampler s, vec2 UV, vec2 Size, vec2 InvSize)
{    
    vec2 Sample[3];
    UV *= Size;

    vec2 tc = floor(UV - 0.5) + 0.5;
    half2 f = UV - tc;
    half2 f2 = f * f;
    half2 f3 = f2 * f;

    half2 w0 = f2 - 0.5 * (f3 + f);
    half2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
    half2 w3 = 0.5 * (f3 - f2);
    half2 w2 = 1 - w0 - w1 - w3;
    
    half2 w12 = w1 + w2;

    Sample[0] = InvSize * (tc - 1.0);
    Sample[1] = InvSize * (tc + w2 / w12);
    Sample[2] = InvSize * (tc + 2.0);

    half cw0 = w12.x * w0.y;
    half cw1 = w0.x * w12.y;
    half cw2 = w12.x * w12.y;
    half cw3 = w3.x * w12.y;
    half cw4 = w12.x * w3.y;

    // Reweight after removing the corners
    float CornerWeights = cw0 + cw1 + cw2 + cw3 + cw4;    
    
    half4 OutColor = textureLod(sampler2D(tex, s), vec2(Sample[1].x, Sample[0].y), 0.0) * cw0;
    OutColor += textureLod(sampler2D(tex, s), vec2(Sample[0].x, Sample[1].y), 0.0) * cw1;
    OutColor += textureLod(sampler2D(tex, s), vec2(Sample[1].x, Sample[1].y), 0.0) * cw2;
    OutColor += textureLod(sampler2D(tex, s), vec2(Sample[2].x, Sample[1].y), 0.0) * cw3;
    OutColor += textureLod(sampler2D(tex, s), vec2(Sample[1].x, Sample[2].y), 0.0) * cw4;

    OutColor /= CornerWeights;
    return OutColor;
}

#else
half3 RGBToYCoCg(half3 c)
{
    half r = half(0.25) * c.r + half(0.5) * c.g + half(0.25) * c.b;
    half g = half(0.5) * c.r - half(0.5) * c.b;
    half b = -half(0.25) * c.r + half(0.5) * c.g - half(0.25) * c.b;
    return half3(r, g, b);
}

half3 YCoCgToRGB(half3 yCoCg)
{
  return half3(yCoCg.x + yCoCg.y - yCoCg.z, yCoCg.x + yCoCg.z, yCoCg.x - yCoCg.y - yCoCg.z);
} 

half RGBToYCoCgX(half3 c)
{
    return c.r * half(0.25) + c.g * half(0.5) + c.b * half(0.25);
} 

half3 Reinhard(half3 hdr) 
{
    return hdr / (hdr + half3(1.0));
}

half3 ReinhardInverse(half3 sdr) 
{
    return sdr / max(half3(1.0) - sdr, half3(0.04762));
}

vec3 Reinhard(vec3 hdr) 
{
    return hdr / (hdr + vec3(1.0));
}

vec3 ReinhardInverse(vec3 sdr) 
{
    return sdr / max(vec3(1.0) - sdr, vec3(0.04762));
}

uint DecodeRenderType(half mask) 
{
    return uint(mask * half(255.0) + half(0.5)) >> 4u;
}

half DecodeMetal(half mask) 
{
    return half(uint(mask * half(255.0) + half(0.5)) & 0xF) / half(15.0);
}

vec3 LinearToSrgb(vec3 lin)
{
    return min(max(lin, HALF_MIN) * 12.92, pow(max(lin, 0.00313067), vec3(1.0/2.4)) * 1.055 - 0.055);
} 

half3 LinearToSrgb(half3 lin)
{
    return min(max(lin, half3(HALF_MIN)) * half3(12.92), pow(max(lin, half3(0.00313067)), half3(1.0/2.4)) * half3(1.055) - half3(0.055));
} 

vec3 SRGBToLinear(vec3 rgb) 
{
    return lerp(rgb / 12.92, pow((rgb + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), rgb));
}

half3 SRGBToLinear(half3 rgb) 
{
    return lerp(rgb / half3(12.92), pow((rgb + half3(0.055)) / half3(1.055), half3(2.4)), step(half3(0.04045), rgb));
}

#endif

//Intrinsic isnan can't be used because it require /Gic to be enabled on fxc that we can't do. So use AnyIsNan instead
//make Nan sqrt(x - 1.0) x < 1.0
//|1符号|8指数|23位数|, 指数位全为1，尾数位非零，该浮点数表示为NaN
bool IsNaN(float x)
{    
    return (floatBitsToUint(x) & 0x7FFFFFFF) > 0x7F800000;
}

bool IsInf(float x)
{
    return (floatBitsToUint(x) & 0x7FFFFFFF) == 0x7F800000;
}

bool IsFinite(float x)
{
    return (asuint(x) & 0x7F800000) != 0x7F800000;
}

float SanitizeFinite(float x)
{
    return IsFinite(x) ? x : 0.0;
}

bool IsPositiveFinite(float x)
{
    return asuint(x) < 0x7F800000;
}

float SanitizePositiveFinite(float x)
{
    return IsPositiveFinite(x) ? x : 0.0;
}

bool AnyIsNan(vec4 v)
{
    return (IsNaN(v.x) || IsNaN(v.y) || IsNaN(v.z) || IsNaN(v.w));
}

bool AnyIsNan(vec3 v)
{
    return (IsNaN(v.x) || IsNaN(v.y) || IsNaN(v.z));
}

//make Inf x / 0.0
bool AnyIsInf(vec4 v)
{
    return (IsInf(v.x) || IsInf(v.y) || IsInf(v.z) || IsInf(v.w));
}

half3 ApplySharpening(half3_t center, half3_t top, half3_t left, half3_t right, half3_t bottom) 
{ 
    half3 result = RGBToYCoCg(center); 
    half unsharpenMask = result.x * half_x(4.0);
    unsharpenMask -= RGBToYCoCgX(top); 
    unsharpenMask -= RGBToYCoCgX(bottom); 
    unsharpenMask -= RGBToYCoCgX(left); 
    unsharpenMask -= RGBToYCoCgX(right); 
    result.x = min(half_x(0.25) * unsharpenMask + result.x, half_x(1.1) * result.x); 
    return YCoCgToRGB(result); 
} 

highp vec2 SafeNormalize(highp vec2 inVec)
{
    highp float dp2 = max(FLT_MIN_SQRT, dot(inVec, inVec));
    return inVec * rsqrt(dp2);
}

highp vec3 SafeNormalize(highp vec3 inVec)
{
    highp float dp3 = max(FLT_MIN_SQRT, dot(inVec, inVec));
    return inVec * rsqrt(dp3);
}

highp vec4 SafeNormalize(highp vec4 inVec)
{
    highp float dp4 = max(FLT_MIN_SQRT, dot(inVec, inVec));
    return inVec * rsqrt(dp4);
}

#ifdef A_HALF
half2 SafeNormalize(half2 inVec)
{
    half dp2 = max(half(HALF_MIN_SQRT), dot(inVec, inVec));
    return inVec * rsqrt(dp2);
}

half3 SafeNormalize(half3 inVec)
{
    half dp3 = max(half(HALF_MIN_SQRT), dot(inVec, inVec));
    return inVec * rsqrt(dp3);
}

half4 SafeNormalize(half4 inVec)
{
    half dp4 = max(half(HALF_MIN_SQRT), dot(inVec, inVec));
    return inVec * rsqrt(dp4);
}

/*bool IsNaN(half x)
{
    return (floatBitsToUint16(x) & 0x7FFF) > 0x7C00;
}*/

bool AnyIsNan(half3 v)
{
    return (isnan(v.x) || isnan(v.y) || isnan(v.z));
}
#endif

highp vec4 SafePow(highp vec4 x, highp float y)
{       
    return pow(max(abs(x), vec4(HALF_EPS)), vec4(y));
}

highp vec3 SafePow(highp vec3 x, highp float y)
{       
    return pow(max(abs(x), vec3(HALF_EPS)), vec3(y));
}

highp vec2 SafePow(highp vec2 x, highp float y)
{       
    return pow(max(abs(x), vec2(HALF_EPS)), vec2(y));
}

highp float SafePow(highp float x, highp float y)
{       
    return pow(max(abs(x), HALF_EPS), y);
}

// Division which returns 1 for (inf/inf) and (0/0).
// If any of the input parameters are NaNs, the result is a NaN.
highp float SafeDiv(highp float numer, highp float denom)
{
    return (numer != denom) ? numer / denom : 1.0;
}
    
float f16tof32(uint x)
{
    return unpackHalf2x16(x).x;   
}

uint f32tof16(float x)
{
    return packHalf2x16(vec2(x, 0.0));
}

vec2 f16tof32(uvec2 x)
{
    return vec2(f16tof32(x.x), f16tof32(x.y));
}

uvec2 f32tof16(vec2 x)
{    
    return uvec2(f32tof16(x.x), f32tof16(x.y));    
}    

vec3 f16tof32(uvec3 x)
{
    return vec3(f16tof32(x.x), f16tof32(x.y), f16tof32(x.z));
}

uvec3 f32tof16(vec3 x)
{    
    return uvec3(f32tof16(x.x), f32tof16(x.y), f32tof16(x.z));    
}  

float asfloat(int x)
{
    return intBitsToFloat(x);
}

float asfloat(uint x)
{
    return uintBitsToFloat(x);
}

float ApproxLog2(float f)
{
    return (float (asuint(f)) / 8388608.0) - 127.0;
}

float ApproxExp2 (float f)
{
    uint param = uint((f + 127.0) * 8388608.0);
    return asfloat(param);
}

float FastAtan2(float x, float y)
{
    float t0 = max(abs(x), abs(y));
    float t1 = min(abs(x), abs(y));
    float t3 = t1 / t0;
    float t4 = t3 * t3;

    // Same polynomial as FastATanPos
    t0 = 0.0872929;
    t0 = t0 * t4 - 0.301895;
    t0 = t0 * t4 + 1.0;
    t3 = t0 * t3;

    t3 = abs(y) > abs(x) ? HALF_PI - t3 : t3;
    t3 = x < 0 ? PI - t3 : t3;
    t3 = y < 0 ? -t3 : t3;

    return t3;
}

float FastAtan(float x)
{
    // Minimax 3 approximation
    vec3 A = x < 1.0 ? vec3(x, 0.0, 1.0) : vec3(1.0/x, HALF_PI, -1.0);
    return A.y + A.z * ((( -0.130234 * A.x - 0.0954105 ) * A.x + 1.00712 ) * A.x - 0.00001203333 );
}

/*
float FastATanPos( float x ) 
{ 
    float t0 = (x < 1.0) ? x : 1.0 / x;
    float t1 = t0 * t0;
    float poly = 0.0872929;
    poly = -0.301895 + poly * t1;
    poly = 1.0 + poly * t1;
    poly = poly * t0;
    return (x < 1.0) ? poly : HALF_PI - poly;
}

float FastAtan( float x )
{
    float t0 = FastATanPos(abs(x));
    return (x < 0) ? -t0: t0;
}
*/

//Only faster in mobile devices, and good for avoiding metal pre-z flickering when using fast math
float FastSin(float x) 
{    
#if 1
    float zeroTo2PI = fmod(x, TWO_PI); //move to range 0-2pi
    zeroTo2PI /= HALF_PI;
    //This calculation is achieved by Desmo; Only 3 instructions !!!
    vec2 core = vec2(zeroTo2PI) + vec2(-1.0, -3.0);
    vec2 result2 = saturate(-core * core + 1.0);
    return result2.x - result2.y;
#else
    return sin(x);
#endif    
}

vec2 FastSin(vec2 x) 
{    
#if 1
    vec2 zeroTo2PI = fmod(x, TWO_PI); //move to range 0-2pi
    zeroTo2PI /= HALF_PI;
    //This calculation is achieved by Desmo; Only 3 instructions !!!
    vec4 core = zeroTo2PI.xxyy + vec4(-1.0, -3.0, -1.0, -3.0);
    vec4 result = saturate(-core * core + 1.0);
    return vec2(result.xz - result.yw);
#else
    return sin(x);
#endif    
}

float FastCos(float x)
{
    return FastSin(x + HALF_PI);
}

//Calculate sin/cos together to save instructions
vec2 FastSinCos(float x) 
{    
    vec2 zeroTo2PI = fmod(vec2(x, x + HALF_PI), TWO_PI); //move to range 0-2pi
    zeroTo2PI /= HALF_PI;
    //This calculation is achieved by Desmo; Only 3 instructions !!!
    vec4 core = zeroTo2PI.xxyy + vec4(-1.0, -3.0, -1.0, -3.0);
    vec4 result = saturate(-core * core + 1.0);
    return vec2(result.xz - result.yw);
}

vec2 FastCosSin(float x) 
{    
    vec2 zeroTo2PI = fmod(vec2(x + HALF_PI, x), TWO_PI); //move to range 0-2pi
    zeroTo2PI /= HALF_PI;
    //This calculation is achieved by Desmo; Only 3 instructions !!!
    vec4 core = zeroTo2PI.xxyy + vec4(-1.0, -3.0, -1.0, -3.0);
    vec4 result = saturate(-core * core + 1.0);
    return vec2(result.xz - result.yw);
}

// 4th order polynomial approximation
// 4 VGRP, 16 ALU Full Rate
// 7 * 10^-5 radians precision
// Reference : Handbook of Mathematical Functions (chapter : Elementary Transcendental Functions), M. Abramowitz and I.A. Stegun, Ed.
float FastACos(float x) 
{
#if 1
    float abs_cos = abs(x);
    float abs_acos = ((-0.0187292993 * abs_cos  + 0.0742610) * abs_cos - 0.2121144) * abs_cos + 1.5707288;
    abs_acos *= sqrt(1.0 - abs_cos);    
    return x < 0.0 ?  PI - abs_acos : abs_acos;
#else
    return acos(x);
#endif    
}

float FastASin(float x)
{
    return HALF_PI - FastACos(x);
}

vec2 FastACos(vec2 x) 
{
#if 1
    vec2 abs_cos = abs(x);
    vec2 abs_acos = ((-0.0187292993 * abs_cos  + 0.0742610) * abs_cos - 0.2121144) * abs_cos + 1.5707288;
    abs_acos *= sqrt(1.0 - abs_cos);    
    return mix(abs_acos, PI - abs_acos, step(x, vec2(0.0)));
#else
    return acos(x);
#endif    
}

/*ue simple max absolute error 9.0x10^-3
float FastASin(float x) 
{
    // Maximum error: 10^-3.35    
    x = clamp(x, -1.0, 1.0);
    float a = abs(x);
    float r = HALF_PI + x * (-0.207034 + 0.0531013 * x);
    r = HALF_PI - r * sqrt(1.0 - a);
    return x >= 0 ? r : -r;
}

float FastACos(float x)
{
    return HALF_PI - FastASin(x);
}
*/

// @param A doesn't have to be normalized, output could be NaN if this is near 0,0,0
// @param B doesn't have to be normalized, output could be NaN if this is near 0,0,0
// @return can be passed to a acosFast() or acos() to compute an angle
float CosBetweenVectors(vec3 A, vec3 B)
{
    // unoptimized: dot(normalize(A), normalize(B))
    return dot(A, B) * rsqrt(max(FLT_MIN_SQRT, dot(A, A) * dot(B, B)));
}

mat3 GetTangentBasis(vec3 tangentZ) 
{
    vec3 up = abs(tangentZ.z) < 0.9999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, tangentZ));
    vec3 tangentY = cross(tangentZ, tangentX);
    return mat3(tangentX, tangentY, tangentZ);
}

mat3 CreateTBN(vec3 N) 
{
    vec3 U;
    if (abs(N.z) > 0.0) 
    {
        float k = sqrt(N.y * N.y + N.z * N.z + FLT_MIN_SQRT);
        U.x = 0.0; U.y = -N.z / k; U.z = N.y / k;
    }
    else 
    {
        float k = sqrt(N.x * N.x + N.y * N.y + FLT_MIN_SQRT);
        U.x = N.y / k; U.y = -N.x / k; U.z = 0.0;
    }

    mat3 TBN = mat3(U, cross(N, U), N);
    //return transpose(TBN); //for vk hlsl
    return TBN;
}


//Returns sign bit of floating point as either 1 or -1.
//#define FastSign(v) (1 - int((asuint(v) & 0x80000000) >> 30))

//copy sign from s to x
float CopySign(float x, float s)
{
    uint sign = 0x80000000u;         
    return asfloat(asuint(x) & ~sign | asuint(s) & sign);        
}

vec2 UniformSampleDiskConcentric(vec2 E)
{
    //Rescale input from [0,1) to (-1,1). This ensures the output radius is in [0,1)
    vec2 p = 2.0 * E - 0.99999994;
    vec2 a = abs(p);
    float lo = min(a.x, a.y);
    float hi = max(a.x, a.y);    
    float flag = a.y >= a.x ? 1.0 : 0.0;
    float phi = (PI / 4.0) * (lo / (hi + FLT_EPS) + 2.0 * flag);
    float radius = hi;
    // copy sign bits from p    
    float c = CopySign(cos(phi), p.x);
    float s = CopySign(sin(phi), p.y);
    return vec2(c, s) * radius;
}

half3 SampleWorldNormal(texture2D normalTex, sampler s, vec2 uv) 
{    
#if 0    
    half2 color = textureLod(sampler2D(normalTex, s), uv, 0.0).rg;  
    half4 nn = mad(vec4(color, 1.0, -1.0), vec4(2.0, 2.0, 1.0, 1.0), vec4(-1.0, -1.0, 0.0, 0.0));
    half l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(max(l, 1e-4));    
    return mad(nn.xyz, vec3(2.0), vec3(0.0, 0.0, -1.0));      
#else
    half3 normal = half3_x(textureLod(sampler2D(normalTex, s), uv, 0.0).xyz);
    return normal * half3_t(2.0) - half3_t(1.0);
#endif    
}

half3 SampleWorldNormal(texture2D normalTex, sampler s, vec2 uv, float lod) 
{    
#if 0    
    half2 color = textureLod(sampler2D(normalTex, s), uv, 0.0).rg;  
    half4 nn = mad(vec4(color, 1.0, -1.0), vec4(2.0, 2.0, 1.0, 1.0), vec4(-1.0, -1.0, 0.0, 0.0));
    half l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(max(l, 1e-4));    
    return mad(nn.xyz, vec3(2.0), vec3(0.0, 0.0, -1.0));      
#else
    half3 normal = half3_x(textureLod(sampler2D(normalTex, s), uv, lod).xyz);
    return normal * half3_t(2.0) - half3_t(1.0);
#endif    
}


uvec3 Rand3DPCG16(uvec3 v)
{        
    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    return v >> 16u;
}

uvec3 Rand3DPCG32(uvec3 v)
{        
    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    v ^= v >> 16u;
    
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    return v;
}

vec2 HashRandom(vec2 p, float frameCount)
{
    vec3 p3 = frac(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    vec3 frameMagicScale = vec3(2.083, 4.867, 8.65);
    p3 += frameCount * frameMagicScale;
    return frac((p3.xx + p3.yz) * p3.zy);
}

vec4 CosineSampleHemisphere(vec2 uv)
{
    float phi = TWO_PI * uv.x;
    float cosTheta = sqrt(uv.y);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    //vec3 H;
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    //float z = cosTheta;
    float pdf = cosTheta * INV_PI;

    return vec4(x, y, cosTheta, pdf);
}

half InterleavedGradientNoise(highp vec2 screenPos) 
{
    // http://www.iryoku.com/downloads/Next-Generation-Post-Processing-in-Call-of-Duty-Advanced-Warfare-v18.pptx (slide 123)    
    return half_x(frac(52.9829189 * frac(dot(screenPos, vec2(0.06711056, 0.00583715)))));
}

half InterleavedGradientNoise(highp vec2 uv, float FrameId)
{
    // magic values are found by experimentation
    uv += FrameId * (vec2(47.0, 17.0) * 0.695);    
    return half_x(frac(52.9829189 * frac(dot(uv, vec2(0.06711056, 0.00583715)))));
}

//UnitVectorToOctahedron(normalize(WorldNormal)) * 0.5 + 0.5;
vec2 UnitVectorToOctahedron(vec3 N)
{
    N.xy /= dot(vec3(1.0), abs(N));

    if( N.z <= 0 )
    {
        N.x = (1.0 - abs(N.y)) * (N.x >= 0.0 ? 1.0 : -1.0);
        N.y = (1.0 - abs(N.x)) * (N.y >= 0.0 ? 1.0 : -1.0);            
    }

    return N.xy;
}

vec3 OctahedronToUnitVector(vec2 Oct)
{
    vec3 N = vec3(Oct, 1.0 - (abs(Oct.x) + abs(Oct.y)));
    float t = max(-N.z, 0.0);    
    N.x += N.x >= 0.0 ? -t : t;
    N.y += N.y >= 0.0 ? -t : t;    
    return normalize(N);
}

float GoldNoise(vec2 xy, float seed)
{
    const float PHI = 1.61803398874989484820459; //Golden Ratio 
    return fract(FastSin(distance(xy * PHI, xy) * seed) * xy.x);
}

half3 FsrRcasH(texture2D tex, sampler s, ivec2 sp, float con)
{    
    half3 b = half3_x(texelFetchOffset(sampler2D(tex, s), sp, 0, ivec2(0, -1)).xyz);
    half3 d = half3_x(texelFetchOffset(sampler2D(tex, s), sp, 0, ivec2(-1, 0)).xyz);
    half3 e = half3_x(texelFetch(sampler2D(tex, s), sp, 0).xyz);
    half3 f = half3_x(texelFetchOffset(sampler2D(tex, s), sp, 0, ivec2(1, 0)).xyz);
    half3 h = half3_x(texelFetchOffset(sampler2D(tex, s), sp, 0, ivec2(0, 1)).xyz);

#ifdef FSR_RCAS_DENOISE
    //rb * 0.5 + g
    half bL = dot(b, half3_t(0.5, 1.0, 0.5));
    half dL = dot(d, half3_t(0.5, 1.0, 0.5));
    half eL = dot(e, half3_t(0.5, 1.0, 0.5));
    half fL = dot(f, half3_t(0.5, 1.0, 0.5));
    half hL = dot(h, half3_t(0.5, 1.0, 0.5));  

    half nz = half_x(0.25) * (bL + dL + fL + hL) - eL;
    nz = saturate(abs(nz) / max(max(bL,max(dL,eL)),max(fL,hL))-min(min(bL,min(dL,eL)),min(fL,hL)));
    nz = half_x(-0.5) * nz + half_x(1.0);
#endif    

    half3 mn4RGB = min(min(b,d),min(f,h));
    half3 mx4RGB = max(max(b,d),max(f,h));    

    half3 hitMinRGB = min(mn4RGB, e) / (half3_t(4.0) * mx4RGB);
    half3 hitMaxRGB = (half3_t(1.0) - max(mx4RGB, e))/min(half3_t(4.0) * mn4RGB - half3_t(4.0), half3_t(-1e-4));

    half3 lobeRGB = max(-hitMinRGB, hitMaxRGB);
    const half RCAS_LIMIT = half_x(-0.25 + 1.0 / 16.0);
    half lobe = max(RCAS_LIMIT,min(max(lobeRGB.r,max(lobeRGB.g,lobeRGB.b)), half_x(0.0))) * half_x(con.x);
#ifdef FSR_RCAS_DENOISE
    lobe *= nz;
#endif
    return (lobe * b + lobe * d + lobe * h + lobe * f + e) / (half_x(4.0) * lobe + half_x(1.0));
}

half3 FsrRcasH(texture2D tex, sampler s, vec2 p, float con)
{    
    vec2 size = vec2(textureSize(sampler2D(tex, s), 0));
    ivec2 sp = ivec2(p * size);
    return FsrRcasH(tex, s, sp, con);
}

mat4 CalculateReflectionMatrix(float d)
{
    mat4 m = mat4(1.0,  0.0, 0.0, 0.0,
                  0.0, -1.0, 0.0, 0.0,
                  0.0,  0.0, 1.0, 0.0,
                  0.0, -2.0 * d, 0.0, 1.0);
    return m;
}

