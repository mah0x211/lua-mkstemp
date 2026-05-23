local testcase = require('testcase')
local assert = require('assert')
local errno = require('errno')
local errno_set = require('errno.set')

local ORIG_IO_TMPFILE = io.tmpfile
local mkstemp

function testcase.before_all()
    mkstemp = require('mkstemp')
end

function testcase.after_each()
    -- reload the module only when io.tmpfile was modified by a test
    if io.tmpfile ~= ORIG_IO_TMPFILE then
        _G.io.tmpfile = ORIG_IO_TMPFILE
        package.loaded['mkstemp'] = nil
        mkstemp = require('mkstemp')
    end
end

function testcase.create_tempfile()
    local tmpl = './tempfile_XXXXXX'
    local f, err, pathname = assert(mkstemp(tmpl))
    assert(f:close())
    assert.is_nil(err)
    assert.is_string(pathname)
    assert(os.remove(pathname))
    assert.not_equal(pathname, tmpl)
end

function testcase.return_error_on_invalid_path()
    local f, err, pathname = mkstemp('./foo/bar/baz/tempfile')
    assert.is_nil(f)
    assert.equal(err.type, errno[err.code])
    assert.is_nil(pathname)
end

function testcase.return_ENAMETOOLONG_error()
    local f, err, pathname = mkstemp('./' .. string.rep('f', 1024 * 8))
    assert.is_nil(f)
    assert.equal(err.type, errno.ENAMETOOLONG)
    assert.is_nil(pathname)
end

function testcase.throw_error_on_invalid_argument()
    local err = assert.throws(mkstemp, {})
    assert.match(err, '#1 .+ [(]string expected', false)
end

function testcase.return_error_if_io_tmpfile_fails()
    _G.io.tmpfile = function()
        errno_set(errno.EMFILE.code)
        return nil, 'failed'
    end
    package.loaded['mkstemp'] = nil
    mkstemp = require('mkstemp')
    local f, err, pathname = mkstemp('./tempfile2_XXXXXX')
    assert.is_nil(f)
    assert.equal(err.type, errno.EMFILE)
    assert.is_nil(pathname)
end

function testcase.throw_error_if_io_tmpfile_not_defined()
    _G.io.tmpfile = nil
    package.loaded['mkstemp'] = nil
    local err = assert.throws(function()
        require('mkstemp')
    end)
    assert.match(err, '"io.tmpfile" function not found')
end

