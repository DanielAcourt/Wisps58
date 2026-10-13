# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Sovereign Framework: Species Primary DataAsset Importer (B-055)
--------------------------------------------------------------
Reads AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv and automatically
generates or updates USovereignSpeciesData Primary DataAssets in
/Game/Data/SpeciesAssets/ with baseline attributes, growth curve models,
and auto-calculated growth stage arrays.
"""

import os
import csv
import sys

# Attempt Unreal Engine Python API import (available inside UE Editor environment)
try:
    import unreal
    IN_UNREAL = True
except ImportError:
    IN_UNREAL = False

# Growth curve enum mapping for C++ ESovereignGrowthCurve
CURVE_ENUM_MAP = {
    "Linear": 0,
    "Exponential": 1,
    "FrontLoaded": 2,
    "Flat": 3
}

def resolve_csv_path():
    """Locates DT_SovereignSpeciesTemplate.csv relative to project root."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Content/Python -> WispsCPPVR -> Repo Root -> AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv
    repo_root = os.path.abspath(os.path.join(script_dir, "..", "..", ".."))
    csv_path = os.path.join(repo_root, "AI_Nexus", "Data", "DT_SovereignSpeciesTemplate.csv")
    if os.path.exists(csv_path):
        return csv_path

    # Fallback search
    alt_path = os.path.abspath(os.path.join(script_dir, "..", "Data", "DT_SovereignSpeciesTemplate.csv"))
    if os.path.exists(alt_path):
        return alt_path

    return csv_path

def import_species_templates(csv_filepath=None, target_package_path="/Game/Data/SpeciesAssets"):
    """
    Parses DT_SovereignSpeciesTemplate.csv and creates/updates USovereignSpeciesData assets.
    """
    if csv_filepath is None:
        csv_filepath = resolve_csv_path()

    if not os.path.exists(csv_filepath):
        msg = f"❌ [Sovereign Importer] CSV file not found: {csv_filepath}"
        if IN_UNREAL:
            unreal.log_error(msg)
        else:
            print(msg)
        return False

    print(f"🏛️ [Sovereign Importer] Reading species template from: {csv_filepath}")

    species_rows = []
    with open(csv_filepath, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            species_rows.append(row)

    print(f"🏛️ [Sovereign Importer] Found {len(species_rows)} species entries.")

    if not IN_UNREAL:
        print("ℹ️ [Sovereign Importer] Running outside Unreal Engine context. CSV validation successful:")
        for r in species_rows:
            print(f"   -> {r.get('Name')} ({r.get('SpeciesTag')}) | Curve: {r.get('GrowthCurveType', 'Exponential')} x{r.get('GrowthScaleFactor', '1.20')}")
        return True

    # --- Unreal Engine Editor Execution ---
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    # Ensure target destination folder exists
    if not editor_asset_lib.does_directory_exist(target_package_path):
        editor_asset_lib.make_directory(target_package_path)

    created_count = 0
    updated_count = 0

    for row in species_rows:
        try:
            asset_name = row.get("Name", "").strip()
            if not asset_name:
                continue

            asset_path = f"{target_package_path}/{asset_name}"
            existing_asset = editor_asset_lib.load_asset(asset_path)

            species_data_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignSpeciesData")
            if not species_data_class:
                unreal.log_error("❌ [Sovereign Importer] Could not load USovereignSpeciesData C++ class!")
                return False

            if existing_asset:
                species_asset = existing_asset
                updated_count += 1
                unreal.log(f"🔄 Updating existing species DataAsset: {asset_name}")
            else:
                factory = unreal.DataAssetFactory()
                factory.set_editor_property("data_asset_class", species_data_class)
                species_asset = asset_tools.create_asset(asset_name, target_package_path, species_data_class, factory)
                created_count += 1
                unreal.log(f"✨ Created new species DataAsset: {asset_name}")

            if not species_asset:
                unreal.log_error(f"❌ Failed to instantiate species asset: {asset_name}")
                continue

            # Set Identity properties
            display_name = row.get("DisplayName", asset_name)
            species_asset.set_editor_property("species_name", unreal.Text(display_name))

            species_tag_str = row.get("SpeciesTag", "").strip()
            if species_tag_str:
                try:
                    tag_obj = unreal.GameplayTag(tag_name=species_tag_str)
                    species_asset.set_editor_property("species_tag", tag_obj)
                    species_asset.set_editor_property("identity_signature", tag_obj)
                except Exception:
                    try:
                        tag_obj = unreal.GameplayTag()
                        tag_obj.set_editor_property("tag_name", unreal.Name(species_tag_str))
                        species_asset.set_editor_property("species_tag", tag_obj)
                        species_asset.set_editor_property("identity_signature", tag_obj)
                    except Exception as e_inner:
                        unreal.log_warning(f"   ⚠️ Could not set gameplay tag [{species_tag_str}]: {e_inner}")

            # Set Movement & Bio flags
            species_asset.set_editor_property("b_can_fly", row.get("bCanFly", "false").lower() == "true")
            species_asset.set_editor_property("b_can_move", row.get("bCanMove", "false").lower() == "true")
            species_asset.set_editor_property("b_is_biological", row.get("bIsBiological", "false").lower() == "true")

            # Set Soft Actor Class Pointer if specified
            actor_class_str = row.get("ActorClass", "").strip()
            if actor_class_str:
                try:
                    actor_class = unreal.load_class(None, actor_class_str)
                    if actor_class:
                        species_asset.set_editor_property("actor_class", actor_class)
                except Exception:
                    pass

            # Growth Curve Configuration
            curve_type_str = row.get("GrowthCurveType", "Exponential").strip()

            # Access C++ Enum for ESovereignGrowthCurve
            try:
                curve_enum_type = unreal.SovereignGrowthCurve.EXPONENTIAL
                if curve_type_str == "Linear":
                    curve_enum_type = unreal.SovereignGrowthCurve.LINEAR
                elif curve_type_str == "FrontLoaded":
                    curve_enum_type = unreal.SovereignGrowthCurve.FRONT_LOADED
                elif curve_type_str == "Flat":
                    curve_enum_type = unreal.SovereignGrowthCurve.FLAT

                species_asset.set_editor_property("growth_curve_type", curve_enum_type)
            except Exception:
                pass

            scale_factor = float(row.get("GrowthScaleFactor", "1.20"))
            species_asset.set_editor_property("growth_scale_factor", scale_factor)

            # Build 8 growth stage instances
            new_stages = []
            for stage_idx in range(8):
                stg = unreal.SovereignGrowthStage()
                if stage_idx == 0:
                    stg.set_editor_property("base_strength", int(float(row.get("BaseStrength", 10))))
                    stg.set_editor_property("base_dexterity", int(float(row.get("BaseDexterity", 10))))
                    stg.set_editor_property("base_constitution", int(float(row.get("BaseConstitution", 10))))
                    stg.set_editor_property("base_intelligence", int(float(row.get("BaseIntelligence", 10))))
                    stg.set_editor_property("base_wisdom", int(float(row.get("BaseWisdom", 10))))
                    stg.set_editor_property("base_charisma", int(float(row.get("BaseCharisma", 10))))
                    stg.set_editor_property("base_luck", int(float(row.get("BaseLuck", 10))))
                    stg.set_editor_property("metabolic_rate", float(row.get("MetabolicRate", 1.0)))
                new_stages.append(stg)

            species_asset.set_editor_property("growth_stages", new_stages)

            # Trigger C++ UFUNCTION(CallInEditor) void GenerateGrowthStages()
            if hasattr(species_asset, "generate_growth_stages"):
                species_asset.generate_growth_stages()

            # Save Asset
            editor_asset_lib.save_loaded_asset(species_asset)
        except Exception as e_row:
            unreal.log_warning(f"⚠️ [Sovereign Importer] Error processing row [{row.get('Name')}]: {e_row}")

    unreal.log(f"✅ [Sovereign Importer] Complete! Created: {created_count} | Updated: {updated_count} species assets in {target_package_path}")
    return True

if __name__ == "__main__":
    import_species_templates()
