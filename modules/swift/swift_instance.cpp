/**************************************************************************/
/*  swift_instance.cpp                                                    */
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

#include "swift_instance.h"

#include "swift_language.h"
#include "swift_script.h"

#include "core/object/object.h"
#include "core/os/memory.h"
#include "scene/main/canvas_item.h"
#include "scene/main/node.h"

bool SwiftInstance::set(const StringName &p_name, const Variant &p_value) {
	if (!SwiftBridge::get_singleton()->has_script_api()) {
		return false;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	CharString name = String(p_name).utf8();
	return abi.instance_set(handle, name.get_data(), (GDExtensionConstVariantPtr)&p_value) != 0;
}

bool SwiftInstance::get(const StringName &p_name, Variant &r_ret) const {
	if (!SwiftBridge::get_singleton()->has_script_api()) {
		return false;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	CharString name = String(p_name).utf8();
	return abi.instance_get(handle, name.get_data(), (GDExtensionVariantPtr)&r_ret) != 0;
}

void SwiftInstance::get_property_list(List<PropertyInfo> *r_properties) const {
	if (script == nullptr || !SwiftBridge::get_singleton()->has_script_api()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	SwiftGodotScriptHandle script_handle = script->get_compiled_handle();
	if (script_handle == nullptr || abi.script_get_property_count == nullptr) {
		return;
	}
	int count = abi.script_get_property_count(script_handle);
	for (int i = 0; i < count; i++) {
		const char *name = abi.script_get_property_name(script_handle, i);
		if (name == nullptr) {
			continue;
		}
		Variant::Type type = Variant::NIL;
		if (abi.script_get_property_type != nullptr) {
			type = (Variant::Type)abi.script_get_property_type(script_handle, i);
		}
		r_properties->push_back(PropertyInfo(type, String::utf8(name)));
	}
}

Variant::Type SwiftInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	if (r_is_valid != nullptr) {
		*r_is_valid = false;
	}
	if (script == nullptr || !SwiftBridge::get_singleton()->has_script_api()) {
		return Variant::NIL;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	SwiftGodotScriptHandle script_handle = script->get_compiled_handle();
	if (script_handle == nullptr || abi.script_get_property_count == nullptr) {
		return Variant::NIL;
	}
	int count = abi.script_get_property_count(script_handle);
	for (int i = 0; i < count; i++) {
		const char *prop_name = abi.script_get_property_name(script_handle, i);
		if (prop_name != nullptr && String::utf8(prop_name) == p_name) {
			if (r_is_valid != nullptr) {
				*r_is_valid = true;
			}
			return abi.script_get_property_type != nullptr ? (Variant::Type)abi.script_get_property_type(script_handle, i) : Variant::NIL;
		}
	}
	return Variant::NIL;
}

void SwiftInstance::get_method_list(List<MethodInfo> *r_list) const {
	if (script == nullptr || !SwiftBridge::get_singleton()->has_script_api()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	SwiftGodotScriptHandle script_handle = script->get_compiled_handle();
	if (script_handle == nullptr || abi.script_get_method_count == nullptr) {
		return;
	}
	int count = abi.script_get_method_count(script_handle);
	for (int i = 0; i < count; i++) {
		const char *name = abi.script_get_method_name(script_handle, i);
		if (name == nullptr) {
			continue;
		}
		MethodInfo mi;
		mi.name = String::utf8(name);
		r_list->push_back(mi);
	}
}

bool SwiftInstance::has_method(const StringName &p_method) const {
	if (script == nullptr || !SwiftBridge::get_singleton()->has_script_api()) {
		return false;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.script_has_method == nullptr) {
		return false;
	}
	CharString method = String(p_method).utf8();
	return abi.script_has_method(script->get_compiled_handle(), method.get_data()) != 0;
}

Variant SwiftInstance::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	Variant ret;
	r_error.error = Callable::CallError::CALL_OK;
	if (!SwiftBridge::get_singleton()->has_script_api()) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
		return ret;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.instance_call == nullptr) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
		return ret;
	}
	CharString method = String(p_method).utf8();
	Vector<GDExtensionConstVariantPtr> arg_ptrs;
	for (int i = 0; i < p_argcount; i++) {
		arg_ptrs.push_back((GDExtensionConstVariantPtr)p_args[i]);
	}
	GDExtensionCallError gde_error{};
	GDExtensionBool ok = abi.instance_call(handle, method.get_data(), p_argcount > 0 ? arg_ptrs.ptrw() : nullptr, p_argcount, (GDExtensionVariantPtr)&ret, &gde_error);
	if (ok == 0) {
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
	} else {
		r_error.error = (Callable::CallError::Error)gde_error.error;
		r_error.argument = gde_error.argument;
		r_error.expected = gde_error.expected;
	}
	return ret;
}

void SwiftInstance::_call_virtual(const StringName &p_method, const Variant **p_args, int p_argcount) {
	if (!has_method(p_method)) {
		return;
	}
	Callable::CallError error;
	callp(p_method, p_args, p_argcount, error);
}

void SwiftInstance::notification(int p_notification, bool p_reversed) {
	if (!SwiftBridge::get_singleton()->has_script_api()) {
		return;
	}
	const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
	if (abi.instance_notification != nullptr) {
		abi.instance_notification(handle, p_notification, p_reversed ? 1 : 0);
	}

	// Map the common engine notifications to the conventional Swift
	// virtual methods, like other languages do.
	Node *node = Object::cast_to<Node>(owner);
	switch (p_notification) {
		case Node::NOTIFICATION_ENTER_TREE:
			_call_virtual("_enter_tree", nullptr, 0);
			break;
		case Node::NOTIFICATION_EXIT_TREE:
			_call_virtual("_exit_tree", nullptr, 0);
			break;
		case Node::NOTIFICATION_READY:
			_call_virtual("_ready", nullptr, 0);
			break;
		case Node::NOTIFICATION_PROCESS: {
			Variant args[1] = { node != nullptr ? Variant(node->get_process_delta_time()) : Variant(0.0) };
			const Variant *arg_ptrs[1] = { &args[0] };
			_call_virtual("_process", arg_ptrs, 1);
			break;
		}
		case Node::NOTIFICATION_PHYSICS_PROCESS: {
			Variant args[1] = { node != nullptr ? Variant(node->get_physics_process_delta_time()) : Variant(0.0) };
			const Variant *arg_ptrs[1] = { &args[0] };
			_call_virtual("_physics_process", arg_ptrs, 1);
			break;
		}
		case CanvasItem::NOTIFICATION_DRAW:
			_call_virtual("_draw", nullptr, 0);
			break;
		default:
			break;
	}
}

Script *SwiftInstance::get_script() const {
	return script;
}

ScriptLanguage *SwiftInstance::get_language() {
	return script_language_swift;
}

SwiftInstance::SwiftInstance(SwiftScript *p_script, Object *p_owner, SwiftGodotInstanceHandle p_handle) {
	script = p_script;
	owner = p_owner;
	handle = p_handle;
}

SwiftInstance::~SwiftInstance() {
	if (handle != nullptr && SwiftBridge::get_singleton()->is_loaded()) {
		const SwiftGodotABI &abi = SwiftBridge::get_singleton()->get_abi();
		if (abi.instance_free != nullptr) {
			abi.instance_free(handle);
		}
	}
	handle = nullptr;
}
