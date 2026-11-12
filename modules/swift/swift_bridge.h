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

#include "core/os/os.h"
#include "core/string/ustring.h"

// Bridge between the C++ module and the Swift runtime library.
//
// The Swift side of the bridge is a regular shared library built with `swiftc`
// (see README.md) that exports a small C ABI. It is loaded at runtime with
// OS::open_dynamic_library(), so the engine itself never needs a Swift
// toolchain to compile: the module builds everywhere, and script execution
// only requires the runtime library to be present next to the executable
// (or in res://bin).
class SwiftBridge {
	static SwiftBridge *singleton;

	void *library_handle = nullptr;

	// C ABI entry points resolved from the Swift runtime library.
	uint32_t (*runtime_version)() = nullptr;
	void (*runtime_init)() = nullptr;
	void (*runtime_shutdown)() = nullptr;

	Error _try_load_from(const String &p_dir);

public:
	static SwiftBridge *get_singleton();

	// Searches for the runtime library and resolves its entry points.
	// Returns OK if the runtime is available, ERR_CANT_OPEN otherwise.
	Error load();
	void unload();
	bool is_loaded() const { return library_handle != nullptr; }

	uint32_t get_runtime_version() const;
	void *get_library_handle() const { return library_handle; }

	static String get_runtime_library_name();

	~SwiftBridge();
};
