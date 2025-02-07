// brdf.cginc
// Physically based shading model
// parameterized with the below options

//________________________________________________________________________
// Diffuse model
// 0: Lambert
// 1: Burley
// 2: Oren-Nayar
// 3: None
#define PHYSICAL_DIFFUSE	0
// Microfacet distribution function
// 0: Blinn
// 1: Beckmann
// 2: GGX
#define PHYSICAL_SPEC_D		2

// Geometric attenuation or shadowing
// 0: Implicit
// 1: Neumann
// 2: Kelemen
// 3: Schlick
// 4: Smith (matched to GGX)
// 5: SmithJointApprox (matched to GGX)
#define PHYSICAL_SPEC_G		5

// Fresnel
// 0: None
// 1: Schlick
// 2: Fresnel
#define PHYSICAL_SPEC_F		1
//________________________________________________________________________



// [Burley 2012, "Physically-Based Shading at Disney"]
vec3 Diffuse_Burley(vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH)
{
	float FD90 = 0.5 + 2.0 * VoH * VoH * Roughness;
	float FdV = 1.0 + (FD90 - 1.0) * exp2((-5.55473 * NoV - 6.98316) * NoV);
	float FdL = 1.0 + (FD90 - 1.0) * exp2((-5.55473 * NoL - 6.98316) * NoL);
	return DiffuseColor * INV_PI * FdV * FdL;
}

// [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
vec3 Diffuse_OrenNayar(vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH)
{
	float VoL = 2.0 * VoH - 1.0;
	float m = Roughness * Roughness;
	float m2 = m * m;
	float C1 = 1.0 - 0.5 * m2 / (m2 + 0.33);
	float Cosri = VoL - NoV * NoL;
	float C2 = 0.45 * m2 / (m2 + 0.09) * Cosri * (Cosri >= 0.0 ? min(1.0, NoL / NoV) : NoL);
	return DiffuseColor / PI * (NoL * C1 + C2);
}

mediump vec3 Diffuse_ScatterHair(mediump vec3 DiffuseColor, mediump float Scatter, mediump float Shadow, mediump float NoV, mediump vec3 N, mediump vec3 L, mediump vec3 V)
{	
	mediump vec3 NewN = normalize(V - N * NoV);

	// Hack approximation for multiple scattering.
	mediump float Wrap = 1.0;
	mediump float NoL = saturate((dot(NewN, L) + Wrap) / square(1.0 + Wrap));
	mediump float DiffuseScatter = INV_PI * NoL * Scatter;
	mediump float Luma = max(dot(DiffuseColor, vec3(0.3, 0.59, 0.11)), 1e-3);
	mediump vec3 Diffuse = max(DiffuseColor / Luma, vec3(1e-3));
	mediump vec3 ScatterTint = pow(Diffuse, vec3(1.001 - Shadow));
	mediump vec3 ScatterColor = sqrt(DiffuseColor) * DiffuseScatter * ScatterTint;	
	
	return clamp(ScatterColor, vec3(0.0), vec3(20.0));
}

//from uncharted 4: The Process of Creating Volumetric-based Materials in Uncharted 4 (P22)
half3 Diffuse_CheapSubSurfaceScattering(half3 DiffuseColor, half Wrap, highp float NoLNosaturate)
{	
	half Diffuse = INV_PI * saturate(NoLNosaturate + Wrap) / (1.0 + Wrap);
	half3 ScatterLight = saturate(DiffuseColor + saturate(NoLNosaturate)) * Diffuse;
	return DiffuseColor * ScatterLight;
}

// [Blinn 1977, "Models of light reflection for computer synthesized pictures"]
float D_Blinn(float Roughness, float NoH)
{
	float m = Roughness * Roughness;
	float m2 = m * m;
	float n = 2.0 / m2 - 2.0;
	return (n + 2.0) / 2.0 * pow(max(abs(NoH), 0.000001f), n);		// 1 mad, 1 exp, 1 mul, 1 log
}

// [Beckmann 1963, "The scattering of electromagnetic waves from rough surfaces"]
float D_Beckmann(float Roughness, float NoH)
{
	float m = Roughness * Roughness;
	float m2 = m * m;
	float NoH2 = NoH * NoH;
	return exp((NoH2 - 1.0) / (m2 * NoH2)) / (m2 * NoH2 * NoH2);
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
half D_GGX(half Roughness, highp float NoH)
{
	half m = Roughness * Roughness;
	half m2 = m * m;
	highp float d = (NoH * m2 - NoH) * NoH + 1.0;	// 2 mad, must highp
	d = m2 / (d * d * PI + 1e-7);			// 2 mul, 1 rcp	
	return d;
}

half D_InvGGX(half Roughness, highp float NoH)
{
	half m = Roughness * Roughness;
	half m2 = m * m;
	highp float d = (NoH - m2 * NoH) * NoH + m2;
	d = (1.0 + 4.0 * m2 * m2 / (d * d + 1e-7)) / (PI * (1.0 + 4.0 * m2));	
	return d;
}

// Anisotropic GGX, right version
// [Burley 2012, "Physically-Based Shading at Disney"]
float D_GGXaniso(float RoughnessX, float RoughnessY, float NoH, vec3 H, vec3 X, vec3 Y)
{
	float mx = RoughnessX * RoughnessX;
	float my = RoughnessY * RoughnessY;
	float XoH = dot(X, H);
	float YoH = dot(Y, H);
	float d = XoH * XoH / (mx*mx) + YoH * YoH / (my*my) + NoH * NoH;
	return 1.0 / (mx*my * d*d) * INV_PI;
}

// Anisotropic GTR2
highp float GTR2_aniso(highp float NdotH, highp float HdotX, highp float HdotY, highp float ax, highp float ay)
{
	//bug->square(HdotX / ax)
	return 1.0 / (PI * ax * ay * square(square(HdotX / ax) + square(HdotY / ay) + NdotH * NdotH));
}

half D_GTR2aniso(highp float Roughness, highp float Anisotropic, highp float NoH, highp vec3 H, highp vec3 B, highp vec3 T)
{
	highp float aspect = sqrt(1.0 - Anisotropic * 0.9);
	highp float a = Roughness * Roughness;
	highp float ax = max(1e-4, a / aspect);
	highp float ay = max(1e-4, a * aspect);    
	return GTR2_aniso(NoH, dot(H, B), dot(H, T), ax, ay);
}

// Kajiya-Kay & Blinn
// http://web.engr.oregonstate.edu/~mjb/cs519/Projects/Papers/HairRendering.pdf
// https://github.com/wdas/brdf/blob/master/src/brdfs/d_blinnphong.brdf
//头发竖着摆则使用B，横着摆则使用T
float StrandSpecular(vec3 T, vec3 V, vec3 L, float exponent)
{
	highp vec3 H = normalize(L + V);
	float dotTH = dot(T, H);
	highp float sinTH = sqrt(max(0.0, 1.0 - dotTH * dotTH));
	highp float dirAtten = smoothstep(-1.0, 0.0, dotTH);

	//	return sinTH;
	sinTH = max(1e-3, sinTH);
	return dirAtten * pow(sinTH, exponent);
}

float D_KajiyaKay(float Roughness, vec3 Tangent, vec3 L, vec3 V)
{
	highp float m = Roughness * Roughness;
	highp float m2 = m * m + 1e-4;
	float n = clamp(2.0 / m2 - 2.0, 1.0, 1000.0);
	float scale = 1.0;//0.25f;

	return (n + 2.0) / 2.0 * StrandSpecular(Tangent, V, L, n) * INV_PI * scale;
}

float G_Implicit()
{
	return 0.25;
}

// [Neumann et al. 1999, "Compact metallic reflectance models"]
float G_Neumann(float NoV, float NoL)
{
	return 1.0 / (4.0 * max(NoL, NoV));
}

// [Kelemen 2001, "A microfacet based coupled specular-matte brdf model with importance sampling"]
float G_Kelemen(vec3 L, vec3 V)
{
	return 1.0 / (2.0 + 2.0 * dot(L, V));
}

// Tuned to match behavior of G_Smith
// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float G_Schlick(float Roughness, float NoV, float NoL)
{
	float k = Roughness * Roughness * 0.5;
	float G_SchlickV = NoV * (1.0 - k) + k;
	float G_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (G_SchlickV * G_SchlickL);
}

// Smith term for GGX modified by Disney to be less "hot" for small roughness values
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
// [Burley 2012, "Physically-Based Shading at Disney"]
float G_Smith(float Roughness, float NoV, float NoL)
{		
	float a = Roughness * Roughness;
	float a2 = a * a;

	float G_SmithV = NoV + sqrt(NoV * (NoV - NoV * a2) + a2);
	float G_SmithL = NoL + sqrt(NoL * (NoL - NoL * a2) + a2);
	return 1.0 / (G_SmithV * G_SmithL + 1e-4);  //感觉这里错了，0.5 / (G_SmithV * G_SmithL + 1e-4)
}

//topameng: should be this
half Vis_Smith(half roughness, half NdotV, half NdotL)
{
    half a = roughness * roughness;
    float a2 = a * a;

    float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    
    return 0.5 / max(lambdaV + lambdaL, 1e-4); 
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
half Vis_SmithJointApprox(half roughness, half nv, half nl)
{
    half a = roughness * roughness;
    half Vis_SmithV = nl * (nv * (1.0 - a) + a);
    half Vis_SmithL = nv * (nl * (1.0 - a) + a);
    // Note: will generate NaNs with roughness = 0.  MinRoughness is used to prevent this
    return 0.5 / max(Vis_SmithV + Vis_SmithL, 1e-4); //mediump must >= 1e-4
}

// Ref: https://cedec.cesa.or.jp/2015/session/ENG/14698.html The Rendering Materials of Far Cry 4
half Vis_SmithJointGGXAniso(half TdotV, half BdotV, half nv, half TdotL, half BdotL, half nl, half roughnessT, half roughnessB) 
{
    // Expects roughnessT and roughnessB to be squared.
    half lambdaV = nl * sqrt(roughnessT * TdotV * TdotV + roughnessB * BdotV * BdotV + nv * nv);
    half lambdaL = nv * sqrt(roughnessT * TdotL * TdotL + roughnessB * BdotL * BdotL + nl * nl);
    // As it might error on dx11 using forward lighting:    
    return 0.5 / max(1e-4, lambdaV + lambdaL);
}

mediump vec3 F_None(mediump vec3 SpecularColor)
{
	return SpecularColor;
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
// [Lagarde 2012, "Spherical Gaussian approximation for Blinn-Phong, Phong and Fresnel"]
mediump vec3 F_Schlick2(mediump vec3 SpecularColor, highp float VoH)
{
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return SpecularColor + clamp(((50.0 * SpecularColor.g) - SpecularColor) * exp2((-5.55473 * VoH - 6.98316) * VoH), 0.0, 1.0);

	//float Fc = exp2( (-5.55473 * VoH - 6.98316) * VoH );	// 1 mad, 1 mul, 1 exp
	//return Fc + (1 - Fc) * SpecularColor;					// 1 add, 3 mad
}

half3 F_Schlick(half3 f0, half vh)
{
    half fc = pow5(1.0 - vh);		
    return (1.0 - fc) * f0 + fc;		
}

mediump vec3 F_Fresnel(mediump vec3 SpecularColor, mediump float VoH)
{
	mediump vec3 SpecularColorSqrt = sqrt(saturate(SpecularColor));
	mediump vec3 n = (vec3(1.0) + SpecularColorSqrt) / (vec3(1.0) - SpecularColorSqrt);
	mediump vec3 g = sqrt(n*n + VoH * VoH - vec3(1.0));
	return 0.5 * sqrt((g - VoH) / (g + VoH)) * (1.0 + sqrt(((g + VoH)*VoH - vec3(1.0)) / ((g - VoH)*VoH + vec3(1.0))));
}

half G_Cloth(half NoV, half NoL)
{	
	return 0.25 / (NoL + NoV - NoL * NoV + 1e-4);
}

mediump vec3 Diffuse(mediump vec3 DiffuseColor, mediump float Roughness, mediump float NoV, mediump float NoL, mediump float VoH)
{	
#if   PHYSICAL_DIFFUSE == 0    
    return INV_PI * NoL * DiffuseColor;
#elif PHYSICAL_DIFFUSE == 1
    return Diffuse_Burley( DiffuseColor, Roughness, NoV, NoL, VoH );
#elif PHYSICAL_DIFFUSE == 2
    return Diffuse_OrenNayar( DiffuseColor, Roughness, NoV, NoL, VoH );
#elif PHYSICAL_DIFFUSE == 3
    return vec3(0.0);
#endif
}

highp float Distribution(highp float Roughness, highp float NoH)
{
#if (PHYSICAL_SPEC_D == 0)
	return D_Blinn(Roughness, NoH);
#elif (PHYSICAL_SPEC_D == 1)
	return D_Beckmann(Roughness, NoH);
#elif (PHYSICAL_SPEC_D == 2)
	return D_GGX(Roughness, NoH);
#endif
}

float GeometricVisibility(float Roughness, float NoV, float NoL, float VoH, vec3 L, vec3 V)
{
#if (PHYSICAL_SPEC_G == 0)
	return G_Implicit();
#elif (PHYSICAL_SPEC_G == 1)
	return G_Neumann(NoV, NoL);
#elif (PHYSICAL_SPEC_G == 2)
	return G_Kelemen(L, V);
#elif (PHYSICAL_SPEC_G == 3)
	return G_Schlick(Roughness, NoV, NoL);
#elif (PHYSICAL_SPEC_G == 4)
	return Vis_Smith(Roughness, NoV, NoL);
#elif PHYSICAL_SPEC_G == 5	
	return Vis_SmithJointApprox(Roughness, NoV, NoL);
#endif
}

half3 Fresnel(half3 SpecularColor, half VoH)
{
#ifdef TWOSIDE
	return SpecularColor;
#endif

#if PHYSICAL_SPEC_F == 0
	return SpecularColor;
#elif PHYSICAL_SPEC_F == 1
	return F_Schlick(SpecularColor, VoH);
#elif PHYSICAL_SPEC_F == 2
	return F_Fresnel(SpecularColor, VoH);
#endif
}

vec3 fix_cube_lookup(vec3 v, float roughness)
{
	float cube_size = 256.0f;
	float M = max(max(abs(v.x), abs(v.y)), abs(v.z));	
	float scale = 1.0 - exp2(roughness*8.0) / cube_size;
	if (abs(v.x) != M) v.x *= scale;
	if (abs(v.y) != M) v.y *= scale;
	if (abs(v.z) != M) v.z *= scale;
	return v;
}

mediump vec3 EnvBRDFApprox(mediump vec3 SpecularColor, float Roughness, float NoV)
{
	// [ Lazarov 2013, "Getting More Physical in Call of Duty: Black Ops II" ]
	// Adaptation to fit our G term.
	const mediump vec4 c0 = vec4(-1.0, -0.0275, -0.573, 0.0229);
	const mediump vec4 c1 = vec4(1.0, 0.0425, 1.0417, -0.0417);
	mediump vec4 r = Roughness * c0 + c1;
	mediump float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	mediump vec2 AB = vec2(-1.0417, 1.0417) * a004 + r.zw;

	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	// Note: this is needed for the 'specular' show flag to work, since it uses a SpecularColor of 0
	//AB.y *= clamp(50.0 * SpecularColor.g, 0.0, 1.0);

	return SpecularColor * AB.x + AB.y;
}

mediump vec3 EnvBRDFGGX(mediump vec3 specColor, float roughness, float nv)
{
    const mediump vec4 c0 = vec4(-1.0, -0.0275, -0.26, 0.0109);
    const mediump vec4 c1 = vec4(1.0, 0.0455, 1.0417, -0.0417);
    mediump vec4 r = roughness * c0 + c1;    
    mediump float a004 = min(r.x * 0.6, pow5(1.0 - nv)) * r.x + r.y;
    mediump vec2 AB = vec2(-1.0417, 1.0417) * a004 + r.zw;		
    //AB.y *= clamp(50.0 * specColor.g, 0.0, 1.0);		
    return AB.x * specColor + AB.y;
}

// Same as EnvBRDFGGX(0.04, Roughness, NoV )    
mediump vec3 EnvBRDFGGXNonmetal(mediump float roughness, mediump float nv) 
{               
    const mediump vec2 c0 = vec2(-1.0, -0.0275); 
    const mediump vec2 c1 = vec2(1.0, 0.0455); 
    mediump vec2 r = c0 * roughness + c1; 
    return vec3(min(r.x * 0.6, pow5(1.0 - nv)) * r.x + r.y);
}

mediump float ComputeTranslucency(mediump vec3 L, mediump vec3 V, mediump vec3 N, mediump float Distortion)
{		
	mediump vec3 H = L + N * Distortion; //normalize ?
	mediump float VoH = saturate(dot(V, -H));
	return pow3(VoH) * pow3(VoH);	
}

mediump float ComputeTranslucency_NoL(mediump float NoLNosaturate)
{	
	mediump float x = saturate(-NoLNosaturate);	
	return pow3(x) * pow3(x);
}

vec3 GetBlinnPhongSpecular(vec3 SpecularBaseColor, float Roughness, float NoH, float Ns)
{
	return Roughness * SpecularBaseColor * pow(NoH, Ns);
}

/*****************************************************************************/
/*      Normalize float pack tools: Anisotropic Pack Application             */
/*      EntryFunction: PackAnisoParamToFloat2/UnPackAnisoParamFromFloat2     */
/*      Author: WuTao                                                        */
/*****************************************************************************/
#define FPOW_TWO(n) ( pow(2.0, float(n)) )

void NFloat_PackOneToTwo(in  float sourceValue,
	const int bitI,
	const int bitF,
	out float packI,
	out float packF)
{
	int bitEnd = bitI + bitF;
	float lip = 0.001;
	packF = modf(sourceValue * FPOW_TWO(bitI) - lip, packI);
	packI *= (1.0 / (FPOW_TWO(bitI) - 1.0));
	packF += lip;
}

void NFloat_UnPackOneFromTwo(out float destValue,
	const int bitI,
	const int bitF,
	in  float packI,
	in  float packF)
{
	int bitEnd = bitI + bitF;
	destValue = packI * (FPOW_TWO(bitI) - 1.0) / FPOW_TWO(bitI) + packF * (1.0 / FPOW_TWO(bitI));
}

void NFloat_PackTwoToOne(in  float sourceValue0,
	in  float sourceValue1,
	const int bitWide0,
	const int bitWide1,
	const int bitMax,
	out float packValue)
{
	int bitEnd = bitWide0 + bitWide1;
	float sourceValue0_t = sourceValue0 * (FPOW_TWO(bitWide0) - 1.0);
	float sourceValue1_t = sourceValue1 * (FPOW_TWO(bitWide1) - 1.0);
	packValue = round(sourceValue1_t) * FPOW_TWO(bitWide0);
	packValue *= 1.0 / (FPOW_TWO(bitMax) - 1.0);
	packValue += (sourceValue0_t + 0.5) / (FPOW_TWO(bitMax) - 1.0);
}

void NFloat_UnPackTwoFromOne(out float destValue0,
	out float destValue1,
	const int bitWide0,
	const int bitWide1,
	const int bitMax,
	in  float packValue)
{
	int bitEnd = bitWide0 + bitWide1;
	float packValue_t = (packValue * (FPOW_TWO(bitMax) - 1.0));
	destValue1 = floor((packValue_t) / FPOW_TWO(bitWide0)) * FPOW_TWO(bitWide0);
	destValue0 = packValue_t - destValue1;
	destValue1 *= (1.0 / (FPOW_TWO(bitEnd) - FPOW_TWO(bitWide0)));
	destValue0 *= (1.0 / (FPOW_TWO(bitWide0) - 2.0));
}

float packTangent(in vec3 normal, in vec3 tangent)
{
	/*****************/
	vec4  refDirection;
	if (abs(dot(vec3(1.0, 0.0, 0.0), normal)) < 0.5)
		refDirection = vec4(1.0, 0.0, 0.0, 0.0);
	else
		refDirection = vec4(0.0, 1.0, 0.0, 0.5);
	/*****************/

	vec3  proj_X = normalize(refDirection.xyz - normal * dot(refDirection.xyz, normal));
	vec3  projTangent = tangent;
	vec3  proj_Y = cross(normal, proj_X);

	float x = dot(projTangent, proj_X);
	float y = dot(projTangent, proj_Y);

	float a = atan(y / x);

	return (a / (3.15 * 2.0) + 0.5) / 2.0 + refDirection.w;

}

vec3 UnpackTangent(vec3 normal, float radian)
{
	/*****************/
	vec4  refDirection;
	if (radian <= 0.5)
		refDirection = vec4(1.0, 0.0, 0.0, 0.0);
	else
		refDirection = vec4(0.0, 1.0, 0.0, 0.5);

	radian -= refDirection.w;
	/*****************/

	vec3  proj_X = normalize(refDirection.xyz - normal * dot(refDirection.xyz, normal));
	vec3  proj_Y = cross(normal, proj_X);

	radian = (radian*2.0 - 0.5) * 3.15 * 2.0;
	vec3  projTangent = cos(radian) * proj_X + sin(radian)*proj_Y;

	return projTangent;
}

/**********************************************************************/
/*    |---- 8 ----|        |--- 4 ---|--- 4 ---|                      */
/*    | tangent-i |        |tangent-f|aniso power|                    */
vec2 PackAnisoParamToFloat2(in vec3 normal, in vec3 tangent, in float anisoPower)
{
	vec2 packVector = vec2(1.0, 1.0);

	float  packTgt = packTangent(normal, tangent);

	float  ti, tf;
	NFloat_PackOneToTwo(packTgt, 8, 4, ti, tf);
	packVector.x = ti;
	NFloat_PackTwoToOne(tf, anisoPower, 4, 4, 8, packVector.y);


	/*
	packVector.y = modf(packTgt * 255, packVector.x);
	packVector.x *= 1./255.;
	packVector.y *= 15./255.;

	float packPower = floor(anisoPower / (16./255.)) * (16./255.);
	packVector.y += packPower;
	/**/

	return packVector;
}

void UnPackAnisoParamFromFloat2(in vec2 packVector, in vec3 normal, out vec3 tangent, out float anisoPower)
{
	/*
	float  packTgt = packVector.x;

	float  y = packVector.y;
	anisoPower = floor(y / (16./255.)) * (16./255.);

	packTgt += (y - anisoPower) / (15./255.) * (1./255);
	tangent = UnpackTangent(normal, packTgt);
	*/
	float  t, tf;
	NFloat_UnPackTwoFromOne(tf, anisoPower, 4, 4, 8, packVector.y);
	NFloat_UnPackOneFromTwo(t, 8, 4, packVector.x, tf);
	tangent = UnpackTangent(normal, t);
}

/*mediump vec3 DeflectNormalBaseOnAnisoDistribution(mediump vec3 Normal, highp vec3 EyeToWorld, highp vec3 Tangent, mediump float Distribution)
{
	highp vec3 ReflectTangent = cross(EyeToWorld, Tangent);
	highp vec3 ReflectNormal = cross(ReflectTangent, Tangent);	
	return normalize(lerp(Normal, ReflectNormal, Distribution * 0.9));
}*/


//SDTV with BT.470
mediump vec3 EncodeRGBToYUV(mediump vec3 C)
{
	//column
	const mediump mat3 YUVOutputMatrix = mat3
	(
		0.299, 	  0.587, 	0.114,
		-0.14713, -0.28886, 0.436,
		0.615, 	  -0.51499, -0.10001
	);
	
	return mul(YUVOutputMatrix, C);
	
	/*mediump float Y = 0.299 * C.r + 0.587 * C.g + 0.114 * C.b;
	mediump float U = -0.14713 * C.r - 0.28886 * C.g + 0.436 * C.b;
	mediump float V = 0.615 * C.r - 0.51499 * C.g - 0.10001 * C.b;
	return vec3(Y, U, V);*/
}

mediump vec3 DecodeRGBFromYUV(mediump vec3 C)
{
	//mediump float Y = C.r;
	//mediump float U = C.g;
	//mediump float V = C.b;

	//mediump float R = Y + 1.13983 * V;
	//mediump float G = Y - 0.39465 * V - 0.5806 * U;
	//mediump float B = Y + 2.03211 * U;
	//return vec3(R, G, B);

	const mediump mat3 YUVInputMatrix = mat3
	(
		1.0, 0.0, 	  	1.13983,
		1.0, -0.39465, -0.5806,
		1.0, 2.03211,   0.0
	);

	return mul(YUVInputMatrix, C);
}

mediump vec2 EncodeRGB888ToRGB565(mediump vec3 color)
{    
    color = floor(color * vec3(31.0, 63.0, 31.0) + 0.5);
    mediump float x = floor(color.y / 8.0);
    mediump float y = color.y - x * 8.0;
    return vec2(x, y) * 32.0 / 255.0 + color.xz / 255.0;
}

mediump vec2 _EncodeRGB888ToRGB565(mediump vec3 color)
{    
	mediump int r = int(color.r * 31.0 + 0.5);
	mediump int g = int(color.g * 63.0 + 0.5);
	mediump int b = int(color.b * 31.0 + 0.5);
	
	mediump int x = ((g & 0x38) << 2) | r;
    mediump int y = ((g & 0x07) << 5) | b;

	return vec2(float(x), float(y)) / 255.0;
}

mediump vec3 DecodeRGB888FromRBG565(mediump vec4 enc)
{
    mediump vec2 color = floor(enc.zw * 255.0 + 0.5);  
    mediump vec2 gHighAndLow = floor(color / 32.0);
    mediump vec2 rb = color - gHighAndLow * 32.0;
    mediump float g = gHighAndLow.x * 8.0 + gHighAndLow.y;//floor(color.x / 32.0) * 8.0 + floor(color.y / 32.0);
    return vec3(rb.x, g, rb.y) * vec3(1.0/31.0, 1.0/63.0, 1.0/31.0);
}

mediump vec3 _DecodeRGB888FromRBG565(mediump vec4 enc)
{
	mediump int x = int(enc.z * 255.0 + 0.5);
	mediump int y = int(enc.w * 255.0 + 0.5);

	mediump int r = x & 0x1f;
    mediump int g = ((x >> 5) << 3) | (y >> 5);
    mediump int b = y & 0x1f;
	
	return (vec3(float(r), float(g), float(b))) * vec3(1.0/31.0, 1.0/63.0, 1.0/31.0);
}

mediump vec2 UnitVectorToOctahedron(mediump vec3 N)
{
    N.xy /= dot(vec3(1.0), abs(N));

    if(N.z <= 0.0)
    {    	   
        N.xy = (vec2(1.0) - abs(N.yx)) * (N.x >= 0.0 && N.y >= 0.0 ? vec2(1.0) : vec2(-1.0));
    }

    return N.xy;	
}

mediump vec3 OctahedronToUnitVector(mediump vec2 Oct)
{
	mediump vec3 N = vec3(Oct, 1.0 - dot(vec2(1.0), abs(Oct)));
    
    if(N.z < 0.0)
    {    	
        N.xy = (vec2(1.0) - abs(N.yx)) * (N.x >= 0.0 && N.y >= 0.0 ? vec2(1.0) : vec2(-1.0));
    }

    return normalize(N);
}

mediump vec2 StereographicEncode(mediump vec3 n)
{
    mediump float scale = 1.7777;
    mediump vec2 enc = n.xy / (n.z + 1.0);
    enc /= scale;
    enc = enc * 0.5 + 0.5;
    return vec2(enc);
}

mediump vec3 StereographicDecode(mediump vec2 enc)
{
    mediump float scale = 1.7777;
    mediump vec3 nn = vec3(enc.xy, 0.0) * vec3(2.0 * scale, 2.0 * scale, 0.0) + vec3(-scale, -scale, 1.0);
    mediump float g = 2.0 / dot(nn.xyz,nn.xyz);
    mediump vec3 n;
    n.xy = g * nn.xy;
    n.z = g - 1.0;
    return n;
}

mediump vec2 SpheremapEncode(mediump vec3 n)
{				
	mediump vec2 enc = normalize(n.xy) * sqrt(-n.z * 0.5 + 0.5);
	return enc * 0.5 + 0.5;

    //两个12bit存入rgb
	//mediump vec2 enc255 = enc * 255.0;
	//mediump vec2 residual = floor(frac(enc255) * 16.0);
	//mediump vec3 enc = vec3(floor(enc255), residual.x * 16.0 + residual.y) / 255.0;
}

mediump vec3 SpheremapDecode(mediump vec4 enc)
{
	//24bits:mediump vec3 c;
	//float nz = floor(c.z * 255.0) / 16.0;
	//mediump vec2 enc = c.xy + vec2(floor(nz) / 16.0, frac(nz)) / 255.0;

	mediump vec4 nn = vec4(enc.xy, 1.0, -1.0) * vec4(2.0, 2.0, 1.0, 1.0) + vec4(-1.0, -1.0, 0.0, 0.0);    
	mediump float l = dot(nn.xyz,-nn.xyw);
	nn.z = l;
	nn.xy *= sqrt(max(l, 1e-3));    
	mediump vec3 n = nn.xyz * 2.0 + vec3(0.0, 0.0, -1.0);        
	return normalize(n);
}

mediump vec2 EncodeNormal888To871(mediump vec3 N)
{
	mediump vec2 n = N.xy * vec2(0.5, 0.5 * 127.0) + vec2(0.5, 0.5 * 127.0 + 0.5);
	mediump float packY = floor(n.y) * 2.0 + sign(N.z);
	return vec2(n.x, packY / 255.0);    
}

mediump vec3 DecodeNormal888From871(mediump vec2 Oct)
{
	mediump float packY = Oct.y * 255.0;
	mediump float y = floor(packY / 2.0);
	mediump float signZ = packY * 2.0 - y * 4.0 - 1.0; // (packY - y * 2.0) * 2.0 + 1.0;
	mediump vec2 xy = vec2(Oct.x, y) * vec2(2.0, 2.0 / 127.0) - 1.0;
	return vec3(xy, signZ * sqrt(max(0.001957, 1.0 - dot(xy, xy))));
}

mediump vec2 EncodeFloatRG(highp float v)
{
	mediump vec2 kEncodeMul = vec2(1.0, 255.0);
	const float kEncodeBit = 1.0/255.0;
	mediump vec2 enc = kEncodeMul * v;
	enc = frac(enc);
	enc.x -= enc.y * kEncodeBit;
	return enc;
}

highp float DecodeFloatRG(mediump vec2 enc)
{
	const mediump vec2 kDecodeDot = vec2(1.0, 1.0/255.0);
	return dot(enc, kDecodeDot);
}

