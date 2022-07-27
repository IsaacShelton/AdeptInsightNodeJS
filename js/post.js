
var is_wasm_initialized = false;

Module['onRuntimeInitialized'] = function() {
    is_wasm_initialized = true;
    if (!Module["noFSInit"] && !FS.init.initialized) FS.init();
    TTY.init();
    preMain();
};

function invokeInsight(query_json_string){
    if(!is_wasm_initialized) return;

    bytes = lengthBytesUTF8(query_json_string);
    var cstring = _malloc(bytes + 1);
    stringToUTF8(query_json_string, cstring, bytes + 1);
    var result_json_cstring = Module._server_main(cstring);
    result_json = UTF8ToString(result_json_cstring);
    _free(cstring);
    _free(result_json_cstring);

    checkUnflushedContent();
    
    return JSON.parse(result_json);
}

setTimeout(() => {
    var filename = "/Users/isaac/Projects/Adept/build/macOS-Debug/import/2.7/basics.adept";

    var query = JSON.stringify(
        {
            "query": "ast",
            "infrastructure": "/Users/isaac/Projects/Adept/build/macOS-Debug/",
            "filename": filename,
            "code": require('fs').readFileSync(filename, "utf8")
        }
    );

    var result = invokeInsight(query);
    console.log(result);
}, 100);
