local M = {}

function M.map(mapf)
	local function mapper(f, s, v)
		local function domap(...)
			v = ...
			if v ~= nil then
				return mapf(...)
			end
		end
		return function()
			return domap(f(s, v))
		end
	end
	return mapper
end
function M.each(mapf)
	return function(f, s, v)
		while true do
			local t = { f(s, v) }
			if #t == 0 then
				break
			end
			mapf(unpack(t))
			v = t[1]
		end
	end
end

function M.filter(ff)
	local function filterer(f, s, v)
		local function dofilter(...)
			v = ...
			if v ~= nil and ff(...) then
				return ...
			end
		end
		return function()
			return dofilter(f(s, v))
		end
	end
	return filterer
end

function M.fold(ff, init)
	local function folder(f, s, v)
		local acc = init
		while true do
			local t = { f(s, v) }
			if #t == 0 then
				break
			end
			acc = ff(acc, unpack(t))
			v = t[0]
		end
		return acc
	end
	return folder
end

M.sum = M.fold(function(a, x)
	return a + x
end)

M.snd = M.map(function(_, x)
	return x
end)

local function global()
	_G.map = M.map
	_G.fold = M.fold
	_G.filter = M.filter
	_G.each = M.each
end

return setmetatable(M, { __call = global })
