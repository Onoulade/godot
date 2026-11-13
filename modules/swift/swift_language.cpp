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

#include "core/io/file_access.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "core/os/os.h"
#include "core/variant/variant.h"

#include "modules/regex/regex.h"

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
}

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

void SwiftLanguage::reload_scripts(const Array &p_scripts, bool p_soft_reload) {
	for (int i = 0; i < p_scripts.size(); i++) {
		Ref<Script> script = p_scripts[i];
		if (script.is_valid()) {
			script->reload();
		}
	}
}

void SwiftLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
	if (p_script.is_valid() && p_script->is_valid()) {
		p_script->reload();
	}
}

void SwiftLanguage::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("swift");
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

static Ref<RegEx> _swift_func_regex() {
	static Ref<RegEx> func_regex;
	if (func_regex.is_null()) {
		func_regex.instantiate();
		func_regex->compile("\\bfunc\\s+([A-Za-z_][A-Za-z0-9_]*)");
	}
	return func_regex;
}

bool SwiftLanguage::validate(const String &p_script, const String &p_path, List<String> *r_functions, List<ScriptError> *r_errors, List<Warning> *r_warnings, HashSet<int> *r_safe_lines) const {
	PackedStringArray lines = p_script.split("\n");
	int depth = 0;
	bool in_block_comment = false;
	for (int i = 0; i < lines.size(); i++) {
		const String &line = lines[i];
		for (int c = 0; c < line.length(); c++) {
			char32_t ch = line[c];
			if (in_block_comment) {
				if (ch == '*' && c + 1 < line.length() && line[c + 1] == '/') {
					in_block_comment = false;
					c++;
				}
				continue;
			}
			if (ch == '"') {
				// Skip the string literal.
				c++;
				while (c < line.length() && line[c] != '"') {
					if (line[c] == '\\') {
						c++;
					}
					c++;
				}
				continue;
			}
			if (ch == '/' && c + 1 < line.length() && line[c + 1] == '/') {
				break;
			}
			if (ch == '/' && c + 1 < line.length() && line[c + 1] == '*') {
				in_block_comment = true;
				c++;
				continue;
			}
			if (ch == '{') {
				depth++;
			} else if (ch == '}') {
				depth--;
			}
		}
		if (r_safe_lines != nullptr) {
			r_safe_lines->insert(i + 1);
		}
		Ref<RegExMatch> match = _swift_func_regex()->search(line);
		if (match.is_valid() && r_functions != nullptr) {
			r_functions->push_back(match->get_string(1));
		}
	}
	if (depth != 0 && r_errors != nullptr) {
		ScriptError error;
		error.path = p_path;
		error.line = lines.size();
		error.message = vformat("Unbalanced braces (depth %d at end of file).", depth);
		r_errors->push_back(error);
	}
	return true;
}

int SwiftLanguage::find_function(const String &p_function, const String &p_code) const {
	PackedStringArray lines = p_code.split("\n");
	Ref<RegEx> regex;
	regex.instantiate();
	regex->compile(vformat("\\bfunc\\s+%s\\s*\\(", p_function));
	for (int i = 0; i < lines.size(); i++) {
		if (regex->search(lines[i]).is_valid()) {
			return i + 1;
		}
	}
	return -1;
}

Error SwiftLanguage::complete_code(const String &p_code, const String &p_path, Object *p_owner, List<CodeCompletionOption> *r_options, bool &r_force, String &r_call_hint) {
	Ref<RegEx> partial_regex;
	partial_regex.instantiate();
	partial_regex->compile("[A-Za-z_][A-Za-z0-9_]*$");
	Ref<RegExMatch> partial_match = partial_regex->search(p_code);
	String prefix = partial_match.is_valid() ? partial_match->get_string(0) : String();
	r_force = false;

	for (const String &word : get_reserved_words()) {
		if (word.begins_with(prefix)) {
			r_options->push_back(CodeCompletionOption(word, CODE_COMPLETION_KIND_KEYWORD, LOCATION_LOCAL, "keyword_color"));
		}
	}
	for (int t = 1; t < Variant::Type::VARIANT_MAX; t++) {
		String type_name = Variant::get_type_name((Variant::Type)t);
		if (type_name.begins_with(prefix) && type_name != prefix) {
			r_options->push_back(CodeCompletionOption(type_name, CODE_COMPLETION_KIND_PLAIN_TEXT, LOCATION_LOCAL, "base_type_color"));
		}
	}
	LocalVector<StringName> global_classes;
	ScriptServer::get_global_class_list(global_classes);
	for (const StringName &class_name : global_classes) {
		if (String(class_name).begins_with(prefix)) {
			r_options->push_back(CodeCompletionOption(class_name, CODE_COMPLETION_KIND_PLAIN_TEXT, LOCATION_OTHER_USER_CODE, "user_type_color"));
		}
	}
	if (p_owner != nullptr) {
		StringName owner_class = p_owner->get_class_name();
		List<MethodInfo> methods;
		ClassDB::get_method_list(owner_class, &methods);
		for (const MethodInfo &mi : methods) {
			if (String(mi.name).begins_with(prefix) && mi.name != prefix) {
				r_options->push_back(CodeCompletionOption(mi.name, CODE_COMPLETION_KIND_PLAIN_TEXT, LOCATION_PARENT_MASK | LOCATION_LOCAL, "member_method_color"));
			}
		}
		List<PropertyInfo> properties;
		ClassDB::get_property_list(owner_class, &properties);
		for (const PropertyInfo &pi : properties) {
			if ((pi.usage & PROPERTY_USAGE_EDITOR) && String(pi.name).begins_with(prefix) && pi.name != prefix) {
				r_options->push_back(CodeCompletionOption(pi.name, CODE_COMPLETION_KIND_PLAIN_TEXT, LOCATION_PARENT_MASK | LOCATION_LOCAL, "member_variable_color"));
			}
		}
	}
	return OK;
}

Error SwiftLanguage::lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) {
	if (ScriptServer::is_global_class(p_symbol)) {
		r_result.type = LOOKUP_RESULT_CLASS;
		r_result.class_name = p_symbol;
		r_result.script_path = ScriptServer::get_global_class_path(p_symbol);
		return OK;
	}
	PackedStringArray lines = p_code.split("\n");
	for (int i = 0; i < lines.size(); i++) {
		if (lines[i].find(p_symbol) != -1) {
			r_result.type = LOOKUP_RESULT_SCRIPT_LOCATION;
			r_result.script_path = p_path;
			r_result.location = i + 1;
			return OK;
		}
	}
	return ERR_DOES_NOT_EXIST;
}
