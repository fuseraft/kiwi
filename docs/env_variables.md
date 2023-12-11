# Environment Variables - `env`

You can access environment variables using `env.{VARIABLE_NAME}`.

```
@path = env.PATH
@search = "/home"

if @path contains @search
    println "found `${search}` in `${path}`"
endif
```