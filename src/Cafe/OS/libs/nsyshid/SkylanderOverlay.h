#pragma once

// ImGui overlay for the emulated Skylanders portal.
//
// Call SkylanderOverlay_render(false) from within an active ImGui frame on the
// TV view.  Press F8 in-game or click the floating "Portal" badge to open the
// full manager window.
void SkylanderOverlay_render(bool isPadView);
void SkylanderOverlay_init();
// Called from the Win32 message handler to forward wheel events that the
// game's input capture would otherwise swallow before ImGui sees them.
void SkylanderOverlay_addWheelDelta(float delta);
// Called from Renderer::ImguiBegin (after SetCurrentContext, before NewFrame)
// to flush the accumulated delta into the active ImGui IO.
void SkylanderOverlay_flushWheelDelta();
