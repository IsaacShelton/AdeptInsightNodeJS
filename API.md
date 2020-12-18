

# API



# Validation Query

#### ValidationRequest


```
{
    "query": "validate",
    "infrastructure": "/Users/isaac/Projects/Adept/bin/",
    "filename": "/Users/isaac/main.adept",
    "code": "import basics\n\nfunc main {\n    print(\"Isaac says: \\\"Hello World!\\\"\")\n}\n"
}
```
#### ValidationResponse

```
[
    {
        "kind", "warning",
        "source": {"index": 0, "stride", 3},
        "message": "Import a file???"
    },
    {
        "kind", "error",
        "source": {"index": 0, "stride", 10},
        "message": "Failed to find file \"test.adept\""
    }
]
```

#### ValidationError

```
"Failed to parse JSON, JSON is corruputed"
"Failed to parse JSON, JSON is incomplete"
"Unrecognized query kind 'avalidate'"
"Failed to read file 'main.adept'"
etc.
```

# AST Query

AST Query is supercedes Validation Query

#### ASTRequest


```
{
    "query": "ast",
    "infrastructure": "/Users/isaac/Projects/Adept/bin/",
    "filename": "/Users/isaac/main.adept",
    "code": "import basics\n\nfunc main {\n    print(\"Isaac says: \\\"Hello World!\\\"\")\n}\n"
}
```

#### ASTResponse

```
{
    "validation": [
            {
                "kind", "warning",
                "source": {"index": 0, "stride", 3},
                "message": "Import a file???"
            },
            {
                "kind", "error",
                "source": {"index": 0, "stride", 10},
                "message": "Failed to find file \"test.adept\""
            }
        ],
        "ast": {
            "functions": [
                {
                    "name": "main",
                    "definition": "func main() void",
                    "arity": 0,
                    "returns": "void",
                    "source": {"index": 23, "stride", 4}
                }
            ]
        }
}
```

#### ASTError

```
"Failed to parse JSON, JSON is corruputed"
"Failed to parse JSON, JSON is incomplete"
"Unrecognized query kind 'astt'"
"Failed to read file 'main.adept'"
etc.
```

