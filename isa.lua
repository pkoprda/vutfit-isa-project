-- Wireshark dissector
-- Author: Peter Koprda <xkoprd00@stud.fit.vutbr.cz>


-- declare our protocol
isa_protocol = Proto("isa", "ISA Protocol")
isa_proto = {}
isa_proto.port = 32323
isa_proto.data = "ISA Protocol Data"

-- create a function to dissect it
function isa_protocol.dissector(buffer, pinfo, tree)
  pinfo.cols.protocol = isa_protocol.name
  length = buffer:len()
  if length == 0 then return end

  -- create subtree for query and response
  subtree = tree:add(isa_protocol, buffer(), isa_proto.data)
  subtree:add("Data length: " .. length)
  if pinfo.src_port == isa_proto.port then
    parse_response(buffer, pinfo, subtree)
  else
    parse_query(buffer, pinfo, subtree)
  end
end

-- parse response from the server
function parse_response(buffer, pinfo, subtree)
  subtree:add("Sender: server")

  -- get response status
  status = buffer(1, 4):string()
  local buff_pos = string.find(status, ' ')
  status = status:gsub('err', 'error'):gsub('"', ''):gsub(' %(', ''):gsub(' ', '')
  subtree:add(buffer(1, buff_pos), "Response: " .. status)
  buff_pos = buff_pos + 1

  if status == "error" then
    -- show error message
    subtree:add(buffer(buff_pos, -1), "Message: " .. buffer(buff_pos, -1):string():sub(2, -3))
  else
    message = buffer(buff_pos, -1):string()
    
    -- split responses by response strings
    local register_command = string.match(message, 'registered%suser.*')
    local login_command = string.match(message, 'user%slogged%sin.*')
    local send_command = string.match(message, 'message%ssent.*')
    local logout_command = string.match(message, 'logged%sout.*')
    local fetch_command = string.match(message, '%(%".*%)')
    local list_command = string.match(message, '%(%(?.*%)%)')
    
    if register_command then
      message_end = string.find(message, register_command)
      subtree:add(buffer(message_end + 2, -1), "Registered user: " .. register_command:sub(17, -3):gsub('\\"', '"'):gsub("\\\\", '\\'))
    elseif login_command then
      local hashed_login = string.match(message, '%"[%w%+/]+=?=?%"')
      message_end = string.find(message, hashed_login)
      subtree:add(buffer(buff_pos, message_end - 3), "Message: " .. message:sub(2, message_end - 3))
      subtree:add(buffer(message_end + 2, -1), "Hashed username: " .. hashed_login:sub(2, -2))
    elseif send_command or logout_command then
      subtree:add(buffer(buff_pos, -1), "Message: " .. message:sub(2, -3))
    elseif fetch_command then
      local payload_subtree = subtree:add(isa_protocol, buffer(4, -1), "Payload")
      message = buffer(4, -1):string():sub(2, -3)
      user_len = message:find('%" %"')
      user = message:sub(2, user_len - 1)
      payload_subtree:add(buffer(buff_pos + 1, user_len), "Message sender: " .. user)
      buff_pos = buff_pos + user_len + 1
      message = buffer(buff_pos + 1, -1):string()
      subject_len = message:find('%" %"')
      subject = message:sub(2, subject_len - 1)
      payload_subtree:add(buffer(buff_pos + 1, subject_len), "Message subject: " .. subject)
      buff_pos = buff_pos + subject_len + 1
      message = buffer(buff_pos + 1, -1):string():sub(2, -4)
      payload_subtree:add(buffer(buff_pos + 1, message:len() + 2), "Message: " .. message)
    else
      local payload_subtree = subtree:add(isa_protocol, buffer(4, -1), "Payload")
      raw_messages = list_command:sub(2, -3)
      payload_subtree:add("Payload length: " .. raw_messages:len())
      payload_subtree:add(buffer(4, -1), "Payload (raw): " .. list_command:sub(1, -2))
      if raw_messages:len() == 0 then payload_subtree:add("Message count: 0"); return end
      local count_messages = list_command:match('%([^%(%)]*%)%)'):match('%d+')
      payload_subtree:add("Message count: " .. count_messages)

      buff_pos = 0
      for i = 1, count_messages do
        message = raw_messages:match("%([^%(^)]*%)")
        local message_subtree = payload_subtree:add(isa_protocol, buffer(buff_pos + 5, message:len()), "Message " .. i)
        message = message:sub(2, -2)
        if raw_messages:find("%) %(") then
          raw_messages = raw_messages:sub(raw_messages:find("%) %(") + 2, -1)
        end
        message = message:sub(message:find(' ') + 1, -1)
        buff_pos = buff_pos + message:find(' ') - 1
        sender = message:sub(2, message:find(' ') - 2)
        message_subtree:add(buffer(buff_pos, sender:len() + 2), "Message sender: " .. sender)
        buff_pos = buff_pos + sender:len() + 2
        message = message:sub(message:find(' ') + 1, -1)
        subject = message:sub(2, -2)
        message_subtree:add(buffer(buff_pos + 1, subject:len() + 2), "Message subject: " .. subject)
        buff_pos = buff_pos + subject:len()
      end
    end
  end
end

-- parse query from the client
function parse_query(buffer, pinfo, subtree)
  subtree:add("Sender: client")
  local command = buffer(1, 8):string()
  local buff_pos = string.find(command, ' ')
  if buff_pos then buff_pos = buff_pos - 1; command = string.sub(command, 1, buff_pos) else buff_pos = 8 end
  subtree:add(buffer(1, buff_pos), "Query: " .. command)
  parse_command(command, subtree, buffer, buff_pos)
end

-- split queries by command names
function parse_command(command, subtree, buffer, buff_pos)

  -- get name of the user for register and login command
  function get_login_name()
    name = buffer(1, buffer:len() - 1):string():match('%s%".*%"%s')
    parsed_name = name:gsub('\\"', '"'):gsub("\\\\", '\\'):sub(3, -3)
    subtree:add(buffer(buff_pos + 2, name:len() - 2), "Name: " .. parsed_name)
    buff_pos = buff_pos + name:len() + 1
    return buff_pos
  end

  if command == "register" or command == "login" then
    buff_pos = get_login_name()
    passwd = buffer(buff_pos, -1):string():sub(2, -3)
    subtree:add(buffer(buff_pos, -1), "Hashed password: " .. passwd)

  elseif command == "send" then
    buff_pos = buff_pos + 1
    message = buffer(buff_pos, -1):string()
    hashed_login = message:match('%"[%w%+/]+=?=?%"')
    subtree:add(buffer(buff_pos + 1, hashed_login:len()), "Hashed username: " .. hashed_login:sub(2, -2))
    buff_pos = buff_pos + hashed_login:len()
    message = message:sub(buff_pos - 2, -1)

    -- subtree for message payload
    local payload_subtree = subtree:add(isa_protocol, buffer(), "Payload")
    payload_subtree:add("Payload length: " .. message:len())
    payload_subtree:add(buffer(buff_pos + 2, -1), "Payload (raw): " .. message:sub(1, -2))
    recipient = message:sub(1, message:find("\" \""))
    payload_subtree:add(buffer(buff_pos + 2, recipient:len()), "Recipient: " .. recipient:sub(2, -2))
    buff_pos = buff_pos + recipient:len() + 1
    message = message:sub(recipient:len() + 2)
    subject = message:sub(1, message:find("\" \""))
    payload_subtree:add(buffer(buff_pos + 2, subject:len()), "Subject: " .. subject:sub(2, -2))
    buff_pos = buff_pos + subject:len() + 1
    message = message:sub(subject:len() + 2)
    payload_subtree:add(buffer(buff_pos + 2, -1), "Body: " .. message:sub(2, -3))

  elseif command == "fetch" then
    buff_pos = buff_pos + 2
    message = buffer(buff_pos, -1):string()
    hashed_login = message:match("[%w%+/]+=?=?")
    subtree:add(buffer(buff_pos, hashed_login:len() + 2), "Hashed login: " .. hashed_login)
    message_id = buffer(buff_pos + message:find(' '), -1):string()
    buff_pos = buff_pos + message:find(' ')
    subtree:add(buffer(buff_pos, message_id:len() - 1), "Message id: " .. message_id:sub(1, -2))

  elseif command == "logout" or command == "list" then
    buff_pos = buff_pos + 1
    message = buffer(buff_pos, -1):string():sub(3, -3)
    subtree:add(buffer(buff_pos + 1, -1), "Hashed username: " .. message)
  end
end

-- load the tcp.port table
tcp_table = DissectorTable.get("tcp.port")
-- register our protocol to handle tcp port 32323
tcp_table:add(isa_proto.port, isa_protocol)
