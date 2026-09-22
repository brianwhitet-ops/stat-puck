#!/usr/bin/env python3
"""Source guard: scoring firmware must not format, and the provisioner warning must match the docs.

This does not flash a board and does not prove LittleFS behavior on silicon.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[3]
MAIN = ROOT / "firmware" / "src" / "main.cpp"
PROVISION = ROOT / "firmware" / "tools" / "provision" / "main.cpp"
README = ROOT / "firmware" / "tools" / "provision" / "README.md"
INI = ROOT / "firmware" / "platformio.ini"
SRC = ROOT / "firmware" / "src"


def fail(message):
    print("FAIL:", message)
    return 1


def warning_from_sketch(text):
    match = re.search(
        r"static const char \*const kWarning\[\] = \{(.*?)\};",
        text,
        re.S,
    )
    if not match:
        return None
    return re.findall(r'"([^"\n]*)"', match.group(1))


def warning_from_readme(text):
    match = re.search(
        r"<!-- warning-begin -->\n```\n(.*?)\n```\n<!-- warning-end -->",
        text,
        re.S,
    )
    if not match:
        return None
    return match.group(1).split("\n")


def env_block(text, name):
    pattern = rf"\[{re.escape(name)}\](.*?)(?=\n\[|\Z)"
    match = re.search(pattern, text, re.S)
    return match.group(1) if match else ""


def main():
    errors = 0
    main_text = MAIN.read_text()
    provision_text = PROVISION.read_text()
    readme_text = README.read_text()
    ini_text = INI.read_text()
    src_text = "\n".join(path.read_text() for path in SRC.rglob("*") if path.is_file())

    if main_text.count("InternalFS.Adafruit_LittleFS::begin()") != 1:
        errors += fail("scoring startup must call mount-only begin() exactly once")
    if "InternalFS.begin(" in src_text or ".format(" in src_text:
        errors += fail("firmware/src contains InternalFS.begin( or .format(")
    if "STORAGE ERROR" not in main_text or "NO DATA ERASED" not in main_text:
        errors += fail("scoring firmware must keep STORAGE ERROR / NO DATA ERASED")
    if "SLAB_FACTORY_PROVISION" in main_text:
        errors += fail("scoring firmware must not reference the provisioner flag")

    if "InternalFS.begin(" in provision_text:
        errors += fail("provisioner must not call InternalFS.begin()")
    if "InternalFS.Adafruit_LittleFS::begin()" not in provision_text:
        errors += fail("provisioner must verify with mount-only begin()")
    if "InternalFS.format()" not in provision_text or "flash_nrf5x_erase" not in provision_text:
        errors += fail("provisioner is missing an explicit erase or format call")
    phrase_at = provision_text.find('strcmp(buf, kErasePhrase)')
    erase_at = provision_text.find("flash_nrf5x_erase")
    if phrase_at < 0 or erase_at < phrase_at:
        errors += fail("page erase must appear after the exact-phrase check")
    if "void loop" in provision_text and provision_text.count("erase_and_format(") != 2:
        # declaration plus the single setup() call
        errors += fail("erase_and_format must be defined once and called once")

    sketch_warning = warning_from_sketch(provision_text)
    readme_warning = warning_from_readme(readme_text)
    if not sketch_warning or sketch_warning != readme_warning:
        errors += fail("Serial warning lines and README warning copy differ")
    if sketch_warning and "ERASE SLAB FS" not in sketch_warning:
        errors += fail("warning copy is missing the erase phrase")

    xiaoble = env_block(ini_text, "env:xiaoble")
    provision_env = env_block(ini_text, "env:xiaoble_provision")
    if not xiaoble or not provision_env:
        errors += fail("platformio.ini is missing xiaoble or xiaoble_provision")
    if "src_dir" in xiaoble:
        errors += fail("scoring env must keep the default src directory")
    if "SLAB_FACTORY_PROVISION" in xiaoble:
        errors += fail("scoring env must not define SLAB_FACTORY_PROVISION")
    if "src_dir = tools/provision" not in provision_env:
        errors += fail("provision env must use src_dir = tools/provision")
    if "SLAB_FACTORY_PROVISION" not in provision_env:
        errors += fail("provision env must define SLAB_FACTORY_PROVISION")
    if "xiaoble_adafruit" not in ini_text:
        errors += fail("board must stay xiaoble_adafruit")

    if errors:
        print(f"{errors} check(s) failed")
        return 1
    print("no-autoformat check passed")
    print(f"warning lines: {len(sketch_warning)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
