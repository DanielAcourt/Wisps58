# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer Engine & Reverse Class Exporter
"""
Agnostic CSV Importer & Class-to-CSV Template Generator for Unreal Engine 5.8+.
Provides schema-driven data asset generation, blueprint synthesis, component attribute injection,
and reverse reflection to generate CSV templates and schema mappings from any UClass.
"""

import os
import csv
import sys
from pathlib import Path

# Import schema parser
current_dir = Path(__file__).resolve().parent
if str(current_dir) not in sys.path:
    sys.path.insert(0, str(current_dir))

import schema_parser

try:
    import unreal
    IN_UNREAL = True
except ImportError:
    IN_UNREAL = False


def apply_property_value(target_obj, prop_name, raw_val, prop_type="String", delimiter=","):
    """
    Reflectively sets prop_name on target_obj using prop_type type coercion.
    """
    if not target_obj or not prop_name or raw_val is None:
        return False

    val_str = str(raw_val).strip()

    try:
        if prop_type.lower() in ["string", "str"]:
            target_obj.set_editor_property(prop_name, val_str)
            return True

        elif prop_type.lower() in ["text"]:
            target_obj.set_editor_property(prop_name, unreal.Text(val_str))
            return True

        elif prop_type.lower() in ["int", "int32", "integer"]:
            target_obj.set_editor_property(prop_name, int(float(val_str if val_str else "0")))
            return True

        elif prop_type.lower() in ["float", "double"]:
            target_obj.set_editor_property(prop_name, float(val_str if val_str else "0.0"))
            return True

        elif prop_type.lower() in ["bool", "boolean"]:
            target_obj.set_editor_property(prop_name, val_str.lower() in ["true", "1", "yes"])
            return True

        elif prop_type.lower() in ["gameplaytag", "tag"]:
            if val_str and val_str.lower() != "none":
                try:
                    gt = unreal.GameplayStatics.request_gameplay_tag(unreal.Name(val_str))
                except Exception:
                    gt = unreal.GameplayTag(tag_name=unreal.Name(val_str))
                target_obj.set_editor_property(prop_name, gt)
                return True

        elif prop_type.lower() in ["gameplaytagcontainer", "tagcontainer"]:
            if val_str:
                tags = [t.strip() for t in val_str.split(delimiter) if t.strip()]
                tag_container = unreal.GameplayTagContainer()
                gt_list = []
                for t in tags:
                    try:
                        gt_list.append(unreal.GameplayStatics.request_gameplay_tag(unreal.Name(t)))
                    except Exception:
                        try:
                            gt_list.append(unreal.GameplayTag(tag_name=unreal.Name(t)))
                        except Exception:
                            pass
                tag_container.set_editor_property("gameplay_tags", gt_list)
                target_obj.set_editor_property(prop_name, tag_container)
                return True

        elif prop_type.lower() in ["softclasspath", "class"]:
            if val_str:
                loaded_cls = unreal.load_class(None, val_str)
                if loaded_cls:
                    target_obj.set_editor_property(prop_name, loaded_cls)
                    return True

        elif prop_type.lower() in ["softobjectpath", "object"]:
            if val_str:
                loaded_obj = unreal.EditorAssetLibrary.load_asset(val_str)
                if loaded_obj:
                    target_obj.set_editor_property(prop_name, loaded_obj)
                    return True

    except Exception as e:
        if IN_UNREAL:
            unreal.log_warning(f"⚠️ [UniversalImporter] Reflection assignment warning for property '{prop_name}' ({prop_type}): {e}")
        else:
            print(f"⚠️ [UniversalImporter] Reflection assignment warning for property '{prop_name}': {e}")

    return False


def synthesize_blueprint_actor(actor_class_str, parent_class_str, schema, data_asset, asset_tools, editor_asset_lib):
    """
    Synthesizes missing Blueprint actor asset if actor_class_str does not exist,
    and binds the synthesized data_asset to the CDO.
    """
    if not IN_UNREAL or not actor_class_str:
        return

    clean_path = actor_class_str.split(".")[0] if "." in actor_class_str else actor_class_str
    if clean_path.endswith("_C"):
        clean_path = clean_path[:-2]

    package_path, bp_name = os.path.split(clean_path)
    bp_asset = editor_asset_lib.load_asset(clean_path)

    if not bp_asset and schema.b_create_missing_blueprints:
        parent_path = parent_class_str if parent_class_str else schema.default_parent_class
        parent_cls = unreal.load_class(None, parent_path)
        if not parent_cls:
            parent_cls = unreal.load_class(None, "/Script/Engine.Actor")

        if parent_cls:
            if not editor_asset_lib.does_directory_exist(package_path):
                editor_asset_lib.make_directory(package_path)

            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", parent_cls)
            bp_asset = asset_tools.create_asset(bp_name, package_path, None, factory)
            if bp_asset:
                unreal.log(f"✨ [UniversalImporter] Synthesized missing Blueprint actor: '{clean_path}' (Parent: {parent_cls.get_name()})")

    if bp_asset:
        try:
            unreal.BlueprintEditorLibrary.compile_blueprint(bp_asset)
            gen_class = unreal.load_class(None, f"{clean_path}.{bp_name}_C")
            if gen_class:
                cdo = unreal.get_default_object(gen_class)
                if cdo and data_asset and schema.cdo_binding_property:
                    try:
                        cdo.set_editor_property(schema.cdo_binding_property, data_asset)
                        unreal.log(f"🔗 [UniversalImporter] Bound DataAsset to CDO property '{schema.cdo_binding_property}' on '{bp_name}'.")
                    except Exception:
                        pass

                cdo.modify()
                bp_asset.modify()
                unreal.BlueprintEditorLibrary.compile_blueprint(bp_asset)
                editor_asset_lib.save_loaded_asset(bp_asset)
        except Exception as e_bp:
            unreal.log_warning(f"⚠️ [UniversalImporter] Blueprint synthesis/CDO binding notice for '{bp_name}': {e_bp}")


def export_csv_template_from_class(class_path, output_csv_path="SampleTemplate.csv", output_schema_path="GeneratedSchema.json"):
    """
    Reflects ANY user UClass (DataAsset, Actor, or Component) and exports:
    1. A sample CSV template with headers for all editable UProperties.
    2. A matching JSON mapping schema.
    """
    if not IN_UNREAL:
        print("❌ [UniversalImporter] export_csv_template_from_class requires running inside Unreal Editor.")
        return False

    uclass = unreal.load_class(None, class_path)
    if not uclass:
        unreal.log_error(f"❌ Could not load class at path '{class_path}'")
        return False

    cdo = unreal.get_default_object(uclass)
    if not cdo:
        unreal.log_error(f"❌ Could not resolve CDO for class '{class_path}'")
        return False

    headers = ["Name", "ParentClass", "ActorClass"]
    schema_rules = [
        {"CSVColumn": "Name", "TargetProperty": "Name", "PropertyType": "String", "FallbackValue": "NewAsset"}
    ]

    # Reflect editable properties on CDO
    for prop in dir(cdo):
        if prop.startswith("_") or prop in ["Name", "ParentClass", "ActorClass"]:
            continue
        try:
            val = cdo.get_editor_property(prop)
            headers.append(prop)
            ptype = "String"
            if isinstance(val, bool):
                ptype = "Bool"
            elif isinstance(val, int):
                ptype = "Int32"
            elif isinstance(val, float):
                ptype = "Float"
            elif isinstance(val, unreal.Text):
                ptype = "Text"

            schema_rules.append({
                "CSVColumn": prop,
                "TargetProperty": prop,
                "PropertyType": ptype,
                "FallbackValue": str(val) if val is not None else ""
            })
        except Exception:
            pass

    # Save sample CSV
    os.makedirs(os.path.dirname(os.path.abspath(output_csv_path)), exist_ok=True)
    with open(output_csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        writer.writerow([f"Sample_{uclass.get_name()}_01", "/Script/Engine.Actor", ""] + [""] * (len(headers) - 3))

    # Save JSON schema
    schema_data = {
        "SchemaVersion": "1.0.0",
        "PluginName": "UniversalDataImporter",
        "TargetDataAssetClass": class_path,
        "TargetPackageDirectory": "/Game/Data/GeneratedAssets",
        "AutoBindArtAssets": True,
        "ArtSearchDirectories": ["/Game/Assets/External", "/Game/Assets"],
        "ColumnToPropertyMap": schema_rules,
        "NestedComponentPropertyMap": [],
        "BlueprintSynthesisRules": {
            "bCreateMissingBlueprints": True,
            "DefaultParentClass": "/Script/Engine.Actor",
            "ParentClassColumn": "ParentClass",
            "CDOBindingProperty": "DataAssetRef",
            "DefaultInputMappingContext": ""
        }
    }

    os.makedirs(os.path.dirname(os.path.abspath(output_schema_path)), exist_ok=True)
    import json
    with open(output_schema_path, 'w', encoding='utf-8') as f:
        json.dump(schema_data, f, indent=2)

    unreal.log(f"✨ [UniversalImporter] Generated CSV template at '{output_csv_path}' and Schema at '{output_schema_path}' for class '{uclass.get_name()}'.")
    return True


def import_csv_with_schema(csv_path, schema_path="", b_overwrite=True, b_auto_bind_art=True):
    """
    Main Agnostic Import Engine:
    Reads any CSV dataset using a JSON schema mapping, creating DataAssets and synthesizing Blueprints.
    """
    if not schema_path:
        schema_path = os.path.join(current_dir, "..", "Resources", "Schemas", "DefaultMappingSchema.json")

    schema = schema_parser.ImporterSchema.load_from_json(schema_path)

    if not csv_path or not os.path.exists(csv_path):
        msg = f"❌ CSV file not found at path: {csv_path}"
        if IN_UNREAL: unreal.log_error(msg)
        else: print(msg)
        return False

    rows = []
    with open(csv_path, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append(r)

    print(f"🏛️ [UniversalImporter] Read {len(rows)} rows from CSV '{csv_path}'")

    if not IN_UNREAL:
        print("ℹ️ Running outside Unreal Editor context. Validation complete.")
        return True

    # --- Unreal Editor Batch Import ---
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    target_dir = schema.target_package_directory
    if not editor_asset_lib.does_directory_exist(target_dir):
        editor_asset_lib.make_directory(target_dir)

    target_class = unreal.load_class(None, schema.target_data_asset_class)
    if not target_class:
        unreal.log_error(f"❌ Could not load target DataAsset class '{schema.target_data_asset_class}'")
        return False

    created_count = 0
    updated_count = 0

    for row in rows:
        asset_name = row.get("Name", row.get("ItemTag", row.get("Tag", ""))).strip().replace(".", "_")
        if not asset_name:
            continue

        asset_path = f"{target_dir}/{asset_name}"
        existing_asset = editor_asset_lib.load_asset(asset_path)

        if existing_asset:
            if not b_overwrite:
                continue
            data_asset = existing_asset
            updated_count += 1
        else:
            factory = unreal.DataAssetFactory()
            factory.set_editor_property("data_asset_class", target_class)
            data_asset = asset_tools.create_asset(asset_name, target_dir, target_class, factory)
            created_count += 1

        if not data_asset:
            continue

        # 1. Column to Property Mapping on DataAsset
        for rule in schema.column_to_property_map:
            val = row.get(rule.csv_column, rule.fallback_value)
            if val is not None:
                apply_property_value(data_asset, rule.target_property, val, rule.property_type, rule.delimiter)

        editor_asset_lib.save_loaded_asset(data_asset)

        # 2. Automated Blueprint Synthesis & CDO Binding
        actor_class_str = row.get("ActorClass", "").strip()
        parent_class_str = row.get(schema.parent_class_column, "").strip()
        if actor_class_str:
            synthesize_blueprint_actor(
                actor_class_str=actor_class_str,
                parent_class_str=parent_class_str,
                schema=schema,
                data_asset=data_asset,
                asset_tools=asset_tools,
                editor_asset_lib=editor_asset_lib
            )

    unreal.log(f"✅ [UniversalImporter] Complete! Created: {created_count} | Updated: {updated_count} assets in '{target_dir}'.")
    return True
