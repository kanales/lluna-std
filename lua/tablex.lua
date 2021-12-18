local tablex = {}

local function nameof(val)
	for name, v in pairs(_G) do
		if v == val then
			return name
		end
	end
	return "?"
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
