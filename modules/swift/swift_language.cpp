/**************************************************************************/
/*  swift_language.cpp                                                    */
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

#include "swift_language.h"

#include "swift_bridge.h"
#include "swift_script.h"

#include "core/os/memory.h"

void SwiftLanguage::_bind_methods() {
}

void SwiftLanguage::init() {
	Error err = SwiftBridge::get_singleton()->load();
	if (err != OK) {
		WARN_PRINT(vformat("Swift language: the Swift runtime library ('%s') was not found. Swift scripts will load, but cannot be instantiated until the runtime is available (see modules/swift/README.md).", SwiftBridge::get_runtime_library_name()));
	}
}

void SwiftLanguage::finish() {
	SwiftBridge::get_singleton()->unload();
#ifdef TOOLS_ENABLED
	if (editor_language != nullptr) {
		memdelete(editor_language);
		editor_language = nullptr;
	}
#endif // TOOLS_ENABLED
}

#ifdef TOOLS_ENABLED
EditorLanguage *SwiftLanguage::get_editor_language() {
	if (editor_language == nullptr) {
		editor_language = memnew(EditorLanguage);
	}
	return editor_language;
}
#endif // TOOLS_ENABLED

Vector<String> SwiftLanguage::get_reserved_words() const {
	static const Vector<String> words = {
		"associatedtype",
		"class",
		"deinit",
		"enum",
		"extension",
		"fileprivate",
		"func",
		"import",
		"init",
		"inout",
		"internal",
		"let",
		"open",
		"operator",
		"private",
		"precedencegroup",
		"protocol",
		"public",
		"rethrows",
		"static",
		"struct",
		"subscript",
		"typealias",
		"var",
		"break",
		"case",
		"catch",
		"continue",
		"default",
		"defer",
		"do",
		"else",
		"fallthrough",
		"for",
		"guard",
		"if",
		"in",
		"repeat",
		"return",
		"throw",
		"switch",
		"where",
		"while",
		"as",
		"any",
		"false",
		"is",
		"nil",
		"self",
		"super",
		"throws",
		"true",
		"try",
	};
	return words;
}

bool SwiftLanguage::is_control_flow_keyword(const String &p_string) const {
	static const HashSet<String> keywords = {
		"break",
		"case",
		"catch",
		"continue",
		"default",
		"defer",
		"do",
		"else",
		"fallthrough",
		"for",
		"guard",
		"if",
		"in",
		"repeat",
		"return",
		"switch",
		"throw",
		"where",
		"while",
	};
	return keywords.has(p_string);
}

Vector<String> SwiftLanguage::get_comment_delimiters() const {
	return { "//", "/* */" };
}

Vector<String> SwiftLanguage::get_doc_comment_delimiters() const {
	return { "///", "/** */" };
}

Vector<String> SwiftLanguage::get_string_delimiters() const {
	return { "\"" };
}

Ref<Script> SwiftLanguage::make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const {
	if (p_template == "Default") {
		Ref<SwiftScript> script;
		script.instantiate();
		script->set_source_code(vformat("class %s: %s {\n\tfunc _ready() {\n\t\t\n\t}\n}\n", p_class_name, p_base_class_name));
		return script;
	}
	return Ref<Script>();
}

Vector<ScriptLanguage::ScriptTemplate> SwiftLanguage::get_built_in_templates(const StringName &p_object) {
	Vector<ScriptTemplate> templates;
	ScriptTemplate t;
	t.inherit = p_object;
	t.name = "Default";
	t.description = vformat("Default Swift script inheriting %s.", p_object);
	t.content = vformat("class NewScript: %s {\n\tfunc _ready() {\n\t\t\n\t}\n}\n", p_object);
	templates.push_back(t);
	return templates;
}

String SwiftLanguage::make_function(const String &p_class, const String &p_name, const PackedStringArray &p_args) const {
	String args;
	for (int i = 0; i < p_args.size(); i++) {
		if (i > 0) {
			args += ", ";
		}
		args += p_args[i];
	}
	return vformat("func %s(%s) {\n\t\n}\n", p_name, args);
}

void SwiftLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {
	global_constants[p_variable] = p_value;
}

void SwiftLanguage::reload_all_scripts() {
	Vector<SwiftScript *> to_reload;
	for (SwiftScript *script : scripts) {
		to_reload.push_back(script);
	}
	for (SwiftScript *script : to_reload) {
		script->reload(true);
	}
}

void SwiftLanguage::reload_scripts(const Array &p_scripts) {
	for (int i = 0; i < p_scripts.size(); i++) {
		Ref<Script> script = p_scripts[i];
		if (script.is_valid()) {
			script->reload();
		}
	}
}

void SwiftLanguage::reload_tool_script(const Ref<Script> &p_script) {
	if (p_script.is_valid() && p_script->is_tool()) {
		p_script->reload();
	}
}

void SwiftLanguage::get_public_constants(List<Pair<String, Variant>> *r_constants) const {
	for (const KeyValue<StringName, Variant> &E : global_constants) {
		r_constants->push_back(Pair<String, Variant>(E.key, E.value));
	}
}
