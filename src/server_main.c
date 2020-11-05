
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_builder.h"
#include "json_builder_ex.h"
#include "json_stringify.h"
#include "query.h"

#include "LEX/lex.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "DRVR/compiler.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

strong_cstr_t ast_all_function_names_as_json(ast_t *ast);
errorcode_t generate_ast_json(const char *filename, const char *adept_root, strong_cstr_t *result_json);

void handle_validation_query(query_t *query, json_builder_t *builder);

extern strong_cstr_t server_main(weak_cstr_t query_json){
	json_builder_t builder;
	json_builder_init(&builder);

	query_t query;
	strong_cstr_t error_message = NULL;
	if(!query_parse(query_json, &query, &error_message)){
		json_build_string(&builder, error_message);
		free(error_message);
		return json_builder_finalize(&builder);
	}
	
	switch(query.kind){
	case QUERY_KIND_VALIDATE:
		handle_validation_query(&query, &builder);
		break;
	default:
		json_build_string(&builder, "Query kind is missing or unrecognized");
		goto cleanup_and_finalize;
	}

cleanup_and_finalize:
	query_free(&query);
	return json_builder_finalize(&builder);


	// strong_cstr_t ast_json;
	// generate_ast_json("/Users/isaac/main.adept", "/Users/isaac/Projects/Adept/bin/", &ast_json);

	// //json_builder_t builder;
	// json_builder_init(&builder);
	// json_build_object_start(&builder);
	// json_build_object_key(&builder, "errorcode");
	// json_build_integer(&builder, 0);
	// json_build_next(&builder);
	// json_build_object_key(&builder, "ast");
	// json_builder_append(&builder, ast_json);
	// json_build_object_end(&builder);

	// free(ast_json);
	// return json_builder_finalize(&builder);
}

void handle_validation_query(query_t *query, json_builder_t *builder){
	if(query->infrastructure == NULL){
		json_build_string(builder, "Validation query is missing field 'infrastructure'");
		return;
	}

	if(query->filename == NULL){
		json_build_string(builder, "Validation query is missing field 'filename'");
		return;
	}

	if(query->code == NULL){
		json_build_string(builder, "Validation query is missing field 'code'");
		return;
	}

	compiler_t compiler;
	compiler_init(&compiler);
	object_t *object = compiler_new_object(&compiler);

	object->filename = strclone(query->filename);
	object->full_filename = filename_absolute(object->filename);
	
	// Force object->full_filename to not be NULL
	if(object->full_filename == NULL) object->full_filename = strclone("");

	// Set compiler root
	compiler.root = strclone(query->infrastructure);

	// NOTE: Passing ownership of 'code' to object instance!!!
	object->buffer = query->code;
	object->buffer_length = strlen(query->code);
	query->code = NULL;
	
	if(lex_buffer(&compiler, object))   goto store_and_cleanup;
	if(parse(&compiler, object))        goto store_and_cleanup;

	length_t i;

store_and_cleanup:
	json_build_array_start(builder);

	// Push warnings
	for(i = 0; i != compiler.warnings_length; i++){
		json_build_object_start(builder);

		json_build_object_key(builder, "kind");
		json_build_string(builder, "warning");
		json_build_next(builder);

		json_build_source(builder, &compiler, compiler.warnings[i].source);
		json_build_next(builder);

		json_build_object_key(builder, "message");
		json_build_string(builder, compiler.warnings[i].message);

		json_build_object_end(builder);
		if(i + 1 != compiler.warnings_length || compiler.error) json_build_next(builder);
	}

	if(compiler.error){
		json_build_object_start(builder);

		json_build_object_key(builder, "kind");
		json_build_string(builder, "error");
		json_build_next(builder);

		json_build_source(builder, &compiler, compiler.error->source);
		json_build_next(builder);

		json_build_object_key(builder, "message");
		json_build_string(builder, compiler.error->message);

		json_build_object_end(builder);
	}

	json_build_array_end(builder);

cleanup:
	compiler_free(&compiler);
	return;
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
	object->full_filename = filename_absolute(object->filename);
	
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
