/**************************************************************************/
/*  swift_editor_language.h                                               */
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

#ifdef TOOLS_ENABLED

#include "core/object/editor_language.h"

// Editor-side support for Swift scripts: lightweight validation, function
// lookup for the script outline, and (stub) completion/lookup.
class SwiftEditorLanguage : public EditorLanguage {
	static SwiftEditorLanguage *singleton;

public:
	static SwiftEditorLanguage *get_singleton() { return singleton; }

	virtual bool validate(const String &p_code, const String &p_path, List<ScriptError> *r_errors, List<Warning> *r_warnings, List<String> *r_functions, HashSet<int> *r_safe_lines) const override;
	virtual int32_t find_function(const String &p_function, const String &p_code) const override;
	virtual Error complete_code(const String &p_code, const String &p_path, Object *p_owner, List<CompletionOption> *r_options, bool &r_force, String &r_call_hint) override;
	virtual Error lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) override;
	virtual Error lookup_code_for_rename(const String &p_code, const String &p_symbol, const String &p_path, LookupResult &r_result) override;

	SwiftEditorLanguage() { singleton = this; }
	~SwiftEditorLanguage() {
		if (singleton == this) {
			singleton = nullptr;
		}
	}
};

#endif // TOOLS_ENABLED
