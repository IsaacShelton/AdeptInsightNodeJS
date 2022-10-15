
#include "ASTQuery.h"

#include "LEX/lex.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "UTIL/string.h"
#include "DRVR/compiler.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

static void add_function_definition(json_builder_t *builder, compiler_t *compiler, ast_func_t *func){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, func->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_build_func_definition(builder, func);
    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, func->source);
    json_build_object_next(builder);
    json_build_object_key(builder, "end");
    json_build_source(builder, compiler, func->end_source);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_function_alias_definition(json_builder_t *builder, compiler_t *compiler, ast_func_alias_t *falias){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, falias->from);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");

    json_builder_append(builder, "\"");
    json_builder_append(builder, "func alias ");
    json_builder_append_escaped(builder, falias->from);

    if(!falias->match_first_of_name){
        json_build_func_parameters(builder, NULL, falias->arg_types, NULL, NULL, falias->arity, falias->required_traits, NULL);
    }
    
    json_builder_append(builder, " => ");
    json_builder_append_escaped(builder, falias->to);

    json_builder_append(builder, "\"");

    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, falias->source);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_composite_definition(json_builder_t *builder, ast_composite_t *composite){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, composite->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_build_composite_definition(builder, composite);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_enum_definition(json_builder_t *builder, ast_enum_t *enum_value){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, enum_value->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"enum ");
    json_builder_append_escaped(builder, enum_value->name);
    json_builder_append(builder, " (");
    for(length_t i = 0; i != enum_value->length; i++){
        json_builder_append_escaped(builder, enum_value->kinds[i]);
        json_builder_append(builder, ", ");
    }
    if(enum_value->length != 0) json_builder_remove(builder, 2); // Remove trailing ', '
    json_builder_append(builder, ")\"");
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_alias_definition(json_builder_t *builder, ast_alias_t *alias){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, alias->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"alias ");
    json_builder_append_escaped(builder, alias->name);
    json_builder_append(builder, " = ");

    strong_cstr_t typename = ast_type_str(&alias->type);
    json_builder_append_escaped(builder, typename);
    free(typename);

    json_builder_append(builder, "\"");
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_constant_definition(json_builder_t *builder, ast_constant_t *constant){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, constant->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"define ");
    json_builder_append_escaped(builder, constant->name);
    json_builder_append(builder, " = ");

    strong_cstr_t value = ast_expr_str(constant->expression);
    json_builder_append_escaped(builder, value);
    free(value);
    json_builder_append(builder, "\"");
    json_build_object_end(builder);

    json_build_object_next(builder);
}

void build_ast(json_builder_t *builder, compiler_t *compiler, object_t *object){
    ast_t *ast = &object->ast;

    json_build_object_start(builder);

    {
        json_build_object_key(builder, "functions");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->funcs_length; i++){
            add_function_definition(builder, compiler, &ast->funcs[i]);
        }
        if(ast->funcs_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "function_aliases");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->func_aliases_length; i++){
            add_function_alias_definition(builder, compiler, &ast->func_aliases[i]);
        }
        if(ast->func_aliases_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "composites");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->composites_length; i++){
            add_composite_definition(builder, &ast->composites[i]);
        }
        for(length_t i = 0; i < ast->poly_composites_length; i++){
            add_composite_definition(builder, (ast_composite_t*) &ast->poly_composites[i]);
        }
        if(ast->composites_length || ast->poly_composites_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "enums");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->enums_length; i++){
            add_enum_definition(builder, &ast->enums[i]);
        }
        if(ast->enums_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "aliases");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->aliases_length; i++){
            add_alias_definition(builder, &ast->aliases[i]);
        }
        if(ast->aliases_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "constants");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->constants_length; i++){
            add_constant_definition(builder, &ast->constants[i]);
        }

        if(ast->constants_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
    }
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

    if (!query->warnings) {
        compiler.traits |= COMPILER_NO_WARN;
        compiler.ignore |= COMPILER_IGNORE_ALL;
    }

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

        json_build_object_key(builder, "source");
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

        json_build_object_key(builder, "source");
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
