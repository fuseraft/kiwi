# Environment Variables - `env`

*Under construction in new interpreter.*

You can access environment variables using `env.{VARIABLE_NAME}`.

```ruby
@path = env.PATH
@search = "/home"

if @path.include? @search
    println "found `${@search}` in `${@path}`"
endif
```