/**************************************************************************/
/*  swift_host.cpp                                                        */
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

#include "swift_host.h"

#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "core/os/os.h"
#include "core/string/node_path.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"

static int _host_variant_get_type(GDExtensionConstVariantPtr p_variant) {
	return (int)((const Variant *)p_variant)->get_type();
}

static GDExtensionBool _host_variant_as_bool(GDExtensionConstVariantPtr p_variant) {
	return (GDExtensionBool)((const Variant *)p_variant)->operator bool();
}

static int64_t _host_variant_as_int(GDExtensionConstVariantPtr p_variant) {
	return (int64_t)((const Variant *)p_variant)->operator int64_t();
}

static double _host_variant_as_double(GDExtensionConstVariantPtr p_variant) {
	return (double)((const Variant *)p_variant)->operator double();
}

static void _host_variant_as_string(GDExtensionConstVariantPtr p_variant, char *r_buffer, int p_buffer_size) {
	if (r_buffer == nullptr || p_buffer_size <= 0) {
		return;
	}
	CharString str = ((const Variant *)p_variant)->operator String().utf8();
	int len = MIN(p_buffer_size - 1, (int)str.length());
	memcpy(r_buffer, str.get_data(), len);
	r_buffer[len] = '\0';
}

static GDExtensionVariantPtr _host_variant_new() {
	return new Variant();
}

static void _host_variant_free(GDExtensionVariantPtr p_variant) {
	delete (Variant *)p_variant;
}

static void _host_variant_set_nil(GDExtensionVariantPtr p_variant) {
	*(Variant *)p_variant = Variant();
}

static void _host_variant_set_bool(GDExtensionVariantPtr p_variant, GDExtensionBool p_value) {
	*(Variant *)p_variant = Variant(p_value != 0);
}

static void _host_variant_set_int(GDExtensionVariantPtr p_variant, int64_t p_value) {
	*(Variant *)p_variant = Variant(p_value);
}

static void _host_variant_set_double(GDExtensionVariantPtr p_variant, double p_value) {
	*(Variant *)p_variant = Variant(p_value);
}

static void _host_variant_set_string(GDExtensionVariantPtr p_variant, const char *p_value) {
	*(Variant *)p_variant = Variant(String::utf8(p_value));
}

static void _host_variant_copy(GDExtensionVariantPtr p_dst, GDExtensionConstVariantPtr p_src) {
	*(Variant *)p_dst = *(const Variant *)p_src;
}

static void _host_variant_set_object(GDExtensionVariantPtr p_variant, GDExtensionObjectPtr p_object) {
	*(Variant *)p_variant = Variant((Object *)p_object);
}

static GDExtensionObjectPtr _host_variant_get_object(GDExtensionConstVariantPtr p_variant) {
	return (GDExtensionObjectPtr)((const Variant *)p_variant)->operator Object *();
}

static GDExtensionBool _host_object_call(GDExtensionObjectPtr p_object, const char *p_method, GDExtensionConstVariantPtr *p_args, int p_arg_count, GDExtensionVariantPtr r_ret, GDExtensionCallError *r_error) {
	Object *object = (Object *)p_object;
	if (object == nullptr) {
		return 0;
	}
	Vector<const Variant *> arg_ptrs;
	for (int i = 0; i < p_arg_count; i++) {
		arg_ptrs.push_back((const Variant *)p_args[i]);
	}
	Callable::CallError error;
	Variant ret = object->callp(StringName(String::utf8(p_method)), p_arg_count > 0 ? arg_ptrs.ptrw() : nullptr, p_arg_count, error);
	if (r_ret != nullptr) {
		*(Variant *)r_ret = ret;
	}
	if (r_error != nullptr) {
		r_error->error = (GDExtensionCallErrorType)error.error;
		r_error->argument = error.argument;
		r_error->expected = error.expected;
	}
	return error.error == Callable::CallError::CALL_OK ? 1 : 0;
}

static GDExtensionBool _host_object_get(GDExtensionObjectPtr p_object, const char *p_name, GDExtensionVariantPtr r_ret) {
	Object *object = (Object *)p_object;
	if (object == nullptr || r_ret == nullptr) {
		return 0;
	}
	Variant value = object->get(StringName(String::utf8(p_name)));
	*(Variant *)r_ret = value;
	return 1;
}

static GDExtensionBool _host_object_set(GDExtensionObjectPtr p_object, const char *p_name, GDExtensionConstVariantPtr p_value) {
	Object *object = (Object *)p_object;
	if (object == nullptr) {
		return 0;
	}
	object->set(StringName(String::utf8(p_name)), *(const Variant *)p_value);
	return 1;
}

static GDExtensionObjectPtr _host_node_get_node(GDExtensionObjectPtr p_node, const char *p_path) {
	Node *node = Object::cast_to<Node>((Object *)p_node);
	if (node == nullptr) {
		return nullptr;
	}
	return node->get_node_or_null(NodePath(String::utf8(p_path)));
}

static GDExtensionObjectPtr _host_node_get_parent(GDExtensionObjectPtr p_node) {
	Node *node = Object::cast_to<Node>((Object *)p_node);
	if (node == nullptr) {
		return nullptr;
	}
	return node->get_parent();
}

static void _host_print(const char *p_message) {
	OS::get_singleton()->print("%s\n", p_message);
}

static void _host_push_error(const char *p_message) {
	ERR_PRINT(String::utf8(p_message));
}

static const SwiftGodotHost _host = {
	_host_variant_get_type,
	_host_variant_as_bool,
	_host_variant_as_int,
	_host_variant_as_double,
	_host_variant_as_string,
	_host_variant_new,
	_host_variant_free,
	_host_variant_set_nil,
	_host_variant_set_bool,
	_host_variant_set_int,
	_host_variant_set_double,
	_host_variant_set_string,
	_host_variant_copy,
	_host_variant_set_object,
	_host_variant_get_object,
	_host_object_call,
	_host_object_get,
	_host_object_set,
	_host_node_get_node,
	_host_node_get_parent,
	_host_print,
	_host_push_error,
};

const SwiftGodotHost *SwiftHost::get_host() {
	return &_host;
}
