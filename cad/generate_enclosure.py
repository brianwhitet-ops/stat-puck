#!/usr/bin/env python3
"""
Stat Puck — enclosure + PCB stack + magnet pocket + mount CAD.
Exports STEP and STL for 3D printing / machining.
Dimensions are prototype-accurate to sourced modules.
"""
from __future__ import annotations

from pathlib import Path

from build123d import (
    Align,
    Axis,
    Box,
    BuildPart,
    BuildSketch,
    Circle,
    Compound,
    Cylinder,
    Location,
    Locations,
    Mode,
    Plane,
    Rectangle,
    export_step,
    export_stl,
    extrude,
    fillet,
)

OUT = Path(__file__).resolve().parent

# Overall device target: pocketable slab
W, D, H = 110.0, 58.0, 15.5  # mm
WALL = 1.6
R = 8.0

# Waveshare 2.9" module outline (vendor): 89.5 x 38.0 mm
EINK_W, EINK_D, EINK_H = 89.5, 38.0, 1.8
# Seeed XIAO nRF52840 ~21 x 17.5 x 5 mm
XIAO_W, XIAO_D, XIAO_H = 21.0, 17.5, 5.0
# Adafruit 500mAh LiPo 503035: 36 x 29 x 4.75
BAT_W, BAT_D, BAT_H = 36.0, 29.0, 4.8
# K&J DC6TP-N52 rubber coated: Ø19.05 x 9.52
MAG_D, MAG_H = 19.2, 9.6
BTN_PITCH = 16.0
BTN_Y = -18.5
SCREW_XY = [(-48, -22), (48, -22), (-48, 22), (48, 22)]


def make_bottom():
    with BuildPart() as bottom:
        with BuildSketch():
            Rectangle(W, D)
        extrude(amount=H * 0.55)
        fillet(bottom.edges().filter_by(Axis.Z), radius=R)
        with Locations((0, 0, WALL)):
            Box(
                W - 2 * WALL,
                D - 2 * WALL,
                H * 0.55 - WALL + 0.2,
                align=(Align.CENTER, Align.CENTER, Align.MIN),
                mode=Mode.SUBTRACT,
            )
        # magnet pocket toward exterior bottom
        with Locations((0, 18, 0)):
            Cylinder(radius=MAG_D / 2 + 0.15, height=MAG_H + 0.4, mode=Mode.SUBTRACT)
        # USB-C cutout
        with Locations((-W / 2 + 0.5, 0, 6.2)):
            Box(8.5, 9.2, 3.6, mode=Mode.SUBTRACT)
        # screw bosses M2
        for x, y in SCREW_XY:
            with Locations((x, y, 0)):
                Cylinder(radius=2.4, height=H * 0.55 - 0.2)
                Cylinder(radius=0.9, height=H * 0.55, mode=Mode.SUBTRACT)
        # gasket groove
        with Locations((0, 0, H * 0.55 - 0.9)):
            Box(
                W - WALL,
                D - WALL,
                0.9,
                align=(Align.CENTER, Align.CENTER, Align.MIN),
                mode=Mode.SUBTRACT,
            )
            Box(
                W - 2.8 * WALL,
                D - 2.8 * WALL,
                1.0,
                align=(Align.CENTER, Align.CENTER, Align.MIN),
            )
    return bottom.part


def make_top():
    with BuildPart() as top:
        with BuildSketch():
            Rectangle(W, D)
        extrude(amount=H * 0.45)
        fillet(top.edges().filter_by(Axis.Z), radius=R)
        with Locations((0, 0, 0)):
            Box(
                W - 2 * WALL,
                D - 2 * WALL,
                H * 0.45 - WALL,
                align=(Align.CENTER, Align.CENTER, Align.MIN),
                mode=Mode.SUBTRACT,
            )
        # e-ink window
        with Locations((0, 8, H * 0.45)):
            Box(
                EINK_W - 4,
                EINK_D - 4,
                WALL + 0.4,
                align=(Align.CENTER, Align.CENTER, Align.MAX),
                mode=Mode.SUBTRACT,
            )
        xs = [-(2 * BTN_PITCH), -BTN_PITCH, 0, BTN_PITCH, 2 * BTN_PITCH]
        for x in xs:
            with Locations((x, BTN_Y, H * 0.45)):
                Cylinder(radius=4.2, height=WALL + 1, mode=Mode.SUBTRACT)
        for x, y in SCREW_XY:
            with Locations((x, y, 0)):
                Cylinder(radius=1.1, height=H * 0.45 + 0.2, mode=Mode.SUBTRACT)
            with Locations((x, y, H * 0.45)):
                Cylinder(radius=2.0, height=1.2, mode=Mode.SUBTRACT)
    return top.part


def make_box(w, d, h):
    with BuildPart() as p:
        Box(w, d, h)
    return p.part


def make_internals():
    parts = {}
    parts["pcb"] = make_box(98, 48, 1.2).moved(Location((0, 0, 6.0)))
    parts["eink"] = make_box(EINK_W, EINK_D, EINK_H).moved(Location((0, 8, 8.5)))

    with BuildPart() as xiao:
        Box(XIAO_W, XIAO_D, XIAO_H)
        with Locations((-XIAO_W / 2 - 1.5, 0, 1.2)):
            Box(7.5, 8.5, 3.2)
    parts["xiao"] = xiao.part.moved(Location((-38, 0, 6.6)))
    parts["battery"] = make_box(BAT_W, BAT_D, BAT_H).moved(Location((22, -2, 3.2)))

    with BuildPart() as mag:
        Cylinder(radius=MAG_D / 2, height=MAG_H)
    parts["magnet"] = mag.part.moved(Location((0, 18, MAG_H / 2 + 0.2)))

    caps = []
    xs = [-(2 * BTN_PITCH), -BTN_PITCH, 0, BTN_PITCH, 2 * BTN_PITCH]
    for x in xs:
        with BuildPart() as btn:
            Cylinder(radius=3.8, height=2.2)
            with Locations((0, 0, -2.0)):
                Box(6.0, 6.6, 4.3)
        caps.append(btn.part.moved(Location((x, BTN_Y, 12.8))))
    parts["buttons"] = Compound(children=caps)

    with BuildPart() as gasket:
        with BuildSketch():
            Rectangle(W - 1.8 * WALL, D - 1.8 * WALL)
            Rectangle(W - 3.2 * WALL, D - 3.2 * WALL, mode=Mode.SUBTRACT)
        extrude(amount=1.0)
    parts["gasket"] = gasket.part.moved(Location((0, 0, H * 0.55 - 0.5)))
    return parts


def make_steel_plate():
    with BuildPart() as plate:
        Box(70, 40, 1.2)
        fillet(plate.edges().filter_by(Axis.Z), radius=4)
    return plate.part


def make_clamp():
    """Optional steering-column clamp with ~25 mm bar bore along X."""
    with BuildPart() as clamp:
        Box(36, 28, 18)
        # approximate round bore with a cylindrical subtraction along X
        bore = Cylinder(radius=12.5, height=40)
        bore = bore.rotate(Axis.Y, 90)
        bore = bore.move(Location((0, 0, 0)))
    # rebuild using boolean outside builder for reliability
    body = Box(36, 28, 18)
    bore = Cylinder(radius=12.5, height=40).rotate(Axis.Y, 90)
    pocket = Cylinder(radius=MAG_D / 2 + 0.2, height=6).move(Location((0, -10, 0)))
    slot = Box(3, 10, 20).move(Location((12, 8, 0)))
    part = body - bore - pocket - slot
    return part


def make_cart_bar():
    return Cylinder(radius=12.0, height=220).rotate(Axis.Y, 90)


def write(name, solid):
    export_step(solid, OUT / f"{name}.step")
    export_stl(solid, OUT / f"{name}.stl")
    print(f"wrote {name}.step/.stl")


def export_all():
    OUT.mkdir(parents=True, exist_ok=True)
    bottom = make_bottom()
    top = make_top().moved(Location((0, 0, H * 0.55)))
    internals = make_internals()
    plate = make_steel_plate().moved(Location((0, 80, 0.6)))
    clamp = make_clamp().moved(Location((90, 0, 12)))
    bar = make_cart_bar().moved(Location((90, 0, 12)))

    for name, solid in {
        "enclosure_bottom": bottom,
        "enclosure_top": top,
        "pcb": internals["pcb"],
        "eink": internals["eink"],
        "xiao": internals["xiao"],
        "battery": internals["battery"],
        "magnet": internals["magnet"],
        "buttons": internals["buttons"],
        "gasket": internals["gasket"],
        "steel_plate": plate,
        "optional_clamp": clamp,
    }.items():
        write(name, solid)

    device = Compound(
        children=[
            bottom,
            top,
            internals["pcb"],
            internals["eink"],
            internals["xiao"],
            internals["battery"],
            internals["magnet"],
            internals["buttons"],
            internals["gasket"],
        ]
    )
    write("device_assembly", device)

    assembly = Compound(children=[device, plate, clamp, bar])
    write("full_kit_assembly", assembly)

    # cart-ready mount demo: clamp + magnet on Club Car / EZGO style bar
    mount_demo = Compound(
        children=[
            bar,
            clamp,
            internals["magnet"].moved(Location((90, -10, 12))),
            bottom.moved(Location((90, -28, 12))).rotate(Axis.X, 90),
        ]
    )
    write("cart_mount_demo", mount_demo)


if __name__ == "__main__":
    export_all()
