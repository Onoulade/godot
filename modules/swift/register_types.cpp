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

#include "swift_bridge.h"
#include "swift_language.h"
#include "swift_resource_format.h"
#include "swift_script.h"
#include "swift_view_host.h"

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/object/class_db.h"

#ifdef TOOLS_ENABLED
#include "swift_lsp.h"

#include "core/io/file_access.h"
#include "editor/editor_node.h"
#include "editor/export/editor_export.h"
#include "editor/script/script_editor_plugin.h"
#include "editor/script/syntax_highlighters.h"

static Ref<EditorSyntaxHighlighter> swift_syntax_highlighter;

class SwiftExportPlugin : public EditorExportPlugin {
	GDSOFTCLASS(SwiftExportPlugin, EditorExportPlugin);

protected:
	virtual void _export_begin(const HashSet<String> &p_features, bool p_debug, const String &p_path, int p_flags) override {
		// Ship the Swift runtime libraries as shared objects: next to the
		// exported executable on Linux/Windows, in Contents/Frameworks of the
		// .app on macOS — exactly where SwiftBridge searches at load time.
		String exe_dir = OS::get_singleton()->get_executable_path().get_base_dir();
		const String lib_names[] = { SwiftBridge::get_runtime_library_name(), SwiftBridge::get_ui_library_name() };
		for (const String &lib_name : lib_names) {
			String lib_path = exe_dir.path_join(lib_name);
			if (FileAccess::exists(lib_path)) {
				add_shared_object(lib_path, {}, "");
			} else {
				WARN_PRINT("Swift export: '" + lib_name + "' was not found next to the editor binary; Swift scripts will not run in the exported project.");
			}
		}
	}

	virtual void _export_file(const String &p_path, const String &p_type, const HashSet<String> &p_features) override {
		if (p_path.get_extension() != "swift") {
			return;
		}
		Vector<uint8_t> file = FileAccess::get_file_as_bytes(p_path);
		if (!file.is_empty()) {
			add_file(p_path, file, true);
		}
		// Bundle the precompiled library cached by the editor, so exported
		// games run Swift scripts without a Swift toolchain. Shipped as a
		// loose file (not packed in the .pck): it is dlopen'ed at runtime.
		String bin_path = p_path + ".swiftbin";
		if (FileAccess::exists(bin_path)) {
			Vector<uint8_t> bin = FileAccess::get_file_as_bytes(bin_path);
			if (!bin.is_empty()) {
				add_file(bin_path, bin, false);
			}
		}
	}

public:
	virtual String get_name() const override { return "Swift"; }
};

static Ref<SwiftExportPlugin> swift_export_plugin;

static void _editor_init() {
	Ref<EditorStandardSyntaxHighlighter> highlighter;
	highlighter.instantiate();
	highlighter->_set_script_language(script_language_swift);
	ScriptEditor::get_singleton()->register_syntax_highlighter(highlighter);
	swift_syntax_highlighter = highlighter;

	swift_export_plugin.instantiate();
	EditorExport::get_singleton()->add_export_plugin(swift_export_plugin);
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

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(SwiftViewHost);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
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
		if (swift_export_plugin.is_valid()) {
			EditorExport::get_singleton()->remove_export_plugin(swift_export_plugin);
			swift_export_plugin.unref();
		}
		SwiftLSPClient::get_singleton()->stop();
	}
#endif // TOOLS_ENABLED
}
