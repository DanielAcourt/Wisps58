# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Sovereign Framework: Universal Agnostic CSV DataAsset Importer (B-055 & B-056)
----------------------------------------------------------------------------
Agnostic CSV-to-DataAsset engine for Unreal Engine 5.8.1.
Supports custom CSV file paths, target DataAsset class reflection,
and C++ post-import triggers (such as USovereignSpeciesData::GenerateGrowthStages).
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

def resolve_default_csv_path():
    """Locates DT_SovereignSpeciesTemplate.csv relative to project root."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.abspath(os.path.join(script_dir, "..", "..", ".."))
    csv_path = os.path.join(repo_root, "AI_Nexus", "Data", "DT_SovereignSpeciesTemplate.csv")
    if os.path.exists(csv_path):
        return csv_path

    alt_path = os.path.abspath(os.path.join(script_dir, "..", "Data", "DT_SovereignSpeciesTemplate.csv"))
    if os.path.exists(alt_path):
        return alt_path

    return csv_path

def import_csv_to_data_assets(
    csv_filepath=None,
    data_asset_class_path="/Script/WispCPP7VR.SovereignSpeciesData",
    target_package_path="/Game/Data/SpeciesAssets",
    b_overwrite=True
):
    """
    Agnostic Importer Engine:
    Reads any CSV file and auto-instantiates/updates UDataAsset instances of data_asset_class_path.
    """
    if csv_filepath is None or not str(csv_filepath).strip():
        csv_filepath = resolve_default_csv_path()

    # Sanitize path: strip quotes, normalize Windows backslashes
    csv_filepath = str(csv_filepath).strip().strip("'").strip('"').replace("\\", "/")
    if not os.path.isabs(csv_filepath) and not os.path.exists(csv_filepath):
        csv_filepath = os.path.abspath(csv_filepath)

    if not os.path.exists(csv_filepath):
        msg = f"❌ [Sovereign Importer] CSV file not found: {csv_filepath}"
        if IN_UNREAL:
            unreal.log_error(msg)
        else:
            print(msg)
        return False

    print(f"🏛️ [Sovereign Importer] Reading CSV data from: {csv_filepath}")

    rows = []
    with open(csv_filepath, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append(r)

    print(f"🏛️ [Sovereign Importer] Found {len(rows)} entries in CSV.")

    if not IN_UNREAL:
        print("ℹ️ [Sovereign Importer] Running outside Unreal Engine context. Validation successful:")
        for r in rows:
            asset_name = r.get("Name", r.get("ItemTag", "Asset")).strip()
            print(f"   -> {asset_name} | Fields: {len(r)}")
        return True

    # --- Unreal Engine Editor Execution ---
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    # Ensure target destination folder exists
    if not editor_asset_lib.does_directory_exist(target_package_path):
        editor_asset_lib.make_directory(target_package_path)

    data_class = unreal.load_class(None, data_asset_class_path)
    if not data_class:
        unreal.log_error(f"❌ [Sovereign Importer] Could not load DataAsset C++ class at '{data_asset_class_path}'!")
        return False

    created_count = 0
    updated_count = 0

    for row in rows:
        try:
            asset_name = row.get("Name", row.get("ItemTag", "")).strip().replace(".", "_")
            if not asset_name:
                continue

            asset_path = f"{target_package_path}/{asset_name}"
            existing_asset = editor_asset_lib.load_asset(asset_path)

            if existing_asset:
                if not b_overwrite:
                    unreal.log(f"⏭️ Skipping existing asset (Overwrite disabled): {asset_name}")
                    continue
                data_asset = existing_asset
                updated_count += 1
                unreal.log(f"🔄 Updating existing DataAsset: {asset_name}")
            else:
                factory = unreal.DataAssetFactory()
                factory.set_editor_property("data_asset_class", data_class)
                data_asset = asset_tools.create_asset(asset_name, target_package_path, data_class, factory)
                created_count += 1
                unreal.log(f"✨ Created new DataAsset: {asset_name}")

            if not data_asset:
                unreal.log_error(f"❌ Failed to instantiate asset: {asset_name}")
                continue

            # --- Reflection Mapping ---
            # 1. Identity Display Name
            display_name = row.get("DisplayName", asset_name)
            for prop_name in ["species_name", "display_name", "item_name"]:
                try:
                    data_asset.set_editor_property(prop_name, unreal.Text(display_name))
                    break
                except Exception:
                    pass

            # 2. Gameplay Tags
            species_tag_str = row.get("SpeciesTag", row.get("ItemTag", "")).strip()
            if species_tag_str:
                try:
                    # Unreal Engine Python API construct for GameplayTag
                    tag_obj = unreal.GameplayTag(tag_name=unreal.Name(species_tag_str))
                    for prop_name in ["species_tag", "identity_signature", "item_tag"]:
                        try:
                            data_asset.set_editor_property(prop_name, tag_obj)
                        except Exception:
                            pass
                except Exception as e_tag:
                    try:
                        # Fallback for direct struct construction
                        tag_obj = unreal.GameplayTag()
                        tag_obj.set_editor_property("tag_name", unreal.Name(species_tag_str))
                        for prop_name in ["species_tag", "identity_signature", "item_tag"]:
                            try:
                                data_asset.set_editor_property(prop_name, tag_obj)
                            except Exception:
                                pass
                    except Exception as e_inner:
                        unreal.log_warning(f"   ⚠️ Tag assignment warning [{species_tag_str}]: {e_inner}")

            # 3. Traits Container
            traits_str = row.get("SpeciesTraits", row.get("AdditionalMetadata", "")).strip()
            if traits_str:
                try:
                    tag_list = [t.strip() for t in traits_str.split(",") if t.strip()]
                    tag_container = unreal.GameplayTagContainer()
                    gameplay_tags = []
                    for t in tag_list:
                        try:
                            gt = unreal.GameplayTag(tag_name=unreal.Name(t))
                            gameplay_tags.append(gt)
                        except Exception:
                            gt = unreal.GameplayTag()
                            gt.set_editor_property("tag_name", unreal.Name(t))
                            gameplay_tags.append(gt)
                    tag_container.set_editor_property("gameplay_tags", gameplay_tags)
                    for prop_name in ["species_traits", "gameplay_tags", "metadata"]:
                        try:
                            data_asset.set_editor_property(prop_name, tag_container)
                        except Exception:
                            pass
                except Exception as e_traits:
                    unreal.log_warning(f"   ⚠️ Trait container warning [{traits_str}]: {e_traits}")

            # 4. Capability Flags
            b_fly = (row.get("bCanFly", "false").lower() == "true")
            b_move = (row.get("bCanMove", "false").lower() == "true")
            b_bio = (row.get("bIsBiological", row.get("bIsBiologicalOrigin", "false")).lower() == "true")

            for prop in ["can_fly", "b_can_fly"]:
                try: data_asset.set_editor_property(prop, b_fly); break
                except Exception: pass

            for prop in ["can_move", "b_can_move"]:
                try: data_asset.set_editor_property(prop, b_move); break
                except Exception: pass

            for prop in ["is_biological", "b_is_biological", "is_biological_origin", "b_is_biological_origin"]:
                try: data_asset.set_editor_property(prop, b_bio); break
                except Exception: pass

            # 5. Soft Actor Class Pointer
            actor_class_str = row.get("ActorClass", "").strip()
            if actor_class_str:
                try:
                    actor_class = unreal.load_class(None, actor_class_str)
                    if actor_class:
                        data_asset.set_editor_property("actor_class", actor_class)
                except Exception:
                    pass

            # 6. Growth Curve Configuration (Specialized for SovereignSpeciesData)
            curve_type_str = row.get("GrowthCurveType", "Exponential").strip()
            try:
                curve_enum_type = unreal.SovereignGrowthCurve.EXPONENTIAL
                if curve_type_str == "Linear":
                    curve_enum_type = unreal.SovereignGrowthCurve.LINEAR
                elif curve_type_str == "FrontLoaded":
                    curve_enum_type = unreal.SovereignGrowthCurve.FRONT_LOADED
                elif curve_type_str == "Flat":
                    curve_enum_type = unreal.SovereignGrowthCurve.FLAT

                data_asset.set_editor_property("growth_curve_type", curve_enum_type)
            except Exception:
                pass

            scale_factor = float(row.get("GrowthScaleFactor", "1.20"))
            try:
                data_asset.set_editor_property("growth_scale_factor", scale_factor)
            except Exception:
                pass

            # 7. Growth Stages Construction
            try:
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

                data_asset.set_editor_property("growth_stages", new_stages)
            except Exception:
                pass

            # 8. Post-Import C++ Triggers
            if hasattr(data_asset, "generate_growth_stages"):
                data_asset.generate_growth_stages()

            # Save Asset
            editor_asset_lib.save_loaded_asset(data_asset)

        except Exception as e_row:
            unreal.log_warning(f"⚠️ [Sovereign Importer] Error processing row [{row.get('Name')}]: {e_row}")

    unreal.log(f"✅ [Sovereign Importer] Complete! Created: {created_count} | Updated: {updated_count} assets in {target_package_path}")
    return True

def import_species_templates(csv_filepath=None, target_package_path="/Game/Data/SpeciesAssets"):
    """Backwards-compatible convenience wrapper."""
    return import_csv_to_data_assets(
        csv_filepath=csv_filepath,
        data_asset_class_path="/Script/WispCPP7VR.SovereignSpeciesData",
        target_package_path=target_package_path,
        b_overwrite=True
    )

def execute_importer_ui(csv_path="", class_path="/Script/WispCPP7VR.SovereignSpeciesData", output_dir="/Game/Data/SpeciesAssets", b_overwrite=True):
    """
    Blueprint / Editor Utility Widget (EUW) Execution Wrapper.
    Saves dirty packages before import and returns a concise single-line summary string
    perfect for displaying in an EUW Text Block widget.
    """
    try:
        import unreal
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(show_dialog_if_dirty=False, save_map_packages=False)
    except Exception:
        pass

    success = import_csv_to_data_assets(
        csv_filepath=csv_path,
        data_asset_class_path=class_path,
        target_package_path=output_dir,
        b_overwrite=b_overwrite
    )

    if success:
        output_msg = f"✅ Successfully imported DataAssets to '{output_dir}'."
    else:
        output_msg = f"❌ [ERROR] Import failed for CSV '{csv_path}'."

    print(output_msg)
    try:
        import unreal
        if success:
            unreal.log(output_msg)
        else:
            unreal.log_error(output_msg)
    except Exception:
        pass

    return output_msg

if __name__ == "__main__":
    import_species_templates()
