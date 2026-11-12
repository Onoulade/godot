def can_build(env, platform):
    return True


def configure(env):
    import shutil

    env.add_module_version_string("swift")

    if shutil.which("swiftc") is None:
        print(
            "Swift module: 'swiftc' was not found in PATH. The module itself will be built, but the Swift runtime library (see README.md) cannot be produced on this machine."
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
