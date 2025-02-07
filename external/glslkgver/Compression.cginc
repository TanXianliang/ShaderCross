//unreal BCCompressionCommon.ush + ETCCompressionCommon.ush
#define BLOCK_MODE_INDIVIDUAL               0
#define BLOCK_MODE_DIFFERENTIAL             1
#define NUM_RGB_TABLES                      8
#define NUM_ALPHA_TABLES                    16

#define HAS_ALPHA 1
#ifndef HAS_ALPHA
#define HAS_ALPHA 1
#endif

#define DIM 4
#define BLOCK_SIZE ((DIM) * (DIM))
#define SMALL_VALUE 0.00001

/*
* supported color_endpoint_mode
*/
#define CEM_LDR_RGB_DIRECT 8
#define CEM_LDR_RGBA_DIRECT 12


vec4 alpha_distance_tables[16] =
{
    vec4(2, 5, 8, 14),
    vec4(2, 6, 9, 12),
    vec4(1, 4, 7, 12),
    vec4(1, 3, 5, 12),
    vec4(2, 5, 7, 11),
    vec4(2, 6, 8, 10),
    vec4(3, 6, 7, 10),
    vec4(2, 4, 7, 10),
    vec4(1, 5, 7, 9),
    vec4(1, 4, 7, 9),
    vec4(1, 3, 7, 9),
    vec4(1, 4, 6, 9),
    vec4(2, 3, 6, 9),
    vec4(0, 1, 2, 9),
    vec4(3, 5, 7, 8),
    vec4(2, 4, 6, 8)
};

vec4 rgb_distance_tables[8] =
{
    vec4(-8, -2, 2, 8),
    vec4(-17, -5, 5, 17),
    vec4(-29, -9, 9, 29),
    vec4(-42, -13, 13, 42),
    vec4(-60, -18, 18, 60),
    vec4(-80, -24, 24, 80),
    vec4(-106, -33, 33, 106),
    vec4(-183, -47, 47, 183)
};

const int integer_from_trits[243] =
{
    0,1,2,    4,5,6,    8,9,10, 
    16,17,18, 20,21,22, 24,25,26,
    3,7,15,   19,23,27, 12,13,14, 
    32,33,34, 36,37,38, 40,41,42,
    48,49,50, 52,53,54, 56,57,58,
    35,39,47, 51,55,59, 44,45,46, 
    64,65,66, 68,69,70, 72,73,74,
    80,81,82, 84,85,86, 88,89,90,
    67,71,79, 83,87,91, 76,77,78,

    128,129,130, 132,133,134, 136,137,138,
    144,145,146, 148,149,150, 152,153,154,
    131,135,143, 147,151,155, 140,141,142,
    160,161,162, 164,165,166, 168,169,170,
    176,177,178, 180,181,182, 184,185,186,
    163,167,175, 179,183,187, 172,173,174,
    192,193,194, 196,197,198, 200,201,202,
    208,209,210, 212,213,214, 216,217,218,
    195,199,207, 211,215,219, 204,205,206,

    96,97,98,    100,101,102, 104,105,106,
    112,113,114, 116,117,118, 120,121,122,
    99,103,111,  115,119,123, 108,109,110, 
    224,225,226, 228,229,230, 232,233,234,
    240,241,242, 244,245,246, 248,249,250,
    227,231,239, 243,247,251, 236,237,238,
    28,29,30,    60,61,62,    92,93,94, 
    156,157,158, 188,189,190, 220,221,222,
    31,63,127,   159,191,255, 252,253,254,

};

// Simple convert vec3 color to 565 uint using 'round' arithmetic
uint vec3ToUint565(in vec3 Color)
{
    vec3 Scale = vec3(31.f, 63.f, 31.f);
    vec3 ColorScaled = round(saturate(Color) * Scale);
    uint ColorPacked = (uint(ColorScaled.r) << 11) | (uint(ColorScaled.g) << 5) | uint(ColorScaled.b);
	
    return ColorPacked;
}

// Convert vec3 color to 565 uint using 'ceil' arithmetic
// Color parameter is inout and is modified to match the converted value
uint vec3ToUint565_Ceil(inout vec3 Color)
{
    vec3 Scale = vec3(31.f, 63.f, 31.f);
    vec3 ColorScaled = ceil(saturate(Color) * Scale);
    uint ColorPacked = (uint(ColorScaled.r) << 11) | (uint(ColorScaled.g) << 5) | uint(ColorScaled.b);
    Color = ColorScaled / Scale;

    return ColorPacked;
}

// Convert vec3 color to 565 uint using 'floor' arithmetic
// Color parameter is inout and is modified to match the converted value
uint vec3ToUint565_Floor(inout vec3 Color)
{
    vec3 Scale = vec3(31.f, 63.f, 31.f);
    vec3 ColorScaled = floor(saturate(Color) * Scale);
    uint ColorPacked = (uint(ColorScaled.r) << 11) | (uint(ColorScaled.g) << 5) | uint(ColorScaled.b);
    Color = ColorScaled / Scale;

    return ColorPacked;
}

// Get min and max values in a single channel block
void GetMinMax(in float Block[16], out float OutMin, out float OutMax)
{
    OutMin = Block[0];
    OutMax = Block[0];

    for (int i = 1; i < 16; ++i)
    {
        OutMin = min(OutMin, Block[i]);
        OutMax = max(OutMax, Block[i]);
    }
}

// Get min and max values in two single channel blocks
void GetMinMax(in float Block0[16], in float Block1[16], out float OutMin0, out float OutMax0, out float OutMin1, out float OutMax1)
{
    OutMin0 = Block0[0];
    OutMax0 = Block0[0];
    OutMin1 = Block1[0];
    OutMax1 = Block1[0];

    for (int i = 1; i < 16; ++i)
    {
        OutMin0 = min(OutMin0, Block0[i]);
        OutMax0 = max(OutMax0, Block0[i]);
        OutMin1 = min(OutMin1, Block1[i]);
        OutMax1 = max(OutMax1, Block1[i]);
    }
}

// Get min and max values in a single three channel block
void GetMinMax(in vec3 Block[16], out vec3 OutMin, out vec3 OutMax)
{
    OutMin = Block[0];
    OutMax = Block[0];

    for (int i = 1; i < 16; ++i)
    {
        OutMin = min(OutMin, Block[i]);
        OutMax = max(OutMax, Block[i]);
    }
}

// Calculate the final packed indices for a color block
uint GetPackedColorBlockIndices(in vec3 Block[16], in vec3 MinColor, in vec3 MaxColor)
{
    uint PackedIndices = 0;

	// Project onto max->min color vector and segment into range [0,3]
    vec3 Range = MinColor - MaxColor;
    float Scale = 3.f / dot(Range, Range);
    vec3 ScaledRange = Range * Scale;
    float Bias = (dot(MaxColor, MaxColor) - dot(MaxColor, MinColor)) * Scale;
	
    for (int i = 15; i >= 0; --i)
    {
		// Compute the distance index for this element
        uint Index = uint(round(dot(Block[i], ScaledRange) + Bias));
		// Convert distance index into the BC index
        uint offset = Index == 3 ? -2 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (3 * (Index == 3));
        Index += offset;
		// OR into the final PackedIndices
        PackedIndices = (PackedIndices << 2) | Index;
    }

    return PackedIndices;
}

// Calculate the final packed indices for an alpha block
// The results are in their final location of the uvec2 indices and will need ORing with the min and max alpha
uvec2 GetPackedAlphaBlockIndices(in float Block[16], in float MinAlpha, in float MaxAlpha)
{
    uvec2 PackedIndices = uvec2(0);

	// Segment alpha max->min into range [0,7]
    float Range = MinAlpha - MaxAlpha;
    float Scale = 7.f / Range;
    float Bias = -Scale * MaxAlpha;

    uint i = 0;
	// The first 5 elements of the block will go into the top 16 bits of the x component
    for (i = 0; i < 5; ++i)
    {
		// Compute the distance index for this element
        uint Index = uint(round(Block[i] * Scale + Bias));
		// Convert distance index into the BC index
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7));
        Index += offset;
		// OR into the final PackedIndices
        PackedIndices.x |= (Index << (i * 3 + 16));
    }

	// The 6th element is split across the x and y components
	{
        i = 5;
        uint Index = uint(round(Block[i] * Scale + Bias));
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7));
        Index += offset;
        PackedIndices.x |= (Index << 31);
        PackedIndices.y |= (Index >> 1);
    }

	// The rest of the elements fill the y component
    for (i = 6; i < 16; ++i)
    {
        uint Index = uint(round(Block[i] * Scale + Bias));
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7 ? 1 : 0));
        Index += offset;
        PackedIndices.y |= (Index << (i * 3 - 16));
    }

    return PackedIndices;
}

// Compress a BC1 block
uvec2 CompressBC1Block(in vec3 Block[16])
{
    vec3 MinColor, MaxColor;
    GetMinMax(Block, MinColor, MaxColor);

    uint MinColor565 = vec3ToUint565_Floor(MinColor);
    uint MaxColor565 = vec3ToUint565_Ceil(MaxColor);

    uint Indices = 0;
    if (MinColor565 < MaxColor565)
    {
        Indices = GetPackedColorBlockIndices(Block, MinColor, MaxColor);
    }
    
    //return uvec4(MaxColor565, MinColor565, Indices & 0xffff, Indices >> 16);
    return uvec2((MinColor565 << 16) | MaxColor565, Indices);
}

// Compress a BC1 block that will be sampled as sRGB
// We expect linear colors as input and convert internally
uvec2 CompressBC1Block_SRGB(in vec3 Block[16])
{
    vec3 MinColor, MaxColor;
    GetMinMax(Block, MinColor, MaxColor);

    uint MinColor565 = vec3ToUint565(LinearToSrgb(MinColor));
    uint MaxColor565 = vec3ToUint565(LinearToSrgb(MaxColor));

    uint Indices = 0;
    if (MinColor565 < MaxColor565)
    {
        Indices = GetPackedColorBlockIndices(Block, MinColor, MaxColor);
    }
    
    //return uvec4(MaxColor565, MinColor565, Indices & 0xffff, Indices >> 16);
    return uvec2((MinColor565 << 16) | MaxColor565, Indices);
}

// Compress a BC3 block
uvec4 CompressBC3Block(in vec3 BlockRGB[16], in float BlockA[16])
{
    vec3 MinColor, MaxColor;
    GetMinMax(BlockRGB, MinColor, MaxColor);

    uint MinColor565 = vec3ToUint565_Floor(MinColor);
    uint MaxColor565 = vec3ToUint565_Ceil(MaxColor);

    float MinAlpha, MaxAlpha;
    GetMinMax(BlockA, MinAlpha, MaxAlpha);

    uint MinAlphaUint = uint(round(MinAlpha * 255.f));
    uint MaxAlphaUint = uint(round(MaxAlpha * 255.f));

    uint ColorIndices = 0;
    if (MinColor565 < MaxColor565)
    {
        ColorIndices = GetPackedColorBlockIndices(BlockRGB, MinColor, MaxColor);
    }

    uvec2 AlphaIndices = uvec2(0);
    if (MinAlphaUint < MaxAlphaUint)
    {
        AlphaIndices = GetPackedAlphaBlockIndices(BlockA, MinAlpha, MaxAlpha);
    }

    return uvec4((MinAlphaUint << 8) | MaxAlphaUint | AlphaIndices.x, AlphaIndices.y, (MinColor565 << 16) | MaxColor565, ColorIndices);
}

// Compress a BC3 block that will be sampled as sRGB
// We expect linear colors as input and convert internally
uvec4 CompressBC3Block_SRGB(in vec3 BlockRGB[16], in float BlockA[16])
{	
    vec3 MinColor, MaxColor;
    GetMinMax(BlockRGB, MinColor, MaxColor);

    uint MinColor565 = vec3ToUint565(LinearToSrgb(MinColor));
    uint MaxColor565 = vec3ToUint565(LinearToSrgb(MaxColor));

    float MinAlpha, MaxAlpha;
    GetMinMax(BlockA, MinAlpha, MaxAlpha);

    uint MinAlphaUint = uint(round(MinAlpha * 255.f));
    uint MaxAlphaUint = uint(round(MaxAlpha * 255.f));

    uint ColorIndices = 0;
    if (MinColor565 < MaxColor565)
    {
        ColorIndices = GetPackedColorBlockIndices(BlockRGB, MinColor, MaxColor);
    }

    uvec2 AlphaIndices = uvec2(0);
    if (MinAlphaUint < MaxAlphaUint)
    {
        AlphaIndices = GetPackedAlphaBlockIndices(BlockA, MinAlpha, MaxAlpha);
    }

    return uvec4((MinAlphaUint << 8) | MaxAlphaUint | AlphaIndices.x, AlphaIndices.y, (MinColor565 << 16) | MaxColor565, ColorIndices);
}

// Compress a BC4 block
uvec2 CompressBC4Block(in float Block[16])
{
    float MinAlpha, MaxAlpha;
    GetMinMax(Block, MinAlpha, MaxAlpha);

    uint MinAlphaUint = uint(round(MinAlpha * 255.f));
    uint MaxAlphaUint = uint(round(MaxAlpha * 255.f));

    uvec2 Indices = uvec2(0);
    if (MinAlphaUint < MaxAlphaUint)
    {
        Indices = GetPackedAlphaBlockIndices(Block, MinAlpha, MaxAlpha);
    }

    uvec2 BC4Block = uvec2(0);
    BC4Block.x = (MinAlphaUint << 8) | MaxAlphaUint;
    BC4Block.x |= Indices.x >> 16;
    BC4Block.y = Indices.y & 0xffff;
    BC4Block.y |= Indices.y >> 16;
    return BC4Block;
}

// Compress a BC5 block
uvec4 CompressBC5Block(in float BlockU[16], in float BlockV[16])
{
    float MinU, MaxU, MinV, MaxV;
    GetMinMax(BlockU, BlockV, MinU, MaxU, MinV, MaxV);

    uint MinUUint = uint(round(MinU * 255.f));
    uint MaxUUint = uint(round(MaxU * 255.f));
    uint MinVUint = uint(round(MinV * 255.f));
    uint MaxVUint = uint(round(MaxV * 255.f));

    uvec2 IndicesU = uvec2(0);
    if (MinUUint < MaxUUint)
    {
        IndicesU = GetPackedAlphaBlockIndices(BlockU, MinU, MaxU);
    }

    uvec2 IndicesV = uvec2(0);
    if (MinVUint < MaxVUint)
    {
        IndicesV = GetPackedAlphaBlockIndices(BlockV, MinV, MaxV);
    }

    return uvec4((MinUUint << 8) | MaxUUint | IndicesU.x, IndicesU.y, (MinVUint << 8) | MaxVUint | IndicesV.x, IndicesV.y);
}

// Get a single scale factor to use for a YCoCg color block
// This increases precision at the expense of potential blending artifacts across blocks
vec2 GetYCoCgScale(vec2 MinCoCg, vec2 MaxCoCg)
{
    MinCoCg = abs(MinCoCg - 128.f / 255.f);
    MaxCoCg = abs(MaxCoCg - 128.f / 255.f);

    float MaxComponent = max(max(MinCoCg.x, MinCoCg.y), max(MaxCoCg.x, MaxCoCg.y));

    return (MaxComponent < 32.f / 255.f) ? vec2(4.f, 0.25f) : (MaxComponent < 64.f / 255.f) ? vec2(2.f, 0.5f) : vec2(1.f, 1.f);
}

void ApplyYCoCgScale(inout vec2 MinCoCg, inout vec2 MaxCoCg, float Scale)
{
    MinCoCg = (MinCoCg - 128.f / 255.f) * Scale + 128.f / 255.f;
    MaxCoCg = (MaxCoCg - 128.f / 255.f) * Scale + 128.f / 255.f;
}

// Inset the CoCg bounding end points
void InsetCoCgEndPoints(inout vec2 MinCoCg, inout vec2 MaxCoGg)
{
    vec2 Inset = (MaxCoGg - MinCoCg - (8.f / 255.f)) / 16.f;
    MinCoCg = saturate(MinCoCg + Inset);
    MaxCoGg = saturate(MaxCoGg - Inset);
}

// Inset the luminance end points
void InsetLumaEndPoints(inout float MinY, inout float MaxY)
{
    float Inset = (MaxY - MinY - (16.f / 255.f)) / 32.f;
    MinY = saturate(MinY + Inset);
    MaxY = saturate(MaxY - Inset);
}

// Select the 2 min/max end points from the CoCg bounding rectangle based on the block contents 
void AdjustMinMaxDiagonalYCoCg(const vec3 Block[16], inout vec2 MinCoCg, inout vec2 MaxCoGg)
{
    vec2 MidCoCg = (MaxCoGg + MinCoCg) * 0.5;

    float Sum = 0.f;
    for (int i = 0; i < 16; i++)
    {
        vec2 Diff = Block[i].yz - MidCoCg;
        Sum += Diff.x * Diff.y;
    }
    if (Sum < 0.f)
    {
        float Temp = MaxCoGg.y;
        MaxCoGg.y = MinCoCg.y;
        MinCoCg.y = Temp;
    }
}

uint CoCgToUint565(inout vec2 CoCg)
{
    vec2 Scale = vec2(31.f, 63.f);
    vec2 ColorScaled = round(saturate(CoCg) * Scale);
    uint ColorPacked = (uint(ColorScaled.r) << 11) | (uint(ColorScaled.g) << 5);
    CoCg = ColorScaled / Scale;

    return ColorPacked;
}

// Calculate the final packed indices for the CoCg part of a color block
uint GetPackedCoCgBlockIndices(in vec3 Block[16], in vec2 MinCoCg, in vec2 MaxCoCg)
{
    uint PackedIndices = 0;

	// Project onto max->min color vector and segment into range [0,3]
    vec2 Range = MinCoCg - MaxCoCg;
    float Scale = 3.0 / dot(Range, Range);
    vec2 ScaledRange = Range * Scale;
    float Bias = (dot(MaxCoCg, MaxCoCg) - dot(MaxCoCg, MinCoCg)) * Scale;

    for (int i = 15; i >= 0; --i)
    {
		// Compute the distance index for this element
        uint Index = uint(round(dot(Block[i].yz, ScaledRange) + Bias));
		// Convert distance index into the BC index
        uint offset = Index == 3 ? -2 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (3 * (Index == 3));
        Index += offset;
		// OR into the final PackedIndices
        PackedIndices = (PackedIndices << 2) | Index;
    }

    return PackedIndices;
}

// Calculate the final packed indices for the Luma part of a color block
uvec2 GetPackedLumaBlockIndices(in vec3 Block[16], in float MinAlpha, in float MaxAlpha)
{
    uvec2 PackedIndices = uvec2(0);

	// Segment alpha max->min into range [0,7]
    float Range = MinAlpha - MaxAlpha;
    float Scale = 7.f / Range;
    float Bias = -Scale * MaxAlpha;

    uint i = 0;
	// The first 5 elements of the block will go into the top 16 bits of the x component
    for (i = 0; i < 5; ++i)
    {
		// Compute the distance index for this element
        uint Index = uint(round(Block[i].x * Scale + Bias));
		// Convert distance index into the BC index
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7));
        Index += offset;
		// OR into the final PackedIndices
        PackedIndices.x |= (Index << (i * 3 + 16));
    }

	// The 6th element is split across the x and y components
	{
        i = 5;
        uint Index = uint(round(Block[i].x * Scale + Bias));
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7));
        Index += offset;
        PackedIndices.x |= (Index << 31);
        PackedIndices.y |= (Index >> 1);
    }

	// The rest of the elements fill the y component
    for (i = 6; i < 16; ++i)
    {
        uint Index = uint(round(Block[i].x * Scale + Bias));
        uint offset = Index == 7 ? -6 : (Index > 0 ? 1 : 0);
        //Index += (Index > 0) - (7 * (Index == 7 ? 1 : 0));
        Index += offset;
        PackedIndices.y |= (Index << (i * 3 - 16));
    }

    return PackedIndices;
}

// Convert a linear RGB block to YCoCg and compress a BC3 block 
uvec4 CompressBC3BlockYCoCg(in vec3 Block[16])
{
    for (int i = 0; i < 16; ++i)
    {
        Block[i] = RGBToYCoCg(Block[i]);
    }

    vec3 MinColor, MaxColor;
    GetMinMax(Block, MinColor, MaxColor);

    AdjustMinMaxDiagonalYCoCg(Block, MinColor.yz, MaxColor.yz);

    vec2 Scale = GetYCoCgScale(MinColor.yz, MaxColor.yz);

    ApplyYCoCgScale(MinColor.yz, MaxColor.yz, Scale.x);

    InsetCoCgEndPoints(MinColor.yz, MaxColor.yz);

    uint MinColor565 = CoCgToUint565(MinColor.yz) | (uint(Scale.x) - 1);
    uint MaxColor565 = CoCgToUint565(MaxColor.yz) | (uint(Scale.x) - 1);

    ApplyYCoCgScale(MinColor.yz, MaxColor.yz, Scale.y);

    uint CoCgIndices = GetPackedCoCgBlockIndices(Block, MinColor.yz, MaxColor.yz);

    InsetLumaEndPoints(MinColor.x, MaxColor.x);

    uint MinLumaUint = uint(round(MinColor.x * 255.0f));
    uint MaxLumaUint = uint(round(MaxColor.x * 255.0f));

    uvec2 Indices = GetPackedLumaBlockIndices(Block, MinColor.x, MaxColor.x);

    return uvec4((MinLumaUint << 8) | MaxLumaUint | Indices.x, Indices.y, (MinColor565 << 16) | MaxColor565, CoCgIndices);
}

void Swap(inout uvec3 A, inout uvec3 B)
{
    uvec3 Temp = A;
    A = B;
    B = Temp;
}

void Swap(inout vec3 A, inout vec3 B)
{
    vec3 Temp = A;
    A = B;
    B = Temp;
}

void Swap(inout vec4 A, inout vec4 B)
{
    vec4 Temp = A;
    A = B;
    B = Temp;
}


void Swap(inout float A, inout float B)
{
    float Temp = A;
    A = B;
    B = Temp;
}

vec3 Quantize10(vec3 X)
{
    return (f32tof16(X) * 1024.0f) / (0x7bff + 1.0f);
}

uint ComputeIndexBC6HIndex(vec3 Color, vec3 BlockVector, float EndPoint0Pos, float EndPoint1Pos)
{
    float Pos = float(f32tof16(dot(Color, BlockVector)));
    float NormalizedPos = saturate((Pos - EndPoint0Pos) / (EndPoint1Pos - EndPoint0Pos));
    return uint(clamp(NormalizedPos * 14.93333f + 0.03333f + 0.5f, 0.0f, 15.0f));
}

// Compress a BC6H block. Evaluates only mode 11 for performance
uvec4 CompressBC6HBlock(in vec3 BlockRGB[16])
{
    // Compute initial endpoints
    vec3 BlockMin = BlockRGB[0];
    vec3 BlockMax = BlockRGB[0];
    {
        for (uint TexelIndex = 1; TexelIndex < 16; ++TexelIndex)
        {
            BlockMin = min(BlockMin, BlockRGB[TexelIndex]);
            BlockMax = max(BlockMax, BlockRGB[TexelIndex]);
        }
    }

    vec3 BlockVector = BlockMax - BlockMin;
    BlockVector = BlockVector / (BlockVector.x + BlockVector.y + BlockVector.z);

    vec3 Endpoint0 = Quantize10(BlockMin);
    vec3 Endpoint1 = Quantize10(BlockMax);
    float EndPoint0Pos = float(f32tof16(dot(BlockMin, BlockVector)));
    float EndPoint1Pos = float(f32tof16(dot(BlockMax, BlockVector)));

    // Check if endpoint swap is required
    uint FixupIndex = ComputeIndexBC6HIndex(BlockRGB[0], BlockVector, EndPoint0Pos, EndPoint1Pos);
    if (FixupIndex > 7)
    {
        Swap(EndPoint0Pos, EndPoint1Pos);
        Swap(Endpoint0, Endpoint1);
    }

    // Compute indices
    uint Indices[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (uint TexelIndex = 0; TexelIndex < 16; ++TexelIndex)
    {
        Indices[TexelIndex] = ComputeIndexBC6HIndex(BlockRGB[TexelIndex], BlockVector, EndPoint0Pos, EndPoint1Pos);
    }

    // Encode mode 11 block
    uvec4 Block = uvec4(0x03, 0, 0, 0);     
    // Encode endpoints
    Block.x |= uint(Endpoint0.x) << 5;
    Block.x |= uint(Endpoint0.y) << 15;
    Block.x |= uint(Endpoint0.z) << 25;
    Block.y |= uint(Endpoint0.z) >> 7;
    Block.y |= uint(Endpoint1.x) << 3;
    Block.y |= uint(Endpoint1.y) << 13;
    Block.y |= uint(Endpoint1.z) << 23;
    Block.z |= uint(Endpoint1.z) >> 9;

    // Encode indices
    Block.z |= Indices[0] << 1;
    Block.z |= Indices[1] << 4;
    Block.z |= Indices[2] << 8;
    Block.z |= Indices[3] << 12;
    Block.z |= Indices[4] << 16;
    Block.z |= Indices[5] << 20;
    Block.z |= Indices[6] << 24;
    Block.z |= Indices[7] << 28;
    Block.w |= Indices[8] << 0;
    Block.w |= Indices[9] << 4;
    Block.w |= Indices[10] << 8;
    Block.w |= Indices[11] << 12;
    Block.w |= Indices[12] << 16;
    Block.w |= Indices[13] << 20;
    Block.w |= Indices[14] << 24;
    Block.w |= Indices[15] << 28;    

    return Block;
}

uvec3 Quantize7(vec3 X)
{
    return (uvec3(X * 0xFF)) >> 1;
}

uint ComputeBC7Index(vec3 Color, vec3 BlockVector, float EndPoint0Pos, float EndPoint1Pos)
{
    float Pos = dot(Color, BlockVector);
    float NormalizedPos = saturate((Pos - EndPoint0Pos) / (EndPoint1Pos - EndPoint0Pos));
    return uint(clamp(NormalizedPos * 14.93333f + 0.03333f + 0.5f, 0.0f, 15.0f));
}

// Least squares optimization to find best endpoints for the selected block indices in a mode 6 BC7 block
void OptimizeEndpointsBC7(in vec3 Texels[16], inout vec3 BlockMin, inout vec3 BlockMax)
{
    vec3 BlockVector = BlockMax - BlockMin;

    float EndPoint0Pos = dot(BlockMin, BlockVector);
    float EndPoint1Pos = dot(BlockMax, BlockVector);

    vec3 AlphaTexelSum = vec3(0.0);
    vec3 BetaTexelSum = vec3(0.0);
    float AlphaBetaSum = 0.0f;
    float AlphaSqSum = 0.0f;
    float BetaSqSum = 0.0f;

    for (uint TexelIndex = 0; TexelIndex < 16; ++TexelIndex)
    {
        uint Index = ComputeBC7Index(Texels[TexelIndex], BlockVector, EndPoint0Pos, EndPoint1Pos);

        float Beta = saturate(Index / 15.0f);
        float Alpha = 1.0f - Beta;

        AlphaTexelSum += Alpha * Texels[TexelIndex];
        BetaTexelSum += Beta * Texels[TexelIndex];

        AlphaBetaSum += Alpha * Beta;

        AlphaSqSum += Alpha * Alpha;
        BetaSqSum += Beta * Beta;
    }

    float Det = AlphaSqSum * BetaSqSum - AlphaBetaSum * AlphaBetaSum;

    if (abs(Det) > 0.1f)
    {
        float RcpDet = 1.0 / Det;
        BlockMin = saturate(RcpDet * (AlphaTexelSum * BetaSqSum - BetaTexelSum * AlphaBetaSum));
        BlockMax = saturate(RcpDet * (BetaTexelSum * AlphaSqSum - AlphaTexelSum * AlphaBetaSum));
    }
}

// Compress a BC7 color only block. Evaluates only mode 6 for performance
uvec4 CompressBC7Block(in vec3 BlockRGB[16])
{
    // Compute initial endpoints
    vec3 BlockMin = BlockRGB[0];
    vec3 BlockMax = BlockRGB[0];
    {
        for (uint TexelIndex = 1; TexelIndex < 16; ++TexelIndex)
        {
            BlockMin = min(BlockMin, BlockRGB[TexelIndex]);
            BlockMax = max(BlockMax, BlockRGB[TexelIndex]);
        }
    }

    #if LEAST_SQUARES_ENDPOINT_OPTIMIZATION
    {
        OptimizeEndpointsBC7(BlockRGB, BlockMin, BlockMax);
    }
    #endif

    vec3 BlockVector = BlockMax - BlockMin;

    uvec3 Endpoint0 = Quantize7(BlockMin);
    uvec3 Endpoint1 = Quantize7(BlockMax);
    float EndPoint0Pos = dot(BlockMin, BlockVector);
    float EndPoint1Pos = dot(BlockMax, BlockVector);

    // Check if endpoint swap is required
    uint FixupIndex = ComputeBC7Index(BlockRGB[0], BlockVector, EndPoint0Pos, EndPoint1Pos);
    if (FixupIndex > 7)
    {
        Swap(EndPoint0Pos, EndPoint1Pos);
        Swap(Endpoint0, Endpoint1);
    }

    // Compute indices
    uint Indices[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (uint TexelIndex = 0; TexelIndex < 16; ++TexelIndex)
    {
        Indices[TexelIndex] = ComputeBC7Index(BlockRGB[TexelIndex], BlockVector, EndPoint0Pos, EndPoint1Pos);
    }
    
    // Encode mode 6 block
    uvec4 Block = uvec4(0x40, 0, 0, 0);

    // Encode endpoints
    Block.x |= Endpoint0.x << 7;
    Block.x |= Endpoint1.x << 14;
    Block.x |= Endpoint0.y << 21;
    Block.x |= Endpoint1.y << 28;
    Block.y |= Endpoint1.y >> 4;
    Block.y |= Endpoint0.z << 3;
    Block.y |= Endpoint1.z << 10;

    // Encode endpoint p-bit

    // Encode indices
    Block.z |= Indices[0] << 1;
    Block.z |= Indices[1] << 4;
    Block.z |= Indices[2] << 8;
    Block.z |= Indices[3] << 12;
    Block.z |= Indices[4] << 16;
    Block.z |= Indices[5] << 20;
    Block.z |= Indices[6] << 24;
    Block.z |= Indices[7] << 28;
    Block.w |= Indices[8] << 0;
    Block.w |= Indices[9] << 4;
    Block.w |= Indices[10] << 8;
    Block.w |= Indices[11] << 12;
    Block.w |= Indices[12] << 16;
    Block.w |= Indices[13] << 20;
    Block.w |= Indices[14] << 24;
    Block.w |= Indices[15] << 28;    

    return Block;
}

/** Read a 4x4 color block ready for BC1 compression. */
void ReadBlockRGB(texture2D SourceTexture, sampler TextureSampler, vec2 UV, vec2 TexelUVSize, out vec3 Block[16])
{
    {
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 2);
        Block[0] = vec3(Red[3], Green[3], Blue[3]);
        Block[1] = vec3(Red[2], Green[2], Blue[2]);
        Block[4] = vec3(Red[0], Green[0], Blue[0]);
        Block[5] = vec3(Red[1], Green[1], Blue[1]);
    }
    {
        vec2 UVOffset = UV + vec2(2.0 * TexelUVSize.x, 0.0);
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        Block[2] = vec3(Red[3], Green[3], Blue[3]);
        Block[3] = vec3(Red[2], Green[2], Blue[2]);
        Block[6] = vec3(Red[0], Green[0], Blue[0]);
        Block[7] = vec3(Red[1], Green[1], Blue[1]);
    }
    {
        vec2 UVOffset = UV + vec2(0.0, 2.0 * TexelUVSize.y);
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        Block[8] = vec3(Red[3], Green[3], Blue[3]);
        Block[9] = vec3(Red[2], Green[2], Blue[2]);
        Block[12] = vec3(Red[0], Green[0], Blue[0]);
        Block[13] = vec3(Red[1], Green[1], Blue[1]);
    }
    {
        vec2 UVOffset = UV + 2.0 * TexelUVSize;
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        Block[10] = vec3(Red[3], Green[3], Blue[3]);
        Block[11] = vec3(Red[2], Green[2], Blue[2]);
        Block[14] = vec3(Red[0], Green[0], Blue[0]);
        Block[15] = vec3(Red[1], Green[1], Blue[1]);
    }
}

void ReadBlockAlpha(texture2D SourceTexture, sampler TextureSampler, vec2 UV, vec2 TexelUVSize, out float Block[16])
{
    {        
        vec4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 3);

        Block[0] = Alpha[3];
        Block[1] = Alpha[2];
        Block[4] = Alpha[0];
        Block[5] = Alpha[1];
    }
    {
        vec2 UVOffset = UV + vec2(2.0 * TexelUVSize.x, 0.0);
        vec4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);

        Block[2] = Alpha[3];
        Block[3] = Alpha[2];
        Block[6] = Alpha[0];
        Block[7] = Alpha[1];
    }
    {
        vec2 UVOffset = UV + vec2(0.0, 2.0 * TexelUVSize.y);
        vec4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);
        Block[8] = Alpha[3];
        Block[9] = Alpha[2];
        Block[12] = Alpha[0];
        Block[13] = Alpha[1];
    }
    {
        vec2 UVOffset = UV + 2.0 * TexelUVSize;
        vec4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);
        Block[10] = Alpha[3];
        Block[11] = Alpha[2];
        Block[14] = Alpha[0];
        Block[15] = Alpha[1];
    }
}

vec4 ReadBlockRGBAUNorm8(texture2D SourceTexture, sampler TextureSampler, vec2 UV, vec2 TexelUVSize, out vec4 Block[16])
{
    vec4 sum = vec4(0.0);
    {                    
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 2);
        half4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 3);
        
        Block[0] = vec4(Red[3], Green[3], Blue[3], Alpha[3]) * 255.0;
        Block[1] = vec4(Red[2], Green[2], Blue[2], Alpha[2]) * 255.0;
        Block[4] = vec4(Red[0], Green[0], Blue[0], Alpha[0]) * 255.0;
        Block[5] = vec4(Red[1], Green[1], Blue[1], Alpha[1]) * 255.0;
        sum += (Block[0] + Block[1] + Block[4] + Block[5]);
    }    
    {
        vec2 UVOffset = UV + vec2(2.0 * TexelUVSize.x, 0.0);
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        half4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);

        Block[2] = vec4(Red[3], Green[3], Blue[3], Alpha[3]) * 255.0;
        Block[3] = vec4(Red[2], Green[2], Blue[2], Alpha[2]) * 255.0;
        Block[6] = vec4(Red[0], Green[0], Blue[0], Alpha[0]) * 255.0;
        Block[7] = vec4(Red[1], Green[1], Blue[1], Alpha[1]) * 255.0;
        sum += (Block[2] + Block[3] + Block[6] + Block[7]);
    }
    {
        vec2 UVOffset = UV + vec2(0.0, 2.0 * TexelUVSize.y);
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        half4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);

        Block[8] = vec4(Red[3], Green[3], Blue[3], Alpha[3]) * 255.0;
        Block[9] = vec4(Red[2], Green[2], Blue[2], Alpha[2]) * 255.0;
        Block[12] = vec4(Red[0], Green[0], Blue[0], Alpha[0]) * 255.0;
        Block[13] = vec4(Red[1], Green[1], Blue[1], Alpha[1]) * 255.0;
        sum += (Block[8] + Block[9] + Block[12] + Block[13]);
    }
    {
        vec2 UVOffset = UV + 2.0 * TexelUVSize;
        half4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        half4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        half4 Blue = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 2);
        half4 Alpha = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 3);

        Block[10] = vec4(Red[3], Green[3], Blue[3], Alpha[3]) * 255.0;
        Block[11] = vec4(Red[2], Green[2], Blue[2], Alpha[2]) * 255.0;
        Block[14] = vec4(Red[0], Green[0], Blue[0], Alpha[0]) * 255.0;
        Block[15] = vec4(Red[1], Green[1], Blue[1], Alpha[1]) * 255.0;
        sum += (Block[10] + Block[11] + Block[14] + Block[15]);
    }
    return sum / 16.0;
}

// Read a 4x4 block of XY channels from a normal texture ready for BC5 compression.
void ReadBlockXY(texture2D SourceTexture, sampler TextureSampler, vec2 UV, vec2 TexelUVSize, out float BlockX[16], out float BlockY[16])
{
    {
        vec4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 0);
        vec4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UV, 1);
        BlockX[0] = Red[3]; BlockY[0] = Green[3];
        BlockX[1] = Red[2]; BlockY[1] = Green[2];
        BlockX[4] = Red[0]; BlockY[4] = Green[0];
        BlockX[5] = Red[1]; BlockY[5] = Green[1];
    }
    {
        vec2 UVOffset = UV + vec2(2.0 * TexelUVSize.x, 0.0);
        vec4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        vec4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        BlockX[2] = Red[3]; BlockY[2] = Green[3];
        BlockX[3] = Red[2]; BlockY[3] = Green[2];
        BlockX[6] = Red[0]; BlockY[6] = Green[0];
        BlockX[7] = Red[1]; BlockY[7] = Green[1];
    }
    {
        vec2 UVOffset = UV + vec2(0.0, 2.0 * TexelUVSize.y);
        vec4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        vec4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        BlockX[8] = Red[3]; BlockY[8] = Green[3];
        BlockX[9] = Red[2]; BlockY[9] = Green[2];
        BlockX[12] = Red[0]; BlockY[12] = Green[0];
        BlockX[13] = Red[1]; BlockY[13] = Green[1];
    }
    {
        vec2 UVOffset = UV + 2.0 * TexelUVSize;
        vec4 Red = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 0);
        vec4 Green = textureGather(sampler2D(SourceTexture, TextureSampler), UVOffset, 1);
        BlockX[10] = Red[3]; BlockY[10] = Green[3];
        BlockX[11] = Red[2]; BlockY[11] = Green[2];
        BlockX[14] = Red[0]; BlockY[14] = Green[0];
        BlockX[15] = Red[1]; BlockY[15] = Green[1];
    }
}


void SelectAlphaMod(float SourceAlpha, float EncodedAlpha, int IndexInTable, inout int SelectedIndex, inout float MinDif)
{
    float Dif = abs(EncodedAlpha - SourceAlpha);
    if (Dif < MinDif)
    {
        MinDif = Dif;
        SelectedIndex = IndexInTable;
    }
}


ivec3 FloatColorToUint555(in vec3 FloatColor)
{
    vec3 Scale = vec3(31.f, 31.f, 31.f);
    vec3 ColorScaled = round(saturate(FloatColor) * Scale);
    return ivec3(ColorScaled);
}

vec3 ExpandColor444(in ivec3 Color444)
{
    ivec3 Color888 = (Color444 << 4) + Color444;
    return Color888 / 255.f;
}

vec3 ExpandColor555(in ivec3 Color555)
{
    ivec3 Color888 = (Color555 << 3) + (Color555 >> 2);
    return Color888 / 255.f;
}

uint SwapEndian32(in uint x)
{
    return ((x & 0x0000ff) << 24) | ((x & 0x00ff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
}

uint SelectRGBTableIndex(float LuminanceR)
{
    // guess a table using sub-block luminance range
    float Range = (LuminanceR + LuminanceR * 0.1) * 255.0;
    if (Range < 8.0)
    {
        return 0;
    }
    else if (Range < 17.0)
    {
        return 1;
    }
    else if (Range < 29.0)
    {
        return 2;
    }
    else if (Range < 42.0)
    {
        return 3;
    }
    else if (Range < 60.0)
    {
        return 4;
    }
    else if (Range < 80.0)
    {
        return 5;
    }
    else if (Range < 106.0)
    {
        return 6;
    }
    return 7;
}


void FindPixelWeights(in vec3 Block[16], in vec3 BaseColor, in uint TableIdx, in int StartX, in int EndX, in int StartY, in int EndY, out uint SubBlockWeights)
{
    //int PIXEL_INDEX_ENCODE_TABLE[4] = { 3, 2, 0, 1 };
    SubBlockWeights = 0;
    
    float TableRangeMax = rgb_distance_tables[TableIdx].w / 255.f;
    float BaseLum = Luminance(BaseColor);

    for (int Y = StartY; Y < EndY; ++Y)
    {
        for (int X = StartX; X < EndX; ++X)
        {
            vec3 OrigColor = Block[4 * Y + X];
            float Diff = Luminance(OrigColor) - BaseLum;
            int EncIndex = 0;
            if (Diff < 0.f)
            {
                EncIndex = (-Diff * 1.58) > TableRangeMax ? 3 : 2;
            }
            else
            {
                EncIndex = (Diff * 1.58) > TableRangeMax ? 1 : 0;
            }
            //int EncIndex = PIXEL_INDEX_ENCODE_TABLE[SelectedIndex];
            int IndexInBlock = X * 4 + Y;
            SubBlockWeights |= ((EncIndex & 1) << IndexInBlock) | ((EncIndex >> 1) << (16 + IndexInBlock));
        }
    }
}

uvec2 CompressBlock_ETC2_RGB(in vec3 Block[16])
{
    // Always use side-by-side mode (flip bit set to 0).
    uint FlipBit = 0;
    
    vec3 BaseColor1_Float = (Block[0] + Block[1] + Block[4] + Block[5] + Block[8] + Block[9] + Block[12] + Block[13]) * 0.125;
    vec3 BaseColor2_Float = (Block[2] + Block[3] + Block[6] + Block[7] + Block[10] + Block[11] + Block[14] + Block[15]) * 0.125;
        
    ivec3 BaseColor1 = FloatColorToUint555(BaseColor1_Float);
    ivec3 BaseColor2 = FloatColorToUint555(BaseColor2_Float);
    ivec3 Diff = BaseColor2 - BaseColor1;

    uint ColorBits;
    vec3 BaseColor1_Quant, BaseColor2_Quant;

    uint BlockMode;
    int MinDiff = -4;
    int MaxDiff = 3;

    bvec3 flag0 = bvec3(Diff.x > MinDiff, Diff.y > MinDiff, Diff.z > MinDiff);
    bvec3 flag1 = bvec3(Diff.x < MaxDiff, Diff.y < MaxDiff, Diff.z < MaxDiff);

    if (all(flag0) && all(flag1))
    {
        // We can use differential mode.
        BlockMode = BLOCK_MODE_DIFFERENTIAL;
        ColorBits = ((Diff.b & 7) << 16) | (BaseColor1.b << 19) | ((Diff.g & 7) << 8) | (BaseColor1.g << 11) | (Diff.r & 7) | (BaseColor1.r << 3);
        BaseColor1_Quant = ExpandColor555(BaseColor1);
        BaseColor2_Quant = ExpandColor555(BaseColor2);
    }
    else
    {
        // We must use the lower precision individual mode.
        BlockMode = BLOCK_MODE_INDIVIDUAL;
        BaseColor1 >>= 1;
        BaseColor2 >>= 1;
        ColorBits = (BaseColor1.b << 20) | (BaseColor2.b << 16) | (BaseColor1.g << 12) | (BaseColor2.g << 8) | (BaseColor1.r << 4) | BaseColor2.r;
        BaseColor1_Quant = ExpandColor444(BaseColor1);
        BaseColor2_Quant = ExpandColor444(BaseColor2);
    }

    float l00 = Luminance(Block[0]);
    float l08 = Luminance(Block[8]);
    float l13 = Luminance(Block[13]);
    float LuminanceR1 = (max3(l00, l08, l13) - min3(l00, l08, l13)) * 0.5;
    uint SubBlock1TableIdx = SelectRGBTableIndex(LuminanceR1);
    uint SubBlock1Weights = 0;
    FindPixelWeights(Block, BaseColor1_Quant, SubBlock1TableIdx, 0, 2, 0, 4, SubBlock1Weights);

    float l02 = Luminance(Block[2]);
    float l10 = Luminance(Block[10]);
    float l15 = Luminance(Block[15]);
    float LuminanceR2 = (max3(l02, l10, l15) - min3(l02, l10, l15)) * 0.5;
    uint SubBlock2TableIdx = SelectRGBTableIndex(LuminanceR2);
    uint SubBlock2Weights = 0;
    FindPixelWeights(Block, BaseColor2_Quant, SubBlock2TableIdx, 2, 4, 0, 4, SubBlock2Weights);
    
    // Both these values need to be big-endian. We can build ModeBits directly in big-endian layout, but for IndexBits
    // it's too hard, so we'll just swap here.
    uint ModeBits = (SubBlock1TableIdx << 29) | (SubBlock2TableIdx << 26) | (BlockMode << 25) | (FlipBit << 24) | ColorBits;
    uint IndexBits = SwapEndian32(SubBlock1Weights | SubBlock2Weights);

    return uvec2(ModeBits, IndexBits);
}

uvec2 CompressBlock_ETC2_Alpha(in float BlockA[16])
{
    float MinAlpha = 1.0;
    float MaxAlpha = 0.0;
    for (int k = 0; k < 16; ++k)
    {
        float A = BlockA[k];
        MinAlpha = min(A, MinAlpha);
        MaxAlpha = max(A, MaxAlpha);
    }

    MinAlpha = round(MinAlpha*255.0);
    MaxAlpha = round(MaxAlpha*255.0);
    
    float AlphaRange = MaxAlpha - MinAlpha;
    const float MidRange = 21.f;// an average range in ALPHA_DISTANCE_TABLES
    float Multiplier = clamp(round(AlphaRange/MidRange), 1.0, 15.0);
    
    int TableIdx = 0;
    vec4 TableValueNeg = vec4(0,0,0,0);
    vec4 TableValuePos = vec4(0,0,0,0);
    
    // iterating through all tables to find a best fit is quite slow
    // instead guess the best table based on alpha range
#if 1
    const int TablesToTest[5] = {15,11,6,2,0};
    for (int i = 0; i < 5; ++i)
    {
        TableIdx = TablesToTest[i];
        TableValuePos = alpha_distance_tables[TableIdx];
                
        float TableRange = (TableValuePos.w*2 + 1)*Multiplier;
        float Dif = TableRange - AlphaRange;
        if (Dif > 0.0)
        {
            i += 5;            
        }
    }
#else    
    for (int i = 0; i < NUM_ALPHA_TABLES; ++i)
    {
        TableIdx = NUM_ALPHA_TABLES - 1 - i;
        TableValuePos = alpha_distance_tables[TableIdx];
                
        float TableRange = (TableValuePos.w * 2.0 + 1.0) * Multiplier;
        float Dif = TableRange - AlphaRange;
        if (Dif >= 0.0)
        {
            i += NUM_ALPHA_TABLES;
        }
    }
#endif    
    TableValueNeg = -(TableValuePos + vec4(1,1,1,1));
    
    TableValuePos*=Multiplier;
    TableValueNeg*=Multiplier;
    
    // make sure an exact value of MinAlpha can always be decoded from a BaseValue
    float BaseValue = clamp(round(-TableValueNeg.w + MinAlpha), 0.0, 255.0);
    
    TableValueNeg = TableValueNeg + BaseValue.xxxx;
    TableValuePos = TableValuePos + BaseValue.xxxx;
    uvec2 BlockWeights = uvec2(0);
    
    for (int PixelIndex = 0; PixelIndex < 16; ++PixelIndex)
    {
        float Alpha = BlockA[PixelIndex]*255.0;
        int SelectedIndex = 0;
        float MinDif = 100000.0;
        
        if (Alpha < TableValuePos.x)
        {
            SelectAlphaMod(Alpha, TableValueNeg.x, 0, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValueNeg.y, 1, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValueNeg.z, 2, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValueNeg.w, 3, SelectedIndex, MinDif);
        }
        else
        {
            SelectAlphaMod(Alpha, TableValuePos.x, 4, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValuePos.y, 5, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValuePos.z, 6, SelectedIndex, MinDif);
            SelectAlphaMod(Alpha, TableValuePos.w, 7, SelectedIndex, MinDif);
        }

        // ETC uses column-major indexing for the pixels in a block...
        int TransposedIndex = (PixelIndex >> 2) | ((PixelIndex & 3) << 2);
        int StartBit = (15 - TransposedIndex) * 3;
        BlockWeights.x |= (StartBit < 32) ? SelectedIndex << StartBit : 0;
        int ShiftRight = (StartBit == 30) ? 2 : 0;
        int ShiftLeft = (StartBit >= 32) ? StartBit - 32 : 0;
        BlockWeights.y |= (StartBit >= 30) ? (SelectedIndex >> ShiftRight) << ShiftLeft : 0;
    }

    int MultiplierInt = int(Multiplier);
    int BaseValueInt = int(BaseValue);
    
    uvec2 AlphaBits;
    AlphaBits.x = SwapEndian32(BlockWeights.y | (TableIdx << 16) | (MultiplierInt << 20) | (BaseValueInt << 24));
    AlphaBits.y = SwapEndian32(BlockWeights.x);

    return AlphaBits;
}

uvec4 CompressBlock_ETC2_RGBA(in vec3 BlockRGB[16], in float BlockA[16])
{
    uvec2 CompressedRGB = CompressBlock_ETC2_RGB(BlockRGB);
    uvec2 CompressedAlpha = CompressBlock_ETC2_Alpha(BlockA);
    return uvec4(CompressedAlpha, CompressedRGB);
}

uvec4 CompressBlock_ETC2_RG(in float BlockU[16], in float BlockV[16])
{
    uvec2 R = CompressBlock_ETC2_Alpha(BlockU);
    uvec2 G = CompressBlock_ETC2_Alpha(BlockV);
    return uvec4(R, G);
}

#define MAX_WEIGHT 5.0
const uint weight_table[6] = {0, 2, 4, 5, 3, 1};

mat4x4 Covariance(inout vec4 texels[BLOCK_SIZE], vec4 mean) 
{
    mat4x4 cov = mat4x4(0.0);
    
    for (int k = 0; k < BLOCK_SIZE; ++k)
    {
        vec4 texel = texels[k] - mean;
        texels[k] = texel;

        for (int i = 0; i < 4; ++i)
        {
            cov[i][0] += texel[i] * texel[0];
            cov[i][1] += texel[i] * texel[1];
            cov[i][2] += texel[i] * texel[2];
            cov[i][3] += texel[i] * texel[3];         
        }
    }

    cov /= BLOCK_SIZE;
    return cov;
}

vec4 EigenVector(mat4x4 m)
{
    // calc the max eigen value by iteration
    vec4 v = vec4(0.3, 0.59, 0.11, 0.0);

    //for (int i = 0; i < 8; ++i)
    {        
        v = mul(m, v);
        float dp2 = dot(v, v);

        if (dp2 < SMALL_VALUE) 
        {
            return vec4(0.57736, 0.57736, 0.57736, 0.0);
        }

        v = v * rsqrt(dp2);
    }

    return v;
}

void FindMinMax(in vec4 texels[BLOCK_SIZE], vec4 m, vec4 k, out vec4 e0, out vec4 e1) 
{
    float t = dot(texels[0], k);
    float min_t = t;
    float max_t = t;

    for (int i = 1; i < BLOCK_SIZE; i++) 
    {
        t = dot(texels[i], k);
        min_t = min(min_t, t);
        max_t = max(max_t, t);
    }

    e0 = k * min_t + m;
    e1 = k * max_t + m;

    e0 = clamp(e0, 0.0, 255.0);
    e1 = clamp(e1, 0.0, 255.0);

#if HAS_ALPHA == 0
    e0.w = 255.0;
    e1.w = 255.0;
#endif

    if (e0.x + e0.y + e0.z > e1.x + e1.y + e1.z)
    {
        Swap(e0, e1);
    }   
}

//principal component analysis
void principal_component_analysis(in vec4 texels[BLOCK_SIZE], vec4 m, out vec4 e0, out vec4 e1)
{    
    mat4x4 cov = Covariance(texels, m);
    vec4 k = EigenVector(cov);
    FindMinMax(texels, m, k, e0, e1);
}

uvec2 endpoint_ise(vec4 e0, vec4 e1)
{
    // encode endpoints        
    uvec4 e0q = uvec4(round(e0));
    uvec4 e1q = uvec4(round(e1));

#if HAS_ALPHA == 0
    e0q.a = 0u;
    e1q.a = 0u;
#endif
    uvec2 ep_ise;
    ep_ise  = e0q.rb;
    ep_ise |= e1q.rb << 8;
    ep_ise |= e0q.ga << 16;
    ep_ise |= e1q.ga << 24;

    // endpoints quantized ise encode    
    return ep_ise;
}

void calculate_quantized_weights(in vec4 texels[BLOCK_SIZE], vec4 ep0, vec4 ep1, out uint weights[BLOCK_SIZE])
{    
    vec4 vec_k = ep1 - ep0;

    if (length(vec_k) < SMALL_VALUE)
    {
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {            
            weights[i] = weight_table[0];
        }
    }
    else
    {
        float projw[BLOCK_SIZE];
        vec_k = normalize(vec_k);
        float w = dot(vec_k, texels[0] - ep0);
        float minw = w;
        float maxw = w;
        projw[0] = w;
        
        for (int i = 1; i < BLOCK_SIZE; ++i)
        {            
            w = dot(vec_k, texels[i] - ep0);
            minw = min(w, minw);
            maxw = max(w, maxw);
            projw[i] = w;
        }

        float invlen = maxw - minw;
        invlen = 1.0 / max(SMALL_VALUE, invlen);
        
        for (int i = 0; i < BLOCK_SIZE; ++i)
        {
            projw[i] = (projw[i] - minw) * invlen;
            uint w = uint(round(projw[i] * MAX_WEIGHT));
            weights[i] = weight_table[w];
        }
    }
}

uvec2 bise_weights(in uint numbers[16])
{    
    int j = 0;
    uint weight[18];

    for (int i = 0; i < 15; i += 5) 
    {
        uint t0 = numbers[i] >> 1;
        uint t1 = numbers[i + 1] >> 1;
        uint t2 = numbers[i + 2] >> 1;
        uint t3 = numbers[i + 3] >> 1;
        uint t4 = numbers[i + 4] >> 1;

        weight[j]     = numbers[i] & 1;
        weight[j + 1] = numbers[i + 1] & 1;
        weight[j + 2] = numbers[i + 2] & 1;
        weight[j + 3] = numbers[i + 3] & 1;
        weight[j + 4] = numbers[i + 4] & 1;
        weight[j + 5] = integer_from_trits[t4 * 81 + t3 * 27 + t2 * 9 + t1 * 3 + t0];

        j += 6;
    }

    uint t0 = numbers[15] >> 1;       
    uint packed = integer_from_trits[t0];        

    uint x = (weight[0] << 31) | (weight[1] << 28) | (weight[2] << 25) | (weight[3] << 23) | (weight[4] << 20) | (weight[6] << 18) | 
             (weight[7] << 15) | (weight[8] << 12) | (weight[9] << 10) | (weight[10] << 7) | (weight[12] << 5) | (weight[13] << 2);
    x |= (weight[5] & 0x1) << 30;       //30
    x |= (weight[5] & 0x2) << 28;       //29    
    x |= (weight[5] & 0x4) << 25;       //27
    x |= (weight[5] & 0x8) << 23;       //26    
    x |= (weight[5] & 0x10) << 20;      //24    
    x |= (weight[5] & 0x20) << 17;      //22
    x |= (weight[5] & 0x40) << 15;      //21    
    x |= (weight[5] & 0x80) << 12;      //19
    
    x |= (weight[11] & 0x1) << 17;      //17
    x |= (weight[11] & 0x2) << 15;      //16
    x |= (weight[11] & 0x4) << 12;      //14
    x |= (weight[11] & 0x8) << 10;      //13    
    x |= (weight[11] & 0x10) << 7;      //11    
    x |= (weight[11] & 0x20) << 4;      //9
    x |= (weight[11] & 0x40) << 2;      //8    
    x |= (weight[11] & 0x80) >> 1;      //6    

    x |= (weight[17] & 0x1) << 4;       //4
    x |= (weight[17] & 0x2) << 2;       //3    
    x |= (weight[17] & 0x4) >> 1;       //1
    x |= (weight[17] & 0x8) >> 3;       //0
    
    uint y = (weight[14] << 31) | (weight[15] << 29) | (weight[16] << 26);
    y |= (weight[17] & 0x10) << 26;    //30              
    y |= (weight[17] & 0x20) << 23;    //28
    y |= (weight[17] & 0x40) << 21;    //27        
    y |= (weight[17] & 0x80) << 18;    //25
    y |= (numbers[15] & 1) << 24;
    y |= (packed & 0x1) << 23;          //23
    y |= (packed & 0x2) << 21;          //22

    return uvec2(x, y);
}

uvec2 weight_ise(in vec4 texels[BLOCK_SIZE], vec4 ep0, vec4 ep1)
{    
    // encode weights
    uint wt_quantized[BLOCK_SIZE];    
    calculate_quantized_weights(texels, ep0, ep1, wt_quantized);

    // weights quantized ise encode
    return bise_weights(wt_quantized);    
}

uvec4 SymbolicToPhysical(uvec2 ep_ise, uvec2 wt_ise)
{
    uvec4 phy_blk = uvec4(0u);
    // weights ise
    phy_blk.wz |= wt_ise.xy;

    // blockmode & partition count, blockmode is 11 bit color_endpoint_mode is 4 bit
#if HAS_ALPHA == 1
    phy_blk.x = (12 << 13) + 67; //98371; // 12<<13 + 67
#else    
    phy_blk.x = 66129;  //8 << 13 + 593(blockmodel)
#endif        

    // endpoints start from ( multi_part ? bits 29 : bits 17 )
    phy_blk.xy |= ((ep_ise.xy & 0x7FFF) << 17);
    phy_blk.yz |= ((ep_ise.xy >> 15) & 0x1FFFF);    

    return phy_blk;
}

uvec4 CompressASTC_4x4(in vec4 texels[BLOCK_SIZE], vec4 mean)
{    
    vec4 ep0, ep1;
    principal_component_analysis(texels, mean, ep0, ep1);

    // endpoints_quant是根据整个128bits减去weights的编码占用和其他配置占用后剩余的bits位数来确定的。        
    uvec2 ep_ise = endpoint_ise(ep0, ep1);
    uvec2 wt_ise = weight_ise(texels, ep0, ep1);

    // reference to arm astc encoder "symbolic_to_physical"
    return SymbolicToPhysical(ep_ise, wt_ise);    
}


