local mathx = {}

local function gcd(a, b)
	if b == 0 then
		return a
	end
	return gcd(b, a % b)
end
mathx.gcd = gcd

function mathx.clamp(x, min, max)
	--[[
    mathx.clamp(x, [min,] max)
  ]]
	if max == nil then
		max = min
		min = 0
	end
	return math.min(math.max(x, min), max)
end

function mathx.lerp(a, b, t)
	-- linear interpolation
	return (1 - t) * a + t * b
end
function mathx.cerp(a, b, t)
	local f = math.sqrt(1 - math.cos(t * math.pi))
  return a * ( 1- f) + b *f
end

function mathx.wrap(x, min, max)
  --[[
    mathx.wrap(x, [min,] max)
  ]]
  if max == nil then min, max = 1, max end
  local m = max - min + 1
  return min + (x - min) % m
end

return mathx
