/**************************************************************************/
/*  swift_resource_format.cpp                                             */
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

#include "swift_resource_format.h"

#include "swift_script.h"

#include "core/io/file_access.h"

Ref<Resource> ResourceFormatLoaderSwift::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), nullptr, vformat("Cannot open Swift script '%s'.", p_path));

	String source = file->get_as_utf8_string();

	Ref<SwiftScript> script;
	script.instantiate();
	script->set_source_code(source);
	Error err = script->reload();

	if (r_error != nullptr) {
		*r_error = err;
	}
	return script;
}

void ResourceFormatLoaderSwift::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("swift");
}

bool ResourceFormatLoaderSwift::handles_type(const String &p_type) const {
	return (p_type == "Script" || p_type == "SwiftScript");
}

String ResourceFormatLoaderSwift::get_resource_type(const String &p_path) const {
	String extension = p_path.get_extension().to_lower();
	if (extension == "swift") {
		return "SwiftScript";
	}
	return "";
}

Error ResourceFormatSaverSwift::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<SwiftScript> script = p_resource;
	ERR_FAIL_COND_V_MSG(script.is_null(), ERR_INVALID_PARAMETER, "Can only save Swift scripts as '.swift' files.");

	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_OPEN, vformat("Cannot write Swift script '%s'.", p_path));

	file->store_string(script->get_source_code());
	file->close();
	return OK;
}

void ResourceFormatSaverSwift::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	if (Object::cast_to<SwiftScript>(*p_resource) != nullptr) {
		p_extensions->push_back("swift");
	}
}

bool ResourceFormatSaverSwift::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<SwiftScript>(*p_resource) != nullptr;
}
