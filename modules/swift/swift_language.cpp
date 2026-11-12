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

#ifdef TOOLS_ENABLED
#include "swift_editor_language.h"
#endif // TOOLS_ENABLED

#include "core/io/file_access.h"
#include "core/os/memory.h"
#include "core/os/os.h"
#include "core/string/regex.h"

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
	if (SwiftEditorLanguage::get_singleton() != nullptr) {
		return SwiftEditorLanguage::get_singleton();
	}
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

String SwiftLanguage::debug_get_error() const {
	return last_error;
}

int SwiftLanguage::debug_get_stack_level_count() const {
	return call_stack.size();
}

String SwiftLanguage::debug_get_stack_level_function(int p_level) const {
	if (p_level < 0 || p_level >= call_stack.size()) {
		return String();
	}
	return call_stack[p_level].func;
}

String SwiftLanguage::debug_get_stack_level_source(int p_level) const {
	if (p_level < 0 || p_level >= call_stack.size()) {
		return String();
	}
	return call_stack[p_level].file;
}

void SwiftLanguage::_call_enter(const StringName &p_method, const String &p_file) {
	CallFrame frame;
	frame.func = p_method;
	frame.file = p_file;
	frame.start_usec = OS::get_singleton()->get_ticks_usec();
	call_stack.push_back(frame);
}

void SwiftLanguage::_call_exit(const StringName &p_method) {
	if (call_stack.is_empty()) {
		return;
	}
	CallFrame frame = call_stack[call_stack.size() - 1];
	call_stack.remove_at(call_stack.size() - 1);
	if (frame.func != p_method) {
		return;
	}
	if (profiling_enabled) {
		uint64_t elapsed = OS::get_singleton()->get_ticks_usec() - frame.start_usec;
		prof_call_count[p_method] = (prof_call_count.has(p_method) ? prof_call_count[p_method] : 0) + 1;
		prof_total_usec[p_method] = (prof_total_usec.has(p_method) ? prof_total_usec[p_method] : 0) + elapsed;
	}
}

void SwiftLanguage::profiling_start() {
	profiling_enabled = true;
	prof_call_count.clear();
	prof_total_usec.clear();
}

void SwiftLanguage::profiling_stop() {
	profiling_enabled = false;
}

int SwiftLanguage::profiling_get_accumulated_data(ProfilingInfo *r_info_arr, int p_info_max) {
	int i = 0;
	for (const KeyValue<StringName, uint64_t> &E : prof_total_usec) {
		if (i >= p_info_max) {
			break;
		}
		r_info_arr[i].signature = E.key;
		r_info_arr[i].call_count = prof_call_count.has(E.key) ? prof_call_count[E.key] : 0;
		r_info_arr[i].total_time = E.value;
		r_info_arr[i].self_time = E.value;
		r_info_arr[i].internal_time = 0;
		i++;
	}
	return i;
}

void SwiftLanguage::get_public_constants(List<Pair<String, Variant>> *r_constants) const {
	for (const KeyValue<StringName, Variant> &E : global_constants) {
		r_constants->push_back(Pair<String, Variant>(E.key, E.value));
	}
}

String SwiftLanguage::get_global_class_name(const String &p_path, String *r_base_type, String *r_icon_path, bool *r_is_abstract, bool *r_is_tool) const {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	if (file.is_null()) {
		return String();
	}
	String source = file->get_as_utf8_string();

	Ref<RegEx> class_regex;
	class_regex.instantiate();
	class_regex->compile("\\bclass\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*(?::\\s*([A-Za-z_][A-Za-z0-9_.]*))?");
	Ref<RegExMatch> match = class_regex->search(source);
	if (match.is_null()) {
		return String();
	}
	if (r_base_type != nullptr) {
		*r_base_type = match->get_string(2);
	}
	if (r_icon_path != nullptr) {
		*r_icon_path = String();
	}
	if (r_is_abstract != nullptr) {
		*r_is_abstract = false;
	}
	if (r_is_tool != nullptr) {
		*r_is_tool = source.contains("@tool");
	}
	return match->get_string(1);
}
