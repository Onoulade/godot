/**************************************************************************/
/*  swift_instance.h                                                      */
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

#include "swift_bridge.h"

#include "core/object/script_instance.h"

class SwiftScript;

// ScriptInstance implementation that forwards everything to a Swift script
// instance living in the Swift runtime (through the C ABI in swift_bridge.h).
class SwiftInstance : public ScriptInstance {
	SwiftScript *script = nullptr;
	Object *owner = nullptr;
	SwiftGodotInstanceHandle handle = nullptr;

	void _call_virtual(const StringName &p_method, const Variant **p_args, int p_argcount);

public:
	virtual bool set(const StringName &p_name, const Variant &p_value) override;
	virtual bool get(const StringName &p_name, Variant &r_ret) const override;
	virtual void get_property_list(List<PropertyInfo> *r_properties) const override;
	virtual Variant::Type get_property_type(const StringName &p_name, bool *r_is_valid = nullptr) const override;
	virtual void validate_property(PropertyInfo &p_property) const override {}
	virtual bool property_can_revert(const StringName &p_name) const override { return false; }
	virtual bool property_get_revert(const StringName &p_name, Variant &r_ret) const override { return false; }
	virtual Object *get_owner() override { return owner; }
	virtual void get_method_list(List<MethodInfo> *r_list) const override;
	virtual bool has_method(const StringName &p_method) const override;
	virtual Variant callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) override;
	virtual void notification(int p_notification, bool p_reversed = false) override;
	virtual Ref<Script> get_script() const override;
	virtual ScriptLanguage *get_language() override;

	SwiftInstance(SwiftScript *p_script, Object *p_owner, SwiftGodotInstanceHandle p_handle);
	~SwiftInstance();
};
