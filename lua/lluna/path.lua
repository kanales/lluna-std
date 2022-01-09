local path = {}
local shutil = require "shutil"
path.__index = path

local SEP = "/"

local function join(one, other)
	return path(tostring(one), tostring(other))
end

function path:__tostring()
	return table.concat(self, SEP)
end

function path.cwd()
	return path(shutil.cwd())
end

function path:mkdir()
	shutil.mkdir(tostring(self))
end

function path:dir()
	local it = shutil.dir(tostring(self))
	if it == nil then
		return
	end
	return function()
		local r = it:next()
		if r ~= nil then
			return r
		end
	end
end

function path:chdir()
	shutil.chdir(tostring(self))
end

function path:rmdir()
	shutil.rmdir(tostring(self))
end

function path:stat()
	shutil.stat(tostring(self))
end

path.__div = join
function path:_new(...)
	local o = {}

	for i = 1, select("#", ...) do
		table.insert(o, tostring(select(i, ...)))
	end

	return setmetatable(o, path)
end

return setmetatable(path, {
	__call = path._new,
})
