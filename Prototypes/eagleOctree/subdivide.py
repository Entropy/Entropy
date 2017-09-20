bpy.ops.object.duplicate()
bpy.context.object.location.x += side
bpy.ops.object.duplicate()
bpy.context.object.location.x += side
bpy.ops.object.duplicate()
bpy.context.object.location.x += side


def subdivide():
    side = bpy.context.object.dimensions.x * 0.5
    bpy.ops.object.duplicate()
    bpy.context.object.dimensions.xyz *= 0.5
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.context.object.location.x -= side * 0.5
    bpy.context.object.location.y -= side * 0.5
    bpy.context.object.location.z -= side * 0.5
    bpy.ops.object.duplicate()
    bpy.context.object.location.x += side
    bpy.ops.object.duplicate()
    bpy.context.object.location.y += side
    bpy.ops.object.duplicate()
    bpy.context.object.location.x -= side
    bpy.ops.object.duplicate()
    bpy.context.object.location.z += side
    bpy.ops.object.duplicate()
    bpy.context.object.location.y -= side
    bpy.ops.object.duplicate()
    bpy.context.object.location.x += side
    bpy.ops.object.duplicate()
    bpy.context.object.location.y += side
