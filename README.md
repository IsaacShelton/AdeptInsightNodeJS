# AdeptInsightNodeJS
Adept Insight Request Fulfiller for Node JS

### Purpose

This library provides information and insight into source code written in Adept.

### How it works

Requests are sent as JSON to `server_main` and then responses are created using the Adept Insight API that runs via WebAssembly.

### Does it require any native libaries

No, despite being mostly C code, it runs completely inside of Node JS.

