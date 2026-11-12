#!/usr/bin/env python3
"""Generate Swift bindings for Godot classes from extension_api.json.

Usage:
    python3 modules/swift/tools/generate_bindings.py --api extension_api.json --classes Node,Node2D,Sprite2D --out GodotBindings.swift
    python3 modules/swift/tools/generate_bindings.py --api extension_api.json --all --out GodotBindings.swift

`extension_api.json` is produced by an editor build with `godot --headless --dump-extension-api`.
Object-typed values are wrapped in generated Swift classes; value types
(Vector2, Color, ...) and Variant are passed opaquely through VariantBox.
"""

import argparse
import ast
import json
import re
import sys

GLOBAL_ENUM_HEADERS = [
    "core/math/math_defs.h",
    "core/error/error_list.h",
    "core/error/error_macros.h",
    "core/input/input_enums.h",
    "core/os/keyboard.h",
    "core/object/property_info.h",
    "core/object/method_info.h",
    "core/io/resource_loader_constants.h",
    "core/io/file_access_pack.h",
    "core/variant/type_info.h",
]


def safe_eval(expr, consts):
    node = ast.parse(expr, mode="eval")

    def ev(n):
        if isinstance(n, ast.Expression):
            return ev(n.body)
        if isinstance(n, ast.Constant) and isinstance(n.value, int):
            return n.value
        if isinstance(n, ast.Name):
            return consts[n.id]
        if isinstance(n, ast.BinOp):
            lv, rv = ev(n.left), ev(n.right)
            if isinstance(n.op, ast.LShift):
                return lv << rv
            if isinstance(n.op, ast.RShift):
                return lv >> rv
            if isinstance(n.op, ast.BitOr):
                return lv | rv
            if isinstance(n.op, ast.BitAnd):
                return lv & rv
            if isinstance(n.op, ast.BitXor):
                return lv ^ rv
            if isinstance(n.op, ast.Add):
                return lv + rv
            if isinstance(n.op, ast.Sub):
                return lv - rv
            if isinstance(n.op, ast.Mult):
                return lv * rv
        if isinstance(n, ast.UnaryOp):
            if isinstance(n.op, ast.USub):
                return -ev(n.operand)
            if isinstance(n.op, ast.Invert):
                return ~ev(n.operand)
        if (
            isinstance(n, ast.Call)
            and isinstance(n.func, ast.Name)
            and n.func.id
            in (
                "int",
                "uint8_t",
                "uint16_t",
                "uint32_t",
                "uint64_t",
                "int8_t",
                "int16_t",
                "int32_t",
                "int64_t",
                "unsigned",
                "char",
                "short",
                "long",
                "float",
                "double",
            )
        ):
            return ev(n.args[0])
        raise ValueError(f"unsupported expression: {ast.dump(n)}")

    return ev(node)


ALL_CONSTS: dict[str, int] = {}


def parse_header_enums(path):
    with open(path) as f:
        text = f.read()
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    enums = {}
    for m in re.finditer(r"enum\s+(?:class\s+)?(\w+)\s*(?::\s*\w+\s*)?\{([^}]*)\}", text):
        name, body = m.group(1), m.group(2)
        consts = {}
        cur = -1
        for item in body.split(","):
            item = item.strip()
            if not item:
                continue
            if "=" in item:
                cname, expr = item.split("=", 1)
                cname = cname.strip()
                expr = re.sub(r"\w+::(\w+)", r"\1", expr)
                expr = re.sub(r"\b(?:u?int\d*_t?|unsigned|char|short|long|float|double)\s*\(([^()]*)\)", r"(\1)", expr)
                val = safe_eval(expr.strip(), {**ALL_CONSTS, **consts})
            else:
                cname = item.strip()
                cur += 1
                val = cur
            if re.fullmatch(r"\w+", cname):
                consts[cname] = val
                cur = val
        if consts:
            enums[name] = consts
            ALL_CONSTS.update(consts)
    return enums


SWIFT_KEYWORDS = {
    "associatedtype",
    "class",
    "deinit",
    "enum",
    "extension",
    "fileprivate",
    "func",
    "import",
    "init",
    "inout",
    "internal",
    "let",
    "open",
    "operator",
    "private",
    "protocol",
    "public",
    "rethrows",
    "static",
    "struct",
    "subscript",
    "typealias",
    "var",
    "break",
    "case",
    "continue",
    "default",
    "defer",
    "do",
    "else",
    "fallthrough",
    "for",
    "guard",
    "if",
    "in",
    "repeat",
    "return",
    "switch",
    "where",
    "while",
    "as",
    "any",
    "catch",
    "false",
    "is",
    "nil",
    "self",
    "Self",
    "super",
    "throws",
    "true",
    "try",
}


def sanitize(name):
    if not name:
        return "unnamed_"
    if name in SWIFT_KEYWORDS or name[0].isdigit():
        return name + "_"
    return name


def is_void(jtype):
    return (jtype or "").lower() in ("void", "nil")


VALUE_TYPES = {
    "vector2": "Vector2",
    "vector3": "Vector3",
    "vector2i": "Vector2i",
    "vector3i": "Vector3i",
    "vector4": "Vector4",
    "vector4i": "Vector4i",
    "rect2": "Rect2",
    "rect2i": "Rect2i",
    "color": "Color",
    "quaternion": "Quaternion",
    "aabb": "AABB",
    "plane": "Plane",
    "transform2d": "Transform2D",
    "transform3d": "Transform3D",
    "basis": "Basis",
    "projection": "Projection",
}

VALUE_TYPE_ACCESSORS = {v: "as" + v for v in VALUE_TYPES.values()}


def snake_upper(s):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", s).upper()


def camel_case(s):
    parts = [p for p in s.lower().split("_") if p]
    if not parts:
        return "value"
    return parts[0] + "".join(p.capitalize() for p in parts[1:])


GLOBAL_ENUMS: dict[str, dict[str, int]] = {}

# "Node.InternalMode" -> {"INTERNAL_MODE_DISABLED": 0, ...}, for enum default values.
CLASS_ENUM_VALUES: dict[str, dict[str, int]] = {}


def enum_case_names(enum_name, values):
    prefix = snake_upper(enum_name) + "_"
    cases = []
    used = set()
    for const, val in sorted(values.items(), key=lambda kv: kv[1]):
        base = const[len(prefix) :] if const.startswith(prefix) else const
        case = sanitize(camel_case(base))
        while case in used:
            case += "_"
        used.add(case)
        cases.append((case, val))
    return cases


def enum_default_literal(class_name, dv):
    if dv is None or not class_name:
        return None
    if "." in class_name:
        values = CLASS_ENUM_VALUES.get(class_name)
        enum_name = class_name.split(".")[-1]
    else:
        values = GLOBAL_ENUMS.get(class_name)
        enum_name = class_name
    if not values:
        return None
    try:
        target = int(str(dv).strip())
    except ValueError:
        return None
    for case, val in enum_case_names(enum_name, values):
        if val == target:
            return "." + case
    return None


def enum_swift_type(class_name, generated):
    # "Node.ProcessMode" -> "Node.ProcessMode" if Node is generated, else Int.
    if not class_name:
        return "Int"
    if "." not in class_name:
        return class_name if class_name in GLOBAL_ENUMS else "Int"
    cls = class_name.split(".")[0]
    if cls in generated:
        return class_name
    return "Int"


def is_enum(jtype, class_name):
    if (jtype or "").lower() != "int" or not class_name:
        return False
    return "." in class_name or class_name in GLOBAL_ENUMS


def swift_type(jtype, class_name, generated):
    t = (jtype or "").lower()
    if t == "bool":
        return "Bool"
    if t == "int":
        if is_enum(jtype, class_name):
            return enum_swift_type(class_name, generated)
        return "Int"
    if t == "float":
        return "Double"
    if t in ("string", "stringname", "nodepath"):
        return "String"
    if t in VALUE_TYPES:
        return VALUE_TYPES[t]
    if t == "object":
        if class_name and class_name in generated:
            return class_name
        return "GodotObject"
    return "VariantBox"


def default_literal(dv):
    if dv is None:
        return None
    s = str(dv).strip()
    if s in ("true", "false"):
        return s
    if re.fullmatch(r"-?\d+", s):
        return s
    if re.fullmatch(r"-?\d+\.\d*([eE][+-]?\d+)?", s):
        return s
    if len(s) >= 2 and s.startswith('"') and s.endswith('"'):
        return s
    return None


def emit_method(m, generated):
    name = sanitize(m.get("name", "unknown"))
    ret = m.get("return_type", {})
    ret_void = is_void(ret.get("type", "void"))
    ret_swift = swift_type(ret.get("type"), ret.get("class_name"), generated)
    args = m.get("arguments", [])
    params = []
    call_args = []
    for a in args:
        st = swift_type(a.get("type"), a.get("class_name"), generated)
        pname = sanitize(a.get("name", "arg"))
        if is_enum(a.get("type"), a.get("class_name")):
            dv = enum_default_literal(a.get("class_name"), a.get("default_value")) if st != "Int" else None
        else:
            dv = default_literal(a.get("default_value"))
        params.append(f"{pname}: {st} = {dv}" if dv is not None else f"{pname}: {st}")
        if is_enum(a.get("type"), a.get("class_name")) and st != "Int":
            call_args.append(f"VariantBox({pname}.rawValue)")
        else:
            call_args.append(f"VariantBox({pname})")
    signature = f"public func {name}({', '.join(params)})"
    lines = []
    if not ret_void:
        lines.append("\t@discardableResult")
    lines.append(
        "\t"
        + signature
        + ("" if ret_void else f" -> {ret_swift}" + ("?" if ret.get("type", "").lower() == "object" else ""))
        + " {"
    )
    mname = m.get("name", "unknown")
    args_joined = ", ".join(call_args)
    call = f'Host.shared.callMethod(object: ptr, method: "{mname}", args: [{args_joined}])'
    lines.append(f"\t\tlet __ret = {call}")
    if ret_void:
        pass
    elif ret_swift == "Bool":
        lines.append("\t\treturn __ret.asBool()")
    elif ret_swift == "Int":
        lines.append("\t\treturn __ret.asInt()")
    elif ret_swift == "Double":
        lines.append("\t\treturn __ret.asDouble()")
    elif ret_swift == "String":
        lines.append("\t\treturn __ret.asString()")
    elif ret_swift in VALUE_TYPE_ACCESSORS:
        lines.append(f"\t\treturn __ret.{VALUE_TYPE_ACCESSORS[ret_swift]}()")
    elif is_enum(ret.get("type"), ret.get("class_name")) and ret_swift != "Int":
        lines.append(f"\t\treturn __ret.asEnum({ret_swift}.self)")
    elif ret_swift == "VariantBox":
        lines.append("\t\treturn __ret")
    else:
        lines.append(f"\t\treturn __ret.asObject({ret_swift}.self)")
    lines.append("\t}")
    return lines


def emit_property(p, method_names):
    name = sanitize(p.get("name", "unknown"))
    if name in method_names:
        return []
    st = swift_type(p.get("type"), p.get("class_name"), set())
    raw = p.get("name")
    if is_enum(p.get("type"), p.get("class_name")) and st != "Int":
        accessor = f".asEnum({st}.self)"
    else:
        accessor = f".as{st}()" if st in ("Bool", "Int", "Double", "String") or st in VALUE_TYPE_ACCESSORS else ""
    lines = [
        f"\tpublic var {name}: {st} {{",
        f'\t\tget {{ Host.shared.getProperty(object: ptr, name: "{raw}")' + accessor + " }",
        f'\t\tset {{ Host.shared.setProperty(object: ptr, name: "{raw}", value: VariantBox(newValue)) }}',
        "\t}",
    ]
    return lines


def emit_enum_cases(name, values, indent):
    return [f"{indent}case {case} = {val}" for case, val in enum_case_names(name, values)]


def emit_enum(en):
    name = sanitize(en.get("name", "Enum"))
    lines = [f"\tpublic enum {name}: Int, Sendable, CaseIterable {{"]
    lines.extend(emit_enum_cases(en.get("name", ""), en.get("values", {}), "\t\t"))
    lines.append("\t}")
    return lines


def emit_top_level_enum(name, values):
    lines = [f"public enum {sanitize(name)}: Int, Sendable, CaseIterable {{"]
    lines.extend(emit_enum_cases(name, values, "\t"))
    lines.append("}")
    lines.append("")
    return lines


def emit_signal(s):
    name = sanitize(s.get("name", "unknown"))
    args = s.get("arguments", [])
    params = []
    sname = s.get("name", "unknown")
    call_args = [f'VariantBox("{sname}")']
    for a in args:
        st = swift_type(a.get("type"), a.get("class_name"), set())
        pname = sanitize(a.get("name", "arg"))
        params.append(f"{pname}: {st}")
        call_args.append(f"VariantBox({pname})")
    args_joined = ", ".join(call_args)
    params_joined = ", ".join(params)
    return [
        f"\tpublic func emit_{name}({params_joined}) {{",
        f'\t\t_ = Host.shared.callMethod(object: ptr, method: "emit_signal", args: [{args_joined}])',
        "\t}",
    ]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--api", default="extension_api.json", help="Path to extension_api.json")
    parser.add_argument("--classes", help="Comma-separated class names to generate")
    parser.add_argument("--all", action="store_true", help="Generate all classes in the API")
    parser.add_argument("--exclude", help="Comma-separated class names to skip (e.g. module classes)")
    parser.add_argument("--out", required=True, help="Output Swift file")
    parser.add_argument(
        "--global-enums-headers",
        default=",".join(GLOBAL_ENUM_HEADERS),
        help="Comma-separated core headers to extract global enums from",
    )
    args = parser.parse_args()

    with open(args.api) as f:
        api = json.load(f)

    for header in args.global_enums_headers.split(","):
        GLOBAL_ENUMS.update(parse_header_enums(header))

    all_classes = {c["name"]: c for c in api.get("classes", [])}
    for c in all_classes.values():
        for en in c.get("enums", []):
            CLASS_ENUM_VALUES[f"{c['name']}.{en.get('name', '')}"] = en.get("values", {})
    if args.all:
        selected = set(all_classes)
    elif args.classes:
        selected = set(args.classes.split(","))
        missing = selected - set(all_classes)
        if missing:
            print(f"Unknown classes: {sorted(missing)}", file=sys.stderr)
            sys.exit(1)
    else:
        print("Provide --classes or --all", file=sys.stderr)
        sys.exit(1)
    if args.exclude:
        selected -= {c.strip() for c in args.exclude.split(",") if c.strip()}

    def depth(name):
        d = 0
        cur = name
        while cur in all_classes and all_classes[cur].get("inherits"):
            cur = all_classes[cur]["inherits"]
            d += 1
        return d

    ordered = sorted(selected, key=lambda n: (depth(n), n))

    out = []
    out.append("// Generated by modules/swift/tools/generate_bindings.py from extension_api.json. Do not edit.")
    out.append(
        "// Regenerate with: python3 modules/swift/tools/generate_bindings.py --api extension_api.json --classes ... --out <this file>"
    )
    out.append("import Foundation")
    out.append("")
    out.append("// Godot's API is main-thread-only, so all bindings are @MainActor:")
    out.append("// Swift 6 strict concurrency then rejects off-main-thread calls at compile time.")
    out.append("@MainActor")
    out.append("public class GodotObject {")
    out.append("\tpublic let ptr: UnsafeMutableRawPointer")
    out.append("\tpublic init(ptr: UnsafeMutableRawPointer) { self.ptr = ptr }")
    out.append("\tpublic convenience init?(raw: UnsafeMutableRawPointer?) {")
    out.append("\t\tguard let raw else { return nil }")
    out.append("\t\tself.init(ptr: raw)")
    out.append("\t}")
    out.append("\t@discardableResult public func call(_ method: String, _ args: [VariantBox] = []) -> VariantBox {")
    out.append("\t\treturn Host.shared.callMethod(object: ptr, method: method, args: args)")
    out.append("\t}")
    out.append("}")
    out.append("")
    for ename in sorted(GLOBAL_ENUMS):
        out.extend(emit_top_level_enum(ename, GLOBAL_ENUMS[ename]))
    for cname in ordered:
        c = all_classes[cname]
        parent = c.get("inherits") or "GodotObject"
        if parent not in selected and parent != "":
            parent = "GodotObject"
        out.append("@MainActor")
        out.append(f"public class {cname}: {parent} {{")
        out.append("\tpublic override init(ptr: UnsafeMutableRawPointer) { super.init(ptr: ptr) }")
        out.append(
            "\tpublic override convenience init() {" if parent != "GodotObject" else "\tpublic convenience init() {"
        )
        out.append(f'\t\tguard let raw = Host.shared.createObject("{cname}") else {{')
        out.append(f'\t\t\tfatalError("SwiftGodot: cannot instantiate Godot class {cname}")')
        out.append("\t\t}")
        out.append("\t\tself.init(ptr: raw)")
        out.append("\t}")
        for en in c.get("enums", []):
            out.extend(emit_enum(en))
        method_names = set()
        for m in c.get("methods", []):
            if m.get("is_virtual"):
                continue
            method_names.add(sanitize(m.get("name")))
            out.extend(emit_method(m, selected))
        for p in c.get("properties", []):
            out.extend(emit_property(p, method_names))
        for s in c.get("signals", []):
            out.extend(emit_signal(s))
        out.append("}")
        out.append("")

    with open(args.out, "w") as f:
        f.write("\n".join(out))
    print(f"Wrote {args.out}: {len(ordered)} classes")


if __name__ == "__main__":
    main()
