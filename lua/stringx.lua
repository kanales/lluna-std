local stringx = {}

function stringx.split(s, sep)
	sep = sep or "%s"
	local t = {}
	local pat = "([^" .. sep .. "]+)"
	for m in string.gmatch(s, pat) do
		table.insert(t, m)
	end

	return t
end

return stringx
