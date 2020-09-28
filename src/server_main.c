
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_builder.h"
#include "json_stringify.h"

#include "LEX/lex.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "DRVR/compiler.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

strong_cstr_t ast_all_function_names_as_json(ast_t *ast);
errorcode_t generate_ast_json(const char *filename, const char *adept_root, strong_cstr_t *result_json);

extern strong_cstr_t server_main(weak_cstr_t query_json){
	strong_cstr_t ast_json;
	generate_ast_json("/Users/isaac/main.adept", "/Users/isaac/Projects/Adept/bin/", &ast_json);

	json_builder_t builder;
	json_builder_init(&builder);
	json_build_object_start(&builder);
	json_build_object_key(&builder, "errorcode");
	json_build_integer(&builder, 0);
	json_build_next(&builder);
	json_build_object_key(&builder, "ast");
	json_builder_append(&builder, ast_json);
	json_build_object_end(&builder);

	free(ast_json);
	return json_builder_finalize(&builder);
}

errorcode_t generate_ast_json(const char *filename, const char *adept_root, strong_cstr_t *result_json){
	json_builder_t builder;
	json_builder_init(&builder);
	
	compiler_t compiler;
	compiler_init(&compiler);
	object_t *object = compiler_new_object(&compiler);
	adept_error_t *result_error = NULL;
	adept_warning_t *result_warnings = NULL;
	length_t result_warnings_length = 0;
	errorcode_t result_code = FAILURE;
	*result_json = NULL;

	object->filename = strclone(filename);
	object->full_filename = object->filename;//filename_absolute(object->filename);
	
	// Force object->full_filename to not be NULL
	if(object->full_filename == NULL) object->full_filename = strclone("");

	// Set compiler root
	compiler.root = strclone(adept_root);

	if(compiler_read_file(&compiler, object)){
		char *s = mallocandsprintf("Failed to read file %s", object->full_filename);
		*result_json = json_stringify_string(s);
		free(s);
		goto cleanup;
	}

	if(lex(&compiler, object)){
		// Failed to lex
		*result_json = json_stringify_string("Failed to lex");
		goto cleanup;
	}

	if(parse(&compiler, object)){
		if(compiler.result_flags & COMPILER_RESULT_SUCCESS){
			goto store_and_cleanup;
		}

		*result_json = json_stringify_string("Failed to parse");
		goto cleanup;
	}

store_and_cleanup:
	result_code = SUCCESS;
	*result_json = ast_all_function_names_as_json(&object->ast);

cleanup:
	if(*result_json == NULL){
		// Error json
		*result_json = json_stringify_string("Failed somewhere");
	}

	compiler_free(&compiler);
	return result_code;
}

strong_cstr_t ast_all_function_names_as_json(ast_t *ast){
	json_builder_t builder;
	json_build_array_start(&builder);

	for(length_t i = 0; i < ast->funcs_length; i++){
		json_build_string(&builder, ast->funcs[i].name);
		if(i + 1 < ast->funcs_length) json_build_next(&builder);
	}

	json_build_array_end(&builder);
	return json_builder_finalize(&builder);
}
