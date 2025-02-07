/*macros.cginc*/	
#define HD_FORWARD          1
#define CLASSIC_DEFERRED    2
#define CLASSIC_FORWARD     3

#define POINT_DEFERRED		1
#define POINT_FORWARD		2
#define POINT_ALPHA   		4

#define MATERIALID_UNLIT                0
#define MATERIALID_STANDARD             1
#define MATERIALID_ANISO                2
#define MATERIALID_TRANSLUCENCY         3
#define MATERIALID_SUBSURFACE           4
#define MATERIALID_SKIN                 5
#define MATERIALID_SKIN_FORWARD         6
#define MATERIALID_CLOTH                7
#define MATERIALID_CLEARCOAT            8
#define MATERIALID_FUR                  9
#define MATERIALID_EYE                  10
#define MATERIALID_HAIR                 11
#define MATERIALID_SKIN_UE4             12
#define MATERIALID_HAIR_UE4             13
#define MATERIALID_CLOTH_FORWARD        14
#define MATERIALID_NODIRECTION          15
#define MATERIALID_TWO_SIDED_FOLIAGE    16
#define MATERIALID_STREE                17
#define MATERIALID_CLOTH_SUBSURFACE     18
#define MATERIALID_HAIR_SUBSURFACE      19
#define MATERIALID_TERRAIN              20
#define MATERIALID_SSSLUT               21
#define MATERIALID_TRESSFX              22
#define MATERIALID_WATER                23

#define MATERIALID_LAMBERT				32
#define MATERIALID_CLOTH_GGX            33
#define MATERIALID_NUM                  34

#define REFLECTION_REFLECT          0
//pbr albedo颜色与specular颜色分开指定, 高光流
#define REFLECTION_SPECULAR         1
//使用金属度电解质系数分解albedo与specular，金属流
#define REFLECTION_METALLIC         2
//没有光照，漫反射高光都是0，材质用自发光贴图
#define REFLECTION_UNLIT            3

#define PLATFORM_DESKTOP	1
#define PLATFORM_MOBILE		2
#define PLATFORM_WIN		4
#define PLATFORM_MAC		8
#define PLATFORM_IOS		16
#define PLATFORM_ANDROID	32

#define VERTEX_POS_INDX             0
#define VERTEX_NORMAL_INDX          1
#define VERTEX_COLOR_INDX           2
#define VERTEX_TANGENT_INDX         3
#define VERTEX_TEX0_INDX            4
#define VERTEX_TEX1_INDX            5
#define VERTEX_TEX2_INDX            6
#define VERTEX_TEX3_INDX            7
#define VERTEX_TEX4_INDX            8
#define VERTEX_TEX5_INDX            9
#define VERTEX_TEX6_INDX            10
#define VERTEX_TEX7_INDX            11
#define VERTEX_TEX8_INDX            12
#define VERTEX_TEX9_INDX            13
#define VERTEX_TEX10_INDX			14

#define VERTEX_BLEND_INDX0          VERTEX_TEX4_INDX
#define VERTEX_BLEND_INDX1          VERTEX_TEX5_INDX

//instancing
#define VERTEX_MATRIX_ROW1_INDX_INSTANCE     10
#define VERTEX_MATRIX_ROW2_INDX_INSTANCE     11
#define VERTEX_MATRIX_ROW3_INDX_INSTANCE     12
#define VERTEX_MATRIX_ROW4_INDX_INSTANCE     13
#define VERTEX_MATRIX_ROW5_INDX_INSTANCE	 14
#define VERTEX_POINT_LIGHT_INDX_INSTANCE     15

#define INSTANCE_ARRAY_COUNT        240
#define UBO_BUFFER_MAX_VEC4_COUNT   1024

#ifdef BONECOUNT_LEVEL
	#if BONECOUNT_LEVEL == 2
		#define MAX_NUM_BONE_PER_SET        660
	#else
		#define MAX_NUM_BONE_PER_SET        240
	#endif
#else
	#define MAX_NUM_BONE_PER_SET        3
#endif

#define VARYING_V2F_VPOS	1
#define VARYING_V2F_NORMAL	2
#define VARYING_VERTEX_UV2	4
#define V2F_UV3				8
#define V2F_UV4				16
#define V2F_UV5				32
#define V2F_UV6				64

#define MRT_NORMAL		1
#define MRT_VELOCITY	2
#define MRT_SUNLIGHT	4
#define MRT_ALBEDO		8

#define TREE_NORMAL 1
#define TREE_GRASS  2
#define TREE_FACINGLEAF 4
#define TREE_LEAF 8
#define TREE_WIND 16

#define VIEWPROBE_SHADOW	1
#define VIEWPROBE_RGB 		2
#define VIEWPROBE_SHADOW_ARRAY 4

#define INSTANCE_NORMAL		1
#define INSTANCE_CLUSTER	2

#define HEIGHT_FOG 4

#define RENDER_TYPE_UNVISABLE   1u
#define RENDER_TYPE_TERRAIN		2u
#define RENDER_TYPE_FORLIAGE	3u
#define RENDER_TYPE_GRASS		4u
#define RENDER_TYPE_STONE		5u
#define RENDER_TYPE_CHARACTER	6u
#define RENDER_TYPE_SCENE_SKIN  7u

#define SHADER_FLOAT16		1
#define SHADER_STORAGE16	2
#define SHADER_SUBGROUPF16  4

#ifndef VARYINGS_MASK
#define VARYINGS_MASK 0
#endif

#ifndef VIEWPROBE_MASK
#define VIEWPROBE_MASK 0
#endif

#ifndef MRT_MASK
#define MRT_MASK 0
#endif

#ifndef SPEEDTREE
#define SPEEDTREE 0
#endif

#ifndef INSTANCE_MASK
#define INSTANCE_MASK 0
#endif

#ifndef LIGHTING_POINT
#define LIGHTING_POINT 0
#endif

#ifndef POSTEFFECT_MASK
#define POSTEFFECT_MASK 0
#endif 

#ifndef SHADER_FEATURES
#define SHADER_FEATURES 0
#endif

#define ALPHATEST_DESCARD_DEF 0.3
#define ALPHA_PLANT_TEST_DESCARD_DEF 0.5

// #if SPEEDTREE != 0
//     #define NUM_USER_TEXCOORDS 8
// #else
//     #define NUM_USER_TEXCOORDS 4
// #endif

#define NUM_USER_TEXCOORDS 4

// Macro To Make SSSS Compile For GLSL
#define SSSS_GLSL_3 1
#define TRUE  1
#define FALSE 0

#define SPECCUBE_LOD_STEPS 6.0

#define PI          3.14159265358979323846
#define TWO_PI      6.28318530717958647693
#define HALF_PI     1.57079632679489661923
#define INV_PI      0.31830988618379067154
#define INV_TWO_PI  0.15915494309189533577

//https://docs.microsoft.com/en-us/windows/win32/dxmath/half-data-type
#define HALF_MAX    65504.0
#define HALF_MIN    6.103515625e-5	// 2^-14, the same value for 10, 11 and 16-bit: https://www.khronos.org/opengl/wiki/Small_Float_Formats
#define HALF_EPS    4.8828125e-4    // 2^-11, machine epsilon: 1 + EPS = 1 (half of the ULP for 1.0f)
#define HALF_MIN_SQRT 0.0078125     // sqrt(HALF_MIN)

#define FLT_MAX		3.402823466e+38
#define FLT_MIN		1.175494351e-38
#define FLT_EPS		5.960464478e-8  // 2^-24, machine epsilon: 1 + EPS = 1 (half of the ULP for 1.0f)
#define FLT_MIN_SQRT 1.084202173e-19

#define COLOR_ERROR vec4(0.86275, 0.0, 0.81961, 1.0)
#define COLOR_PINK2 vec2(0.10588, 0.09804)

#ifndef REVERSE_DEPTH_Z
#define SKY_DEPTH 1.0
#define NEAR_DEPTH 0.0
#define NotSkyDepth(d) d < SKY_DEPTH
#define IsSkyDepth(d) d >= SKY_DEPTH
#define DepthOpLess(d0, d1) d0 < d1
#define DepthOpLequal(d0, d1) d0 <= d1
#define DepthOpGreat(d0, d1) d0 > d1
#define DepthOpGequal(d0, d1) d0 >= d1
#define GetNearDepth(d0, d1) min(d0, d1)
#define GetFarDepth(d0, d1) max(d0, d1)
#else
#define SKY_DEPTH 0.0
#define NEAR_DEPTH 1.0
#define NotSkyDepth(d) d > SKY_DEPTH
#define IsSkyDepth(d) d <= SKY_DEPTH
#define DepthOpLess(d0, d1) d1 < d0
#define DepthOpLequal(d0, d1) d1 <= d0
#define DepthOpGreat(d0, d1) d1 > d0
#define DepthOpGequal(d0, d1) d1 >= d0
#define GetNearDepth(d0, d1) max(d0, d1)
#define GetFarDepth(d0, d1) min(d0, d1)
#endif

#if MRT_MASK == MRT_NORMAL
#define LOC_NORMAL 1
#elif MRT_MASK == MRT_VELOCITY
#define LOC_VELOCITY 1
#elif MRT_MASK == MRT_SUNLIGHT
#define LOC_SUNLIGHT 1
#elif MRT_MASK == MRT_ALBEDO
#define LOC_ALBEDO 1
#elif MRT_MASK == (MRT_NORMAL | MRT_ALBEDO)
#define LOC_NORMAL 1
#define LOC_ALBEDO 2
#elif MRT_MASK == (MRT_NORMAL | MRT_VELOCITY)
#define LOC_NORMAL 1
#define LOC_VELOCITY 2
#elif MRT_MASK == (MRT_NORMAL | MRT_SUNLIGHT)
#define LOC_NORMAL 1
#define LOC_SUNLIGHT 2
#elif MRT_MASK == (MRT_VELOCITY | MRT_ALBEDO)
#define LOC_ALBEDO 1
#define LOC_VELOCITY 2
#elif MRT_MASK == (MRT_VELOCITY | MRT_SUNLIGHT)
#define LOC_VELOCITY 1
#define LOC_SUNLIGHT 2
#elif MRT_MASK == (MRT_NORMAL | MRT_VELOCITY | MRT_SUNLIGHT)
#define LOC_NORMAL 1
#define LOC_VELOCITY 2
#define LOC_SUNLIGHT 3
#elif MRT_MASK == (MRT_NORMAL | MRT_ALBEDO | MRT_VELOCITY)
#define LOC_NORMAL 1
#define LOC_ALBEDO 2
#define LOC_VELOCITY 3
#elif MRT_MASK == (MRT_NORMAL | MRT_ALBEDO | MRT_SUNLIGHT)
#define LOC_NORMAL 1
#define LOC_ALBEDO 2
#define LOC_SUNLIGHT 3
#elif MRT_MASK == (MRT_NORMAL | MRT_ALBEDO | MRT_VELOCITY | MRT_SUNLIGHT)
#define LOC_NORMAL 1
#define LOC_ALBEDO 2
#define LOC_VELOCITY 3
#define LOC_SUNLIGHT 4
#endif

//x * (1 - t) + y * t
#define lerp(x, y, t) mix(x, y, t)
#define frac(x) fract(x)
//使用宏非函数才能保证精度与输入参数一致
#define mul(v, m) ((m) * (v))
#define square(x) ((x) * (x))
#define mad(a, b, c) ((a) * (b) + (c))
//#define mad(a, b, c) fma(a, b, c)
#define rsqrt(x) inversesqrt(x)
#define fmod(x, y) mod(x, y)
#define atan2(x, y) atan(y, x)
#define asuint(f) floatBitsToUint(f)
#define asint(f) floatBitsToInt(f)
#define ddx(x) dFdx(x)
#define ddy(x) dFdy(x)

#define USE_HALF_PRECISION

#ifdef USE_HALF_PRECISION
	#if SHADER_FEATURES & SHADER_FLOAT16
		#extension GL_EXT_shader_16bit_storage:require
		#extension GL_EXT_shader_explicit_arithmetic_types:require		

		#define half float16_t
		#define half2 f16vec2
		#define half3 f16vec3
		#define half4 f16vec4	
		#define A_HALF
	#else
		#define half mediump float
		#define half2 mediump vec2
		#define half3 mediump vec3
		#define half4 mediump vec4		
	#endif

  	#if SHADER_FEATURES & SHADER_SUBGROUPF16
    	#extension GL_EXT_shader_subgroup_extended_types_float16:require
  	#endif
#else
	#define half highp float
	#define half2 highp vec2
	#define half3 highp vec3
	#define half4 highp vec4
#endif

#if 1
#define vecp highp
#else
#define vecp mediump
#endif

#ifdef A_HALF
	#define half2_t f16vec2
	#define half3_t f16vec3
	#define half4_t f16vec4

	#define half_x(x) float16_t(x)
	#define half2_x(x) f16vec2(x)
	#define half3_x(x) f16vec3(x)
	#define half4_x(x) f16vec4(x)

	#define AH3_AF3(x) vec3(x)
	#define AH_AF(x) float(x)

	float saturate(float v) 
	{
		return clamp(v, 0.0, 1.0);
	}

	float Luminance(vec3 LinearColor) 
	{
		return dot(LinearColor, vec3(0.3, 0.59, 0.11));
	}

	float Luminance(vec4 LinearColor) 
	{
		return dot(LinearColor.rgb, vec3(0.3, 0.59, 0.11));
	}

	half Luminance(half3_t LinearColor) 
	{
		return dot(LinearColor, half3_t(0.3, 0.59, 0.11));
	}

	half Luminance(half4_t LinearColor) 
	{
		return dot(LinearColor.rgb, half3_t(0.3, 0.59, 0.11));
	}

	#define DEFINE_SATURATE(t) t saturate(t v) {return clamp(v, t(0.0), t(1.0));}

	DEFINE_SATURATE(vec2)
	DEFINE_SATURATE(vec3)
	DEFINE_SATURATE(vec4)
	DEFINE_SATURATE(half)
	DEFINE_SATURATE(half2)
	DEFINE_SATURATE(half3)
	DEFINE_SATURATE(half4)

	float rcp(float x) 
	{
		return 1.0 / (x);
	}

	#define DEFINE_RCP(t) t rcp(t v) {return t(1.0) / (v);}

	DEFINE_RCP(vec2)
	DEFINE_RCP(vec3)
	DEFINE_RCP(vec4)
	DEFINE_RCP(half)
	DEFINE_RCP(half2)
	DEFINE_RCP(half3)
	DEFINE_RCP(half4)
#else
	#define half2_t vec2
	#define half3_t vec3
	#define half4_t vec4

	#define half_x(x) (x)
	#define half2_x(x) (x)
	#define half3_x(x) (x)
	#define half4_x(x) (x)

	#define AH3_AF3(x) (x)
	#define AH_AF(x)   (x)

	#define log10(x) (log2(x) * 0.30103001)
	#define saturate(v) clamp(v, 0.0, 1.0)
	#define rcp(x) 1.0 / (x)
	#define Luminance(LinearColor) dot(LinearColor.rgb, vec3(0.3, 0.59, 0.11))
#endif

//调用者保护pow, 不要全体替换基础函数. pow 会被转换为exp2(y * log2(x)), log2(0) 为-INF
//#define pow(x, y) pow(max(x, 1e-3), y)  //prevent NaNs possible cases of pow(0, 0)

// pow5 uses the same amount of instructions as generic pow(), but has 2 advantages:
// 1) better instruction pipelining
// 2) no need to worry about NaNs
#define pow2(x) ((x) * (x))
#define pow3(x) ((x) * (x) * (x))
#define pow4(x) ((x)*(x) * (x)*(x))
#define pow5(x) ((x)*(x) * (x)*(x) * (x))
#define pow6(x) ((x)*(x) * (x)*(x) * (x)*(x))

#define min3(a, b, c) min(a, min(b, c))
#define max3(a, b, c) max(a, max(b, c))

#define Luma4(c) (c.g * 2.0 + c.r + c.b)
#define cossin(angle) sin(vec2(angle + HALF_PI, angle))
#define sincos(angle) sin(vec2(angle, angle + HALF_PI))

#if 1
// Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
#define GammaToLinear(sRGB) (sRGB) * ((sRGB) * ((sRGB) * 0.305306011 + 0.682171111) + 0.012522878)
#else
#define GammaToLinear(sRGB) (sRGB) * (sRGB)
#endif
#define PARAMTER_TEXCOORD(PARAM, slot) (PARAM.TexCoords[slot])

#if SHADER_API == 300
	#define UNIFORM_BINDING(x) layout(std140)
#elif SHADER_API == 310
	#define UNIFORM_BINDING(x) layout(binding = x, std140)	
#endif

#if SHADER_API == 450
	#define UNIFORM_OUT(x) 		layout(location = x) out
	#define UNIFORM_FLAT_OUT(x)	layout(location = x) flat out
	#define UNIFORM_IN(x) 		layout(location = x) in
	#define UNIFORM_FLAT_IN(x)	layout(location = x) flat in

	#define gl_InstanceID gl_InstanceIndex
#else
	#define UNIFORM_OUT(x)	out
	#define UNIFORM_FLAT_OUT(x)	flat out
	#define UNIFORM_IN(x)	in
	#define UNIFORM_FLAT_IN(x)	flat in
#endif

//opengl 左手矩阵
#define OPENGL_MATRIX_LH    

#ifdef OPENGL_MATRIX_LH
#if SHADER_API == 450
//vulkan获取ndcZ
#if 1
#define SAMPLE_DEPTH_TEXTURE_LOD(t, s, uv) textureLod(sampler2D(t, s), uv, 0.0).r
#else
highp float SAMPLE_DEPTH_TEXTURE(highp texture2D t, sampler s, highp vec2 uv) 
{	
	highp vec2 size = vec2(textureSize(sampler2D(t, s), 0));
	highp ivec2 iUV = ivec2(size * uv);
	return texelFetch(sampler2D(t, s), iUV, 0).r;		
}
#endif
#else
//通过深度纹理，获取GL ndc Z值[-1,1] 
#define SAMPLE_DEPTH_TEXTURE_LOD(sampler, s, uv) (textureLod(sampler, uv, 0.0).r * 2.0 - 1.0)
#endif

#define PrepareNDCZOut(p) (p)
#else
//DX左手ndc Z[0,1]
#define SAMPLE_DEPTH_TEXTURE_LOD(sampler, s, uv) textureLod(sampler, uv, 0.0).r

// 这个转换操作对于GL是必须的不能删除，但不能应用于VK
highp vec4 PrepareNDCZOut(highp vec4 p)
{
	highp vec4 pos = p;
	pos.z = dot(pos.zw, vec2(2.0, -1.0));
	return pos;
}
#endif

//左手投影变换对viewZ的变换
highp float LinearEyeDepth(highp float ndcZ, highp float n, highp float f)
{	
#ifdef REVERSE_DEPTH_Z	
	return (n * f) / (n + ndcZ * (f - n));	
#else	
	return (n * f) / (f - ndcZ * (f - n));	
#endif	
}

highp float LinearDepthToNDC(highp float linearZ, highp float n, highp float f)
{
#ifdef REVERSE_DEPTH_Z	
	return ((n * f) / linearZ - n) / (f - n);
#else
	return ((n * f) / linearZ - n) / (n - f) ;
#endif
}

highp float LinearEye01Depth(highp float ndcZ, highp float n, highp float f)
{	
#ifdef REVERSE_DEPTH_Z	
	return n / (n + ndcZ * (f - n));	
#else	
	return n / (f - ndcZ * (f - n));	
#endif	
}

#if 1
//FMA cycles 减少25%
highp vec4 mulp4(highp vec4 p, highp mat4 m)
{	
	return mad(vec4(p.x), m[0], mad(vec4(p.y), m[1], mad(vec4(p.z), m[2], m[3])));
}

highp vec4 mulp4(highp vec3 p, highp mat4 m)
{	
	return mad(vec4(p.x), m[0], mad(vec4(p.y), m[1], mad(vec4(p.z), m[2], m[3])));
}
#else
highp vec4 mulp4(highp vec4 p, highp mat4 m)
{
	return mul(p, m);
}
#endif

void nop() {}

