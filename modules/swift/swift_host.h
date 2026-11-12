/**************************************************************************/
/*  swift_host.h                                                          */
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

// Host services exposed by the engine to the Swift runtime, so the Swift
// side can marshal Variants and call engine objects without going through
// the raw GDExtension function table. Implemented in swift_host.cpp with
// direct in-tree access; handed to the runtime via swiftgodot_host_init().
struct SwiftGodotHost {
	int (*variant_get_type)(GDExtensionConstVariantPtr p_variant) = nullptr;
	GDExtensionBool (*variant_as_bool)(GDExtensionConstVariantPtr p_variant) = nullptr;
	int64_t (*variant_as_int)(GDExtensionConstVariantPtr p_variant) = nullptr;
	double (*variant_as_double)(GDExtensionConstVariantPtr p_variant) = nullptr;
	void (*variant_as_string)(GDExtensionConstVariantPtr p_variant, char *r_buffer, int p_buffer_size) = nullptr;
	GDExtensionVariantPtr (*variant_new)() = nullptr;
	void (*variant_free)(GDExtensionVariantPtr p_variant) = nullptr;
	void (*variant_set_nil)(GDExtensionVariantPtr p_variant) = nullptr;
	void (*variant_set_bool)(GDExtensionVariantPtr p_variant, GDExtensionBool p_value) = nullptr;
	void (*variant_set_int)(GDExtensionVariantPtr p_variant, int64_t p_value) = nullptr;
	void (*variant_set_double)(GDExtensionVariantPtr p_variant, double p_value) = nullptr;
	void (*variant_set_string)(GDExtensionVariantPtr p_variant, const char *p_value) = nullptr;
	void (*variant_copy)(GDExtensionVariantPtr p_dst, GDExtensionConstVariantPtr p_src) = nullptr;
	void (*variant_set_object)(GDExtensionVariantPtr p_variant, GDExtensionObjectPtr p_object) = nullptr;
	GDExtensionObjectPtr (*variant_get_object)(GDExtensionConstVariantPtr p_variant) = nullptr;

	GDExtensionBool (*object_call)(GDExtensionObjectPtr p_object, const char *p_method, GDExtensionConstVariantPtr *p_args, int p_arg_count, GDExtensionVariantPtr r_ret, GDExtensionCallError *r_error) = nullptr;
	GDExtensionBool (*object_get)(GDExtensionObjectPtr p_object, const char *p_name, GDExtensionVariantPtr r_ret) = nullptr;
	GDExtensionBool (*object_set)(GDExtensionObjectPtr p_object, const char *p_name, GDExtensionConstVariantPtr p_value) = nullptr;
	GDExtensionObjectPtr (*node_get_node)(GDExtensionObjectPtr p_node, const char *p_path) = nullptr;
	GDExtensionObjectPtr (*node_get_parent)(GDExtensionObjectPtr p_node) = nullptr;

	void (*print)(const char *p_message) = nullptr;
	void (*push_error)(const char *p_message) = nullptr;
};

class SwiftHost {
public:
	static const SwiftGodotHost *get_host();
};
