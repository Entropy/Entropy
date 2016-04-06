struct ViewInfo
{
    mat4  viewMatrixInverse;
    vec2  viewportDims;
    vec2  rcpViewportDims;    
    float nearClip;
    float farClip;
};

layout (std140) uniform ViewInfoBlock
{
    ViewInfo viewInfo;     
};

