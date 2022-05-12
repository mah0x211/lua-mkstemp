local assert = require('assert')
local errno = require('errno')
local errno_set = require('errno.set')
local mkstemp = require('mkstemp')

-- test that create tempfile
local pahtname = './tempfile_XXXXXX'
local f, err = assert(mkstemp(pahtname))
assert(f:close())
assert.is_nil(err)
assert(os.remove(pahtname))
assert.not_equal(pahtname, './' .. 'lua_XXXXXX')

-- test that returns an error
f, err = mkstemp('./foo/bar/baz/tempfile')
assert.is_nil(f)
assert.equal(err.type, errno[err.code])

-- test that throws an error
err = assert.throws(mkstemp, {})
assert.match(err, '#1 .+ [(]string expected', false)

-- test that return error if io.tmpfile function returns error
_G.io.tmpfile = function()
    errno_set(errno.EMFILE.code)
    return nil, 'failed'
end
package.loaded['mkstemp'] = nil
mkstemp = require('mkstemp')
f, err = mkstemp('./tempfile2_XXXXXX')
assert.is_nil(f)
assert.equal(err.type, errno.EMFILE)

-- test that throws an error if io.tmpfile function is not defined
_G.io.tmpfile = nil
package.loaded['mkstemp'] = nil
err = assert.throws(function()
    require('mkstemp')
end)
assert.match(err, '"io.tmpfile" function not found')

