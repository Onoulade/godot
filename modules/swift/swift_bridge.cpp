/**************************************************************************/
/*  swift_bridge.cpp                                                      */
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

#include "swift_bridge.h"

#include "swift_host.h"

#include "core/os/memory.h"

// Engine-side GDExtension function resolver, handed to the Swift runtime so
// it can call back into the engine (variants, object calls, ...).
extern GDExtensionInterfaceFunctionPtr gdextension_get_proc_address(const char *p_name);

SwiftBridge *SwiftBridge::singleton = nullptr;

SwiftBridge *SwiftBridge::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(SwiftBridge);
	}
	return singleton;
}

String SwiftBridge::get_runtime_library_name() {
#if defined(_WIN32)
	return "swiftgodot_runtime.dll";
#elif defined(__APPLE__)
	return "libswiftgodot_runtime.dylib";
#else
	return "libswiftgodot_runtime.so";
#endif
}

void *SwiftBridge::_resolve(const char *p_name, bool p_optional) {
	void *symbol = nullptr;
	Error err = OS::get_singleton()->get_dynamic_library_symbol_handle(library_handle, p_name, symbol, p_optional);
	if (err != OK) {
		if (!p_optional) {
			OS::get_singleton()->close_dynamic_library(library_handle);
			library_handle = nullptr;
			return nullptr;
		}
		return nullptr;
	}
	return symbol;
}

Error SwiftBridge::_try_load_from(const String &p_dir) {
	String path = p_dir.path_join(get_runtime_library_name());
	void *handle = nullptr;
	Error err = OS::get_singleton()->open_dynamic_library(path, handle);
	if (err != OK) {
		return err;
	}
	library_handle = handle;

	void *version_sym = _resolve("swiftgodot_runtime_version", false);
	if (version_sym == nullptr) {
		return ERR_CANT_OPEN;
	}
	abi.runtime_version = reinterpret_cast<uint32_t (*)()>(version_sym);

	abi.runtime_init = reinterpret_cast<void (*)(GDExtensionInterfaceGetProcAddress)>(_resolve("swiftgodot_init", true));
	abi.host_init = reinterpret_cast<void (*)(const SwiftGodotHost *)>(_resolve("swiftgodot_host_init", true));
	abi.runtime_shutdown = reinterpret_cast<void (*)()>(_resolve("swiftgodot_shutdown", true));

	abi.script_compile = reinterpret_cast<SwiftGodotScriptHandle (*)(const char *, const char *)>(_resolve("swiftgodot_script_compile", true));
	abi.script_free = reinterpret_cast<void (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_free", true));
	abi.script_is_valid = reinterpret_cast<GDExtensionBool (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_is_valid", true));
	abi.script_get_error = reinterpret_cast<void (*)(SwiftGodotScriptHandle, char *, int)>(_resolve("swiftgodot_script_get_error", true));
	abi.script_get_class_name = reinterpret_cast<const char *(*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_get_class_name", true));
	abi.script_is_tool = reinterpret_cast<GDExtensionBool (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_is_tool", true));
	abi.script_get_method_count = reinterpret_cast<int (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_get_method_count", true));
	abi.script_get_method_name = reinterpret_cast<const char *(*)(SwiftGodotScriptHandle, int)>(_resolve("swiftgodot_script_get_method_name", true));
	abi.script_has_method = reinterpret_cast<GDExtensionBool (*)(SwiftGodotScriptHandle, const char *)>(_resolve("swiftgodot_script_has_method", true));
	abi.script_get_signal_count = reinterpret_cast<int (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_get_signal_count", true));
	abi.script_get_signal_name = reinterpret_cast<const char *(*)(SwiftGodotScriptHandle, int)>(_resolve("swiftgodot_script_get_signal_name", true));
	abi.script_has_signal = reinterpret_cast<GDExtensionBool (*)(SwiftGodotScriptHandle, const char *)>(_resolve("swiftgodot_script_has_signal", true));
	abi.script_get_property_count = reinterpret_cast<int (*)(SwiftGodotScriptHandle)>(_resolve("swiftgodot_script_get_property_count", true));
	abi.script_get_property_name = reinterpret_cast<const char *(*)(SwiftGodotScriptHandle, int)>(_resolve("swiftgodot_script_get_property_name", true));
	abi.script_get_property_type = reinterpret_cast<int (*)(SwiftGodotScriptHandle, int)>(_resolve("swiftgodot_script_get_property_type", true));
	abi.script_get_property_usage = reinterpret_cast<int (*)(SwiftGodotScriptHandle, int)>(_resolve("swiftgodot_script_get_property_usage", true));

	abi.scene_build = reinterpret_cast<GDExtensionObjectPtr (*)(GDExtensionObjectPtr)>(_resolve("swiftgodot_scene_build", true));
	abi.scene_invalidate = reinterpret_cast<void (*)(GDExtensionObjectPtr)>(_resolve("swiftgodot_scene_invalidate", true));

	abi.instance_create = reinterpret_cast<SwiftGodotInstanceHandle (*)(SwiftGodotScriptHandle, GDExtensionObjectPtr)>(_resolve("swiftgodot_instance_create", true));
	abi.instance_free = reinterpret_cast<void (*)(SwiftGodotInstanceHandle)>(_resolve("swiftgodot_instance_free", true));
	abi.instance_call = reinterpret_cast<GDExtensionBool (*)(SwiftGodotInstanceHandle, const char *, GDExtensionConstVariantPtr *, int, GDExtensionVariantPtr, GDExtensionCallError *)>(_resolve("swiftgodot_instance_call", true));
	abi.instance_notification = reinterpret_cast<void (*)(SwiftGodotInstanceHandle, int, GDExtensionBool)>(_resolve("swiftgodot_instance_notification", true));
	abi.instance_set = reinterpret_cast<GDExtensionBool (*)(SwiftGodotInstanceHandle, const char *, GDExtensionConstVariantPtr)>(_resolve("swiftgodot_instance_set", true));
	abi.instance_get = reinterpret_cast<GDExtensionBool (*)(SwiftGodotInstanceHandle, const char *, GDExtensionVariantPtr)>(_resolve("swiftgodot_instance_get", true));

	if (abi.runtime_init != nullptr) {
		abi.runtime_init(&gdextension_get_proc_address);
	}
	if (abi.host_init != nullptr) {
		abi.host_init(SwiftHost::get_host());
	}
	return OK;
}

Error SwiftBridge::load() {
	if (library_handle != nullptr) {
		return OK;
	}

	Vector<String> search_dirs;
	search_dirs.push_back(OS::get_singleton()->get_executable_path().get_base_dir());
	search_dirs.push_back("res://bin");
	search_dirs.push_back("res://");
	for (const String &dir : search_dirs) {
		if (_try_load_from(dir) == OK) {
			return OK;
		}
	}
	library_handle = nullptr;
	return ERR_CANT_OPEN;
}

void SwiftBridge::unload() {
	if (library_handle == nullptr) {
		return;
	}
	if (abi.runtime_shutdown != nullptr) {
		abi.runtime_shutdown();
	}
	OS::get_singleton()->close_dynamic_library(library_handle);
	library_handle = nullptr;
	abi = SwiftGodotABI();
}

uint32_t SwiftBridge::get_runtime_version() const {
	if (abi.runtime_version == nullptr) {
		return 0;
	}
	return abi.runtime_version();
}

SwiftBridge::~SwiftBridge() {
	unload();
}
