/**************************************************************************/
/*  swift_bridge.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/extension/gdextension_interface.gen.h"
#include "core/os/os.h"
#include "core/string/ustring.h"

struct SwiftGodotHost;

// Bridge between the C++ module and the Swift runtime library
// (modules/swift/runtime, built with `swiftc`).
//
// The Swift side is a regular shared library that exports the C ABI below
// and receives the engine's GDExtension function resolver at init, exactly
// like a GDExtension library does. The engine side loads it at runtime with
// OS::open_dynamic_library(), so the engine itself never needs a Swift
// toolchain to compile.

typedef void *SwiftGodotScriptHandle;
typedef void *SwiftGodotInstanceHandle;

// C ABI exported by the Swift runtime (see modules/swift/runtime).
// Names are passed as null-terminated UTF-8 strings; variants and objects
// use the GDExtension pointer types (in-tree, a Variant* / Object* casts to
// them directly).
struct SwiftGodotABI {
	uint32_t (*runtime_version)() = nullptr;
	void (*runtime_init)(GDExtensionInterfaceGetProcAddress p_get_proc_address) = nullptr;
	void (*host_init)(const SwiftGodotHost *p_host) = nullptr;
	void (*runtime_shutdown)() = nullptr;

	SwiftGodotScriptHandle (*script_compile)(const char *p_path, const char *p_source) = nullptr;
	// Loads a precompiled script library (".swiftbin" next to the source,
	// produced by the editor at compile time) instead of running swiftc;
	// used in exported games, which have no Swift toolchain.
	SwiftGodotScriptHandle (*script_load_precompiled)(const char *p_library_path, const char *p_path, const char *p_source) = nullptr;
	// Writes the path of the library a script handle was loaded from (if any).
	void (*script_get_library_path)(SwiftGodotScriptHandle p_script, char *r_buffer, int p_buffer_size) = nullptr;
	void (*script_free)(SwiftGodotScriptHandle p_script) = nullptr;
	GDExtensionBool (*script_is_valid)(SwiftGodotScriptHandle p_script) = nullptr;
	void (*script_get_error)(SwiftGodotScriptHandle p_script, char *r_buffer, int p_buffer_size) = nullptr;
	const char *(*script_get_class_name)(SwiftGodotScriptHandle p_script) = nullptr;
	GDExtensionBool (*script_is_tool)(SwiftGodotScriptHandle p_script) = nullptr;
	int (*script_get_method_count)(SwiftGodotScriptHandle p_script) = nullptr;
	const char *(*script_get_method_name)(SwiftGodotScriptHandle p_script, int p_index) = nullptr;
	GDExtensionBool (*script_has_method)(SwiftGodotScriptHandle p_script, const char *p_method) = nullptr;
	int (*script_get_signal_count)(SwiftGodotScriptHandle p_script) = nullptr;
	const char *(*script_get_signal_name)(SwiftGodotScriptHandle p_script, int p_index) = nullptr;
	GDExtensionBool (*script_has_signal)(SwiftGodotScriptHandle p_script, const char *p_signal) = nullptr;
	int (*script_get_property_count)(SwiftGodotScriptHandle p_script) = nullptr;
	const char *(*script_get_property_name)(SwiftGodotScriptHandle p_script, int p_index) = nullptr;
	int (*script_get_property_type)(SwiftGodotScriptHandle p_script, int p_index) = nullptr;
	int (*script_get_property_usage)(SwiftGodotScriptHandle p_script, int p_index) = nullptr;

	// Declarative UI (PR #2): build the root node of a Swift scene and
	// invalidate it (scoped re-render through the attribute graph).
	GDExtensionObjectPtr (*scene_build)(GDExtensionObjectPtr p_host) = nullptr;
	void (*scene_invalidate)(GDExtensionObjectPtr p_host) = nullptr;

	SwiftGodotInstanceHandle (*instance_create)(SwiftGodotScriptHandle p_script, GDExtensionObjectPtr p_owner) = nullptr;
	void (*instance_free)(SwiftGodotInstanceHandle p_instance) = nullptr;
	GDExtensionBool (*instance_call)(SwiftGodotInstanceHandle p_instance, const char *p_method, GDExtensionConstVariantPtr *p_args, int p_arg_count, GDExtensionVariantPtr r_ret, GDExtensionCallError *r_error) = nullptr;
	void (*instance_notification)(SwiftGodotInstanceHandle p_instance, int p_notification, GDExtensionBool p_reversed) = nullptr;
	GDExtensionBool (*instance_set)(SwiftGodotInstanceHandle p_instance, const char *p_name, GDExtensionConstVariantPtr p_value) = nullptr;
	GDExtensionBool (*instance_get)(SwiftGodotInstanceHandle p_instance, const char *p_name, GDExtensionVariantPtr r_value) = nullptr;
};

class SwiftBridge {
	static SwiftBridge *singleton;

	void *library_handle = nullptr;
	SwiftGodotABI abi;

	void *_resolve(const char *p_name, bool p_optional);
	Error _try_load_from(const String &p_dir);

public:
	static SwiftBridge *get_singleton();

	// Searches for the runtime library and resolves its entry points.
	// Returns OK if the runtime is available, ERR_CANT_OPEN otherwise.
	Error load();
	void unload();
	bool is_loaded() const { return library_handle != nullptr; }

	const SwiftGodotABI &get_abi() const { return abi; }
	// True when the runtime exposes the full script/instance API
	// (compile + instantiation), i.e. scripts can actually run.
	bool has_script_api() const { return is_loaded() && abi.script_compile != nullptr && abi.instance_create != nullptr; }

	uint32_t get_runtime_version() const;
	void *get_library_handle() const { return library_handle; }

	static String get_runtime_library_name();

	~SwiftBridge();
};
