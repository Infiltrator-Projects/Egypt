#pragma once
#include "ui_core.hpp"

namespace egypt {

class Game {
#include "game_public.inl"
#include "game_state.inl"
#include "game_buildings.inl"
#include "game_terrain.inl"
#include "game_agents.inl"
#include "game_world_ui.inl"
#include "game_hud.inl"
#include "game_inspector.inl"
};

} // namespace egypt
