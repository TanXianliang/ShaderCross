/*MaterialCG.cginc*/
#define REFLECTION_CAPTURE_ROUGHEST_MIP 1.0
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2

#define Diffuse_Lambert(Color, NoL) (half3_x(INV_PI * NoL) * (Color))
//#define DeflectNormalBaseOnAnisoDistribution(N, E, T, D) normalize(lerp(N, cross(cross(E, T), T), D * 0.9))  
#define DeflectNormalBaseOnAnisoDistribution(N, E, T, D) normalize(lerp(N, T * dot(T, E) - E * dot(T, T), D * half_x(0.9))) 
#define ShiftTangent(T, N, shift) normalize((shift) * (N) + T)

struct LightInput
{
	half3 LightDirection;
	half ShadowMask;	
	half3 LightColor;
	half LightAttenuation;	 
};

struct IndirectLight
{
	half3 Diffuse;
	half3 Specular;
	half3 Normal;
};

struct SurfaceInput
{
	half3 DiffuseBaseColor;	
	half NoLNosaturate;
	half3 SpecularBaseColor;
	half NoL;	

	half3 Emissive;
	half NoV;

	half3 ViewDir;
	half Roughness;
	half3 WorldNormal;
	half AO;
	highp vec3 HalfDir;	
	half Metal;
	
#if SM_MaterialID == MATERIALID_EYE
	half3 InnerNormal;      
#elif SM_MaterialID == MATERIALID_TRANSLUCENCY || SM_MaterialID == MATERIALID_SSSLUT
	half3 TranslucencyColor;
#elif SM_MaterialID == MATERIALID_ANISO	
	half3 Tangent;	
	half3 Binormal;
	half Anisotropic;
	half AnisoAngle;
#elif SM_MaterialID == MATERIALID_HAIR || SM_MaterialID == MATERIALID_HAIR_SUBSURFACE
	half3 Tangent;
	half Scatter;
#elif SM_MaterialID == MATERIALID_HAIR_UE4
	half Specular;
	half Scatter;  
#elif SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE	
	half3 Tangent;
	half Anisotropic;
	half3 Binormal;
	half Cloth;
	half Translucency;
	half Specular;
	half AnisoAngle;
#elif SM_MaterialID == MATERIALID_CLOTH_GGX		
	half Cloth;
	half Translucency;
	half Specular;
#elif SM_MaterialID == MATERIALID_TWO_SIDED_FOLIAGE
	half SubsurfaceProportion;
	half SubsurfaceWrap;   
#elif SM_MaterialID == MATERIALID_SKIN || SM_MaterialID == MATERIALID_SKIN_UE4	
	half SSSWidth;
	half Translucency;
#endif

	half MaterialHeight;
	half ReflectIntensity;
	float ReflectDistortion;
};

#if SM_Reflection == REFLECTION_METALLIC
	#define BRDFDiffuseSetup(Pixel) half3_x(Pixel.Albedo * (1.0 - Pixel.Reflectivity))
	#define BRDFSpecularSetup(Pixel) half3_x(lerp(vec3(0.08 * Pixel.Fresnel), Pixel.Albedo, Pixel.Reflectivity))

#elif SM_Reflection == REFLECTION_SPECULAR
	#define BRDFDiffuseSetup(Pixel) half3_x(Pixel.Albedo)
	#define BRDFSpecularSetup(Pixel) half3_x(Pixel.SpecularColor)
#elif SM_Reflection == REFLECTION_REFLECT
	#define BRDFDiffuseSetup(Pixel) half3_x(Pixel.Albedo * (1.0 - Pixel.Reflectivity))
	#define BRDFSpecularSetup(Pixel) half3_x(Pixel.SpecularColor * Pixel.Reflectivity)
#elif SM_Reflection == REFLECTION_UNLIT
	#define BRDFDiffuseSetup(Pixel) half3_t(0.0)
	#define BRDFSpecularSetup(Pixel) half3_t(0.0)
#endif

half3 ConvermapTransform(vec3 WorldPosition)
{
    vec2 vConverUV = GetCovermapUV(WorldPosition.xz);
	half3 vBase = half3_t(1.0);
	bvec2 sml1 = bvec2(step(vec2(0.005), vConverUV));
	bvec2 smh1 = bvec2(step(vConverUV, vec2(0.995)));

	if(all(sml1) && all(smh1))
	{
		half3 clr = half3_x(textureLod(sampler2D(tConver, tConver_sampler), vConverUV, 0.0).rgb);
		vBase = saturate(clr * half3_t(1.5));
	}

	return vBase;
}

SurfaceInput FragmentSetup(in ParamsMainPixelNode Pixel, in MaterialPixelParameters V2F)
{	
	SurfaceInput Surface;
	Surface.ViewDir = half3_x(V2F.ViewDir);	

	Surface.DiffuseBaseColor = BRDFDiffuseSetup(Pixel);
#ifdef COVERMAP
	Surface.DiffuseBaseColor *= ConvermapTransform(V2F.WorldPosition);
#endif
	Surface.SpecularBaseColor = BRDFSpecularSetup(Pixel);
	Surface.Emissive = half3_x(Pixel.Emissive);
	Surface.Roughness = half_x(clamp(Pixel.Roughness * GetRoughnessScale(), 0.0885, 1.0));
	Surface.WorldNormal = half3_x(Pixel.Normal);
	Surface.AO = half_x(Pixel.AO);
	Surface.NoV = half_x(saturate(dot(Pixel.Normal, V2F.ViewDir)));	
	Surface.NoLNosaturate = half_x(0.0);
	Surface.NoL = half_x(0.0);
	Surface.Metal = half_x(Pixel.Reflectivity);
	Surface.HalfDir = vec3(0.0);
	Surface.MaterialHeight = half_x(Pixel.MaterialHeight);
	Surface.ReflectIntensity = half_x(Pixel.ReflectIntensity);
	Surface.ReflectDistortion = half_x(Pixel.ReflectDistortion);
      

#if SM_MaterialID == MATERIALID_ANISO || SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE		
	//Get Anisotropic Tangent
	half angle = half_x(Pixel.AnisoAngle * TWO_PI);
	Surface.AnisoAngle = half_x(Pixel.AnisoAngle);
	half2 sc = sin(half2_t(angle, angle + HALF_PI));
	half3 WorldBinormal = half3_t(cross(Pixel.Normal, Pixel.Tangent));
	//half e = sign(dot(WorldBinormal, vsout_Binormal));
	half3 AnisoTangent = sc.y * half3_x(Pixel.Tangent) + sc.x * WorldBinormal;	
	half3 Binormal = cross(half3_x(Pixel.Normal), AnisoTangent);
#endif

#if SM_MaterialID == MATERIALID_EYE
	Surface.InnerNormal = half3_x(Pixel.InnerNormal);
#elif SM_MaterialID == MATERIALID_TRANSLUCENCY || SM_MaterialID == MATERIALID_SSSLUT
	Surface.TranslucencyColor = half3_x(Pixel.TranslucencyColor);
#elif SM_MaterialID == MATERIALID_ANISO
	Surface.Anisotropic = half_x(Pixel.Anisotropic);
	Surface.Tangent = AnisoTangent;
	Surface.Binormal = Binormal;
#elif SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE
	Surface.Anisotropic = half_x(Pixel.Anisotropic);
	Surface.Tangent = AnisoTangent;
	Surface.Binormal = Binormal;
	Surface.Cloth = half_x(saturate(Pixel.Cloth));
	Surface.Translucency = half_x(Pixel.SSSTranslucency);
	Surface.Specular = half_x(Pixel.Fresnel);
#elif SM_MaterialID == MATERIALID_CLOTH_GGX		
	Surface.Cloth = half_x(saturate(Pixel.Cloth));
	Surface.Translucency = half_x(Pixel.SSSTranslucency);
	Surface.Specular = half_x(Pixel.Fresnel);
#elif SM_MaterialID == MATERIALID_HAIR || SM_MaterialID == MATERIALID_HAIR_SUBSURFACE	
	Surface.Tangent = half3_x(Pixel.Tangent);
	Surface.Scatter = half_x(Pixel.Scatter);
#elif SM_MaterialID == MATERIALID_SKIN || SM_MaterialID == MATERIALID_SKIN_UE4
	Surface.SSSWidth = clamp(half_x(Pixel.SSSWidth), half_x(0.0001), half_x(0.025));
	Surface.Translucency = half_x(Pixel.SSSTranslucency);
#elif SM_MaterialID == MATERIALID_HAIR_UE4
	Surface.Specular = half_x(Pixel.Fresnel);
	Surface.Scatter = half_x(Pixel.Scatter);
#elif SM_MaterialID == MATERIALID_TWO_SIDED_FOLIAGE
	Surface.SubsurfaceProportion = half_x(Pixel.SubsurfaceProportion);
	Surface.SubsurfaceWrap = half_x(Pixel.SubsurfaceWrap);
#endif	

	return Surface;
}

//temp
SurfaceInput FragmentSetup(in ParamsMainPixelNode Pixel, half3 ViewDir)
{
	MaterialPixelParameters V2F;
	V2F.ViewDir = ViewDir;
	V2F.WorldPosition = vec3(0.0);
	return FragmentSetup(Pixel, V2F);
}

LightInput MainLightSetup()
{
	LightInput MainLight;
	MainLight.LightDirection = half3_x(GetSunLightDir());
	MainLight.LightColor = half3_x(GetSunLightDiffuse().rgb);	
	MainLight.ShadowMask = half_x(1.0);
	MainLight.LightAttenuation = half_x(1.0);

	return MainLight;
}

half3 EnvBRDFGGX(in SurfaceInput S)
{            
	const half4 c0 = half4_t(-1.0, -0.0275, -0.26, 0.0109);
    const half4 c1 = half4_t(1.0, 0.0455, 1.0417, -0.0417);
    half4 r = mad(half4_t(S.Roughness), c0, c1);    
    half a004 = min(r.x * half_x(0.6), pow5(half_x(1.0) - S.NoV)) * r.x + r.y;
    half2 AB = half2_t(-1.0417, 1.0417) * a004 + r.zw;		
    AB.y *= saturate(half_x(20.0) * S.SpecularBaseColor.g);
    half3 specular = AB.xxx * S.SpecularBaseColor + AB.yyy; //must keep this    
    return specular;
}

half3 EnvBRDFGGXNonmetal(in SurfaceInput S)
{
    const half2 c0 = half2_t(-1.0, -0.0275); 
    const half2 c1 = half2_t(1.0, 0.0455);
    half2 r = mad(half2_t(S.Roughness), c0, c1); 
    half a004 = min(r.x * half_x(0.6), pow5(half_x(1.0) - S.NoV)) * r.x + r.y;    
    return half3_t(a004);    
}


#if SM_MaterialID == MATERIALID_SKIN || SM_MaterialID == MATERIALID_SKIN_UE4
half3 SkinTransmittance(in SurfaceInput S) 
{     
    half3 Color = half3_t(0.0);

    if (S.Translucency > half_x(0.1))
    {
    	half scale = half_x(8.25) * (half_x(1.0) - S.Translucency) / S.SSSWidth;
		half d = scale * half_x(0.025);//just use fake depth, Translucency should include depth

		half dd = -d * d;
		half4 edd = exp(half4_t(dd) / half4_t(0.187, 0.567, 1.99, 7.41));						
#if 1		
		const mediump mat4x3 m = mat4x3(0.118, 0.198, 0.0,
										0.113, 0.007, 0.007,
										0.358, 0.004, 0.0,
										0.078, 0.000, 0.0);

		half3 profile = half3_x(m * edd);		
#else		
		half3 profile = //vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) + 
						//vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) + 
						vec3(0.118, 0.198, 0.0) * edd.r +
						vec3(0.113, 0.007, 0.007) * edd.g +
						vec3(0.358, 0.004, 0.0) * edd.b + 
						vec3(0.078, 0.0, 0.0)   * edd.a;
#endif						

    	Color = half_x(INV_PI) * saturate(-S.NoLNosaturate) * profile;
	}

	return Color;
}
#elif SM_MaterialID == MATERIALID_ANISO || SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE
float D_GTR2aniso(in SurfaceInput S, highp float NoH)
{	
#if 1	
	half aspect = sqrt(half_x(1.0) - half_x(0.9) * S.Anisotropic) + half_x(1e-4);
	half a2 = S.Roughness * S.Roughness;
	half ax = a2 / aspect;
	half ay = a2 * aspect;

	float xh = dot(S.HalfDir, AH3_AF3(S.Binormal));
	float yh = dot(S.HalfDir, AH3_AF3(S.Tangent));
	//must highp, for mi12 don't use (xh/ ax)^2 
	highp float d = xh * xh / (ax * ax) + yh * yh / (ay * ay) + NoH * NoH; 
	d = 1.0 / (PI * ax * ay * d * d + 1e-7);
	return d;
#else
	half aspect = sqrt(1.0 - S.Anisotropic * 0.9) + 1e-4;
	half a = S.Roughness * S.Roughness;
	half ax = max(1e-4, a / aspect);
	half ay = max(1e-4, a * aspect);    

	half HdotX = dot(S.HalfDir, S.Binormal);
	half HdotY = dot(S.HalfDir, S.Tangent);
	highp float d = ax * ay * square(square(HdotX / ax) + square(HdotY / ay) + NoH * NoH); //must highp
	return INV_PI / (d + 1e-7);			
#endif	
}
#endif

#if SM_MaterialID == MATERIALID_ANISO
half3 ComputeBRDFSpecular(in SurfaceInput S)
{	
	highp float NoH = saturate(dot(S.HalfDir, AH3_AF3(S.WorldNormal)));

	half a = S.Roughness * S.Roughness;
	half aspect = sqrt(half_x(1.0) - half_x(0.9) * S.Anisotropic) + half_x(1e-4);
	half ax = a / aspect;
	half ay = a * aspect;

	float xh = dot(S.HalfDir, AH3_AF3(S.Binormal));
	float yh = dot(S.HalfDir, AH3_AF3(S.Tangent));
	//must highp, for mi12 don't use (xh/ ax)^2 
	highp float d = xh * xh / (ax * ax) + yh * yh / (ay * ay) + NoH * NoH; 
	d = S.NoL / (PI * ax * ay * d * d + 1e-7);
	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);    
	d = 0.5 * d / max(Vis_SmithV + Vis_SmithL, half_x(1e-4));

	half DG = half_x(d);

//#if defined(TWOSIDE) || defined(TERRAIN)
	half3 F = S.SpecularBaseColor;
//#else
//	half VoH = saturate(dot(S.HalfDir, S.ViewDir));	
//	half fc = pow5(1.0 - VoH);		    
//	half3 F = lerp(S.SpecularBaseColor, vec3(1.0), fc); 
//#endif		

	//half D = D_GTR2aniso(S, NoH);
	//half V = Vis_SmithJointApprox(S.Roughness, S.NoV, S.NoL);
	//half3 F = Fresnel(S.SpecularBaseColor, VoH);;
	///return clamp(DG * F, vec3(0.0), vec3(20.0));
	return DG * F;
}
#elif SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE
half3 ComputeBRDFSpecular(in SurfaceInput S)
{	
	highp float NoH = saturate(dot(S.HalfDir, AH3_AF3(S.WorldNormal)));
    half FuzzyColor = half_x(0.5);
//#if defined(TWOSIDE) || defined(TERRAIN)
	half F1 = FuzzyColor * S.Specular * half_x(2.0);
	half3 F2 = S.SpecularBaseColor;
//#else
//	half VoH = saturate(dot(S.HalfDir, S.ViewDir));
//	half fc = pow5(1.0 - VoH);		
//  half3 F1 = FuzzyColor * (1.0 - fc) + fc;	
//  half3 F2 = lerp(S.SpecularBaseColor, vec3(1.0), fc);
//#endif	

    // Cloth - Asperity Scattering - Inverse Beckmann Layer
    //half D1 = D_InvGGX(S.Roughness, NoH);    
    //half G1 = G_Cloth(S.NoV, S.NoL);
    half a = S.Roughness * S.Roughness;
	half m2 = a * a;
	highp float d = (NoH - m2 * NoH) * NoH + m2;	
	d = S.NoL * (1.0 + 4.0 * m2 * m2 / (d * d + 1e-7)) / (PI + 4.0 * PI * m2);		
	half Vis_Cloth = lerp(S.NoL, half_x(1.0), S.NoV);
	d = d / max(4.0 * Vis_Cloth, 1e-4); 	
	half DG1 = half_x(d);
    
    //Aniso Specular
    //half D2 = D_GTR2aniso(S, NoH);
	half aspect = sqrt(half_x(1.0) - half_x(0.9) * S.Anisotropic) + half_x(1e-4);
	half ax = a / aspect;
	half ay = a * aspect;

	float xh = dot(S.HalfDir, AH3_AF3(S.Binormal));
	float yh = dot(S.HalfDir, AH3_AF3(S.Tangent));
	//must highp, for mi12 don't use (xh/ ax)^2 
	d = xh * xh / (ax * ax) + yh * yh / (ay * ay) + NoH * NoH; 
	d = S.NoL / (PI * ax * ay * d * d + 1e-7);		
	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);    	
	d = 0.5 * d / max(Vis_SmithV + Vis_SmithL, half_x(1e-4));

	half DG2 = half_x(d);	
    //half3 F2 = Fresnel(S.SpecularBaseColor, VoH);
	//half G2 = Vis_SmithJointApprox(S.Roughness, S.NoV, S.NoL);
    
    half3 Specular = lerp(DG2 * F2, half3_t(DG1 * F1), S.Cloth);
    return Specular;
}
#elif SM_MaterialID == MATERIALID_CLOTH_GGX
half3 ComputeBRDFSpecular(in SurfaceInput S)
{	
	highp float NoH = saturate(dot(S.HalfDir, AH3_AF3(S.WorldNormal)));
	half FuzzyColor = half_x(0.5);//BRDFSpecularBase;
	half F1 = FuzzyColor * S.Specular * half_x(2.0);
	half3 F2 = S.SpecularBaseColor;	

	half a = S.Roughness * S.Roughness;
	half m2 = a * a;
	highp float d = (NoH - m2 * NoH) * NoH + m2;
	d = S.NoL * (1.0 + 4.0 * m2 * m2 / (d * d + 1e-7)) / (PI + 4.0 * PI * m2);		
	half Vis_Cloth = lerp(S.NoL, half_x(1.0), S.NoV);
	d = d / max(4.0 * Vis_Cloth, 1e-4); 	
	half DG1 = half_x(d);

	//GGX Specular	
	d = (NoH * m2 - NoH) * NoH + 1.0;		// 2 mad, must highp
	d = m2 / (d * d * PI + 1e-7);			// 2 mul, 1 rcp	
		
	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);   
    d = 0.5 * S.NoL * d / max(Vis_SmithV + Vis_SmithL, half_x(1e-4));
	half DG2 = half_x(d);
	//half3 F2 = Fresnel(S.SpecularBaseColor, VoH);
	//half G2 = Vis_SmithJointApprox(S.Roughness, S.NoV, S.NoL);
    
    half3 Specular = lerp(DG2 * F2, half3_t(DG1 * F1), S.Cloth);
    return Specular;
}
#elif SM_MaterialID == MATERIALID_HAIR || SM_MaterialID == MATERIALID_HAIR_SUBSURFACE
half3 ComputeBRDFSpecular(SurfaceInput S)
{		
	//half VoH = saturate(dot(AH3_AF3(S.ViewDir), S.HalfDir));
	// shift tangents
	half shiftTex = half_x(0.01);//tex2D(tSpecShift, uv) - 0.5;
	half primaryShift = half_x(HAIR_PRIMARY_SHIFT);//0.05;
	half secondaryShift = half_x(HAIR_SECOND_SHIFT);//0.1;
	half3 t1 = ShiftTangent(S.Tangent, S.WorldNormal, primaryShift + shiftTex);
	half3 t2 = ShiftTangent(S.Tangent, S.WorldNormal, secondaryShift + shiftTex);

	half a = S.Roughness * S.Roughness;	
		
	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);    
    half G = half_x(0.5) / max(Vis_SmithV + Vis_SmithL, half_x(1e-4)); 
    G *= half_x(0.5) * S.Scatter * S.Scatter * S.NoL;

	// BRDF Specular		
	half m2 = a * a;
	half n = clamp(half_x(2.0) / m2 - half_x(2.0), half_x(1.0), half_x(1000.0));
	half scale = half_x(1.0);//0.25;
	scale *= (n * half_x(0.5) + half_x(1.0)) * half_x(INV_PI);
	G *= scale;
	
	half dotTH1 = half_x(dot(S.HalfDir, AH3_AF3(t1)));
	half dotTH2 = half_x(dot(S.HalfDir, AH3_AF3(t2)));

	half sinTH = sqrt(max(half_x(HALF_MIN), half_x(1.0) - dotTH1 * dotTH1));
	half dirAtten = smoothstep(half_x(-1.0), half_x(0.0), dotTH1);	
	sinTH = max(half_x(1e-3), sinTH);
	highp float d = G * dirAtten * pow(sinTH, n);
	half DG1 = half_x(d);

	sinTH = sqrt(max(half_x(HALF_MIN), half_x(1.0) - dotTH2 * dotTH2));
	dirAtten = smoothstep(half_x(-1.0), half_x(0.0), dotTH2);	
	sinTH = max(half_x(1e-3), sinTH);
	d = G * dirAtten * pow(sinTH, n);
	half DG2 = half_x(d);

	half3 F1 = S.SpecularBaseColor;
    half3 F2 = S.DiffuseBaseColor;
     
    return DG1 * F1 + DG2 * F2;
}
#elif SM_MaterialID == MATERIALID_NODIRECTION
half3 ComputeBRDFSpecular(SurfaceInput S)
{
	return vec3(0.0);
}
#elif SM_MaterialID == MATERIALID_SSSLUT
half3 ComputeBRDFSpecular(SurfaceInput S)
{		
	highp float NoH = saturate(dot(S.HalfDir, AH3_AF3(S.WorldNormal)));

	half a = S.Roughness * S.Roughness;
	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);    

	half a2 = a * a;	
	highp float d = (NoH * a2 - NoH) * NoH + 1.0;	// 2 mad, must highp	
	d = a2 / (d * d * PI + 1e-7);			// 2 mul, 1 rcp	

	a = clamp(S.Roughness * half_x(0.7), half_x(0.08), half_x(1.0));
	a2 = pow4(a);
	highp float d1 = (NoH * a2 - NoH) * NoH + 1.0;
	d1 = a2 / (d1 * d1 * PI + 1e-7);

	d = lerp(d, d1, 0.6);
	d = 0.5 * S.NoL * d / max(Vis_SmithV + Vis_SmithL, half_x(1e-4)); 
    half DG = half_x(d);

	half3 F = S.SpecularBaseColor;	
	return DG * F;
}
#else
half3 ComputeBRDFSpecular(SurfaceInput S)
{		
	//half D = D_GGX(S.Roughness, NoH);
	//half G = Vis_SmithJointApprox(S.Roughness, S.NoV, S.NoL);	
	//half3 F = Fresnel(S.SpecularBaseColor, VoH);
	highp float NoH = saturate(dot(S.HalfDir, AH3_AF3(S.WorldNormal)));

	half a = S.Roughness * S.Roughness;	
	half a2 = a * a;
	highp float d = (NoH * a2 - NoH) * NoH + 1.0;	// 2 mad, must highp
	d = AH_AF(a2) / (d * d * PI + FLT_EPS);				// 2 mul, 1 rcp	

	half Vis_SmithV = S.NoL * lerp(S.NoV, half_x(1.0), a);
	half Vis_SmithL = S.NoV * lerp(S.NoL, half_x(1.0), a);    
	d = 0.5 * d * S.NoL / max(Vis_SmithV + Vis_SmithL, half_x(HALF_MIN)); 
    half DG = half_x(d);

#if SPEEDTREE != 0 || defined(TWOSIDE)
	half3 F = S.SpecularBaseColor;
#elif defined(SHADER_MACRO_TO_SPECIALIZATION_CONSTANTS_ENABLE)
	half3 F = S.SpecularBaseColor;
	if ((g_SpecializationConstants & RuntimeMacro_TwoSide_To_SPC) == 0u)
	{
		half VoH = half_x(saturate(dot(S.HalfDir, AH3_AF3(S.ViewDir))));	
		half fc = pow5(half_x(1.0) - VoH);		    
		F = lerp(S.SpecularBaseColor, half3_t(1.0), fc);
	}
#else
	half VoH = half_x(saturate(dot(S.HalfDir, AH3_AF3(S.ViewDir))));	
	half fc = pow5(half_x(1.0) - VoH);		    
	half3 F = lerp(S.SpecularBaseColor, half3_t(1.0), fc);
#endif	
	
	return DG * F;
}
#endif

#if SM_MaterialID == MATERIALID_HAIR || SM_MaterialID == MATERIALID_HAIR_SUBSURFACE	
half3 Diffuse_ScatterHair(SurfaceInput S, half Shadow, half3 L)
{	
	half3 NewN = normalize(S.ViewDir - S.WorldNormal * S.NoV);

	// Hack approximation for multiple scattering.
	half Wrap = half_x(1.0);
	half NoL = saturate((dot(NewN, L) + Wrap) / square(half_x(1.0) + Wrap));
	half DiffuseScatter = half_x(INV_PI) * NoL * S.Scatter;
	half Luma = max(Luminance(S.DiffuseBaseColor), half_x(1e-3));
	half3 Diffuse = max(S.DiffuseBaseColor / Luma, half3_t(1e-3));
	half3 ScatterTint = pow(Diffuse, half3_t(half_x(1.001) - Shadow));
	half3 ScatterColor = sqrt(S.DiffuseBaseColor) * DiffuseScatter * ScatterTint;	
		
	return ScatterColor;
}

#elif SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE || SM_MaterialID == MATERIALID_CLOTH_GGX
half3 Diffuse_CheapSubSurfaceScattering(SurfaceInput S)
{		
	half Diffuse = saturate(S.NoLNosaturate + S.Cloth) / (half_x(1.0) + S.Cloth);
	//Diffuse /= (1.0 + S.Cloth);
	half3 ScatterLight = S.DiffuseBaseColor * S.DiffuseBaseColor + S.NoL * S.DiffuseBaseColor;
	return half_x(INV_PI) * Diffuse * ScatterLight;
}
#endif	

#if SM_MaterialID == MATERIALID_HAIR_SUBSURFACE
half3 SubsurfaceDiffuse(SurfaceInput S, half3 L)
{
	half VoL = saturate(dot(S.ViewDir, -L));
	// GGX scatter distribution
	//half D = D_GGX(0.6, VoL);	
	half d = (VoL * half_x(0.1296) - VoL) * VoL + half_x(1.0);	
	half D = half_x(0.1296) / (d * d * half_x(PI));
	return S.NoL * D * S.DiffuseBaseColor;   
}
#elif SM_MaterialID == MATERIALID_TWO_SIDED_FOLIAGE
half3 SubsurfaceDiffuse(SurfaceInput S, half3 L)
{
	half NoL = saturate(S.SubsurfaceProportion * (-S.NoLNosaturate + S.SubsurfaceWrap) / square(half_x(1.0) + S.SubsurfaceWrap));
	half VoL = saturate(dot(S.ViewDir, -L));
	// GGX scatter distribution
	//half D = D_GGX(0.6, VoL);
	float d = (VoL * 0.1296 - VoL) * VoL + 1.0;	
	float D = 0.1296 / max(d * d * PI, 1e-7);
	return NoL * half_x(D) * S.DiffuseBaseColor;
}
#elif SM_MaterialID == MATERIALID_CLOTH || SM_MaterialID == MATERIALID_CLOTH_GGX
half3 SubsurfaceDiffuse(SurfaceInput S, half3 L)
{
	half NoL = clamp(-S.NoLNosaturate, half_x(1e-1), half_x(1.0));
	NoL = pow3(NoL) * pow3(NoL);				
	return S.Translucency * NoL * S.DiffuseBaseColor;
}
#elif SM_MaterialID == MATERIALID_CLOTH_SUBSURFACE
half3 SubsurfaceDiffuse(SurfaceInput S, half3 L)
{
	half Wrap = half_x(0.5);
	half NoL = saturate((-S.NoLNosaturate + Wrap) / square(half_x(1.0) + Wrap));
	// GGX scatter distribution
	half VoL = saturate(dot(S.ViewDir, -L));
	//half D = D_GGX(0.6, VoL);			
	half d = (VoL * half_x(0.1296) - VoL) * VoL + half_x(1.0);
	half D = half_x(0.1296) / (d * d * half_x(PI));
	return S.Translucency * NoL * D * S.DiffuseBaseColor;
}
#elif SM_MaterialID == MATERIALID_TRANSLUCENCY	
half ComputeTranslucency(SurfaceInput S, half3 L, half Distortion)
{		
	half3 H = L + S.WorldNormal * Distortion; //normalize ?
	half VoH = saturate(dot(S.ViewDir, -H));
	return pow3(VoH) * pow3(VoH);	
}
#elif SM_MaterialID == MATERIALID_SSSLUT
half3 Diffuse_SSSLUT(SurfaceInput S)
{
	/*highp float VoH = saturate(dot(S.HalfDir, S.ViewDir));
	float FD90 = 0.5 + 2.0 * VoH * VoH * S.Roughness;
	float FdV = 1.0 + (FD90 - 1.0) * exp2((-5.55473 * S.NoV - 6.98316) * S.NoV);
	float FdL = 1.0 + (FD90 - 1.0) * exp2((-5.55473 * S.NoL - 6.98316) * S.NoL);
	*/
	const half c0 = half_x(0.36);
	const half c1 = half_x(0.25) / c0;

	vec3 Sharpness = S.TranslucencyColor;
	Sharpness =  vec3(1.0) / max(Sharpness, vec3(1e-4));	
	Sharpness = exp(-Sharpness);

	half3 eml  = half3_x(Sharpness);
	half3 em2l = eml * eml;
	half3 rl   = S.TranslucencyColor;
 
	half3 scale = half3_t(1.0) + half3_t(2.0) * em2l - rl;
	half3 bias  = (eml - em2l) * rl - em2l;

	half3 x = sqrt(max(half3_t(1.0) - scale, half_x(HALF_MIN)));
	half x0 = c0 * S.NoLNosaturate;
	half3 x1 = c1 * x;

	half3 n = x0 + x1;
	half3 n2 = n * n / x;
	x0 = abs(x0);
	half3 y = half3_t(x1.x > x0 ? n2.x : S.NoL, x1.y > x0 ? n2.y : S.NoL, x1.z > x0 ? n2.z : S.NoL);
	float sssweight = sin(PI * (S.NoL * 0.5 + 0.5));
	half3 NoL = lerp(scale * y + bias, half3_t(S.NoL), half3_t(S.NoL * 0.5 + 0.5));

	return S.DiffuseBaseColor * half3_t(NoL * INV_PI);
}

#else 
half3 SubsurfaceDiffuse(SurfaceInput S, half3 L)
{
	return half3_t(0.0);
}
#endif

half4 EncodeNormalAlbedo(in SurfaceInput S)
{
	half4 enc = half4_t(0.0);
	half2 n = normalize(S.WorldNormal.xy);
	n *= sqrt(max(-S.WorldNormal.z * half_x(0.5) + half_x(0.5), half_x(HALF_MIN)));
	enc.xy = n * half2_t(0.5) + half2_t(0.5);

	half3 color = floor(mad(S.DiffuseBaseColor, half3_t(31.0, 63.0, 31.0), half3_t(0.5)));
	half x = floor(color.y / half_x(8.0));
	half y = color.y - x * half_x(8.0);
	enc.zw = (half2_t(x, y) * half2_t(32.0) + color.xz) / half2_t(255.0);

	return enc;
}

highp float OIT_Weight(float a)
{	
	highp float z = 1.0 / gl_FragCoord.w;
	return clamp(20.0 * a * a * a/(0.1 + pow4(z/2000.0)), 1e-2, 1e2);		
}

highp float OIT_Weight1(float alpha) 
{
	highp float z = gl_FragCoord.z;
    highp float a = alpha * 8.0 + 0.01;
#ifdef REVERSE_DEPTH_Z    
    highp float b = z * 0.95 + 0.05;
#else    
    highp float b = -z * 0.95 + 1.0;
#endif    
    return clamp(a * a * a * 1e7 * b * b * b / 3.0, 1e-2, 1e2);
}

highp vec2 ViewSphereCoordToUV(highp vec3 n)
{
	highp float len = n.x * n.x + n.z * n.z;	
	vec2 r = vec2(n.x * rsqrt(len), n.y * rsqrt(n.y * n.y + len));
	r = FastACos(r);	
	highp float v = r.y * INV_PI;
	r.x = n.z < 0.0 ? TWO_PI - r.x : r.x;	
	r.y = v < 0.4 ? v * 0.25 : v * 1.5 - 0.5;	
	r.x = r.x * INV_TWO_PI + 0.25;
	return r;
}

highp vec2 ViewSphereCoordToUV1(highp vec3 n)
{
	highp float len = n.x * n.x + n.z * n.z;	
	highp float x = FastACos(n.x * rsqrt(len));
	highp float y = -FastASin(n.y * rsqrt(n.y * n.y + len));	
	x = n.z < 0.0 ? TWO_PI - x : x;
	highp float v = y * INV_PI + 0.5;
	v = v < 0.4 ? v * 0.25 : v * 1.5 - 0.5;	
	return vec2(x * INV_TWO_PI + 0.25, v); 
}

float encode_int4bit_float4bit(uint x, float y)
{
	return saturate(float(x << 4u) / 255.0 + (15.0 / 255.0 * y));
}

void DecodeTypeMetal(float value, out uint t, out float metal)
{
	uint a = uint(value * 255.0 + 0.5);
	t =	a >> 4u;
	metal = float(a & 0x0F) / 15.0;	
}

#ifdef A_HALF
void DecodeTypeMetal(half value, out uint t, out half metal)
{
	uint a = uint(value * half_x(255.0) + half_x(0.5));
	t =	a >> 4u;
	metal = half_x(a & 0x0F) / half_x(15.0);	
}
#endif
