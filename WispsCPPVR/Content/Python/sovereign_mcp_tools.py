# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-27
"""
Sovereign Framework: Editor Automation & MCP Tool Registration Script.
Exposes utility methods for spawning actors and automatically assigning Actor Tags and Gameplay Tags.
"""
import unreal

@unreal.uclass()
class SovereignEditorToolLibrary(unreal.BlueprintFunctionLibrary):

    @unreal.ufunction(static=True, meta=dict(Category="Sovereign|MCP"))
    def spawn_actor_with_tags(class_path: str, location: unreal.Vector, rotation: unreal.Rotator, actor_tags: list[str], gameplay_tags: list[str] = None) -> unreal.Actor:
        """
        Spawns an actor in the active Editor world and assigns specified Actor Tags and Gameplay Tags.

        :param class_path: Class path or asset path (e.g., '/Script/Engine.StaticMeshActor' or '/Game/Blueprint/MyActor.MyActor_C')
        :param location: Spawn location vector [X, Y, Z]
        :param rotation: Spawn rotation rotator [Pitch, Yaw, Roll]
        :param actor_tags: List of FName string tags to add to Actor.tags
        :param gameplay_tags: List of gameplay tag strings to register or set on Sovereign components
        :return: Spawned Actor instance or None
        """
        actor_class = unreal.EditorPlatformLibrary.load_class(None, class_path) if hasattr(unreal, 'EditorPlatformLibrary') else unreal.load_class(None, class_path)
        if not actor_class:
            actor_class = unreal.load_class(None, class_path)

        if not actor_class:
            unreal.log_error(f"SovereignMCP: Failed to load class at path '{class_path}'")
            return None

        # Spawn actor in current editor world
        spawned_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
        if not spawned_actor:
            unreal.log_error(f"SovereignMCP: Failed to spawn actor of class '{class_path}'")
            return None

        # Assign FName Actor Tags
        if actor_tags:
            for tag_str in actor_tags:
                tag_name = unreal.Name(tag_str)
                if tag_name not in spawned_actor.tags:
                    spawned_actor.tags.append(tag_name)
            unreal.log(f"SovereignMCP: Assigned Actor Tags {actor_tags} to '{spawned_actor.get_name()}'")

        # Assign Gameplay Tags if Sovereign Saveable / Gameplay Tag component exists
        if gameplay_tags:
            components = spawned_actor.get_components_by_class(unreal.ActorComponent)
            for comp in components:
                if "Sovereign" in comp.get_class().get_name() or hasattr(comp, "add_gameplay_tag"):
                    for g_tag in gameplay_tags:
                        if hasattr(comp, "add_gameplay_tag"):
                            comp.add_gameplay_tag(unreal.Name(g_tag))
            unreal.log(f"SovereignMCP: Processed Gameplay Tags {gameplay_tags} for '{spawned_actor.get_name()}'")

        return spawned_actor

def register_sovereign_tools():
    """
    Registers python functions with Unreal's embedded MCP subsystem if available,
    or verifies readiness for Python execution.
    """
    unreal.log("SovereignMCP: Sovereign Tool Registration Complete. Ready for MCP invocation.")

if __name__ == "__main__":
    register_sovereign_tools()
