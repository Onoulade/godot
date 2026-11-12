/**************************************************************************/
/*  swift_view_host.h                                                     */
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

#include "scene/main/node.h"

// Node that hosts a declarative Swift scene (see modules/swift/runtime,
// target SwiftGodotUI). On ready, it asks the Swift runtime for the scene's
// root node (ABI: swiftgodot_scene_build) and parents it. invalidate()
// triggers a scoped re-render (ABI: swiftgodot_scene_invalidate).
// The "root_view" property names the Swift `struct X: View` to render
// (several hosts can host different scenes).
class SwiftViewHost : public Node {
	GDCLASS(SwiftViewHost, Node);

	String root_view;
	bool scene_built = false;

	void set_root_view(const String &p_name) { root_view = p_name; }
	String get_root_view() const { return root_view; }

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void invalidate();

	SwiftViewHost() {}
};
