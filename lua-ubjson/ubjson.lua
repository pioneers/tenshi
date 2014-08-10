-- Global dependencies here.

local abs = math.abs
local floor = math.floor
local dumpint = string.dumpint
local dumpfloat = string.dumpfloat
local ipairs, pairs, error, type =
      ipairs, pairs, error, type
local insert = table.insert


local _ENV = nil -- Block global read and write


-- Calculate 2 ^ v
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

-- ubjson tag -> size in bytes
local int_tag_size = {
  U = 1,
  i = 1,
  I = 2,
  l = 4,
  L = 8,
}

-- Use doubles to serialize Lua numbers.
local use_double = false

-- Get the smallest tag and size to hold a value.
local function int_tag(val)
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

-- If val can be represented by a fixed size value type, return the tag for
-- that type, otherwise return the Lua type string.
local function val_tag(val)
  local t = type(val)
  if t == 'number' then
    t = int_tag(val)
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

-- Pre-declare encode_inner
local encode_inner

-- Determines whether an table should become an array or a table.
-- Also determines length, and whether the optimized container optimization can
-- be applied.
-- returns use_obj, length, max_index, shared_tag, write_val
-- where
-- use_obj is true iff the table should become a ubjson object (not an array).
-- length is the number of entries in the array or table.
-- max_index is the largest integer index.
-- shared_tag is a ubjson type tag if the optimized container format can be
--     applied, otherwise is the string 'mixed'
-- write_val is a function which writes a value for the object or array.
-- write_val has the same type as encode_inner
--   (value, buffer, memo, depth) -> ()
--   where value is the value to be serialized
--         buffer is a table of strings which will be concatenated together to
--             produce the output
--         memo is a table mapping currently entered tables to true
--         depth is the recursion depth from the user's call
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

  local write_val = encode_inner

  if t ~= nil and #t == 1 then
    local size = int_tag_size[t]
    if size then
      write_val = function(val, buffer, memo)
        insert(buffer, dumpint(val, size, 'b'))
      end
    elseif t == 'f' then
      write_val = function(val, buffer, memo)
        insert(buffer, dumpfloat(val, 'f', 'b'))
      end
    elseif t == 'd' then
      write_val = function(val, buffer, memo)
        insert(buffer, dumpfloat(val, 'd', 'b'))
      end
    else
      -- Tag should be 'T', 'F', 'Z'
      write_val = function(val, buffer, memo)
      end
    end
  end

  if max > 2 * length then 
    -- This table is a sparse array, so don't create a ubjson array.
    return true, length, max, t, write_val
  end

  return non_int, length, max, t, write_val
end

local function encode_int(val, buffer)
  local tag, size = int_tag(val)
  insert(buffer, tag)
  insert(buffer, dumpint(val, size, 'b'))

  -- TODO(kzentner): Huge int support?
end

-- 
function encode_inner(val, buffer, memo, depth)
  if memo[val] then
    error("Cannot serialize circular data structure.", depth)
  end
  if depth == nil then
    error("Depth missing.")
  end

  local t = type(val)
  if t == 'number' then
    if floor(val) == val then
      encode_int(val, buffer)
    else
      if use_double then
        insert(buffer, 'D')
        insert(buffer, dumpfloat(val, 'd', 'b'))
      else
        insert(buffer, 'd')
        insert(buffer, dumpfloat(val, 'f', 'b'))
      end
    end
  elseif t == 'nil' then
    insert(buffer, 'Z')
  elseif t == 'boolean' then
    if val then
      insert(buffer, 'T')
    else
      insert(buffer, 'F')
    end
  elseif t == 'string' then
    insert(buffer, 'S')
    encode_int(#val, buffer)
    insert(buffer, val)
  elseif t == 'table' then
    memo[val] = true
    local use_obj, length, max, tag, write_val = array_helper(val)
    if use_obj then
      insert(buffer, '{')
    else
      insert(buffer, '[')
    end

    if tag ~= nil and #tag == 1 then
      insert(buffer, '$')
      insert(buffer, tag)
    end

    insert(buffer, '#')
    encode_int(length, buffer)

    if use_obj then
      for k, v in pairs(val) do
        local str = k .. ''
        encode_int(#str, buffer)
        insert(buffer, str)
        write_val(v, buffer, memo, depth + 1)
      end
    else
      for k = 1, max do
        write_val(val[k], buffer, memo, depth + 1)
      end
    end
    memo[val] = nil
  end
end

local function encode(value, state)
  local buffer = {}
  local memo = {}
  encode_inner(value, buffer, memo, 3)
  local out = ''
  for k, v in pairs(buffer) do
    out = out .. v
  end
  return out
end

local function decode_int(str, offset, depth, error_context)
  local c = str:sub(offset, offset)
  local int_size = int_tag_size[c]
  if int_size == nil then
    error(error_context .. " length did not have an integer tag.", depth)
  end
  local i = str:undumpint(offset + 1, int_size, 'b')
  if c == 'U' and i < 0 then
    -- Undo twos-complement
    i = 256 + i
  end
  return i, offset + 1 + int_size
end

-- Returns function with signature
-- (str, offset, depth) -> val, new_offset, skip
-- where str is the input string
--       offset is the index into str to start reading at
--       depth is the recursion depth from the user's call
--       val is the read value
--       new_offset is the offset after the read element
--       skip is whether the object should be recognized
--           (used to implement noop)
local function get_read_func(tag)
  local int_size = int_tag_size[tag]
  if tag == 'C' then
    int_size = 1
  end
  if int_size ~= nil then
    return function(str, offset, depth)
      return str:undumpint(offset, int_size, 'b'), offset + int_size
    end
  elseif tag == 'd' then
    return function(str, offset, depth)
      return str:undumpfloat(offset, 'f', 'b'), offset + 4
    end
  elseif tag == 'D' then
    return function(str, offset, depth)
      return str:undumpfloat(offset, 'd', 'b'), offset + 8
    end
  elseif tag == 'T' then
    return function(str, offset, depth)
      return true, offset
    end
  elseif tag == 'F' then
    return function(str, offset, depth)
      return false, offset
    end
  elseif tag == 'Z' then
    return function(str, offset, depth)
      return nil, offset
    end
  elseif tag == 'N' then
    return function(str, offset, depth)
      return nil, offset, true
    end
  else
    return nil
  end
end

-- Decodes a string. Does not read the type tag, so that it can be used to
-- decode ubjson object keys.
local function decode_str(str, offset, depth)
  local str_length, str_start = decode_int(str, offset,
                                           depth + 1,
                                           "String at offset " .. offset)
  -- -1 since str:sub includes the end address.
  return str:sub(str_start, str_start + str_length - 1), str_start + str_length
end

-- Recursive function used to decode object.
-- (str, offset, depth) -> (val, new_offset, skip)
-- where str is the input string
--       offset is the index into str to start reading at
--       depth is the recursion depth from the user's call
--       val is the read value
--       new_offset is the offset after the read element
--       skip is whether the object should be recognized
--           (used to implement noop)
local function decode_inner(str, offset, depth)
  if depth == nil then
    error('Depth missing')
  end

  local c = str:sub(offset, offset)
  local int_size = int_tag_size[c]
  if int_size ~= nil then
    return str:undumpint(offset + 1, int_size, 'b'), offset + 1 + int_size
  elseif c == 'C' then
    return str:undumpint(offset + 1, 1, 'b'), offset + 2
  elseif c == 'S' or c == 'H' then
    -- TODO(kzentner): How to handle huge numbers?
    return decode_str(str, offset + 1, depth + 1)
  elseif c == 'T' then
    return true, offset + 1
  elseif c == 'F' then
    return false, offset + 1
  elseif c == 'Z' then
    return nil, offset + 1
  elseif c == 'N' then
    return nil, offset + 1, true
  elseif c == '[' or c == '{' then
    local start_offset = offset + 1
    local tag = str:sub(start_offset, start_offset)
    local length = nil
    local out = {}
    local read_val = decode_inner
    if tag == '$' then
      start_offset = start_offset + 1
      local t = str:sub(start_offset, start_offset)
      start_offset = start_offset + 1
      tag = str:sub(start_offset, start_offset)
      read_val = get_read_func(t)
      if read_val == nil then
        if c == '[' then
          error("Type tag for non value type in array at offset " .. offset,
                depth)
        else
          error("Type tag for non value type in object at offset " .. offset,
                depth)
        end
      end
    end
    -- TODO(kzentner): Do not construct the error message every time.
    if tag == '#' then
      local msg
      if c == '[' then
        msg = 'Array'
      else
        msg = 'Object'
      end
      msg = msg .. ' length at offset ' .. offset
      length, start_offset = decode_int(str, start_offset + 1,
                                        depth + 1, msg)
    else
      start_offset = start_offset - 1
    end

    local elt_offset = start_offset
    local key, val, skip
    if c == '[' then
      if length ~= nil then
        for i = 1, length do
          val, elt_offset, skip = read_val(str, elt_offset, depth + 1)
          if not skip then
            out[i] = val
          end
        end
      else
        while str:sub(elt_offset, elt_offset) ~= ']' do
          val, elt_offset, skip = read_val(str, elt_offset, depth + 1)
          if not skip then
            out[i] = val
          end
        end
      end
    else
      if length ~= nil then
        for i = 1, length do
          key, elt_offset = decode_str(str, elt_offset, depth + 1)
          val, elt_offset, skip = read_val(str, elt_offset, depth + 1)
          if not skip then
            out[key] = val
          end
        end
      else
        while str:sub(elt_offset, elt_offset) ~= '}' do
          key, elt_offset = decode_str(str, elt_offset, depth + 1)
          val, elt_offset, skip = read_val(str, elt_offset, depth + 1)
          if not skip then
            out[key] = val
          end
        end
      end
    end
    return out, elt_offset
  else
    error('Unrecognized type tag ' .. c .. ' at offset ' .. offset .. '.',
          depth)
  end
end

local function decode(str, offset)
  if offset == nil then
    offset = 1
  end
  return decode_inner(str, offset, 1)
end

local ubjson = {
  version = 'lua-ubjson 0.1',
  encode = encode,
  decode = decode
}

return ubjson
