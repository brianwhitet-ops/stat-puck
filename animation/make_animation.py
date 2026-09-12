#!/usr/bin/env python3
"""
Assembly animation for Stat Puck.
Prefers Blender if available; otherwise renders STL frames with trimesh+matplotlib + ffmpeg.
"""
from __future__ import annotations

import math
import os
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
CAD = ROOT / "cad"
OUT = Path(__file__).resolve().parent
FRAMES = OUT / "frames"
VIDEO = OUT / "assembly_animation.mp4"

# Assembly order with on-screen labels
STEPS = [
    ("enclosure_bottom", "1  Bottom case"),
    ("magnet", "2  N52 rubber magnet"),
    ("battery", "3  500 mAh LiPo"),
    ("pcb", "4  PCB / interconnect"),
    ("xiao", "5  XIAO nRF52840"),
    ("eink", "6  2.9\" e-ink"),
    ("buttons", "7  B3W buttons"),
    ("gasket", "8  Silicone gasket"),
    ("enclosure_top", "9  Top case + screws"),
]


def have_blender() -> str | None:
    for c in ["blender", "/snap/bin/blender"]:
        if shutil.which(c):
            return c
    for p in Path("/tmp").glob("blender-*/blender"):
        if p.is_file():
            return str(p)
    return None


def render_with_trimesh():
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d.art3d import Poly3DCollection
    import trimesh

    FRAMES.mkdir(parents=True, exist_ok=True)
    for old in FRAMES.glob("*.png"):
        old.unlink()

    meshes = []
    for name, label in STEPS:
        path = CAD / f"{name}.stl"
        m = trimesh.load_mesh(path)
        if isinstance(m, trimesh.Scene):
            m = trimesh.util.concatenate(tuple(m.geometry.values()))
        meshes.append((name, label, m))

    # color palette (matte, not purple/glow)
    colors = [
        (0.25, 0.25, 0.27, 0.95),
        (0.12, 0.12, 0.12, 1.0),
        (0.55, 0.62, 0.45, 0.95),
        (0.15, 0.35, 0.22, 0.9),
        (0.2, 0.2, 0.22, 1.0),
        (0.85, 0.85, 0.82, 1.0),
        (0.35, 0.35, 0.38, 1.0),
        (0.4, 0.4, 0.42, 0.7),
        (0.22, 0.22, 0.24, 0.95),
    ]

    # explode offsets along +Z for intro, then settle
    n_intro = 18
    n_settle = 12
    n_hold = 10
    frame_i = 0

    def draw(ax, visible_count, explode_t, title):
        ax.cla()
        ax.set_axis_off()
        ax.set_title(title, fontsize=14, color="#222", pad=8, fontweight="bold")
        xs, ys, zs = [], [], []
        for i in range(visible_count):
            name, label, mesh = meshes[i]
            # explode then settle toward z=0 offset
            base = np.array(mesh.triangles)
            z_off = (1.0 - explode_t) * (8 + i * 6)
            tris = base.copy()
            tris[:, :, 2] += z_off
            poly = Poly3DCollection(tris, alpha=colors[i][3])
            poly.set_facecolor(colors[i][:3])
            poly.set_edgecolor((0.1, 0.1, 0.1, 0.15))
            ax.add_collection3d(poly)
            xs.append(tris[:, :, 0])
            ys.append(tris[:, :, 1])
            zs.append(tris[:, :, 2])
            # label last added part
            if i == visible_count - 1:
                c = tris.mean(axis=(0, 1))
                ax.text(c[0], c[1], c[2] + 8, label, color="#111", fontsize=11)

        if xs:
            ax.auto_scale_xyz(
                [float(np.min(np.concatenate([a.ravel() for a in xs]))),
                 float(np.max(np.concatenate([a.ravel() for a in xs])))],
                [float(np.min(np.concatenate([a.ravel() for a in ys]))),
                 float(np.max(np.concatenate([a.ravel() for a in ys])))],
                [float(np.min(np.concatenate([a.ravel() for a in zs]))),
                 float(np.max(np.concatenate([a.ravel() for a in zs])))],
            )
        ax.view_init(elev=22, azim=35 + frame_i * 0.4)
        ax.set_box_aspect((1.6, 1.0, 0.6))

    paths = []
    for step in range(len(meshes)):
        # fly in
        for k in range(n_intro):
            t = k / (n_intro - 1)
            explode = 1.0 - (1 - t)  # 0→1 settle from exploded? use 0 at start exploded
            # start exploded (explode_t=0), end assembled (1)
            explode_t = t
            fig = plt.figure(figsize=(12.8, 7.2), dpi=100, facecolor="#d9d4cc")
            ax = fig.add_subplot(111, projection="3d", facecolor="#d9d4cc")
            draw(ax, step + 1, explode_t, "Stat Puck — assembly")
            # legend of completed parts
            legend = "\n".join(lbl for _, lbl, _ in meshes[: step + 1])
            fig.text(0.02, 0.98, legend, va="top", ha="left", fontsize=9, color="#222", family="monospace")
            fp = FRAMES / f"frame_{frame_i:04d}.png"
            fig.savefig(fp, facecolor=fig.get_facecolor())
            plt.close(fig)
            paths.append(fp)
            frame_i += 1
        for _ in range(n_hold):
            fig = plt.figure(figsize=(12.8, 7.2), dpi=100, facecolor="#d9d4cc")
            ax = fig.add_subplot(111, projection="3d", facecolor="#d9d4cc")
            draw(ax, step + 1, 1.0, "Stat Puck — assembly")
            legend = "\n".join(lbl for _, lbl, _ in meshes[: step + 1])
            fig.text(0.02, 0.98, legend, va="top", ha="left", fontsize=9, color="#222", family="monospace")
            fp = FRAMES / f"frame_{frame_i:04d}.png"
            fig.savefig(fp, facecolor=fig.get_facecolor())
            plt.close(fig)
            paths.append(fp)
            frame_i += 1

    # final orbit
    for k in range(36):
        fig = plt.figure(figsize=(12.8, 7.2), dpi=100, facecolor="#d9d4cc")
        ax = fig.add_subplot(111, projection="3d", facecolor="#d9d4cc")
        # hijack view via frame_i
        draw(ax, len(meshes), 1.0, "Stat Puck — assembled")
        legend = "\n".join(lbl for _, lbl, _ in meshes)
        fig.text(0.02, 0.98, legend, va="top", ha="left", fontsize=9, color="#222", family="monospace")
        fp = FRAMES / f"frame_{frame_i:04d}.png"
        fig.savefig(fp, facecolor=fig.get_facecolor())
        plt.close(fig)
        paths.append(fp)
        frame_i += 1

    subprocess.check_call(
        [
            "ffmpeg",
            "-y",
            "-framerate",
            "18",
            "-i",
            str(FRAMES / "frame_%04d.png"),
            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            str(VIDEO),
        ]
    )
    print("wrote", VIDEO)


def write_blender_script(path: Path):
    path.write_text(
        f"""
import bpy, math, os
from mathutils import Vector

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
# Blender 4.2+
scene.render.engine = 'BLENDER_EEVEE_NEXT'
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.fps = 24
scene.render.image_settings.file_format = 'PNG'

world = bpy.data.worlds.new('World')
scene.world = world
world.use_nodes = True
bg = world.node_tree.nodes['Background']
bg.inputs[0].default_value = (0.85, 0.83, 0.78, 1)
bg.inputs[1].default_value = 1.0

bpy.ops.object.light_add(type='AREA', location=(0.2, -0.3, 0.45))
bpy.context.object.data.energy = 250
bpy.context.object.data.size = 0.5
bpy.ops.object.light_add(type='AREA', location=(-0.2, 0.2, 0.3))
bpy.context.object.data.energy = 80

bpy.ops.object.camera_add(location=(0.22, -0.26, 0.16), rotation=(math.radians(68), 0, math.radians(42)))
scene.camera = bpy.context.object

parts = {STEPS!r}
cad = r'{CAD}'
imported = []
greys = [(0.25,0.25,0.27),(0.05,0.05,0.05),(0.5,0.6,0.4),(0.1,0.35,0.2),(0.2,0.2,0.22),(0.85,0.85,0.8),(0.35,0.35,0.38),(0.4,0.4,0.42),(0.22,0.22,0.24)]

for i,(name,label) in enumerate(parts):
    fp = os.path.join(cad, name + '.stl')
    if hasattr(bpy.ops.wm, 'stl_import'):
        bpy.ops.wm.stl_import(filepath=fp)
    else:
        bpy.ops.import_mesh.stl(filepath=fp)
    obj = bpy.context.selected_objects[0]
    obj.name = name
    obj.scale = (0.001, 0.001, 0.001)
    bpy.ops.object.transform_apply(scale=True)
    mat = bpy.data.materials.new(name + '_mat')
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Base Color'].default_value = (*greys[i % len(greys)], 1)
    bsdf.inputs['Roughness'].default_value = 0.55
    obj.data.materials.append(mat)
    z0 = 0.025 + i * 0.018
    obj.location.z += z0
    imported.append((obj, label, z0))

# label object
bpy.ops.object.text_add(location=(-0.09, -0.05, 0.08))
label_obj = bpy.context.object
label_obj.name = 'PartLabel'
label_obj.data.body = 'Stat Puck'
label_obj.scale = (0.01, 0.01, 0.01)
mat_t = bpy.data.materials.new('label_mat')
mat_t.use_nodes = True
mat_t.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (0.05,0.05,0.05,1)
label_obj.data.materials.append(mat_t)

frame = 1
for i,(obj,label,z0) in enumerate(imported):
    obj.hide_render = True
    obj.hide_viewport = True
    obj.keyframe_insert(data_path='hide_render', frame=frame)
    obj.keyframe_insert(data_path='hide_viewport', frame=frame)
    frame += 2
    obj.hide_render = False
    obj.hide_viewport = False
    obj.keyframe_insert(data_path='hide_render', frame=frame)
    obj.keyframe_insert(data_path='hide_viewport', frame=frame)
    obj.location.z = z0
    obj.keyframe_insert(data_path='location', frame=frame)
    label_obj.data.body = label
    label_obj.keyframe_insert(data_path='location', frame=frame)
    frame += 16
    obj.location.z = 0.0
    obj.keyframe_insert(data_path='location', frame=frame)
    frame += 8

scene.frame_start = 1
scene.frame_end = frame + 48
cam = scene.camera
base = frame
for f in range(base, scene.frame_end + 1):
    ang = (f - base) / 48 * math.tau
    cam.location = (0.26*math.cos(ang), -0.26*math.sin(ang), 0.15)
    cam.keyframe_insert(data_path='location', frame=f)
    # look at origin
    direction = Vector((0,0,0.02)) - cam.location
    cam.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
    cam.keyframe_insert(data_path='rotation_euler', frame=f)

out_dir = r'{OUT / "blender_frames"}'
os.makedirs(out_dir, exist_ok=True)
scene.render.filepath = out_dir + '/frame_'
bpy.ops.render.render(animation=True)
print('BLENDER_RENDER_DONE', scene.frame_end)
"""
    )


def main():
    blender = have_blender()
    if blender:
        script = OUT / "blender_assemble.py"
        write_blender_script(script)
        (OUT / "blender_frames").mkdir(exist_ok=True)
        try:
            subprocess.check_call([blender, "-b", "-P", str(script)])
            frames = list((OUT / "blender_frames").glob("frame_*.png"))
            if not frames:
                raise RuntimeError("no blender frames")
            subprocess.check_call(
                [
                    "ffmpeg",
                    "-y",
                    "-framerate",
                    "24",
                    "-i",
                    str(OUT / "blender_frames" / "frame_%04d.png"),
                    "-c:v",
                    "libx264",
                    "-pix_fmt",
                    "yuv420p",
                    str(VIDEO),
                ]
            )
            print("wrote", VIDEO, "via blender")
            return
        except Exception as e:
            print("Blender path failed:", e, "— falling back")
    print("Using trimesh/matplotlib fallback")
    render_with_trimesh()


if __name__ == "__main__":
    main()
