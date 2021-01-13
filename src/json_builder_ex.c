
#include "AST/ast_type.h"
#include "json_builder_ex.h"

void json_build_source(json_builder_t *builder, compiler_t *compiler, source_t source){
    json_build_object_key(builder, "source");
    json_build_object_start(builder);
    json_build_object_key(builder, "object");
    json_build_string(builder, compiler->objects[source.object_index]->full_filename);
    json_build_next(builder);
    json_build_object_key(builder, "index");
    json_build_integer(builder, source.index);
    json_build_next(builder);
    json_build_object_key(builder, "stride");
    json_build_integer(builder, source.stride);
    json_build_object_end(builder);
}

void json_build_func_definition(json_builder_t *builder, ast_func_t *func){
    char *s;

    json_builder_append(builder, "\"");
    json_builder_append_escaped(builder, func->name);
    json_builder_append(builder, "(");

    for(length_t i = 0; i != func->arity; i++){
        bool is_last = i + 1 == func->arity;

        if(func->arg_names){
            while(!is_last && ast_types_identical(&func->arg_types[i], &func->arg_types[i + 1])){
                json_builder_append_escaped(builder, func->arg_names[i]);
                if(func->arg_defaults && func->arg_defaults[i]) json_builder_append(builder, "?");
                json_builder_append(builder, ", ");
                is_last = ++i + 1 == func->arity;
            }

            json_builder_append_escaped(builder, func->arg_names[i]);
            if(func->arg_defaults && func->arg_defaults[i]) json_builder_append(builder, "?");
            json_builder_append(builder, " ");
        }

        if(func->arg_type_traits[i] & AST_FUNC_ARG_TYPE_TRAIT_POD){
            json_builder_append(builder, " POD ");
        }

        s = ast_type_str(&func->arg_types[i]);
        json_builder_append_escaped(builder, s);
        free(s);

        if(!is_last){
            json_builder_append(builder, ", ");
        } else if(func->traits & AST_FUNC_VARARG){
            json_builder_append(builder, ", ...");
        } else if(func->traits & AST_FUNC_VARIADIC){
            json_builder_append(builder, ", ");
            json_builder_append_escaped(builder, func->variadic_arg_name);
            json_builder_append(builder, " ...");
        }
    }

    json_builder_append(builder, ") ");

    s = ast_type_str(&func->return_type);
    json_builder_append_escaped(builder, s);
    free(s);
    json_builder_append(builder, "\"");
}
