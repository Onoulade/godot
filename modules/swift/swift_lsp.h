/**************************************************************************/
/*  swift_lsp.h                                                           */
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

#include "core/io/file_access.h"
#include "core/object/editor_language.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/templates/list.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

// Persistent sourcekit-lsp client used for semantic completion of Swift
// scripts in the editor (LSP over stdio, JSON-RPC with Content-Length
// framing). The binary is located via the "swift/lsp_path" project setting,
// the SOURCEKIT_LSP environment variable, then the PATH. If unavailable,
// completion falls back to the local keyword/type/member lists.
class SwiftLSPClient : public Object {
	GDCLASS(SwiftLSPClient, Object);

	static SwiftLSPClient *singleton;

	Ref<FileAccess> stdio;
	int pid = -1;
	int next_request_id = 1;
	bool initialized = false;
	HashMap<String, int> document_versions; // uri -> version
	Vector<uint8_t> read_buffer;

	String _find_binary();
	bool _start();
	void _send(const Dictionary &p_message);
	bool _read_message(Dictionary &r_message, int p_timeout_ms);
	String _uri_for_path(const String &p_path);
	Error _request(const String &p_method, const Dictionary &p_params, Variant &r_result, int p_timeout_ms);
	void _sync_document(const String &p_uri, const String &p_text);

public:
	static SwiftLSPClient *get_singleton();

	bool is_running() const { return stdio.is_valid(); }
	void stop();

	// Appends LSP completion items for the document; p_code contains the
	// 0xFFFF sentinel at the caret position.
	Error complete(const String &p_code, const String &p_path, List<EditorLanguage::CompletionOption> *r_options);
};

#endif // TOOLS_ENABLED
