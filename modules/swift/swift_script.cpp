/**************************************************************************/
/*  swift_script.cpp                                                      */
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

#include "swift_script.h"

#include "swift_bridge.h"
#include "swift_language.h"

void SwiftScript::_bind_methods() {
}

bool SwiftScript::can_instantiate() const {
	// Instantiation requires the Swift runtime library (see swift_bridge.h).
	return valid && SwiftBridge::get_singleton()->is_loaded();
}

StringName SwiftScript::get_global_name() const {
	if (get_path().is_empty()) {
		return StringName();
	}
	return get_path().get_file().get_basename();
}

Error SwiftScript::reload(bool p_keep_state) {
	// Compilation is delegated to the Swift runtime (see swift_bridge.h).
	// Until the runtime is available, a non-empty source is considered valid.
	valid = !source_code.is_empty();
	return valid ? OK : ERR_PARSE_ERROR;
}

ScriptLanguage *SwiftScript::get_language() const {
	return script_language_swift;
}

SwiftScript::SwiftScript() {
	if (script_language_swift != nullptr) {
		script_language_swift->scripts.insert(this);
	}
}

SwiftScript::~SwiftScript() {
	if (script_language_swift != nullptr) {
		script_language_swift->scripts.erase(this);
	}
}
