-- @description Kora REAPER: Prefix Selected Tracks
-- @version 1.0
local r=reaper
if not r or not r.CountSelectedTracks then return end
local n=r.CountSelectedTracks(0)
if n==0 then r.ShowMessageBox("Select one or more tracks first.","Kora",0);return end
local ok,prefix=r.GetUserInputs("Kora · Prefix Selected Tracks",1,"Prefix:","Kora | ")
if not ok or prefix=="" then return end
r.Undo_BeginBlock();r.PreventUIRefresh(1)
for i=0,n-1 do
 local t=r.GetSelectedTrack(0,i)
 local _,name=r.GetSetMediaTrackInfo_String(t,"P_NAME","",false)
 r.GetSetMediaTrackInfo_String(t,"P_NAME",prefix..name,true)
end
r.PreventUIRefresh(-1);r.TrackList_AdjustWindows(false);r.UpdateArrange()
r.Undo_EndBlock("Kora: prefix selected tracks",-1)
