local tablex = {}

local function nameof(val)
	for name, v in pairs(_G) do
		if v == val then
			return name
		end
	end
	return "?"
end

local random = math.random
local floor = math.floor
local function randi(n)
	-- returns number in [0, n]
	return floor(0.5 + random() * n)
end

function tablex.shuffle(t)
	for i = #t, 2, -1 do
		local j = randi(i)
		t[i], t[j] = t[j], t[i]
	end
	return t
end

function tablex.extend(dst, src, no_overwrite)
	for k, v in pairs(src) do
		if no_overwrite and dst[k] ~= nil then
			error(string.format("%s.%s has value %s", nameof(dst), k, tostring(v)))
		end
		dst[k] = v
	end
end

return tablex
