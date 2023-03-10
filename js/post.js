
Module['onRuntimeInitialized'] = function() {
    if (!Module["noFSInit"] && !FS.init.initialized) FS.init();
    TTY.init();

    var filename = "/Users/isaac/Projects/Adept/build/macOS-Debug/import/2.8/basics.adept";

    var query = JSON.stringify(
        {
            "query": "ast",
            "infrastructure": "/Users/isaac/Projects/Adept/build/macOS-Debug/",
            "filename": filename,
            "code": require('fs').readFileSync(filename, "utf8"),
            "features": ['include-arg-info', 'include-calls']
        }
    );

    var result = invokeInsight(query);
    console.log(result);

    /*
        // More detailed info
        // console.log(result?.ast?.functions[390] ?? result);
        // console.log(Array.from(result?.calls.entries()).filter(a => a[1].length > 10).map(a => [result.ast.functions[a[0]].name, a[1], result.ast.functions[a[0]].source]) ?? result);
        // console.log(Array.from(result?.calls.entries()).filter(a => a[1].length > 10).map(a => a[1]) ?? result);
        // console.log(result?.calls ?? result);
    */
};

function invokeInsight(query_json_string){
    if(!is_wasm_initialized) return;

    var bytes = lengthBytesUTF8(query_json_string);
    var cstring = _malloc(bytes + 1);
    stringToUTF8(query_json_string, cstring, bytes + 1);
    var result_json_cstring = Module._server_main(cstring);
    var result_json = UTF8ToString(result_json_cstring);
    _free(cstring);
    _free(result_json_cstring);

    checkUnflushedContent();

    return JSON.parse(result_json);
}
