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
            # In Unreal Python API, set_editor_property accepts string directly for FText
            try:
                target_obj.set_editor_property(prop_name, unreal.TextLibrary.conv_string_to_text(val_str))
            except Exception:
                target_obj.set_editor_property(prop_name, val_str)
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
                gt = None
                try:
                    gt = unreal.GameplayTagsManager.get_gameplay_tags_manager().request_gameplay_tag(unreal.Name(val_str))
                except Exception:
                    try:
                        gt = unreal.GameplayTag()
                    except Exception:
                        pass
                if gt:
                    target_obj.set_editor_property(prop_name, gt)
                    return True

        elif prop_type.lower() in ["gameplaytagcontainer", "tagcontainer"]:
            if val_str:
                tags = [t.strip() for t in val_str.split(delimiter) if t.strip()]
                tag_container = unreal.GameplayTagContainer()
                for t in tags:
                    try:
                        gt = unreal.GameplayTagsManager.get_gameplay_tags_manager().request_gameplay_tag(unreal.Name(t))
                        tag_container.add_tag(gt)
                    except Exception:
                        try:
                            gt = unreal.GameplayTag()
                            tag_container.add_tag(gt)
                        except Exception:
                            pass

                target_obj.set_editor_property(prop_name, tag_container)
                return True

        elif prop_type.lower() in ["softclasspath", "class"]:
            if val_str:
                loaded_cls = unreal.load_class(None, val_str)
                if loaded_cls:
                    target_obj.set_editor_property(prop_name, loaded_cls)
                    return True

        elif prop_type.lower() in ["staticmesh", "mesh"]:
            if val_str and IN_UNREAL and unreal.EditorAssetLibrary.does_asset_exist(val_str):
                mesh_obj = unreal.EditorAssetLibrary.load_asset(val_str)
                if mesh_obj and isinstance(mesh_obj, unreal.StaticMesh):
                    target_obj.set_editor_property(prop_name, mesh_obj)
                    return True

        elif prop_type.lower() in ["skeletalmesh"]:
            if val_str and IN_UNREAL and unreal.EditorAssetLibrary.does_asset_exist(val_str):
                sk_obj = unreal.EditorAssetLibrary.load_asset(val_str)
                if sk_obj and isinstance(sk_obj, unreal.SkeletalMesh):
                    target_obj.set_editor_property(prop_name, sk_obj)
                    return True

        elif prop_type.lower() in ["material", "materialinterface"]:
            if val_str and IN_UNREAL and unreal.EditorAssetLibrary.does_asset_exist(val_str):
                mat_obj = unreal.EditorAssetLibrary.load_asset(val_str)
                if mat_obj and isinstance(mat_obj, unreal.MaterialInterface):
                    target_obj.set_editor_property(prop_name, mat_obj)
                    return True

        elif prop_type.lower() in ["texture", "texture2D"]:
            if val_str and IN_UNREAL and unreal.EditorAssetLibrary.does_asset_exist(val_str):
                tex_obj = unreal.EditorAssetLibrary.load_asset(val_str)
                if tex_obj and isinstance(tex_obj, unreal.Texture2D):
                    target_obj.set_editor_property(prop_name, tex_obj)
                    return True

        elif prop_type.lower() in ["softobjectpath", "object"]:
            if val_str and IN_UNREAL and unreal.EditorAssetLibrary.does_asset_exist(val_str):
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


def discover_and_bind_art_assets(data_asset, asset_name, schema, editor_asset_lib):
    """
    Automated Art Asset Discovery & Soft Pointer Auto-Binding.
    Scans schema.art_search_directories AND queries AssetRegistry globally for StaticMesh (SM_*),
    SkeletalMesh (SK_*), MaterialInterface (MI_* / M_*), and Texture2D (T_* / UI_*), binding soft object pointers.
    """
    if not IN_UNREAL or not data_asset or not schema.auto_bind_art_assets:
        return

    stripped_name = asset_name.replace("Sample_", "").replace("BP_", "").replace("Item_", "").replace("Species_", "").replace("Object_", "")
    clean_name = asset_name.replace(" ", "").replace("_", "")
    clean_stripped = stripped_name.replace(" ", "").replace("_", "")
    name_variants = list(dict.fromkeys([asset_name, stripped_name, clean_name, clean_stripped]))

    search_dirs = schema.art_search_directories

    # 1. Static Mesh (SM_<Variant> or <Variant>)
    b_found = False
    for sdir in search_dirs:
        if b_found: break
        for name_var in name_variants:
            if b_found: break
            for prefix in ["SM_", ""]:
                if b_found: break
                sm_path = f"{sdir}/{prefix}{name_var}"
                if editor_asset_lib.does_asset_exist(sm_path):
                    sm_asset = editor_asset_lib.load_asset(sm_path)
                    if sm_asset and isinstance(sm_asset, unreal.StaticMesh):
                        for prop in ["mesh", "static_mesh", "stage_mesh"]:
                            try:
                                data_asset.set_editor_property(prop, sm_asset)
                                unreal.log(f"🎨 [UniversalImporter Auto-Art] Bound StaticMesh '{sm_path}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass

    # AssetRegistry Global Search Fallback for StaticMesh
    if not b_found:
        try:
            ar = unreal.AssetRegistryHelpers.get_asset_registry()
            try:
                class_path = unreal.TopLevelAssetPath("/Script/Engine", "StaticMesh")
                assets = ar.get_assets_by_class(class_path, True)
            except Exception:
                try:
                    assets = ar.get_assets_by_class_name(unreal.Name("StaticMesh"), True)
                except Exception:
                    assets = ar.get_assets_by_class(unreal.Name("StaticMesh"), True)

            for asset_data in assets:
                aname = str(asset_data.asset_name)
                if any(aname == f"SM_{v}" or aname == v for v in name_variants):
                    sm_obj = editor_asset_lib.load_asset(str(asset_data.package_name))
                    if sm_obj and isinstance(sm_obj, unreal.StaticMesh):
                        for prop in ["mesh", "static_mesh", "stage_mesh"]:
                            try:
                                data_asset.set_editor_property(prop, sm_obj)
                                unreal.log(f"🎨 [UniversalImporter AssetRegistry] Found & Bound StaticMesh '{asset_data.package_name}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass
                        if b_found: break
        except Exception as e_ar:
            pass

    # 2. Skeletal Mesh (SK_<Variant> or <Variant>)
    b_found = False
    for sdir in search_dirs:
        if b_found: break
        for name_var in name_variants:
            if b_found: break
            for prefix in ["SK_", ""]:
                if b_found: break
                sk_path = f"{sdir}/{prefix}{name_var}"
                if editor_asset_lib.does_asset_exist(sk_path):
                    sk_asset = editor_asset_lib.load_asset(sk_path)
                    if sk_asset and isinstance(sk_asset, unreal.SkeletalMesh):
                        for prop in ["skeletal_mesh", "stage_skeletal_mesh"]:
                            try:
                                data_asset.set_editor_property(prop, sk_asset)
                                unreal.log(f"🎨 [UniversalImporter Auto-Art] Bound SkeletalMesh '{sk_path}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass

    # 3. Material (MI_<Variant> or M_<Variant> or <Variant>)
    b_found = False
    for sdir in search_dirs:
        if b_found: break
        for name_var in name_variants:
            if b_found: break
            for prefix in ["MI_", "M_", ""]:
                if b_found: break
                mat_path = f"{sdir}/{prefix}{name_var}"
                if editor_asset_lib.does_asset_exist(mat_path):
                    mat_asset = editor_asset_lib.load_asset(mat_path)
                    if mat_asset and isinstance(mat_asset, unreal.MaterialInterface):
                        for prop in ["material", "species_material", "stage_material"]:
                            try:
                                data_asset.set_editor_property(prop, mat_asset)
                                unreal.log(f"🎨 [UniversalImporter Auto-Art] Bound Material '{mat_path}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass

    # AssetRegistry Global Search Fallback for Material
    if not b_found:
        try:
            ar = unreal.AssetRegistryHelpers.get_asset_registry()
            try:
                class_path = unreal.TopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant")
                assets = ar.get_assets_by_class(class_path, True)
                if not assets:
                    class_path = unreal.TopLevelAssetPath("/Script/Engine", "Material")
                    assets = ar.get_assets_by_class(class_path, True)
            except Exception:
                try:
                    assets = ar.get_assets_by_class_name(unreal.Name("MaterialInstanceConstant"), True)
                except Exception:
                    assets = ar.get_assets_by_class(unreal.Name("MaterialInstanceConstant"), True)

            for asset_data in assets:
                aname = str(asset_data.asset_name)
                if any(aname == f"MI_{v}" or aname == f"M_{v}" or aname == v for v in name_variants):
                    mat_obj = editor_asset_lib.load_asset(str(asset_data.package_name))
                    if mat_obj and isinstance(mat_obj, unreal.MaterialInterface):
                        for prop in ["material", "species_material", "stage_material"]:
                            try:
                                data_asset.set_editor_property(prop, mat_obj)
                                unreal.log(f"🎨 [UniversalImporter AssetRegistry] Found & Bound Material '{asset_data.package_name}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass
                        if b_found: break
        except Exception:
            pass

    # 4. Texture / Icon (T_<Variant> or UI_<Variant> or <Variant>)
    b_found = False
    for sdir in search_dirs:
        if b_found: break
        for name_var in name_variants:
            if b_found: break
            for prefix in ["T_", "UI_", ""]:
                if b_found: break
                tex_path = f"{sdir}/{prefix}{name_var}"
                if editor_asset_lib.does_asset_exist(tex_path):
                    tex_asset = editor_asset_lib.load_asset(tex_path)
                    if tex_asset and isinstance(tex_asset, unreal.Texture2D):
                        for prop in ["icon", "texture", "icon_texture"]:
                            try:
                                data_asset.set_editor_property(prop, tex_asset)
                                unreal.log(f"🎨 [UniversalImporter Auto-Art] Bound Texture2D '{tex_path}' to property '{prop}' on '{asset_name}'")
                                b_found = True
                                break
                            except Exception:
                                pass


def apply_component_properties(cdo, row, schema):
    """
    Reflectively traverses attached components on CDO and applies matching ComponentPropertyRules.
    """
    if not IN_UNREAL or not cdo or not schema.nested_component_property_map:
        return

    for rule in schema.nested_component_property_map:
        val = row.get(rule.csv_column, rule.fallback_value)
        if val is None:
            continue

        comp_cls = unreal.load_class(None, rule.component_class) if rule.component_class else None
        if not comp_cls:
            comp_cls = unreal.ActorComponent

        try:
            comps = cdo.get_components_by_class(comp_cls)
            for comp in comps:
                apply_property_value(comp, rule.target_property, val, rule.property_type)
        except Exception:
            pass


def synthesize_blueprint_actor(actor_class_str, parent_class_str, schema, data_asset, row, asset_tools, editor_asset_lib):
    """
    Synthesizes missing Blueprint actor asset if actor_class_str does not exist,
    binds the synthesized data_asset to the CDO, and injects component properties.
    """
    if not IN_UNREAL or not actor_class_str:
        return

    clean_path = actor_class_str.split(".")[0] if "." in actor_class_str else actor_class_str
    if clean_path.endswith("_C"):
        clean_path = clean_path[:-2]

    package_path, bp_name = os.path.split(clean_path)
    bp_asset = editor_asset_lib.load_asset(clean_path) if editor_asset_lib.does_asset_exist(clean_path) else None

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
                if cdo:
                    if data_asset and schema.cdo_binding_property:
                        for prop in [schema.cdo_binding_property, "data_asset", "data_asset_ref", "species_data"]:
                            try:
                                cdo.set_editor_property(prop, data_asset)
                                unreal.log(f"🔗 [UniversalImporter] Bound DataAsset to CDO property '{prop}' on '{bp_name}'.")
                                break
                            except Exception:
                                pass

                    # Injective Component Attribute Reflection
                    apply_component_properties(cdo, row, schema)

                cdo.modify()
                bp_asset.modify()
                unreal.BlueprintEditorLibrary.compile_blueprint(bp_asset)
                editor_asset_lib.save_loaded_asset(bp_asset)
        except Exception as e_bp:
            unreal.log_warning(f"⚠️ [UniversalImporter] Blueprint synthesis/CDO binding notice for '{bp_name}': {e_bp}")


def export_csv_template_from_class(class_path="/Script/UniversalDataImporter.UniversalDataAsset", output_csv_path="", output_schema_path=""):
    """
    Reflects ANY user UClass (DataAsset, Actor, or Component) and exports:
    1. A sample CSV template with headers for all editable UProperties.
    2. A matching JSON mapping schema.
    """
    if not IN_UNREAL:
        print("❌ [UniversalImporter] export_csv_template_from_class requires running inside Unreal Editor.")
        return False

    if not class_path or not str(class_path).strip():
        class_path = "/Script/UniversalDataImporter.UniversalDataAsset"

    uclass = unreal.load_class(None, class_path)
    if not uclass:
        # Fall back to standard DataAsset if plugin module class isn't loaded yet
        unreal.log_warning(f"⚠️ Could not load class '{class_path}', falling back to '/Script/Engine.DataAsset'.")
        class_path = "/Script/Engine.DataAsset"
        uclass = unreal.load_class(None, class_path)

    if not uclass:
        unreal.log_error(f"❌ Could not load class at path '{class_path}'")
        return False

    if not output_csv_path or not str(output_csv_path).strip():
        output_csv_path = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "SampleTemplates", "GeneratedTemplate.csv"))

    if not output_schema_path or not str(output_schema_path).strip():
        output_schema_path = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "Schemas", "GeneratedSchema.json"))

    output_csv_path = str(output_csv_path).strip().strip("'").strip('"').replace("\\", "/")
    output_schema_path = str(output_schema_path).strip().strip("'").strip('"').replace("\\", "/")

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
        "ArtSearchDirectories": [
            "/Game/Art/Meshes",
            "/Game/Art/SkeletalMeshes",
            "/Game/Art/Materials",
            "/Game/Art/Textures",
            "/Game/Art",
            "/UniversalDataImporter/Art/Meshes",
            "/UniversalDataImporter/Art/Materials",
            "/UniversalDataImporter/Art/Textures",
            "/Game/Assets/External",
            "/Game/Assets"
        ],
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


def import_csv_with_schema(csv_path="", schema_path="", b_overwrite=True, b_auto_bind_art=True):
    """
    Main Agnostic Import Engine:
    Reads any CSV dataset using a JSON schema mapping, creating DataAssets and synthesizing Blueprints.
    """
    # Auto-resolve CSV path if empty or invalid
    if not csv_path or not str(csv_path).strip() or not os.path.exists(str(csv_path).strip()):
        gen_csv = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "SampleTemplates", "GeneratedTemplate.csv"))
        sample_csv = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "SampleTemplates", "SampleSpeciesTemplate.csv"))
        if os.path.exists(gen_csv):
            csv_path = gen_csv
        elif os.path.exists(sample_csv):
            csv_path = sample_csv

    # Auto-resolve Schema path if empty or invalid
    if not schema_path or not str(schema_path).strip() or not os.path.exists(str(schema_path).strip()):
        gen_schema = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "Schemas", "GeneratedSchema.json"))
        default_schema = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "Schemas", "DefaultMappingSchema.json"))
        if os.path.exists(gen_schema):
            schema_path = gen_schema
        elif os.path.exists(default_schema):
            schema_path = default_schema

    schema = schema_parser.ImporterSchema.load_from_json(schema_path)

    csv_path = str(csv_path).strip().strip("'").strip('"').replace("\\", "/")
    if not csv_path or not os.path.exists(csv_path):
        msg = f"❌ CSV file not found at path: '{csv_path}'. Run export_csv_template() first or pass a valid CSV path."
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
        existing_asset = editor_asset_lib.load_asset(asset_path) if editor_asset_lib.does_asset_exist(asset_path) else None

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

        # 1b. Auto-bind Mesh, Material, and Texture Soft Pointers
        if b_auto_bind_art and schema.auto_bind_art_assets:
            discover_and_bind_art_assets(
                data_asset=data_asset,
                asset_name=asset_name,
                schema=schema,
                editor_asset_lib=editor_asset_lib
            )

        editor_asset_lib.save_loaded_asset(data_asset)

        # 2. Automated Blueprint Synthesis & CDO Binding & Component Attribute Injection
        actor_class_str = row.get("ActorClass", "").strip()
        parent_class_str = row.get(schema.parent_class_column, "").strip()
        if actor_class_str:
            synthesize_blueprint_actor(
                actor_class_str=actor_class_str,
                parent_class_str=parent_class_str,
                schema=schema,
                data_asset=data_asset,
                row=row,
                asset_tools=asset_tools,
                editor_asset_lib=editor_asset_lib
            )

    unreal.log(f"✅ [UniversalImporter] Complete! Created: {created_count} | Updated: {updated_count} assets in '{target_dir}'.")
    return True
