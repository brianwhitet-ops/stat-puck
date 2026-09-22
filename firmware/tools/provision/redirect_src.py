# PlatformIO 6.2 ignores src_dir outside [platformio]. This pre-script
# selects the factory sketch for env:xiaoble_provision only.
Import("env")

import os
import sys

src = os.path.join(env.subst("$PROJECT_DIR"), "tools", "provision")
sketch = os.path.join(src, "main.cpp")
try:
    text = open(sketch, encoding="utf-8").read()
except OSError as exc:
    sys.stderr.write("Provisioner sketch missing: %s\n" % exc)
    env.Exit(1)

if "SLAB FACTORY PROVISIONER" not in text or "InternalFS.begin(" in text:
    sys.stderr.write("Refusing to build: %s is not the factory provisioner\n" % sketch)
    env.Exit(1)

env.Replace(PROJECT_SRC_DIR=src)
print("xiaoble_provision sources: %s" % src)
