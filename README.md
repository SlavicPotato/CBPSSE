# CBPSSE
Modified version of [CBP Physics for Skyrim SE](https://github.com/cbpphysics/CBPSSE) by [polygonhell](https://github.com/polygonhell)

## Changes
* Decouple simulation speed from framerate
* Track all loaded actors instead of just those in the player cell
* Track actors via events, no polling (cell/process lists scanning)
* Version independent (requires [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444))
* Simulate any valid node
* Per actor/race physics and node configuration via [in-game UI](https://i.imgur.com/Vf1kF8L.jpg) (ImGUI)
* Collisions (currently implements sphere, capsule and box collider shapes)
* Renderer to aid in configuring collision bodies
* Physics configuration overrides for individual armors

## Build deps
* [sse-build-resources](https://github.com/SlavicPotato/sse-build-resources)
* [ImGui](https://github.com/ocornut/imgui)
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
* [reactphysics3d](https://github.com/DanielChappuis/reactphysics3d)
* [DirectXTK](https://github.com/Microsoft/DirectXTK)
* [boost](https://github.com/boostorg/boost)
* [assimp (OBJ format only)](https://github.com/assimp/assimp)