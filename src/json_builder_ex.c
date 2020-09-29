
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
