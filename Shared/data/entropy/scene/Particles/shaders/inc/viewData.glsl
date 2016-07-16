struct ViewData
{
    mat4  inverseViewMatrix;
    vec2  viewportSize;
    vec2  rcpViewportSize;    
    float nearClip;
    float farClip;
};

layout (std140) uniform uViewBlock
{
    ViewData viewData;     
};

