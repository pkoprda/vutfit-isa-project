-- declare our protocol
isa_protocol = Proto("isa","ISA Protocol")
isa_proto = {}
isa_proto.port = 32323
isa_proto.data = "ISA Protocol Data"

-- create a function to dissect it
function isa_protocol.dissector(buffer, pinfo, tree)
  pinfo.cols.protocol = isa_protocol.name
  length = buffer:len()
  if length == 0 then return end

  local subtree = tree:add(isa_protocol, buffer(), isa_proto.data)

  subtree:add("Data length: " .. length)
  if pinfo.src_port == isa_proto.port then 
    parse_response(buffer, pinfo, subtree)
  else
    parse_query(buffer, pinfo, subtree)
  end
end

function parse_response(buffer, pinfo, subtree)
  subtree:add("Sender: server")
  status = buffer(1, 4):string()
  local buff_pos = string.find(status, ' ')
  status = status:gsub('err', 'error'):gsub('"', ''):gsub(' ', '')
  subtree:add(buffer(1, buff_pos), "Response: " .. status)
  buff_pos = buff_pos + 1

  if status == "error" then
    subtree:add(buffer(buff_pos, -1), "Message: " .. buffer(buff_pos, -1):string():sub(2, -3))
  else
    message = buffer(buff_pos, -1):string()
    local passwd = string.match(message, "[%w%+/]+==?")
    if passwd then
      message_end = string.find(message, passwd)
      subtree:add(buffer(buff_pos, message_end - 3), "Message: " .. string.sub(message, 2, message_end - 4))
      subtree:add(buffer(message_end + 2, -1), "Password: " .. passwd)
    end
  end
  -- local message = string.match(buffer(buff_pos, -1):string(), '.*registered.*')
  -- if message then subtree:add(buffer(buff_pos + 1, -1), "Message: " .. message:sub(2, -3)) end
end


function parse_query(buffer, pinfo, subtree)
  subtree:add("Sender: client")
  local command = buffer(1, 8):string()
  local buff_pos = string.find(command, ' ')
  if buff_pos then buff_pos = buff_pos - 1; command = string.sub(command, 1, buff_pos) else buff_pos = 8 end

  subtree:add(buffer(1, buff_pos), "Query: " .. command)
  parse_command(command, subtree, buffer, buff_pos)
end

function parse_command(command, subtree, buffer, buff_pos)
  function get_login_name()
    name = buffer(1, buffer:len() - 1):string():match("%s\".*\"%s")
    parsed_name = name:gsub('\\"', '"'):gsub("\\\\", '\\'):sub(3, -3)
    subtree:add(buffer(buff_pos + 2, name:len() - 2), "Name: " .. parsed_name)
    buff_pos = buff_pos + 1 + name:len()
    return buff_pos
  end

  if command == "register" or command == "login" then
    buff_pos = get_login_name()
    passwd = buffer(buff_pos, -1):string():sub(2, -3)
    subtree:add(buffer(buff_pos, -1), "Password: " .. dec(passwd))
    -- if command == "list" then


  end
end

-- source: http://lua-users.org/wiki/BaseSixtyFour
-- Lua 5.1+ base64 v3.0 (c) 2009 by Alex Kloss <alexthkloss@web.de>
-- licensed under the terms of the LGPL2

-- character table string
local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- encoding
function enc(data)
  return ((data:gsub('.', function(x) 
      local r,b='',x:byte()
      for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
      return r;
  end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
      if (#x < 6) then return '' end
      local c=0
      for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
      return b:sub(c+1,c+1)
  end)..({ '', '==', '=' })[#data%3+1])
end


-- decoding
function dec(data)
  local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
  data = string.gsub(data, '[^'..b..'=]', '')
  return (data:gsub('.', function(x)
      if (x == '=') then return '' end
      local r,f='',(b:find(x)-1)
      for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
      return r;
  end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
      if (#x ~= 8) then return '' end
      local c=0
      for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
      return string.char(c)
  end))
end

-- END

-- load the tcp.port table
tcp_table = DissectorTable.get("tcp.port")
-- register our protocol to handle tcp port 32323
tcp_table:add(isa_proto.port, isa_protocol)

-- register <username> <password>
-- login <username> <password>
-- list
-- send <recipient> <subject> <body>
-- fetch <id>
-- logout

