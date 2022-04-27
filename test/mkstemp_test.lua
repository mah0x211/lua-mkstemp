local assert = require('assert')
local errno = require('errno')
local mkstemp = require('mkstemp')

-- test that create tempfile
local pahtname = './tempfile_XXXXXX'
local f, err, eno = assert(mkstemp(pahtname))
assert(f:close())
assert.is_nil(err)
assert.is_nil(eno)
assert(os.remove(pahtname))
assert.not_equal(pahtname, './' .. 'lua_XXXXXX')

-- test that returns an error
f, err, eno = mkstemp('./foo/bar/baz/tempfile')
assert.is_nil(f)
assert.equal(err, errno[eno].message)

-- test that throws an error
err = assert.throws(mkstemp, {})
assert.match(err, '#1 .+ [(]string expected', false)

