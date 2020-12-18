
#include "ASTQuery.h"

#include "LEX/lex.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "DRVR/compiler.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

void build_ast(json_builder_t *builder, compiler_t *compiler, object_t *object){
    ast_t *ast = &object->ast;

    json_build_object_start(builder);

    json_build_object_key(builder, "functions");
    json_build_array_start(builder);
    for(length_t i = 0; i != ast->funcs_length; i++){
        ast_func_t *func = &ast->funcs[i];

        json_build_object_start(builder);
        json_build_object_key(builder, "name");
        json_build_string(builder, func->name);
        json_build_object_next(builder);
        json_build_object_key(builder, "definition");
        json_build_func_definition(builder, func);
        json_build_object_end(builder);

        if(i + 1 != ast->funcs_length){
            json_build_object_next(builder);
        }
    }
    json_build_array_end(builder);

    json_build_object_end(builder);
}

void build_identifierTokens(json_builder_t *builder, object_t *object){
    tokenlist_t *tokenlist = &object->tokenlist;
    token_t *tokens = tokenlist->tokens;
    source_t *sources = tokenlist->sources;
    bool has_at_least_one_identifier_token = false;

    json_build_array_start(builder);

    for(length_t i = 0; i != tokenlist->length; i++){
        if(tokens[i].id != TOKEN_WORD) continue;

        if(has_at_least_one_identifier_token){
            json_build_object_next(builder);
        } else {
            has_at_least_one_identifier_token = true;
        }

        token_t *token = &tokens[i];
        source_t *source = &sources[i];

        json_build_object_start(builder);
        json_build_object_key(builder, "content");
        json_build_string(builder, (weak_cstr_t) token->data);
        json_build_object_next(builder);
        json_build_object_key(builder, "range");

        int start_line, start_character, end_line, end_character;
        lex_get_location(object->buffer, source->index, &start_line, &start_character);
        lex_get_location(object->buffer, source->index + source->stride, &end_line, &end_character);

        // zero-indexed
        start_line--;
        start_character--;
        end_line--;
        end_character--;

        json_build_object_start(builder);

        json_build_object_key(builder, "start");

        json_build_object_start(builder);
        json_build_object_key(builder, "line");
        json_build_integer(builder, start_line);
        json_build_object_next(builder);
        json_build_object_key(builder, "character");
        json_build_integer(builder, start_character);
        json_build_object_end(builder);
        json_build_object_next(builder);




        json_build_object_key(builder, "end");

        json_build_object_start(builder);
        json_build_object_key(builder, "line");
        json_build_integer(builder, end_line);
        json_build_object_next(builder);
        json_build_object_key(builder, "character");
        json_build_integer(builder, end_character);
        json_build_object_end(builder);
        
        json_build_object_end(builder);
        json_build_object_end(builder);
    }

    json_build_array_end(builder);
}

void handle_ast_query(query_t *query, json_builder_t *builder){
    bool lexing_succeeded = false;
    bool validation_succeeded = false;

    if(query->infrastructure == NULL){
        json_build_string(builder, "AST query is missing field 'infrastructure'");
        return;
    }

    if(query->filename == NULL){
        json_build_string(builder, "AST query is missing field 'filename'");
        return;
    }

    if(query->code == NULL){
        json_build_string(builder, "AST query is missing field 'code'");
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
    lexing_succeeded = true;

    strong_cstr_t identifierTokens = NULL;

    {
        json_builder_t identifierTokensBuilder;
        json_builder_init(&identifierTokensBuilder);

        build_identifierTokens(&identifierTokensBuilder, object);

        identifierTokens = json_builder_finalize(&identifierTokensBuilder);
    }
    
    if(parse(&compiler, object))        goto store_and_cleanup;
    validation_succeeded = true;

    length_t i;

store_and_cleanup:
    json_build_object_start(builder);
    json_build_object_key(builder, "validation");

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

    json_build_next(builder);
    json_build_object_key(builder, "ast");

    if(validation_succeeded){
        build_ast(builder, &compiler, object);
    } else {
        json_build_null(builder);
    }

    json_build_array_next(builder);
    json_build_object_key(builder, "identifierTokens");
    
    if(lexing_succeeded){
        json_builder_append(builder, identifierTokens);
        free(identifierTokens);
    } else {
        json_build_null(builder);
    }
    json_build_object_end(builder);

cleanup:
    compiler_free(&compiler);
    return;
}
