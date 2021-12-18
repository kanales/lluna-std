local heap = {}
local insert, pop = table.insert, table.pop

local function siftup(h, pos)
	-- TODO implement + test
end

local function siftdown(h, start, pos)
	-- TODO test
	local new = h[pos]
	while pos > start do
		local pidx = (pos - 1) / 2
		local parent = h[pidx]
		if new < parent then
			h[pos] = parent
			pos = pidx
		else
			break
		end
	end
	h[pos] = new
end

function heap.push(h, item)
	insert(h, item)
	siftdown(h, 1, #h)
end

function heap.pop(h, item)
	local out = table.pop(h)
	if #h > 0 then
		local ret = h[1]
		h[1] = out
		siftup(h, 0)
		return ret
	end
	return out
end

return heap
