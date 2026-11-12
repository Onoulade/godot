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

#include "core/string/regex.h"

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

#endif // TOOLS_ENABLED
