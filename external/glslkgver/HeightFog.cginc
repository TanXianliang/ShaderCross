//HeightFog.cginc
layout(std140) uniform HeightFogData
{
    highp float _FogDensity;  
    highp float _FogHeightFalloff;
    highp float _MinFogOpacity;
    highp float _StartDistance;
    highp float _FogCutoffDistance;
    highp float _FogHeight;
    highp float _DirectionalInscatteringExponent;
    highp float _DirectionalInscatteringStartDistance;    
    
    mediump vec4 _FogColor;
    mediump vec4 _DirectionalInscatteringColor;    
};

mediump vec4 GetHeightFog(highp vec3 pos, mediump vec3 lightDir, highp float ExponentialFogDensity)
{                                       
    highp vec3 worldViewDir = pos;            
    highp float RayLength = length(worldViewDir);
    highp vec3 viewDir = worldViewDir / RayLength;  //归一化
    highp float RayOriginTerms = ExponentialFogDensity;            
    highp float RayDirectionZ = worldViewDir.y;

    //RayLength /= 100.0;
                
    // if it's lower than -127.0, then exp2() goes crazy in OpenGL's GLSL.
    highp float Falloff = max(-127.0, _FogHeightFalloff * RayDirectionZ);   
    // Calculate the "shared" line integral (this term is also used for the directional light inscattering) by adding the two line integrals together (from two different height falloffs and densities)
    highp float ExponentialHeightLineIntegralShared =  ExponentialFogDensity * (1.0 - exp2(-Falloff)) / Falloff * 0.001;                  
    highp float ExponentialHeightLineIntegral = ExponentialHeightLineIntegralShared * max(RayLength - _StartDistance, 0.0);                      
    
    // Setup a cosine lobe around the light direction to approximate inscattering from the directional light off of the ambient haze;
    mediump vec3 DirectionalLightInscattering = _DirectionalInscatteringColor.rgb * pow(saturate(dot(viewDir, lightDir)), _DirectionalInscatteringExponent);

    // Calculate the line integral of the eye ray through the haze, using a special starting distance to limit the inscattering to the distance
    highp float DirExponentialHeightLineIntegral = ExponentialHeightLineIntegralShared * max(RayLength - _DirectionalInscatteringStartDistance, 0.0);
    // Calculate the amount of light that made it through the fog using the transmission equation
    highp float DirectionalInscatteringFogFactor = saturate(exp2(-DirExponentialHeightLineIntegral));
    
    // 来自太阳光的散射
    mediump vec3 DirectionalInscattering = DirectionalLightInscattering * (1.0 - DirectionalInscatteringFogFactor);
    
    highp float ExpFogFactor = max(saturate(exp2(-ExponentialHeightLineIntegral)), _MinFogOpacity);            
    
    if (_FogCutoffDistance != 0.0 && RayLength > _FogCutoffDistance)
    {
       ExpFogFactor = 1.0;
       DirectionalInscattering = vec3(0.0);
    }

    mediump vec4 fogColor = vec4(_FogColor.rgb * (1.0 - ExpFogFactor) + DirectionalInscattering, ExpFogFactor);                         
    return fogColor;
}


