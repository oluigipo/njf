/*
Not a Json File

You have only one instruction: define NJF_IMPLEMENTATION in a single
translation unit before including this header.

Names that begins with NJF_ are macros/types/enums defined by this header.
And those that begins with njf_ are functions.
Do not use the functions that begins with njf__.

*/

#ifndef NJF_HEADER
#define NJF_HEADER

#ifdef __cplusplus
extern "C" {
#endif
	
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	//////// Header
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
#include <stdint.h>
	
#ifndef NJF_malloc
#   include <stdlib.h>
	
#   define NJF_malloc(x) malloc(x)
#   define NJF_free(x) free(x)
#endif
	
#ifndef NJF_Bool
#   define NJF_Bool int
#endif
	
#ifndef NJF_Number
#   define NJF_Number long long
#endif
	
#define NJF_false ((NJF_Bool)(size_t)NULL)
#define NJF_true (!NJF_false)
	
	struct NJF_String {
		const uint8_t* ptr;
		size_t length;
	} typedef NJF_String;
	
	struct NJF_Body {
		const uint8_t* buffer;
		size_t length;
	} typedef NJF_Body;
	
	struct NJF_File {
		void* buffer;
		
		NJF_Body body;
	} typedef NJF_File;
	
	typedef unsigned int NJF_Kind;
	enum NJF_Kind {
		NJF_Kind_Invalid = 0,
		NJF_Kind_NotFound = 0,
		
		NJF_Kind_Ident,
		NJF_Kind_Number,
	};
	
	struct NJF_Object {
		NJF_Body* parent;
		size_t offset;
		NJF_Bool valid;
		
		NJF_String name;
		const uint8_t* has_body; // is NULL if object does not have a body
		const uint8_t* body_end;
	} typedef NJF_Object;
	
	struct NJF_Flag {
		NJF_Object* object;
		size_t offset;
		NJF_Kind kind;
		
		union {
			NJF_String ident;
			NJF_Number number;
		} as;
	} typedef NJF_Flag;
	
	//////////////////////////////////////////////////////////////
	//////// Functions
	
	// Loads and allocates a buffer.
	NJF_Bool njf_load_file(const char* path, NJF_File* file);
	// Just loads from an existing buffer.
	NJF_Body njf_load_buffer(const uint8_t* buffer, size_t length);
	// Frees a buffer previously allocated.
	void njf_free(NJF_File* file);
	
	// Returns the number of object within the body
	size_t njf_object_count(const NJF_Body* body);
	
	// Returns a body inside the object. You can check if an object has a
	// body by doing 'object.has_body != NULL'. This function returns a
	// zero'd body if the object does not have one.
	NJF_Body njf_object_body(NJF_Object* object);
	
	// Returns the first object of the body. You can check if this object
	// is valid by doing 'object.valid == NJF_true'. OBS: NJF_true is just
	// a 1.
	NJF_Object njf_first_object(NJF_Body* body);
	
	// Transforms the object into the very next object in the body. Returns
	// if this new object is valid, the same as doing 'object.valid == NJF_true'.
	NJF_Bool njf_next_object(NJF_Object* object);
	
	// Returns the number of flags in the object
	size_t njf_flag_count(const NJF_Object* object);
	
	// Returns the first flag in the object. You can check if this flag is valid
	// by doing 'flag.kind != NJF_Flag_Invalid'.
	NJF_Flag njf_first_flag(NJF_Object* object);
	
	// Transforms the flag into the very next flag in the object. Returns
	// if this new flag is valid, the same as doing 'flag.kind != NJF_Flag_Invalid'.
	NJF_Bool njf_next_flag(NJF_Flag* flag);
	
	
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	//////// Implementation
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
#ifdef NJF_IMPLEMENTATION
#include <stdio.h>
	
	static NJF_Bool njf__is_number(uint8_t ch) {
		return ch >= '0' && ch <= '9';
	}
	
	static NJF_Bool njf__is_empty_space(uint8_t ch) {
		return (ch == ' '  ||
				ch == '\t' ||
				ch == '\r' ||
				ch == '\v' ||
				ch == '\n');
	}
	
	static const uint8_t* njf__consume_white_spaces_left(const uint8_t* ptr, const uint8_t* limit) {
		while (ptr < limit && njf__is_empty_space(*ptr))
		{ ++ptr; }
		
		return ptr;
	}
	
	static const uint8_t* njf__consume_white_spaces_right(const uint8_t* limit, const uint8_t* ptr) {
		while (ptr > limit && njf__is_empty_space(*ptr))
		{ --ptr; }
		
		return ptr;
	}
	
	static const uint8_t* njf__end_of_string(const uint8_t* begin, const uint8_t* end) {
		const uint8_t* str = begin;
		
		for (; str[0] != '"'; ++str) {
			if (str >= end) return NULL;
			if (str[0] == '\\' && str[1] == '"') ++str;
		}
		
		return str;
	}
	
	NJF_Bool njf_load_file(const char* path, NJF_File* file) {
		FILE* handle = fopen(path, "r");
		if (!handle) {
			return NJF_false;
		}
		
		fseek(handle, 0, SEEK_END);
		size_t length = ftell(handle);
		rewind(handle);
		
		uint8_t* buffer = NJF_malloc(length + 1);
		if (!buffer) {
			fclose(handle);
			return NJF_false;
		}
		
		length = fread(buffer, 1, length, handle);
		buffer[length] = 0;
		fclose(handle);
		
		file->buffer = buffer;
		
		const uint8_t* begin = buffer;
		const uint8_t* end = begin + length;
		
		// Trim
		begin = njf__consume_white_spaces_left(begin, end);
		end = njf__consume_white_spaces_right(begin, end);
		
		file->body.buffer = begin;
		file->body.length = end - begin;
		
		return NJF_true;
	}
	
	void njf_free(NJF_File* file) {
		NJF_free(file->buffer);
		
		file->buffer = NULL;
		file->body.buffer = NULL;
		file->body.length = 0;
	}
	
	NJF_Body njf_load_buffer(const uint8_t* buffer, size_t length) {
		NJF_Body body;
		
		const uint8_t* begin = buffer;
		const uint8_t* end = begin + length;
		
		// Trim
		begin = njf__consume_white_spaces_left(begin, end);
		end = njf__consume_white_spaces_right(begin, end);
		
		body.buffer = begin;
		body.length = end - begin;
		
		return body;
	}
	
	size_t njf_object_count(const NJF_Body* body) {
		// TODO: make this a procedure better
		
		NJF_Body private_body = *body;
		size_t count = 0;
		
		for (NJF_Object object = njf_first_object(&private_body); object.valid; njf_next_object(&object)) {
			++count;
		}
		
		return count;
	}
	
	NJF_Body njf_object_body(NJF_Object* object) {
		NJF_Body body = { 0 };
		if (!object->has_body) return body;
		
		const uint8_t* begin = njf__consume_white_spaces_left(object->has_body + 1, object->body_end - 1);
		const uint8_t* end = njf__consume_white_spaces_right(begin, object->body_end - 1);
		
		body.buffer = begin;
		body.length = end - begin;
		
		return body;
	}
	
	NJF_Object njf_first_object(NJF_Body* body) {
		NJF_Object object;
		object.parent = body;
		object.offset = 0;
		
		njf_next_object(&object);
		
		return object;
	}
	
	NJF_Bool njf_next_object(NJF_Object* object) {
		size_t base_offset = object->offset;
		
		object->valid = NJF_false;
		object->has_body = NULL;
		
		const uint8_t* begin = object->parent->buffer + base_offset;
		const uint8_t* end = begin + object->parent->length;
		
		begin = njf__consume_white_spaces_left(begin, end);
		
		if (end - begin < 2 || begin[0] != '"') return NJF_false;
		
		// Parse Name
		const uint8_t* name = begin + 1;
		const uint8_t* name_end = njf__end_of_string(name, end);
		
		if (!name_end) return NJF_false;
		
		size_t name_len = name_end - name;
		
		object->valid = NJF_true;
		object->name.ptr = name;
		object->name.length = name_len;
		
		object->offset = base_offset + name_len + 2;
		
		// Find beginning of it's body, if any
		const uint8_t* body_begin = name + name_len + 1;
		
		for (; *body_begin != '{'; ++body_begin, ++object->offset) {
			if (body_begin >= end ||
				*body_begin == '}' ||
				*body_begin == '"') return NJF_true;
		}
		
		object->has_body = body_begin;
		
		//- Find end of the body
		begin = object->has_body + 1;
		end = begin + object->parent->length;
		
		begin = njf__consume_white_spaces_left(begin, end);
		const uint8_t* saved_begin = begin;
		
		int nested = 1;
		while (nested > 0 && begin < end) {
			if (*begin == '"') {
				begin = njf__end_of_string(begin, end);
				
				if (begin >= end) return NJF_false;
				
				++begin;
				continue;
			}
			
			if (*begin == '{') nested++;
			if (*begin == '}') nested--;
			
			++begin;
		}
		
		if (nested > 0) return NJF_false;
		
		object->body_end = begin;
		object->offset += object->body_end - object->has_body + 1;
		
		return NJF_true;
	}
	
	size_t njf_flag_count(const NJF_Object* object) {
		// TODO: make this a procedure better
		
		NJF_Object private_object = *object;
		size_t count = 0;
		
		for (NJF_Flag flag = njf_first_flag(&private_object); flag.kind != NJF_Kind_NotFound; njf_next_flag(&flag)) {
			++count;
		}
		
		return count;
	}
	
	NJF_Flag njf_first_flag(NJF_Object* object) {
		NJF_Flag flag;
		
		flag.object = object;
		flag.offset = 0;
		
		njf_next_flag(&flag);
		
		return flag;
	}
	
	NJF_Bool njf_next_flag(NJF_Flag* flag) {
		const uint8_t* begin = flag->object->name.ptr + flag->object->name.length + 1;
		const uint8_t* end = flag->object->parent->buffer + flag->object->parent->length;
		
		const uint8_t* real_begin = begin;
		const uint8_t* saved_begin = njf__consume_white_spaces_left(begin + flag->offset, end);
		begin = saved_begin;
		
		flag->kind = NJF_Kind_NotFound;
		
		if (begin >= end || *begin == '{' || *begin == '"' || *begin == '}') return NJF_false;
		
		if (*begin == '-' || njf__is_number(*begin)) {
			// Number
			flag->kind = NJF_Kind_Number;
			NJF_Number result = 0;
			NJF_Number sign = 1;
			
			if (*begin == '-') {
				sign = -1;
				++begin;
			}
			
			while (begin < end && njf__is_number(*begin)) {
				++begin;
				
				result *= 10;
				result += *begin - '0';
			}
			
			flag->as.number = result * sign;
			flag->offset = begin - real_begin;
			return NJF_true;
		}
		
		// Ident
		flag->kind = NJF_Kind_Ident;
		while (begin < end && (!njf__is_empty_space(*begin) && *begin != '}' && *begin != '"'))
		{ ++begin; }
		
		flag->as.ident.ptr = saved_begin;
		flag->as.ident.length = begin - saved_begin;
		flag->offset = begin - real_begin;
		return NJF_true;
	}
	
#endif // NJF_IMPLEMENTATION
	
#ifdef __cplusplus
}
#endif

#endif // NJF_HEADER
