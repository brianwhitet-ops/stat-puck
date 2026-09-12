import bpy, math, os
from mathutils import Vector

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.render.engine = 'BLENDER_WORKBENCH'
scene.display.shading.light = 'STUDIO'
scene.display.shading.color_type = 'MATERIAL'
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.fps = 24
scene.render.image_settings.file_format = 'PNG'
scene.render.film_transparent = False

world = bpy.data.worlds.new('World')
scene.world = world
world.use_nodes = True
world.node_tree.nodes['Background'].inputs[0].default_value = (0.82, 0.80, 0.76, 1)

bpy.ops.object.light_add(type='SUN', location=(0.3, -0.4, 0.6))
bpy.context.object.data.energy = 2.5

# Wider camera so labels fit
bpy.ops.object.camera_add(location=(0.28, -0.32, 0.18))
cam = bpy.context.object
cam.data.lens = 35
scene.camera = cam

parts = [
    ("enclosure_bottom", "1  Bottom case"),
    ("magnet", "2  N52 rubber magnet"),
    ("battery", "3  500 mAh LiPo"),
    ("pcb", "4  PCB"),
    ("xiao", "5  XIAO nRF52840"),
    ("eink", "6  2.9in e-ink"),
    ("buttons", "7  B3W buttons"),
    ("gasket", "8  Silicone gasket"),
    ("enclosure_top", "9  Top case"),
]
cad = '/agent/stat-puck/cad'
greys = [
    (0.28,0.28,0.30),(0.08,0.08,0.08),(0.45,0.55,0.38),
    (0.12,0.32,0.18),(0.22,0.22,0.24),(0.88,0.88,0.84),
    (0.40,0.40,0.43),(0.50,0.50,0.52),(0.24,0.24,0.26)
]
imported = []
for i,(name,label) in enumerate(parts):
    bpy.ops.wm.stl_import(filepath=os.path.join(cad, name + '.stl'))
    obj = bpy.context.selected_objects[0]
    obj.name = name
    obj.scale = (0.001, 0.001, 0.001)
    bpy.ops.object.transform_apply(scale=True)
    mat = bpy.data.materials.new(name+'_mat')
    mat.use_nodes = True
    mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (*greys[i], 1)
    obj.data.materials.append(mat)
    imported.append((obj, label))

import numpy as np
cents = [np.array(obj.location) for obj,_ in imported]
centroid = sum(cents)/len(cents)
for obj,_ in imported:
    obj.location -= Vector(centroid)
imported = [(obj, label, obj.location.copy()) for obj, label in imported]

# HUD-like label in camera space using text parented... simpler: fixed world label left of model
bpy.ops.object.text_add(location=(-0.14, 0.0, 0.06))
label_obj = bpy.context.object
label_obj.data.body = 'Stat Puck assembly'
label_obj.data.align_x = 'LEFT'
label_obj.scale = (0.012,0.012,0.012)
label_obj.rotation_euler = (math.radians(90), 0, math.radians(40))
mat_t = bpy.data.materials.new('tmat')
mat_t.use_nodes = True
mat_t.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (0.08,0.08,0.08,1)
label_obj.data.materials.append(mat_t)

# title
bpy.ops.object.text_add(location=(-0.14, 0.0, 0.09))
title = bpy.context.object
title.data.body = 'STAT PUCK'
title.scale = (0.014,0.014,0.014)
title.rotation_euler = (math.radians(90), 0, math.radians(40))
title.data.materials.append(mat_t)

frame = 1
for i,(obj,label,home) in enumerate(imported):
    obj.hide_render = True
    obj.hide_viewport = True
    obj.keyframe_insert('hide_render', frame=frame)
    obj.keyframe_insert('hide_viewport', frame=frame)
    frame += 1
    obj.hide_render = False
    obj.hide_viewport = False
    obj.keyframe_insert('hide_render', frame=frame)
    obj.keyframe_insert('hide_viewport', frame=frame)
    obj.location = home + Vector((0,0, 0.05 + i*0.012))
    obj.keyframe_insert('location', frame=frame)
    label_obj.data.body = label
    label_obj.keyframe_insert('location', frame=frame)
    frame += 12
    obj.location = home
    obj.keyframe_insert('location', frame=frame)
    frame += 5

scene.frame_start = 1
scene.frame_end = frame + 60
base = frame
for f in range(base, scene.frame_end+1):
    ang = math.radians(40) + (f-base)/60 * math.tau * 0.85
    cam.location = (0.30*math.cos(ang), -0.30*math.sin(ang), 0.16)
    cam.keyframe_insert('location', frame=f)
    direction = Vector((0,0,0.01)) - cam.location
    cam.rotation_euler = direction.to_track_quat('-Z','Y').to_euler()
    cam.keyframe_insert('rotation_euler', frame=f)
    # keep label readable-ish
    label_obj.rotation_euler = (math.radians(90), 0, cam.rotation_euler.z)
    label_obj.keyframe_insert('rotation_euler', frame=f)

out = '/agent/stat-puck/animation/blender_frames'
os.makedirs(out, exist_ok=True)
for old in os.listdir(out):
    if old.endswith('.png'): os.remove(os.path.join(out, old))
scene.render.filepath = out + '/frame_'
bpy.ops.render.render(animation=True)
print('DONE', scene.frame_end)
