/**
 *                  _______      _______      _______      _______
 *                 /       |    /       |    /       |    /       |
 *                |   (----    |   (----    |   (----    |   (----
 *                 \   \        \   \        \   \        \   \
 *              ----)   |    ----)   |    ----)   |    ----)   |
 *             |_______/    |_______/    |_______/    |_______/
 *
 *        S E P A R A B L E   S U B S U R F A C E   S C A T T E R I N G
 *
 *                           http://www.iryoku.com/
*                       
**/
//-----------------------------------------------------------------------------
// Configurable Defines

/**
 * Light diffusion should occur on the surface of the object, not in a screen 
 * oriented plane. Setting SSSS_FOLLOW_SURFACE to 1 will ensure that diffusion
 * is more accurately calculated, at the expense of more memory accesses.
 */
#ifndef SSSS_FOLLOW_SURFACE
#define SSSS_FOLLOW_SURFACE 0
#endif

/**
 * This define allows to specify a different source for the SSS strength
 * (instead of using the alpha channel of the color framebuffer). This is
 * useful when the alpha channel of the mian color buffer is used for something
 * else.
 */
#ifndef SSSS_STREGTH_SOURCE
//#define SSSS_STREGTH_SOURCE (colorM.a)         //Alpha Is The Speed Of Pixel Motion In SSSSDemo
#define SSSS_STREGTH_SOURCE 1.0
#endif


/**
 * Here you have ready-to-use kernels for quickstarters. Three kernels are 
 * readily available, with varying quality.
 * To create new kernels take a look into SSS::calculateKernel, or simply
 * push CTRL+C in the demo to copy the customized kernel into the clipboard.
 *
 * Note: these preset kernels are not used by the demo. They are calculated on
 * the fly depending on the selected values in the interface, by directly using
 * SSS::calculateKernel.
 *
 * Quality ranges from 0 to 2, being 2 the highest quality available.
 * The quality is with respect to 1080p; for 720p Quality=0 suffices.
 */
#define SSSS_QUALITY 0

#if SSSS_QUALITY == 2
#define SSSS_N_SAMPLES 25
mediump vec4 kernel[] = vec4[](
    vec4(0.530605, 0.613514, 0.739601, 0),
    vec4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
    vec4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
    vec4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
    vec4(0.00700976, 0.00049366, 0.000151938, -1.6875),
    vec4(0.0094389, 0.00139119, 0.000416598, -1.33333),
    vec4(0.0128496, 0.00356329, 0.00132016, -1.02083),
    vec4(0.017924, 0.00711691, 0.00347194, -0.75),
    vec4(0.0263642, 0.0119715, 0.00684598, -0.520833),
    vec4(0.0410172, 0.0199899, 0.0118481, -0.333333),
    vec4(0.0493588, 0.0367726, 0.0219485, -0.1875),
    vec4(0.0402784, 0.0657244, 0.04631, -0.0833333),
    vec4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
    vec4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
    vec4(0.0402784, 0.0657244, 0.04631, 0.0833333),
    vec4(0.0493588, 0.0367726, 0.0219485, 0.1875),
    vec4(0.0410172, 0.0199899, 0.0118481, 0.333333),
    vec4(0.0263642, 0.0119715, 0.00684598, 0.520833),
    vec4(0.017924, 0.00711691, 0.00347194, 0.75),
    vec4(0.0128496, 0.00356329, 0.00132016, 1.02083),
    vec4(0.0094389, 0.00139119, 0.000416598, 1.33333),
    vec4(0.00700976, 0.00049366, 0.000151938, 1.6875),
    vec4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
    vec4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
    vec4(0.000973794, 1.11862e-005, 9.43437e-007, 3)
);

#elif SSSS_QUALITY == 1
#define SSSS_N_SAMPLES 17
mediump vec4 kernel[] = vec4[](
    vec4(0.536343, 0.624624, 0.748867, 0),
    vec4(0.00317394, 0.000134823, 3.77269e-005, -2),
    vec4(0.0100386, 0.000914679, 0.000275702, -1.53125),
    vec4(0.0144609, 0.00317269, 0.00106399, -1.125),
    vec4(0.0216301, 0.00794618, 0.00376991, -0.78125),
    vec4(0.0347317, 0.0151085, 0.00871983, -0.5),
    vec4(0.0571056, 0.0287432, 0.0172844, -0.28125),
    vec4(0.0582416, 0.0659959, 0.0411329, -0.125),
    vec4(0.0324462, 0.0656718, 0.0532821, -0.03125),
    vec4(0.0324462, 0.0656718, 0.0532821, 0.03125),
    vec4(0.0582416, 0.0659959, 0.0411329, 0.125),
    vec4(0.0571056, 0.0287432, 0.0172844, 0.28125),
    vec4(0.0347317, 0.0151085, 0.00871983, 0.5),
    vec4(0.0216301, 0.00794618, 0.00376991, 0.78125),
    vec4(0.0144609, 0.00317269, 0.00106399, 1.125),
    vec4(0.0100386, 0.000914679, 0.000275702, 1.53125),
    vec4(0.00317394, 0.000134823, 3.77269e-005, 2)
);

#elif SSSS_QUALITY == 0
#define SSSS_N_SAMPLES 11
mediump vec4 kernel[] = vec4[]
(
    vec4(0.560479, 0.669086, 0.784728, 0),
    vec4(0.00471691, 0.000184771, 5.07566e-005, -2),
    vec4(0.0192831, 0.00282018, 0.00084214, -1.28),
    vec4(0.03639, 0.0130999, 0.00643685, -0.72),
    vec4(0.0821904, 0.0358608, 0.0209261, -0.32),
    vec4(0.0771802, 0.113491, 0.0793803, -0.08),
    vec4(0.0771802, 0.113491, 0.0793803, 0.08),
    vec4(0.0821904, 0.0358608, 0.0209261, 0.32),
    vec4(0.03639, 0.0130999, 0.00643685, 0.72),
    vec4(0.0192831, 0.00282018, 0.00084214, 1.28),
    vec4(0.00471691, 0.000184771, 5.07565e-005, 2)
);

#else
#error Quality must be one of {0,1,2}
#endif

//-----------------------------------------------------------------------------
// Separable SSS Transmittance Function

mediump vec3 SSSSTransmittance(
        /**
         * This parameter allows to control the transmittance effect. Its range
         * should be 0..1. Higher values translate to a stronger effect.
         */
        float translucency,

        /**
         * This parameter should be the same as the 'SSSSBlurPS' one. See below
         * for more details.
         */
        float sssWidth,        
        highp vec3 worldPosition,
        mediump vec3 worldNormal,
        //Light vector: lightWorldPosition - worldPosition.         
        mediump vec3 light) 
{
    //Calculate the scale of the effect.             
    float scale = 8.25 * (1.0 - translucency) / min(sssWidth, 0.025);
       
#if 0
    /**
     * First we shrink the position inwards the surface to avoid artifacts:
     * (Note that this can be done once for all the lights)
     */
    highp vec4 shrinkedPos = vec4(worldPosition - 0.5 * worldNormal, 1.0); //0.005f * 100 = 0.5f m -> cm

    /**
     * Now we calculate the thickness from the light point of view:
     */
    highp vec4 shadowPosition = mul(shrinkedPos, GetLightViewMat());
    shadowPosition = mul(shadowPosition, GetLightProjMat0());    
     // 'd1' has a range of 0..1    
    highp float d1 = textureLod(tShadowImage0, shadowPosition.xy / shadowPosition.w, 0.0).r;
    //原作者搞错了了范围，其实相当于d2 * lightFarPlane(demo为10.0). d1 可以忽略了(by topameng)
    highp float d2 = shadowPosition.z;
    highp float d =  0.4;
#else    
    float deltaWorldNormal = length(fwidth(worldNormal));
	float deltaWorldPosition = length(fwidth(worldPosition));

	//float Curvature = step(0.2, clamp(deltaWorldNormal * 0.1 / deltaWorldPosition, 0.0, 1.0));
    float Curvature = clamp(deltaWorldNormal / deltaWorldPosition, 0.4, 1.0) - 0.4;
    
    //clamp(scale * clamp(0.001 / Curvature, 0.001, 1.0) / 10.0, 0.4, 80.0);
    highp float d = scale * clamp(max(1.0 - Curvature, 0.0) / 2.0, 0.001, 1.0) / 80.0;
#endif    

    /**
     * Armed with the thickness, we can now calculate the color by means of the
     * precalculated transmittance profile.
     * (It can be precomputed into a texture, for maximum performance):
     */
    float dd = -d * d;
    mediump vec3 profile = vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
                     vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
                     vec3(0.118, 0.198, 0.0)   * exp(dd / 0.187)  +
                     vec3(0.113, 0.007, 0.007) * exp(dd / 0.567)  +
                     vec3(0.358, 0.004, 0.0)   * exp(dd / 1.99)   +
                     vec3(0.078, 0.0,   0.0)   * exp(dd / 7.41);

    /** 
     * Using the profile, we finally approximate the transmitted lighting from
     * the back of the object:
     */
    return profile * saturate(0.3 + dot(light, -worldNormal));
}

//-----------------------------------------------------------------------------
// Separable SSS Reflectance Pixel Shader
highp float ConvertLinearDepthToViewDepth(highp vec2 cameraPlane, float fLinearDepth)
{
	return cameraPlane.y * fLinearDepth;
}

//MaxSampler * MaxNum(use .dds need to transform uv)
void CaculateSSSParamters(sampler2D texProfile, int SSSIndex, vec2 TextureSize)
{
	/*vec2 UV = vec2(0);
	float fSSSIndex = float(SSSIndex);
	UV.y = fSSSIndex / TextureSize.y;//SSSIndex / TextureSize.y
		
	for (int i = 0; i < SSSS_N_SAMPLES; ++i)
	{
		float fi = float(i);
		UV.x = fi / TextureSize.x;
		kernel[i] = SSSSSample(TextureProfile, UV);
	}*/

    ivec2 uv = textureSize(texProfile, 0);
    uv.y -= 1;

    for (int i = 0; i < SSSS_N_SAMPLES; i++)
    {        
        uv.x = i;
        kernel[i] = texelFetch(texProfile, uv, 0);
    }    
}

mediump vec4 SSSSBlurPS(
        highp vec2 texcoord,

        /**
         * This is a SRGB or HDR color input buffer, which should be the final
         * color frame, resolved in case of using multisampling. The desired
         * SSS strength should be stored in the alpha channel (1 for full
         * strength, 0 for disabling SSS). If this is not possible, you an
         * customize the source of this value using SSSS_STREGTH_SOURCE.
         *
         * When using non-SRGB buffers, you
         * should convert to linear before processing, and back again to gamma
         * space before storing the pixels (see Chapter 24 of GPU Gems 3 for
         * more info)
         *
         * IMPORTANT: WORKING IN A NON-LINEAR SPACE WILL TOTALLY RUIN SSS!
         */
        mediump sampler2D colorTex,											

        /**
         * This parameter specifies the global level of subsurface scattering
         * or, in other words, the width of the filter. It's specified in
         * world space units.
         */
        mediump float sssWidth,

        /**
         Direction of the blur:
         - First pass:   vec2(1.0, 0.0)
         - Second pass:  vec2(0.0, 1.0)
         */
        highp vec2 dir,       
        highp float ndcZ) 
{
    //SSSS_FOV must be set to the value used to render the scene.
    //0.959931076; //55度
    //const float SSSS_FOVY = 0.349065850;  
    const float SSSS_FOVY = 0.959931076;

	sssWidth = max(sssWidth, 0.025);
    // Fetch color of current pixel:
    mediump vec4 colorM = textureLod(colorTex, texcoord, 0.0);

    // Initialize the stencil buffer in case it was not already available:
    //if (initStencil) // (Checked in compile time, it's optimized away)
    //    if (SSSS_STREGTH_SOURCE == 0.0) discard;

    // Fetch linear depth of current pixel:	
	highp float depthM = ndcZ;

    // Calculate the sssWidth scale (1.0 for a unit plane sitting on the projection window):
    highp float distanceToProjectionWindow = 1.0 / tan(0.5 * SSSS_FOVY);
    highp float scale = distanceToProjectionWindow / depthM;

    // Calculate the final step to fetch the surrounding pixels:
    highp vec2 finalStep = sssWidth * scale * dir;
    //finalStep *= SSSS_STREGTH_SOURCE; // Modulate it using the alpha channel.
    finalStep *= 0.333333; //the kernels range from -3 to 3.
	
	// Fetch the kenerls
	//CaculateSSSParamters(profileTex, sssProfileID, sssProfileTexSize);

    // Accumulate the center sample:
    mediump vec4 colorBlurred = colorM;
    colorBlurred.rgb *= kernel[0].rgb;        

    // Accumulate the other samples:    
    for (int i = 1; i < SSSS_N_SAMPLES; i++) 
    {
        // Fetch color and depth for current sample:
        highp vec2 offset = texcoord + kernel[i].a * finalStep;
        mediump vec4 color = textureLod(colorTex, offset, 0.0);        

	//In Order To Eliminate Incorrect Outline,暂时未支持
    #if SSSS_FOLLOW_SURFACE == 1
        // If the difference in depth is huge, we lerp color back to "colorM":
        highp float depthN = SAMPLE_DEPTH_DXTETURE(tSceneDepth, tSceneDepth_sampler, offset); //需要调整渲染顺序和传入Depth纹理
        depthN = GetCameraDepth(depthN) / 100.0;
        mediump float s = saturate(300.0 * distanceToProjectionWindow * sssWidth * abs(depthM - depthN));
        color.rgb = mix(color.rgb, colorM.rgb, s);
	#else
		color.rgb = mix(colorM.rgb, color.rgb, color.a);        
    #endif

        // Accumulate:
        colorBlurred.rgb += kernel[i].rgb * color.rgb;                
    }
	
    return colorBlurred;    
}
//_SeparableSSS_PS_

mediump vec4 SSSSBlurPS(highp vec2 texcoord, mediump texture2D colorTex, sampler texSampler, mediump float sssWidth, highp vec2 dir, highp float ndcZ) 
{
    const float SSSS_FOVY = 0.959931076;    
    sssWidth = max(sssWidth, 0.025);    
    mediump vec4 colorM = textureLod(sampler2D(colorTex, texSampler), texcoord, 0.0);    
    highp float depthM = ndcZ;

    highp float distanceToProjectionWindow = 1.0 / tan(0.5 * SSSS_FOVY);
    highp float scale = distanceToProjectionWindow / depthM;
    
    highp vec2 finalStep = sssWidth * scale * dir;    
    finalStep *= 0.333333; //the kernels range from -3 to 3.

    mediump vec4 colorBlurred = colorM;
    colorBlurred.rgb *= kernel[0].rgb;        
    
    for (int i = 1; i < SSSS_N_SAMPLES; i++) 
    {
        // Fetch color and depth for current sample:
        highp vec2 offset = texcoord + kernel[i].a * finalStep;
        mediump vec4 color = textureLod(sampler2D(colorTex, texSampler), offset, 0.0);
    #if SSSS_FOLLOW_SURFACE == 1                
        float depthN = color.a;
        float s = saturate(300.0 * distanceToProjectionWindow * sssWidth * abs(depthM - depthN));      
        color.rgb = mix(colorM.rgb, color.rgb, s);
    #endif        
        colorBlurred.rgb += kernel[i].rgb * color.rgb;                
    }
    
    return colorBlurred;    
}
