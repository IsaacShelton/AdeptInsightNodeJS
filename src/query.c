
#define JSMN_HEADER
#include "query.h"
#include "json_stringify.h"
#include "UTIL/jsmn.h"
#include "UTIL/jsmn_helper.h"
#include "UTIL/util.h"

void query_init(query_t *query){
    query->kind = QUERY_KIND_UNRECOGNIZED;
    query->infrastructure = NULL;
    query->filename = NULL;
}

void query_free(query_t *query){
    free(query->filename);
    free(query->infrastructure);
}

successful_t query_parse(weak_cstr_t json, query_t *out_query, strong_cstr_t *out_error){
    length_t json_length = strlen(json);
    jsmn_parser parser;
    jsmn_init(&parser);

    int required_tokens = jsmn_parse(&parser, json, json_length, NULL, 0);
    jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * required_tokens);

    jsmn_init(&parser);
    int parse_error = jsmn_parse(&parser, json, json_length, tokens, required_tokens);

    if(parse_error < 0 || required_tokens != parse_error){
        if(out_error) *out_error = mallocandsprintf("Failed to parse JSON, %s", jsmn_helper_parse_fail_reason(parse_error));
        free(tokens);
        return false;
    }

    // Ensure request is an object
    if(!jsmn_helper_get_object(json, tokens, required_tokens, 0)){
        free(tokens);
        return false;
    }

    jsmntok_t master_object_token = tokens[0];
    length_t section_token_index = 1;
    length_t total_sections = master_object_token.size;
    char key[128];
    char tmp[1024];

    query_init(out_query);

    for(length_t section = 0; section != total_sections; section++){
        // Get next key inside request
        if(!jsmn_helper_get_string(json, tokens, required_tokens, section_token_index++, key, sizeof(key))){
            if(out_error) *out_error = mallocandsprintf("Failed to process JSON", jsmn_helper_parse_fail_reason(parse_error));
            goto cleanup_and_fail;
        }

        if(strcmp(key, "query") == 0){
            // "query" : "..."
            if(!jsmn_helper_get_string(json, tokens, required_tokens, section_token_index, tmp, sizeof(tmp))){
                *out_error = mallocandsprintf("Expected string value for 'query'");
                goto cleanup_and_fail;
            }

            if(!query_set_kind_by_name(out_query, tmp)){
                if(out_error){
                    strong_cstr_t escaped = json_stringify_string(tmp);
                    *out_error = mallocandsprintf("Unrecognized query kind %s", escaped);
                    free(escaped);
                }
                goto cleanup_and_fail;
            }
        } else if(strcmp(key, "infrastructure") == 0){
            // "infrastructure" : "..."
            if(!jsmn_helper_get_string(json, tokens, required_tokens, section_token_index, tmp, sizeof(tmp))){
                *out_error = mallocandsprintf("Expected string value for '%s'", key);
                goto cleanup_and_fail;
            }
            
            out_query->infrastructure = strclone(tmp);
        } else if(strcmp(key, "filename") == 0){
            // "filename" : "..."
            if(!jsmn_helper_get_string(json, tokens, required_tokens, section_token_index, tmp, sizeof(tmp))){
                *out_error = mallocandsprintf("Expected string value for '%s'", key);
                goto cleanup_and_fail;
            }
            
            out_query->filename = strclone(tmp);
        } else {
            // "???" : "???"
            if(out_error) *out_error = mallocandsprintf("Unrecognized key '%s'", key);
            goto cleanup_and_fail;
        }

        section_token_index += jsmn_helper_subtoken_count(tokens, section_token_index);
    }

    free(tokens);
    return true;

cleanup_and_fail:
    free(tokens);
    query_free(out_query);
    return false;
}

successful_t query_set_kind_by_name(query_t *out_query, weak_cstr_t kind_name){
    if(strcmp(kind_name, "validate") == 0){
        out_query->kind = QUERY_KIND_VALIDATE;
        return true;
    }

    return false;
}
