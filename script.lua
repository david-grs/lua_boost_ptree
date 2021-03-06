function tprint (tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    elseif type(v) == 'boolean' then
      print(formatting .. tostring(v))      
    else
      print(formatting .. v)
    end
  end
end

function recv(v)
  print(type(v))
  tprint(v, 0)
end


local obj = {key= "str"}
send({key= "str", miaou=1})
send({key= "str", miaou={1,2,3}})
