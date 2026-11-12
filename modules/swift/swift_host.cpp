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
#include "core/math/aabb.h"
#include "core/math/basis.h"
#include "core/math/color.h"
#include "core/math/plane.h"
#include "core/math/projection.h"
#include "core/math/quaternion.h"
#include "core/math/rect2.h"
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/vector2i.h"
#include "core/math/vector3.h"
#include "core/math/vector3i.h"
#include "core/math/vector4.h"
#include "core/object/class_db.h"
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

static void _host_variant_as_vector2(GDExtensionConstVariantPtr p_variant, double *r_out) {
	Vector2 v = ((const Variant *)p_variant)->operator Vector2();
	r_out[0] = v.x;
	r_out[1] = v.y;
}

static void _host_variant_set_vector2(GDExtensionVariantPtr p_variant, double p_x, double p_y) {
	*(Variant *)p_variant = Variant(Vector2(p_x, p_y));
}

static void _host_variant_as_vector3(GDExtensionConstVariantPtr p_variant, double *r_out) {
	Vector3 v = ((const Variant *)p_variant)->operator Vector3();
	r_out[0] = v.x;
	r_out[1] = v.y;
	r_out[2] = v.z;
}

static void _host_variant_set_vector3(GDExtensionVariantPtr p_variant, double p_x, double p_y, double p_z) {
	*(Variant *)p_variant = Variant(Vector3(p_x, p_y, p_z));
}

static void _host_variant_as_vector2i(GDExtensionConstVariantPtr p_variant, int64_t *r_out) {
	Vector2i v = ((const Variant *)p_variant)->operator Vector2i();
	r_out[0] = v.x;
	r_out[1] = v.y;
}

static void _host_variant_set_vector2i(GDExtensionVariantPtr p_variant, int64_t p_x, int64_t p_y) {
	*(Variant *)p_variant = Variant(Vector2i(p_x, p_y));
}

static void _host_variant_as_vector3i(GDExtensionConstVariantPtr p_variant, int64_t *r_out) {
	Vector3i v = ((const Variant *)p_variant)->operator Vector3i();
	r_out[0] = v.x;
	r_out[1] = v.y;
	r_out[2] = v.z;
}

static void _host_variant_set_vector3i(GDExtensionVariantPtr p_variant, int64_t p_x, int64_t p_y, int64_t p_z) {
	*(Variant *)p_variant = Variant(Vector3i(p_x, p_y, p_z));
}

static void _host_variant_as_color(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Color c = ((const Variant *)p_variant)->operator Color();
	r_out[0] = c.r;
	r_out[1] = c.g;
	r_out[2] = c.b;
	r_out[3] = c.a;
}

static void _host_variant_set_color(GDExtensionVariantPtr p_variant, float p_r, float p_g, float p_b, float p_a) {
	*(Variant *)p_variant = Variant(Color(p_r, p_g, p_b, p_a));
}

static void _host_variant_as_quaternion(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Quaternion q = ((const Variant *)p_variant)->operator Quaternion();
	r_out[0] = q.x;
	r_out[1] = q.y;
	r_out[2] = q.z;
	r_out[3] = q.w;
}

static void _host_variant_set_quaternion(GDExtensionVariantPtr p_variant, float p_x, float p_y, float p_z, float p_w) {
	*(Variant *)p_variant = Variant(Quaternion(p_x, p_y, p_z, p_w));
}

static void _host_variant_as_rect2(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Rect2 r = ((const Variant *)p_variant)->operator Rect2();
	r_out[0] = r.position.x;
	r_out[1] = r.position.y;
	r_out[2] = r.size.x;
	r_out[3] = r.size.y;
}

static void _host_variant_set_rect2(GDExtensionVariantPtr p_variant, float p_px, float p_py, float p_sx, float p_sy) {
	*(Variant *)p_variant = Variant(Rect2(Vector2(p_px, p_py), Vector2(p_sx, p_sy)));
}

static void _host_variant_as_aabb(GDExtensionConstVariantPtr p_variant, float *r_out) {
	::AABB b = ((const Variant *)p_variant)->operator ::AABB();
	r_out[0] = b.position.x;
	r_out[1] = b.position.y;
	r_out[2] = b.position.z;
	r_out[3] = b.size.x;
	r_out[4] = b.size.y;
	r_out[5] = b.size.z;
}

static void _host_variant_set_aabb(GDExtensionVariantPtr p_variant, const float *p_in) {
	*(Variant *)p_variant = Variant(AABB(Vector3(p_in[0], p_in[1], p_in[2]), Vector3(p_in[3], p_in[4], p_in[5])));
}

static void _host_variant_as_plane(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Plane p = ((const Variant *)p_variant)->operator Plane();
	r_out[0] = p.normal.x;
	r_out[1] = p.normal.y;
	r_out[2] = p.normal.z;
	r_out[3] = p.d;
}

static void _host_variant_set_plane(GDExtensionVariantPtr p_variant, float p_a, float p_b, float p_c, float p_d) {
	*(Variant *)p_variant = Variant(Plane(Vector3(p_a, p_b, p_c), p_d));
}

static void _host_variant_as_transform2d(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Transform2D t = ((const Variant *)p_variant)->operator Transform2D();
	for (int i = 0; i < 3; i++) {
		r_out[i * 2 + 0] = t.columns[i].x;
		r_out[i * 2 + 1] = t.columns[i].y;
	}
}

static void _host_variant_set_transform2d(GDExtensionVariantPtr p_variant, const float *p_in) {
	Transform2D t;
	for (int i = 0; i < 3; i++) {
		t.columns[i] = Vector2(p_in[i * 2 + 0], p_in[i * 2 + 1]);
	}
	*(Variant *)p_variant = Variant(t);
}

static void _host_variant_as_transform3d(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Transform3D t = ((const Variant *)p_variant)->operator Transform3D();
	for (int i = 0; i < 3; i++) {
		r_out[i * 3 + 0] = t.basis.rows[i].x;
		r_out[i * 3 + 1] = t.basis.rows[i].y;
		r_out[i * 3 + 2] = t.basis.rows[i].z;
	}
	r_out[9] = t.origin.x;
	r_out[10] = t.origin.y;
	r_out[11] = t.origin.z;
}

static void _host_variant_set_transform3d(GDExtensionVariantPtr p_variant, const float *p_in) {
	Transform3D t;
	for (int i = 0; i < 3; i++) {
		t.basis.rows[i] = Vector3(p_in[i * 3 + 0], p_in[i * 3 + 1], p_in[i * 3 + 2]);
	}
	t.origin = Vector3(p_in[9], p_in[10], p_in[11]);
	*(Variant *)p_variant = Variant(t);
}

static void _host_variant_as_basis(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Basis b = ((const Variant *)p_variant)->operator Basis();
	for (int i = 0; i < 3; i++) {
		r_out[i * 3 + 0] = b.rows[i].x;
		r_out[i * 3 + 1] = b.rows[i].y;
		r_out[i * 3 + 2] = b.rows[i].z;
	}
}

static void _host_variant_set_basis(GDExtensionVariantPtr p_variant, const float *p_in) {
	Basis b;
	for (int i = 0; i < 3; i++) {
		b.rows[i] = Vector3(p_in[i * 3 + 0], p_in[i * 3 + 1], p_in[i * 3 + 2]);
	}
	*(Variant *)p_variant = Variant(b);
}

static void _host_variant_as_projection(GDExtensionConstVariantPtr p_variant, float *r_out) {
	Projection p = ((const Variant *)p_variant)->operator Projection();
	for (int i = 0; i < 4; i++) {
		r_out[i * 4 + 0] = p.columns[i].x;
		r_out[i * 4 + 1] = p.columns[i].y;
		r_out[i * 4 + 2] = p.columns[i].z;
		r_out[i * 4 + 3] = p.columns[i].w;
	}
}

static void _host_variant_set_projection(GDExtensionVariantPtr p_variant, const float *p_in) {
	Projection p;
	for (int i = 0; i < 4; i++) {
		p.columns[i] = Vector4(p_in[i * 4 + 0], p_in[i * 4 + 1], p_in[i * 4 + 2], p_in[i * 4 + 3]);
	}
	*(Variant *)p_variant = Variant(p);
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

static GDExtensionObjectPtr _host_object_create(const char *p_class_name) {
	StringName class_name = StringName(String::utf8(p_class_name));
	if (!ClassDB::can_instantiate(class_name)) {
		ERR_PRINT(String("SwiftHost: cannot instantiate class ") + String::utf8(p_class_name));
		return nullptr;
	}
	return (GDExtensionObjectPtr)ClassDB::instantiate(class_name);
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
	_host_variant_as_vector2,
	_host_variant_set_vector2,
	_host_variant_as_vector3,
	_host_variant_set_vector3,
	_host_variant_as_vector2i,
	_host_variant_set_vector2i,
	_host_variant_as_vector3i,
	_host_variant_set_vector3i,
	_host_variant_as_color,
	_host_variant_set_color,
	_host_variant_as_quaternion,
	_host_variant_set_quaternion,
	_host_variant_as_rect2,
	_host_variant_set_rect2,
	_host_variant_as_aabb,
	_host_variant_set_aabb,
	_host_variant_as_plane,
	_host_variant_set_plane,
	_host_variant_as_transform2d,
	_host_variant_set_transform2d,
	_host_variant_as_transform3d,
	_host_variant_set_transform3d,
	_host_variant_as_basis,
	_host_variant_set_basis,
	_host_variant_as_projection,
	_host_variant_set_projection,
	_host_object_call,
	_host_object_get,
	_host_object_set,
	_host_node_get_node,
	_host_node_get_parent,
	_host_print,
	_host_push_error,
	_host_object_create,
};

const SwiftGodotHost *SwiftHost::get_host() {
	return &_host;
}
