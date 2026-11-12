/**************************************************************************/
/*  swift_editor_language.cpp                                             */
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

#include "swift_editor_language.h"

#ifdef TOOLS_ENABLED

#include "core/object/class_db.h"
#include "core/string/regex.h"
#include "core/variant/variant.h"

SwiftEditorLanguage *SwiftEditorLanguage::singleton = nullptr;

static Ref<RegEx> _get_func_regex() {
	static Ref<RegEx> func_regex;
	if (func_regex.is_null()) {
		func_regex.instantiate();
		func_regex->compile("\\bfunc\\s+([A-Za-z_][A-Za-z0-9_]*)");
	}
	return func_regex;
}

bool SwiftEditorLanguage::validate(const String &p_code, const String &p_path, List<ScriptError> *r_errors, List<Warning> *r_warnings, List<String> *r_functions, HashSet<int> *r_safe_lines) const {
	PackedStringArray lines = p_code.split("\n");
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
		Ref<RegExMatch> match = _get_func_regex()->search(line);
		if (match.is_valid() && r_functions != nullptr) {
			r_functions->push_back(match->get_string(1));
		}
	}
	if (depth != 0 && r_errors != nullptr) {
		ScriptError error;
		error.path = p_path;
		error.start_line = lines.size();
		error.end_line = lines.size();
		error.message = vformat("Unbalanced braces (depth %d at end of file).", depth);
		r_errors->push_back(error);
	}
	return true;
}

int32_t SwiftEditorLanguage::find_function(const String &p_function, const String &p_code) const {
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

Error SwiftEditorLanguage::complete_code(const String &p_code, const String &p_path, Object *p_owner, List<CompletionOption> *r_options, bool &r_force, String &r_call_hint) {
	Ref<RegEx> partial_regex;
	partial_regex.instantiate();
	partial_regex->compile("[A-Za-z_][A-Za-z0-9_]*$");
	Ref<RegExMatch> partial_match = partial_regex->search(p_code);
	String prefix = partial_match.is_valid() ? partial_match->get_string(0) : String();
	r_force = false;

	ScriptLanguage *lang = ScriptServer::get_language_for_extension("swift");
	if (lang != nullptr) {
		for (const String &word : lang->get_reserved_words()) {
			if (word.begins_with(prefix)) {
				r_options->push_back(CompletionOption(word, CompletionKind::KEYWORD, CompletionLocation::LOCAL, "keyword_color"));
			}
		}
	}
	for (int t = 1; t < Variant::Type::VARIANT_MAX; t++) {
		String type_name = Variant::get_type_name((Variant::Type)t);
		if (type_name.begins_with(prefix) && type_name != prefix) {
			r_options->push_back(CompletionOption(type_name, CompletionKind::PLAIN_TEXT, CompletionLocation::LOCAL, "base_type_color"));
		}
	}
	LocalVector<StringName> global_classes;
	ScriptServer::get_global_class_list(global_classes);
	for (const StringName &class_name : global_classes) {
		if (String(class_name).begins_with(prefix)) {
			r_options->push_back(CompletionOption(class_name, CompletionKind::PLAIN_TEXT, CompletionLocation::OTHER_USER_CODE, "user_type_color"));
		}
	}
	if (p_owner != nullptr) {
		StringName owner_class = p_owner->get_class_name();
		List<MethodInfo> methods;
		ClassDB::get_method_list(owner_class, &methods);
		for (const MethodInfo &mi : methods) {
			if (String(mi.name).begins_with(prefix) && mi.name != prefix) {
				r_options->push_back(CompletionOption(mi.name, CompletionKind::PLAIN_TEXT, CompletionLocation::PARENT_MASK | CompletionLocation::LOCAL, "member_method_color"));
			}
		}
		List<PropertyInfo> properties;
		ClassDB::get_property_list(owner_class, &properties);
		for (const PropertyInfo &pi : properties) {
			if ((pi.usage & PROPERTY_USAGE_EDITOR) && String(pi.name).begins_with(prefix) && pi.name != prefix) {
				r_options->push_back(CompletionOption(pi.name, CompletionKind::PLAIN_TEXT, CompletionLocation::PARENT_MASK | CompletionLocation::LOCAL, "member_variable_color"));
			}
		}
	}
	return OK;
}

Error SwiftEditorLanguage::lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) {
	if (ScriptServer::is_global_class(p_symbol)) {
		r_result.type = LookupResult::Type::CLASS;
		r_result.class_name = p_symbol;
		r_result.script_path = ScriptServer::get_global_class_path(p_symbol);
		return OK;
	}
	PackedStringArray lines = p_code.split("\n");
	for (int i = 0; i < lines.size(); i++) {
		if (lines[i].find(p_symbol) != -1) {
			r_result.type = LookupResult::Type::SCRIPT_LOCATION;
			r_result.script_path = p_path;
			r_result.location = i + 1;
			return OK;
		}
	}
	return ERR_DOES_NOT_EXIST;
}

Error SwiftEditorLanguage::lookup_code_for_rename(const String &p_code, const String &p_symbol, const String &p_path, LookupResult &r_result) {
	return lookup_code(p_code, p_symbol, p_path, nullptr, r_result);
}

#endif // TOOLS_ENABLED
