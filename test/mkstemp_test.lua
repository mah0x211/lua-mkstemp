local assert = require('assert')
local errno = require('errno')
local errno_set = require('errno.set')
local mkstemp = require('mkstemp')

-- test that create tempfile
local tmpl = './tempfile_XXXXXX'
local f, err, pathname = assert(mkstemp(tmpl))
assert(f:close())
assert.is_nil(err)
assert.is_string(pathname)
assert(os.remove(pathname))
assert.not_equal(pathname, tmpl)

-- test that returns an error
f, err, pathname = mkstemp('./foo/bar/baz/tempfile')
assert.is_nil(f)
assert.equal(err.type, errno[err.code])
assert.is_nil(pathname)

-- test that return ENAMETOOLONG error
f, err, pathname = mkstemp('./' .. string.rep('f', 1024 * 8))
assert.is_nil(f)
assert.equal(err.type, errno.ENAMETOOLONG)
assert.is_nil(pathname)

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
f, err, pathname = mkstemp('./tempfile2_XXXXXX')
assert.is_nil(f)
assert.equal(err.type, errno.EMFILE)
assert.is_nil(pathname)

-- test that throws an error if io.tmpfile function is not defined
_G.io.tmpfile = nil
package.loaded['mkstemp'] = nil
err = assert.throws(function()
    require('mkstemp')
end)
assert.match(err, '"io.tmpfile" function not found')

