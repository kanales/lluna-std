local lluna = {}

function lluna.with(ctx, f)
	local mt = getmetatable(ctx)
	if mt.__enter then
		mt.__enter()
	end
	f()
	if mt.__exit then
		mt.__exit()
	end
end

function lluna.printf(...)
	local fstr = string.format(...)
	print(fstr)
end

function lluna.repr(v)
	local r = getmetatable(v).__repr
	if r ~= nil then
		if type(r) == "function" then
			return r(v)
		end
		return r
	end

	return tostring(v)
end

function lluna.pretty(t)
	if type(t) ~= "table" then
		return tostring(t)
	end

	local ps = {}
	for k, v in pairs(t) do
		table.insert(ps, tostring(k) .. " = " .. tostring(v))
	end
	return "{ " .. table.concat(ps, ", ") .. " }"
end

local function global()
	local tablex = require "tablex"
	local stringx = require "stringx"
  local mathx = require "mathx"
  local extend = tablex.extend

	extend(table, tablex)

	-- string extensions
	extend(string, stringx)
	local string_mt = getmetatable ""
	string_mt.__mul = string.rep

  extend(math, mathx )

	extend(_G, lluna)
	extend(_G, { path = require "path" })

end

return setmetatable(lluna, { __call = global })
