-- @description Kora REAPER: Track Health Dashboard
-- @version 1.0
local r=reaper
if not r or not r.CountTracks then return end
local n=r.CountTracks(0);local selected=r.CountSelectedTracks(0);local muted,soloed,empty=0,0,0
for i=0,n-1 do
 local t=r.GetTrack(0,i)
 if r.GetMediaTrackInfo_Value(t,"B_MUTE")>0 then muted=muted+1 end
 if r.GetMediaTrackInfo_Value(t,"I_SOLO")>0 then soloed=soloed+1 end
 if r.CountTrackMediaItems(t)==0 then empty=empty+1 end
end
local _,name=r.GetProjectName(0,"")
r.ShowMessageBox(table.concat({"KORA · TRACK HEALTH","Project: "..(name~="" and name or "(unsaved)"),string.format("Tempo: %.2f BPM",r.Master_GetTempo()),"Tracks: "..n,"Selected: "..selected,"Muted: "..muted,"Soloed: "..soloed,"Without media: "..empty},"\n"),"Kora Track Health",0)
