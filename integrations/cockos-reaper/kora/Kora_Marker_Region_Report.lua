-- @description Kora REAPER: Marker and Region Report
-- @version 1.0
local r=reaper
if not r or not r.CountProjectMarkers then return end
local _,markers,regions=r.CountProjectMarkers(0)
local lines={"KORA · ARRANGEMENT INDEX","Markers: "..markers,"Regions: "..regions,""}
for i=0,markers+regions-1 do
 local ok,is_region,pos,finish,name,index=r.EnumProjectMarkers3(0,i)
 if ok and ok>0 then
  local at=r.format_timestr_pos(pos,"",0)
  if is_region then at=at.." – "..r.format_timestr_pos(finish,"",0) end
  lines[#lines+1]=string.format("%s %d · %s · %s",is_region and "REGION" or "MARKER",index,at,name)
 end
end
r.ShowConsoleMsg(table.concat(lines,"\n").."\n")
r.ShowMessageBox("Index written to the ReaScript console.\nEntries: "..(markers+regions),"Kora Arrangement Index",0)
