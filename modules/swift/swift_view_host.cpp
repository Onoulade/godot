/**************************************************************************/
/*  swift_view_host.cpp                                                   */
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

#include "swift_view_host.h"

#include "swift_bridge.h"

#include "core/object/class_db.h"

void SwiftViewHost::_bind_methods() {
	ClassDB::bind_method(D_METHOD("invalidate"), &SwiftViewHost::invalidate);
	ClassDB::bind_method(D_METHOD("set_root_view", "name"), &SwiftViewHost::set_root_view);
	ClassDB::bind_method(D_METHOD("get_root_view"), &SwiftViewHost::get_root_view);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "root_view", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_root_view", "get_root_view");
}

void SwiftViewHost::_notification(int p_what) {
	if (p_what == Node::NOTIFICATION_READY && !scene_built) {
		const SwiftBridge *bridge = SwiftBridge::get_singleton();
		if (bridge->is_loaded() && bridge->get_abi().scene_build != nullptr) {
			GDExtensionObjectPtr root = bridge->get_abi().scene_build((GDExtensionObjectPtr)this);
			Node *root_node = Object::cast_to<Node>((Object *)root);
			if (root_node != nullptr) {
				add_child(root_node);
				scene_built = true;
			}
		}
	}
}

void SwiftViewHost::invalidate() {
	const SwiftBridge *bridge = SwiftBridge::get_singleton();
	if (bridge->is_loaded() && bridge->get_abi().scene_invalidate != nullptr) {
		bridge->get_abi().scene_invalidate((GDExtensionObjectPtr)this);
	}
}
