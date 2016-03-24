const float NUM_SLICES_X = 20.0f * 2.0;
const float NUM_SLICES_Y = 11.0f;
const float NUM_SLICES_Z = 16.0f;
const int   MAX_POINT_LIGHTS = 1024;

struct PointLight
{
    vec4  position;
    vec4  color;
    float radius;
    float intensity;
};

layout (std140) uniform PointLightBlock
{
    PointLight PointLights[ MAX_POINT_LIGHTS ];
};

uniform usampler3D     uLightPointerTex;
uniform usamplerBuffer uLightIndexTex;

void GetLightOffsetAndCount( const in vec2 _coord_xy, const in float _vertexZ, out int _lightIndexOffset, out int _pointLightCount )
{
    float linearDepth = ( -_vertexZ - viewInfo.nearClip ) / ( viewInfo.farClip - viewInfo.nearClip );
    int slice = int( max( linearDepth * NUM_SLICES_Z, 0.0f ) );

    // TODO: tile size is 64, SHR by 6 to find x,y index
    ivec3 cluster_coord;
    cluster_coord.xy = ivec2( _coord_xy * viewInfo.rcpViewportDims * ivec2( NUM_SLICES_X, NUM_SLICES_Y ) );
    cluster_coord.z = slice;

    uvec4 light_data = texelFetch( uLightPointerTex, cluster_coord, 0 );
    _lightIndexOffset = int( light_data.x );
    _pointLightCount = int( light_data.y );
}

PointLight GetPointLight( int _lightIndex )
{
    uint index = texelFetch( uLightIndexTex, _lightIndex ).r; 
    return PointLights[ index ];
}

