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

#include "core/os/memory.h"

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

Error SwiftBridge::_try_load_from(const String &p_dir) {
	String path = p_dir.path_join(get_runtime_library_name());
	void *handle = nullptr;
	Error err = OS::get_singleton()->open_dynamic_library(path, handle);
	if (err != OK) {
		return err;
	}

	void *symbol = nullptr;
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(handle, "swiftgodot_runtime_version", symbol);
	if (err != OK) {
		OS::get_singleton()->close_dynamic_library(handle);
		return err;
	}
	runtime_version = reinterpret_cast<uint32_t (*)()>(symbol);

	symbol = nullptr;
	OS::get_singleton()->get_dynamic_library_symbol_handle(handle, "swiftgodot_init", symbol, true);
	runtime_init = reinterpret_cast<void (*)()>(symbol);

	symbol = nullptr;
	OS::get_singleton()->get_dynamic_library_symbol_handle(handle, "swiftgodot_shutdown", symbol, true);
	runtime_shutdown = reinterpret_cast<void (*)()>(symbol);

	library_handle = handle;
	if (runtime_init != nullptr) {
		runtime_init();
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
	return ERR_CANT_OPEN;
}

void SwiftBridge::unload() {
	if (library_handle == nullptr) {
		return;
	}
	if (runtime_shutdown != nullptr) {
		runtime_shutdown();
	}
	OS::get_singleton()->close_dynamic_library(library_handle);
	library_handle = nullptr;
	runtime_version = nullptr;
	runtime_init = nullptr;
	runtime_shutdown = nullptr;
}

uint32_t SwiftBridge::get_runtime_version() const {
	if (runtime_version == nullptr) {
		return 0;
	}
	return runtime_version();
}

SwiftBridge::~SwiftBridge() {
	unload();
}
