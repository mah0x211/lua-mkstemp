# lua-mkstemp

[![test](https://github.com/mah0x211/lua-mkstemp/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-mkstemp/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-mkstemp/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-mkstemp)

generate a unique temporary file name from the template, creates and opens the file.


## Installation

```
luarocks install mkstemp
```


## Usage

```lua
local mkstemp = require('mkstemp')
local f, err, pathname = mkstemp('/tmp/lua_XXXXXX')
assert(f, err)
print(pathname) -- /tmp/lua_Wo6kHb
```


## Error Handling

the following functions return the `error` object created by https://github.com/mah0x211/lua-errno module.


## f, err, pathname = mkstemp( tmpl )

this function takes the given file name template `tmpl` and overwrites a portion of it to create a file name and the template file.

see `man mkstemp` for more details.

**Parameters**

- `tmpl:string`: filename template.

**Returns**

- `f:file`: a lua file handle.
- `err:error`: `error` object on failure.
- `pathname:string`: pathname of the created file.
