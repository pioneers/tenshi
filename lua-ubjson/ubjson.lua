
--local _ENV = nil -- blocking globals in Lua 5.2
local math = require 'math'

local ubjson = { version = 'lua-ubjson 0.1' }

local pow2 = function(v)
  local out = 1
  for i = 0, v do
    -- Use floats, since this may end up very large.
    out = out * 2.0
  end
  return out
end

-- Mapping from maximum value -> ubjson tag
local int_maxes = {
  pow2( 7),
  pow2(15),
  pow2(31),
  pow2(63),
}

local int_tags = {
  'i',
  'I',
  'l',
  'L',
}

local int_tag_size = {
  U = 1,
  i = 1,
  I = 2,
  l = 4,
  L = 8,
}

local use_double = false

local function int_data(val)
  if val >= 0 and val < 256 then
    return 'U', 1
  end
  local last_key = 'i'
  local size = 1
  -- Calculate the absolute value.
  if val < 0 then
    val = -val
    -- Because twos-complement can hold slightly larger negatives than
    -- positives.
    if val ~= 0 then
      val = val - 1
    end
  end
  for idx, max in ipairs(int_maxes) do
    if val > max then
      return last_key, size
    else
      last_key = int_tags[idx]
      size = size * 2
    end
  end
  return last_key, size / 2
end

local function val_tag(val)
  local t = type(val)
  if t == 'number' then
    t = int_data(val)
  elseif t == 'boolean' then
    if t then
      return 'T'
    else
      return 'F'
    end
  elseif t == 'nil' then
    return 'Z'
  end
  return t
end

local encode = nil

local function array_helper(val)
  local t = nil
  local length = 0
  local non_int = false
  local max = 0

  for k, v in pairs(val) do
    if type(k) ~= 'number' or k <= 0 then
      non_int = true
    elseif k > max then
      max = k
    end
    if t == nil then
      t = val_tag(v)
    end
    if t ~= val_tag(v) then
      t = 'mixed'
    end
    length = length + 1
  end

  local function write_val(val, buffer, memo)
    encode(val, buffer, memo)
  end

  if #t == 1 then
    local size = int_tag_size[t]
    if size then
      write_val = function(val, buffer, memo)
        table.insert(buffer, string.dumpint(val, size, 'b'))
      end
    elseif t == 'f' then
      write_val = function(val, buffer, memo)
        table.insert(buffer, string.dumpfloat(val, 'f', 'b'))
      end
    elseif t == 'd' then
      write_val = function(val, buffer, memo)
        table.insert(buffer, string.dumpfloat(val, 'd', 'b'))
      end
    else
      -- Tag should be 'T', 'F', 'Z'
      write_val = function(val, buffer, memo)
      end
    end
  end

  if max > 2 * length then 
    -- This table is a sparse array, so don't create a ubjson array.
    return true, length, max, t, encode
  end

  return non_int, length, max, t, write_val
end

local function encode_int(val, buffer)
  local tag, size = int_data(val)
  table.insert(buffer, tag)
  table.insert(buffer, string.dumpint(val, size, 'b'))

  -- TODO(kzentner): Huge int support?
end

encode = function(val, buffer, memo)
  if memo[val] then
    error("Cannot serialize circular data structure.")
  end
  local t = type(val)
  if t == 'number' then
    if math.floor(val) == val then
      encode_int(val, buffer)
    else
      if use_double then
        table.insert(buffer, 'D')
        table.insert(buffer, string.dumpfloat(val, 'd', 'b'))
      else
        table.insert(buffer, 'd')
        table.insert(buffer, string.dumpfloat(val, 'f', 'b'))
      end
    end
  elseif t == 'nil' then
    table.insert(buffer, 'Z')
  elseif t == 'boolean' then
    if t then
      table.insert(buffer, 'T')
    else
      table.insert(buffer, 'F')
    end
  elseif t == 'string' then
    table.insert(buffer, 'S')
    encode_int(#val, buffer)
    table.insert(buffer, val)
  elseif t == 'table' then
    memo[val] = true
    local use_obj, length, max, tag, write_val = array_helper(val)
    if use_obj then
      table.insert(buffer, '{')
    else
      table.insert(buffer, '[')
    end

    if #tag == 1 then
      table.insert(buffer, '$')
      table.insert(buffer, tag)
    end

    table.insert(buffer, '#')
    encode_int(length, buffer)

    if use_obj then
      for k, v in pairs(val) do
        local str = k .. ''
        encode_int(#str, buffer)
        table.insert(buffer, str)
        write_val(v, buffer, memo)
      end
    else
      for k = 1, max do
        write_val(val[k], buffer, memo)
      end
    end
    memo[val] = nil
  end
end

function ubjson.encode(value, state)
  local buffer = {}
  local memo = {}
  encode(value, buffer, memo)
  out = ''
  for k, v in pairs(buffer) do
    out = out .. v
  end
  return out
end

function hex(str)
  local out = ''
  for i = 1, #str do
    out = out .. string.format('%02x', str:byte(i))
  end
  return out
end

function ascii(str)
  local out = ''
  for i = 1, #str do
    local c = str:sub(i, i)
    if c:match('%g') then
      out = out .. c .. ' '
    else
      out = (out .. '  ')
    end
  end
  return out
end

local function test(val)
  local enc = ubjson.encode(val) 
  print('length:', #enc)
  print(hex(enc))
  print(ascii(enc))
end

test({1, 2, 3, 4})
test({1, 2, 3, 4, 5, -1})
test({0, 1, 2, 3, 4, 5})
test({PiELESDigitalVals = {true, true, true, true, true, true, true, true},
      PiELESAnalogVals = {127, 127, 127, 127, 127, 127, 127}})

test{[1] = 1, [3] = 4, [14] = 5}
