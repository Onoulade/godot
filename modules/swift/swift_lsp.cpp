/**************************************************************************/
/*  swift_lsp.cpp                                                         */
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

#include "swift_lsp.h"

#ifdef TOOLS_ENABLED

#include "core/config/project_settings.h"
#include "core/io/json.h"
#include "core/os/memory.h"
#include "core/os/os.h"
#include "core/templates/hash_set.h"

SwiftLSPClient *SwiftLSPClient::singleton = nullptr;

SwiftLSPClient *SwiftLSPClient::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(SwiftLSPClient);
	}
	return singleton;
}

String SwiftLSPClient::_find_binary() {
	OS *os = OS::get_singleton();
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps->has_setting("swift/lsp_path")) {
		String custom = ps->get_setting("swift/lsp_path");
		if (!custom.is_empty() && FileAccess::exists(custom)) {
			return custom;
		}
	}
	String env = os->get_environment("SOURCEKIT_LSP");
	if (!env.is_empty() && FileAccess::exists(env)) {
		return env;
	}
	String exe = os->has_feature("windows") ? "sourcekit-lsp.exe" : "sourcekit-lsp";
	String path_env = os->get_environment("PATH");
	String sep = os->has_feature("windows") ? ";" : ":";
	for (const String &dir : path_env.split(sep)) {
		if (dir.is_empty()) {
			continue;
		}
		String candidate = dir.path_join(exe);
		if (FileAccess::exists(candidate)) {
			return candidate;
		}
	}
	return String();
}

String SwiftLSPClient::_uri_for_path(const String &p_path) {
	String path = p_path.replace("\\", "/");
	path = path.replace(" ", "%20").replace("#", "%23");
	return "file://" + path;
}

void SwiftLSPClient::_send(const Dictionary &p_message) {
	if (stdio.is_null()) {
		return;
	}
	Dictionary msg = p_message;
	msg["jsonrpc"] = "2.0";
	CharString body = JSON::stringify(msg).utf8();
	CharString header = ("Content-Length: " + itos(body.length()) + "\r\n\r\n").utf8();
	stdio->store_buffer((const uint8_t *)header.get_data(), header.length());
	stdio->store_buffer((const uint8_t *)body.get_data(), body.length());
}

bool SwiftLSPClient::_read_message(Dictionary &r_message, int p_timeout_ms) {
	if (stdio.is_null()) {
		return false;
	}
	OS *os = OS::get_singleton();
	uint64_t deadline = os->get_ticks_msec() + p_timeout_ms;
	while (os->get_ticks_msec() < deadline) {
		uint8_t buf[8192];
		uint64_t n = stdio->get_buffer(buf, sizeof(buf));
		if (n > 0) {
			int old_size = read_buffer.size();
			read_buffer.resize(old_size + n);
			memcpy(read_buffer.ptrw() + old_size, buf, n);
		}
		// Parse one framed message: headers, then Content-Length body bytes.
		int header_end = -1;
		for (int i = 0; i + 3 < read_buffer.size(); i++) {
			if (read_buffer[i] == '\r' && read_buffer[i + 1] == '\n' && read_buffer[i + 2] == '\r' && read_buffer[i + 3] == '\n') {
				header_end = i;
				break;
			}
		}
		if (header_end != -1) {
			String header = String::utf8((const char *)read_buffer.ptr(), header_end);
			int content_length = 0;
			for (const String &line : header.split("\r\n")) {
				if (line.begins_with("Content-Length:")) {
					content_length = line.get_slice(":", 1).strip_edges().to_int();
				}
			}
			if (content_length > 0 && read_buffer.size() >= header_end + 4 + content_length) {
				String body = String::utf8((const char *)read_buffer.ptr() + header_end + 4, content_length);
				read_buffer = read_buffer.slice(header_end + 4 + content_length, read_buffer.size());
				Variant parsed = JSON::parse_string(body);
				if (parsed.get_type() == Variant::DICTIONARY) {
					r_message = parsed;
					return true;
				}
			}
		}
		if (n == 0) {
			os->delay_usec(2000);
		}
	}
	return false;
}

Error SwiftLSPClient::_request(const String &p_method, const Dictionary &p_params, Variant &r_result, int p_timeout_ms) {
	if (stdio.is_null()) {
		return ERR_CANT_OPEN;
	}
	int id = next_request_id++;
	Dictionary msg;
	msg["id"] = id;
	msg["method"] = p_method;
	msg["params"] = p_params;
	_send(msg);
	Dictionary response;
	uint64_t deadline = OS::get_singleton()->get_ticks_msec() + p_timeout_ms;
	while (OS::get_singleton()->get_ticks_msec() < deadline) {
		int remaining_ms = (int)(deadline - OS::get_singleton()->get_ticks_msec());
		if (!_read_message(response, remaining_ms < 100 ? 100 : remaining_ms)) {
			break;
		}
		if (response.has("id") && (int)response["id"] == id) {
			if (response.has("error")) {
				return ERR_CANT_ACQUIRE_RESOURCE;
			}
			r_result = response.has("result") ? response["result"] : Variant();
			return OK;
		}
		// Skip notifications and unrelated responses.
	}
	return ERR_TIMEOUT;
}

bool SwiftLSPClient::_start() {
	if (stdio.is_valid()) {
		return true;
	}
	String bin = _find_binary();
	if (bin.is_empty()) {
		return false;
	}
	List<String> args;
	Dictionary spawned = OS::get_singleton()->execute_with_pipe(bin, args, false);
	if (!spawned.has("stdio") || !spawned.has("pid")) {
		return false;
	}
	stdio = spawned["stdio"];
	pid = spawned["pid"];
	read_buffer.clear();
	document_versions.clear();

	String root = ProjectSettings::get_singleton()->globalize_path("res://");
	Dictionary completion_item;
	completion_item["snippetSupport"] = false;
	Dictionary completion;
	completion["completionItem"] = completion_item;
	Dictionary text_document;
	text_document["completion"] = completion;
	Dictionary capabilities;
	capabilities["textDocument"] = text_document;
	Dictionary params;
	params["processId"] = (int)OS::get_singleton()->get_process_id();
	params["rootUri"] = _uri_for_path(root);
	params["capabilities"] = capabilities;
	Variant result;
	if (_request("initialize", params, result, 10000) != OK) {
		stop();
		return false;
	}
	Dictionary notification;
	notification["method"] = "initialized";
	notification["params"] = Dictionary();
	_send(notification);
	initialized = true;
	return true;
}

void SwiftLSPClient::_sync_document(const String &p_uri, const String &p_text) {
	if (!document_versions.has(p_uri)) {
		Dictionary text_document;
		text_document["uri"] = p_uri;
		text_document["languageId"] = "swift";
		text_document["version"] = 1;
		text_document["text"] = p_text;
		Dictionary params;
		params["textDocument"] = text_document;
		Dictionary msg;
		msg["method"] = "textDocument/didOpen";
		msg["params"] = params;
		_send(msg);
		document_versions[p_uri] = 1;
	} else {
		int version = document_versions[p_uri] + 1;
		Dictionary text_document;
		text_document["uri"] = p_uri;
		text_document["version"] = version;
		Array changes;
		Dictionary change;
		change["text"] = p_text;
		changes.push_back(change);
		Dictionary params;
		params["textDocument"] = text_document;
		params["contentChanges"] = changes;
		Dictionary msg;
		msg["method"] = "textDocument/didChange";
		msg["params"] = params;
		_send(msg);
		document_versions[p_uri] = version;
	}
}

static ScriptLanguage::CodeCompletionKind _lsp_kind_to_completion_kind(int p_lsp_kind) {
	switch (p_lsp_kind) {
		case 2: // Method
		case 3: // Function
		case 4: // Constructor
			return ScriptLanguage::CODE_COMPLETION_KIND_FUNCTION;
		case 5: // Field
		case 10: // Property
			return ScriptLanguage::CODE_COMPLETION_KIND_MEMBER;
		case 6: // Variable
			return ScriptLanguage::CODE_COMPLETION_KIND_VARIABLE;
		case 7: // Class
		case 8: // Interface
			return ScriptLanguage::CODE_COMPLETION_KIND_CLASS;
		case 13: // Enum
			return ScriptLanguage::CODE_COMPLETION_KIND_ENUM;
		case 14: // EnumMember
		case 21: // Constant (LSP has no constant; enum members are close)
			return ScriptLanguage::CODE_COMPLETION_KIND_CONSTANT;
		case 15: // Keyword
			return ScriptLanguage::CODE_COMPLETION_KIND_KEYWORD;
		case 18: // File
			return ScriptLanguage::CODE_COMPLETION_KIND_FILE_PATH;
		default:
			return ScriptLanguage::CODE_COMPLETION_KIND_PLAIN_TEXT;
	}
}

static String _lsp_kind_color(int p_lsp_kind) {
	switch (p_lsp_kind) {
		case 15:
			return "keyword_color";
		case 2:
		case 3:
		case 4:
			return "member_method_color";
		case 5:
		case 10:
			return "member_variable_color";
		case 7:
		case 8:
			return "user_type_color";
		default:
			return String();
	}
}

Error SwiftLSPClient::complete(const String &p_code, const String &p_path, List<ScriptLanguage::CodeCompletionOption> *r_options) {
	if (!initialized && !_start()) {
		return ERR_CANT_OPEN;
	}
	int caret = p_code.find(String::chr(0xFFFF));
	if (caret == -1) {
		return ERR_INVALID_PARAMETER;
	}
	String text = p_code.replace(String::chr(0xFFFF), "");
	String before = p_code.left(caret);
	PackedStringArray lines = before.split("\n");
	int line = lines.size() - 1; // 0-based
	// LSP counts UTF-16 code units; for ASCII this matches the char count.
	int character = lines[lines.size() - 1].length();

	String uri = _uri_for_path(ProjectSettings::get_singleton()->globalize_path(p_path));
	_sync_document(uri, text);

	Dictionary text_document;
	text_document["uri"] = uri;
	Dictionary position;
	position["line"] = line;
	position["character"] = character;
	Dictionary params;
	params["textDocument"] = text_document;
	params["position"] = position;
	Variant result;
	if (_request("textDocument/completion", params, result, 2000) != OK) {
		return ERR_TIMEOUT;
	}

	Array items;
	if (result.get_type() == Variant::ARRAY) {
		items = result;
	} else if (result.get_type() == Variant::DICTIONARY && ((Dictionary)result).has("items")) {
		items = ((Dictionary)result)["items"];
	}
	HashSet<String> existing;
	for (const ScriptLanguage::CodeCompletionOption &opt : *r_options) {
		existing.insert(opt.display);
	}
	for (int i = 0; i < items.size(); i++) {
		Dictionary item = items[i];
		if (!item.has("label")) {
			continue;
		}
		String label = item["label"];
		if (existing.has(label)) {
			continue;
		}
		existing.insert(label);
		int lsp_kind = item.has("kind") ? (int)item["kind"] : 1;
		ScriptLanguage::CodeCompletionOption option(label, _lsp_kind_to_completion_kind(lsp_kind), ScriptLanguage::LOCATION_OTHER, _lsp_kind_color(lsp_kind));
		if (item.has("insertText")) {
			option.insert_text = item["insertText"];
		} else if (item.has("textEdit")) {
			Dictionary edit = item["textEdit"];
			if (edit.has("newText")) {
				option.insert_text = edit["newText"];
			}
		}
		r_options->push_back(option);
	}
	return OK;
}

void SwiftLSPClient::stop() {
	if (stdio.is_valid() && initialized) {
		Variant result;
		_request("shutdown", Dictionary(), result, 1000);
		Dictionary notification;
		notification["method"] = "exit";
		_send(notification);
	}
	if (pid != -1) {
		OS::get_singleton()->kill(pid);
		pid = -1;
	}
	stdio.unref();
	initialized = false;
	read_buffer.clear();
	document_versions.clear();
}

#endif // TOOLS_ENABLED
