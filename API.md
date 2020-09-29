

# API



# Validation Query

#### ValidationRequest


```
{
	"query": "validate",
	"infrastructure": "/Users/isaac/Projects/Adept/bin/",
	"filename": "/Users/isaac/main.adept"
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

