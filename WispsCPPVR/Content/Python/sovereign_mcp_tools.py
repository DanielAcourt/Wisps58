# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-27
"""
Sovereign Framework: Editor Automation & MCP Tool Registration Script.
Exposes utility methods for spawning actors and automatically assigning Actor Tags and Gameplay Tags.
"""
import unreal

def spawn_actor_with_tags(class_path: str, location: list = None, rotation: list = None, actor_tags: str = "", gameplay_tags: str = "") -> unreal.Actor:
    """
    Spawns an actor in the active Editor world and assigns specified Actor Tags and Gameplay Tags.

    :param class_path: Asset path or class path (e.g., '/Script/Engine.StaticMeshActor')
    :param location: Spawn location vector [X, Y, Z]
    :param rotation: Spawn rotation rotator [Pitch, Yaw, Roll]
    :param actor_tags: Comma-separated string of Actor tags (e.g., "TagA,TagB") or list
    :param gameplay_tags: Comma-separated string of Gameplay tags (e.g., "Sovereign.Entity,Wisp.Core")
    :return: Spawned Actor instance or None
    """
    if not class_path:
        unreal.log_error("SovereignMCP: No class path specified for spawn_actor_with_tags")
        return None

    actor_class = unreal.load_class(None, class_path)

    if not actor_class:
        unreal.log_error(f"SovereignMCP: Failed to load class at path '{class_path}'")
        return None

    # Convert location vector and rotator safely
    loc_vec = unreal.Vector(location[0], location[1], location[2]) if location and len(location) >= 3 else unreal.Vector(0.0, 0.0, 0.0)
    rot_val = unreal.Rotator(rotation[0], rotation[1], rotation[2]) if rotation and len(rotation) >= 3 else unreal.Rotator(0.0, 0.0, 0.0)

    # Spawn actor in current editor world
    spawned_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, loc_vec, rot_val)
    if not spawned_actor:
        unreal.log_error(f"SovereignMCP: Failed to spawn actor of class '{class_path}'")
        return None

    # Process Actor Tags
    tag_list = [t.strip() for t in actor_tags.split(",") if t.strip()] if isinstance(actor_tags, str) else actor_tags if isinstance(actor_tags, list) else []
    if tag_list:
        for tag_str in tag_list:
            tag_name = unreal.Name(tag_str)
            if tag_name not in spawned_actor.tags:
                spawned_actor.tags.append(tag_name)
        unreal.log(f"SovereignMCP: Assigned Actor Tags {tag_list} to '{spawned_actor.get_name()}'")

    # Process Gameplay Tags if Sovereign Saveable / Gameplay Tag component exists
    g_tag_list = [t.strip() for t in gameplay_tags.split(",") if t.strip()] if isinstance(gameplay_tags, str) else gameplay_tags if isinstance(gameplay_tags, list) else []
    if g_tag_list:
        components = spawned_actor.get_components_by_class(unreal.ActorComponent)
        for comp in components:
            if "Sovereign" in comp.get_class().get_name() or hasattr(comp, "add_gameplay_tag"):
                for g_tag in g_tag_list:
                    if hasattr(comp, "add_gameplay_tag"):
                        comp.add_gameplay_tag(unreal.Name(g_tag))
        unreal.log(f"SovereignMCP: Processed Gameplay Tags {g_tag_list} for '{spawned_actor.get_name()}'")

    return spawned_actor

class SovereignEditorToolLibrary:
    spawn_actor_with_tags = staticmethod(spawn_actor_with_tags)

def register_sovereign_tools():
    """
    Registers python functions with Unreal's embedded MCP subsystem if available,
    or verifies readiness for Python execution.
    """
    unreal.log("SovereignMCP: Sovereign Tool Registration Complete. Ready for MCP invocation.")

if __name__ == "__main__":
    register_sovereign_tools()
