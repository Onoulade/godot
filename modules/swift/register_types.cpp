/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "swift_language.h"
#include "swift_resource_format.h"
#include "swift_script.h"

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/object/class_db.h"

#ifdef TOOLS_ENABLED
#include "swift_editor_language.h"

#include "editor/editor_node.h"
#include "editor/script/script_editor_plugin.h"
#include "editor/script/syntax_highlighters.h"

static Ref<EditorSyntaxHighlighter> swift_syntax_highlighter;

static void _editor_init() {
	Ref<EditorStandardSyntaxHighlighter> highlighter;
	highlighter.instantiate();
	highlighter->_set_script_language(script_language_swift);
	ScriptEditor::get_singleton()->register_syntax_highlighter(highlighter);
	swift_syntax_highlighter = highlighter;
}
#endif // TOOLS_ENABLED

SwiftLanguage *script_language_swift = nullptr;
static Ref<ResourceFormatLoaderSwift> resource_loader_swift;
static Ref<ResourceFormatSaverSwift> resource_saver_swift;

void initialize_swift_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		GDREGISTER_CLASS(SwiftScript);
		GDREGISTER_CLASS(SwiftLanguage);

		script_language_swift = memnew(SwiftLanguage);
		ScriptServer::register_language(script_language_swift);

		resource_loader_swift.instantiate();
		ResourceLoader::add_resource_format_loader(resource_loader_swift);

		resource_saver_swift.instantiate();
		ResourceSaver::add_resource_format_saver(resource_saver_swift);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		memnew(SwiftEditorLanguage);
		EditorNode::add_init_callback(_editor_init);
	}
#endif // TOOLS_ENABLED
}

void uninitialize_swift_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		ScriptServer::unregister_language(script_language_swift);

		ResourceLoader::remove_resource_format_loader(resource_loader_swift);
		resource_loader_swift.unref();

		ResourceSaver::remove_resource_format_saver(resource_saver_swift);
		resource_saver_swift.unref();

		memdelete(script_language_swift);
		script_language_swift = nullptr;
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		if (swift_syntax_highlighter.is_valid()) {
			ScriptEditor::get_singleton()->unregister_syntax_highlighter(swift_syntax_highlighter);
			swift_syntax_highlighter.unref();
		}
		if (SwiftEditorLanguage::get_singleton() != nullptr) {
			memdelete(SwiftEditorLanguage::get_singleton());
		}
	}
#endif // TOOLS_ENABLED
}
