#pragma once

// ImGui overlay for the emulated Skylanders portal.
//
// Call SkylanderOverlay_render(false) from within an active ImGui frame on the
// TV view.  Press F8 in-game or click the floating "Portal" badge to open the
// full manager window.
void SkylanderOverlay_render(bool isPadView);
void SkylanderOverlay_init();
