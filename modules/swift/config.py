def can_build(env, platform):
    return True


def _find_tool(name):
    import os
    import shutil
    import subprocess

    path = shutil.which(name)
    if path is not None:
        return path
    # Xcode selected without CLI tools in PATH: ask xcrun, then usual spots.
    xcrun = shutil.which("xcrun")
    if xcrun is not None:
        try:
            out = subprocess.run([xcrun, "--find", name], capture_output=True, text=True, timeout=30)
            if out.returncode == 0 and out.stdout.strip() and os.path.exists(out.stdout.strip()):
                return out.stdout.strip()
        except Exception:
            pass
    for candidate in ("/usr/bin/" + name, "/usr/local/bin/" + name, "/opt/homebrew/bin/" + name):
        if os.path.exists(candidate):
            return candidate
    return None


def configure(env):
    env.add_module_version_string("swift")

    if _find_tool("swiftc") is None:
        print(
            "Swift module: 'swiftc' was not found (PATH/xcrun). The module itself will be built, but the Swift runtime library (see README.md) cannot be produced on this machine."
        )


def is_enabled():
    # The module is disabled by default. Use module_swift_enabled=yes to enable it.
    return False


def get_doc_classes():
    return [
        "SwiftLanguage",
        "SwiftScript",
        "SwiftViewHost",
    ]


def get_doc_path():
    return "doc_classes"
