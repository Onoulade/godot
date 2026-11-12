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
#include "swift_instance.h"
#include "swift_language.h"

#include "core/os/memory.h"

void SwiftScript::_bind_methods() {
}

bool SwiftScript::can_instantiate() const {
	// Instantiation requires the Swift runtime with the full script API.
	return valid && SwiftBridge::get_singleton()->has_script_api() && compiled_handle != nullptr;
}

StringName SwiftScript::get_global_name() const {
	if (compiled_handle != nullptr && SwiftBridge::get_singleton()->is_loaded()) {
		const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
		if (abi.script_get_class_name != nullptr) {
			const char *class_name = abi.script_get_class_name(compiled_handle);
			if (class_name != nullptr && class_name[0] != '\0') {
				return String::utf8(class_name);
			}
		}
	}
	if (get_path().is_empty()) {
		return StringName();
	}
	return get_path().get_file().get_basename();
}

ScriptInstance *SwiftScript::instance_create(Object *p_this) {
	if (!can_instantiate()) {
		return nullptr;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	SwiftGodotInstanceHandle handle = abi.instance_create(compiled_handle, (GDExtensionObjectPtr)p_this);
	if (handle == nullptr) {
		return nullptr;
	}
	return memnew(SwiftInstance(this, p_this, handle));
}

Error SwiftScript::reload(bool p_keep_state) {
	const SwiftBridge *bridge = SwiftBridge::get_singleton();
	if (compiled_handle != nullptr && bridge->is_loaded()) {
		const SwiftGodotABI &abi = bridge->get_abi();
		if (abi.script_free != nullptr) {
			abi.script_free(compiled_handle);
		}
		compiled_handle = nullptr;
	}
	valid = false;

	if (bridge->has_script_api()) {
		const SwiftGodotABI &abi = bridge->get_abi();
		CharString path = get_path().utf8();
		CharString source = source_code.utf8();
		compiled_handle = abi.script_compile(path.get_data(), source.get_data());
		if (compiled_handle != nullptr) {
			valid = abi.script_is_valid != nullptr && abi.script_is_valid(compiled_handle) != 0;
			if (!valid && abi.script_get_error != nullptr) {
				char error_buffer[1024];
				error_buffer[0] = '\0';
				abi.script_get_error(compiled_handle, error_buffer, sizeof(error_buffer));
				ERR_PRINT(vformat("Swift script '%s' failed to compile: %s", get_path(), String::utf8(error_buffer)));
			}
		}
	} else {
		// Without the runtime, a non-empty source is considered valid,
		// but the script cannot be instantiated.
		valid = !source_code.is_empty();
	}
	return valid ? OK : ERR_PARSE_ERROR;
}

bool SwiftScript::has_method(const StringName &p_method) const {
	if (compiled_handle == nullptr || !SwiftBridge::get_singleton()->is_loaded()) {
		return false;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_has_method == nullptr) {
		return false;
	}
	CharString method = String(p_method).utf8();
	return abi.script_has_method(compiled_handle, method.get_data()) != 0;
}

MethodInfo SwiftScript::get_method_info(const StringName &p_method) const {
	MethodInfo mi;
	if (has_method(p_method)) {
		mi.name = p_method;
	}
	return mi;
}

bool SwiftScript::is_tool() const {
	if (compiled_handle != nullptr && SwiftBridge::get_singleton()->is_loaded()) {
		const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
		if (abi.script_is_tool != nullptr) {
			return abi.script_is_tool(compiled_handle) != 0;
		}
	}
	return false;
}

ScriptLanguage *SwiftScript::get_language() const {
	return script_language_swift;
}

bool SwiftScript::has_script_signal(const StringName &p_signal) const {
	if (compiled_handle == nullptr || !SwiftBridge::get_singleton()->is_loaded()) {
		return false;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_has_signal == nullptr) {
		return false;
	}
	CharString signal = String(p_signal).utf8();
	return abi.script_has_signal(compiled_handle, signal.get_data()) != 0;
}

void SwiftScript::get_script_signal_list(List<MethodInfo> *r_signals) const {
	if (compiled_handle == nullptr || !SwiftBridge::get_singleton()->is_loaded()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_get_signal_count == nullptr) {
		return;
	}
	int count = abi.script_get_signal_count(compiled_handle);
	for (int i = 0; i < count; i++) {
		const char *name = abi.script_get_signal_name(compiled_handle, i);
		if (name == nullptr) {
			continue;
		}
		MethodInfo mi;
		mi.name = String::utf8(name);
		r_signals->push_back(mi);
	}
}

void SwiftScript::get_script_method_list(List<MethodInfo> *r_list) const {
	if (compiled_handle == nullptr || !SwiftBridge::get_singleton()->is_loaded()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_get_method_count == nullptr) {
		return;
	}
	int count = abi.script_get_method_count(compiled_handle);
	for (int i = 0; i < count; i++) {
		const char *name = abi.script_get_method_name(compiled_handle, i);
		if (name == nullptr) {
			continue;
		}
		MethodInfo mi;
		mi.name = String::utf8(name);
		r_list->push_back(mi);
	}
}

void SwiftScript::get_script_property_list(List<PropertyInfo> *r_list) const {
	if (compiled_handle == nullptr || !SwiftBridge::get_singleton()->is_loaded()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_get_property_count == nullptr) {
		return;
	}
	int count = abi.script_get_property_count(compiled_handle);
	for (int i = 0; i < count; i++) {
		const char *name = abi.script_get_property_name(compiled_handle, i);
		if (name == nullptr) {
			continue;
		}
		Variant::Type type = Variant::NIL;
		if (abi.script_get_property_type != nullptr) {
			type = (Variant::Type)abi.script_get_property_type(compiled_handle, i);
		}
		r_list->push_back(PropertyInfo(type, String::utf8(name)));
	}
}

SwiftScript::SwiftScript() {
	if (script_language_swift != nullptr) {
		script_language_swift->scripts.insert(this);
	}
}

SwiftScript::~SwiftScript() {
	if (compiled_handle != nullptr && SwiftBridge::get_singleton()->is_loaded()) {
		const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
		if (abi.script_free != nullptr) {
			abi.script_free(compiled_handle);
		}
		compiled_handle = nullptr;
	}
	if (script_language_swift != nullptr) {
		script_language_swift->scripts.erase(this);
	}
}
