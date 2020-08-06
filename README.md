# CBPSSE
Modified version of [CBP Physics for Skyrim SE](https://github.com/cbpphysics/CBPSSE) by [polygonhell](https://github.com/polygonhell)

## Changes
* Decouple simulation speed from framerate
* Use a real-time, high-resolution monotonic timer in physics calculations
* Track all loaded actors instead of just those in the player cell
* Track actors via events, no polling (cell/process lists scanning)
* Version independent (requires [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444))
* Simulate any valid node
* Per actor/race configuration via in-game UI (ImGUI)
* Collisions (reactphysics3d)

## Build deps
* [sse-build-resources](https://github.com/SlavicPotato/sse-build-resources)
* [ImGui](https://github.com/ocornut/imgui)
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
* [reactphysics3d](https://github.com/DanielChappuis/reactphysics3d)