/**************************************************************************/
/*  swift_language.h                                                      */
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

#include "core/object/script_language.h"

class SwiftScript;

class SwiftLanguage : public ScriptLanguage {
	GDCLASS(SwiftLanguage, ScriptLanguage);

	friend class SwiftScript;
	friend class SwiftInstance;

	HashMap<StringName, Variant> global_constants;
	HashSet<SwiftScript *> scripts;

	struct CallFrame {
		StringName func;
		String file;
		uint64_t start_usec = 0;
	};
	Vector<CallFrame> call_stack;
	bool profiling_enabled = false;
	HashMap<StringName, uint64_t> prof_call_count;
	HashMap<StringName, uint64_t> prof_total_usec;
	String last_error;

	void _call_enter(const StringName &p_method, const String &p_file);
	void _call_exit(const StringName &p_method);

protected:
	static void _bind_methods();

public:
	virtual String get_name() const override { return "Swift"; }
	virtual void init() override;
	virtual String get_type() const override { return "SwiftScript"; }
	virtual String get_extension() const override { return "swift"; }
	virtual void finish() override;

	// Editor-side support lives directly on ScriptLanguage in 4.7 (there is
	// no separate EditorLanguage class): validate/find_function/
	// auto_indent_code are pure virtual there.
	virtual bool validate(const String &p_script, const String &p_path, List<String> *r_functions, List<ScriptError> *r_errors, List<Warning> *r_warnings, HashSet<int> *r_safe_lines) const override;
	virtual int find_function(const String &p_function, const String &p_code) const override;
	virtual void auto_indent_code(String &p_code, int p_from_line, int p_to_line) const override {}
	virtual Error complete_code(const String &p_code, const String &p_path, Object *p_owner, List<CodeCompletionOption> *r_options, bool &r_force, String &r_call_hint) override;
	virtual Error lookup_code(const String &p_code, const String &p_symbol, const String &p_path, Object *p_owner, LookupResult &r_result) override;

	virtual Vector<String> get_reserved_words() const override;
	virtual bool is_control_flow_keyword(const String &p_string) const override;
	virtual Vector<String> get_comment_delimiters() const override;
	virtual Vector<String> get_doc_comment_delimiters() const override;
	virtual Vector<String> get_string_delimiters() const override;
	virtual Ref<Script> make_template(const String &p_template, const String &p_class_name, const String &p_base_class_name) const override;
	virtual Vector<ScriptTemplate> get_built_in_templates(const StringName &p_object) override;
	virtual bool supports_builtin_mode() const override { return false; }
	virtual String make_function(const String &p_class, const String &p_name, const PackedStringArray &p_args) const override;
	virtual ScriptNameCasing preferred_file_name_casing() const override { return SCRIPT_NAME_CASING_PASCAL_CASE; }
	virtual void add_global_constant(const StringName &p_variable, const Variant &p_value) override;

	virtual String debug_get_error() const override;
	virtual int debug_get_stack_level_count() const override;
	virtual int debug_get_stack_level_line(int p_level) const override { return -1; }
	virtual String debug_get_stack_level_function(int p_level) const override;
	virtual String debug_get_stack_level_source(int p_level) const override;
	virtual void debug_get_stack_level_locals(int p_level, List<String> *r_locals, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	virtual void debug_get_stack_level_members(int p_level, List<String> *r_members, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	virtual void debug_get_globals(List<String> *r_globals, List<Variant> *r_values, int p_max_subitems = -1, int p_max_depth = -1) override {}
	virtual String debug_parse_stack_level_expression(int p_level, const String &p_expression, int p_max_subitems = -1, int p_max_depth = -1) override { return String(); }

	virtual void reload_all_scripts() override;
	virtual void reload_scripts(const Array &p_scripts, bool p_soft_reload) override;
	virtual void reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;

	virtual void get_public_functions(List<MethodInfo> *r_functions) const override {}
	virtual void get_public_constants(List<Pair<String, Variant>> *r_constants) const override;
	virtual void get_public_annotations(List<MethodInfo> *r_annotations) const override {}

	virtual void profiling_start() override;
	virtual void profiling_stop() override;
	virtual void profiling_set_save_native_calls(bool p_enable) override {}
	virtual int profiling_get_accumulated_data(ProfilingInfo *r_info_arr, int p_info_max) override;
	virtual int profiling_get_frame_data(ProfilingInfo *r_info_arr, int p_info_max) override { return 0; }

	virtual bool handles_global_class_type(const String &p_type) const override { return p_type == "swift"; }
	virtual String get_global_class_name(const String &p_path, String *r_base_type = nullptr, String *r_icon_path = nullptr, bool *r_is_abstract = nullptr, bool *r_is_tool = nullptr) const override;

	SwiftLanguage() {}
	~SwiftLanguage() {}
};

extern SwiftLanguage *script_language_swift;
