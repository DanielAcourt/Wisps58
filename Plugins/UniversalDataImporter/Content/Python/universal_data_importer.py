# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer: Agnostic CSV Importer & Class-to-CSV Template Generator
"""
Agnostic CSV Importer & Class-to-CSV Template Generator for Unreal Engine 5.8+.
Provides schema-driven data asset generation, blueprint synthesis, component attribute injection,
and reverse reflection to generate CSV templates and schema mappings from any UClass.
"""

import os
import csv
import sys
import json
from pathlib import Path

current_dir = Path(__file__).resolve().parent
if str(current_dir) not in sys.path:
    sys.path.insert(0, str(current_dir))

import schema_parser

try:
    import unreal
    IN_UNREAL = True
except ImportError:
    IN_UNREAL = False


def _split_delimited_items(raw_val, delimiter=";"):
    """Splits a delimited string or sequence into clean non-empty items."""
    if raw_val is None:
        return []

    if isinstance(raw_val, (list, tuple, set)):
        items = [str(x).strip() for x in raw_val]
        return [x for x in items if x and x.lower() not in {"none", "null", "nan"}]

    text = str(raw_val).strip()
    if not text or text.lower() in {"none", "null", "nan"}:
        return []

    sep = delimiter if delimiter else ";"
    if sep not in text and ";" in text:
        sep = ";"
    elif sep not in text and "," in text:
        sep = ","

    return [part.strip() for part in text.split(sep) if part.strip()]


def _resolve_enum_value(enum_class_path, raw_val):
    """Resolves a string/alias against an Unreal Python enum class (returning an enum entry, never a raw str)."""
    if raw_val is None:
        return None

    clean_val = str(raw_val).strip()
    if not clean_val:
        return None

    if "::" in clean_val:
        clean_val = clean_val.split("::")[-1].strip()

    normalized_upper = (
        clean_val.replace("-", "_")
        .replace(" ", "_")
        .replace(".", "_")
        .upper()
    )

    py_enum_cls = None
    if IN_UNREAL and enum_class_path:
        enum_short_name = enum_class_path.split(".")[-1].strip()
        # Unreal Python strips leading 'E' from C++ UENUM names (e.g. ESovereignElement -> SovereignElement)
        stripped_e = (
            enum_short_name[1:]
            if len(enum_short_name) > 1 and enum_short_name.startswith("E") and enum_short_name[1].isupper()
            else enum_short_name
        )
        for cls_candidate in (enum_short_name, stripped_e):
            py_enum_cls = getattr(unreal, cls_candidate, None)
            if py_enum_cls is not None:
                break

    if py_enum_cls is not None:
        aliases = {
            normalized_upper,
            clean_val.upper(),
            "SKIRMISHER_SNIPER" if normalized_upper in {"SKIRMISHER", "RANGED", "SNIPER", "ARCHER"} else normalized_upper,
            "AGGRESSIVE_BRUTE" if normalized_upper in {"MELEE", "TANK", "BRUTE", "CASTER", "MAGE", "SWORD", "KNIGHT", "SPEAR"} else normalized_upper,
            "SUPPORT_HEALER" if normalized_upper in {"SUPPORT", "HEALER"} else normalized_upper,
            "TACTICAL_LEADER" if normalized_upper in {"LEADER", "COMMANDER"} else normalized_upper,
            "NONE" if normalized_upper in {"PHYSICAL", "NORMAL", "NULL", "EMPTY"} else normalized_upper,
            "ELECTRIC" if normalized_upper in {"LIGHTNING", "THUNDER", "VOLT"} else normalized_upper,
            "AIR" if normalized_upper in {"WIND", "GALE", "STORM"} else normalized_upper,
        }

        reserved = {"cast", "get_display_name", "name", "static_enum", "value"}
        members = [
            attr for attr in dir(py_enum_cls)
            if not attr.startswith("_")
            and not attr.endswith("_MAX")
            and attr not in reserved
        ]

        for cand in aliases:
            if cand in members:
                return getattr(py_enum_cls, cand)

        for cand in aliases:
            if len(cand) >= 3:
                for m in members:
                    if m.startswith(cand) or cand in m:
                        return getattr(py_enum_cls, m)

        for fallback_name in ("DEFAULT", "NONE"):
            if fallback_name in members:
                return getattr(py_enum_cls, fallback_name)

    return None

def _resolve_skill_soft_refs(raw_val, delimiter=";", search_paths=None):
    """Resolves semicolon/comma-separated Skill GameplayTags or asset names into SovereignSkillDefinition assets."""
    if not IN_UNREAL or not raw_val:
        return []

    if not search_paths:
        search_paths = ["/Game/Data/GeneratedCombat/Skills", "/Game/Data", "/Game"]

    tokens = _split_delimited_items(raw_val, delimiter=delimiter)
    if not tokens:
        return []

    skill_assets = []
    try:
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        for spath in search_paths:
            if unreal.EditorAssetLibrary.does_directory_exist(spath):
                assets = ar.get_assets_by_path(unreal.Name(spath), recursive=True)
                for ad in assets:
                    obj = unreal.EditorAssetLibrary.load_asset(str(ad.package_name))
                    if obj and obj.get_class().get_name() == "SovereignSkillDefinition":
                        skill_assets.append(obj)
    except Exception:
        pass

    resolved = []
    for token in tokens:
        matched_asset = None
        for sk in skill_assets:
            try:
                sk_tag = sk.get_editor_property("skill_tag")
                if str(sk_tag.tag_name) == token or str(sk.get_name()).lower() == token.lower():
                    matched_asset = sk
                    break
            except Exception:
                pass

        if not matched_asset:
            sanitized = token.replace(".", "_")
            candidates = [token, sanitized, f"DA_{sanitized}"]
            for spath in search_paths:
                for cand in candidates:
                    full_path = f"{spath}/{cand}"
                    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
                        matched_asset = unreal.EditorAssetLibrary.load_asset(full_path)
                        break
                if matched_asset:
                    break

        if matched_asset:
            resolved.append(matched_asset)
        else:
            unreal.log_warning(f"⚠️ [UniversalImporter] Could not resolve SovereignSkillDefinition for '{token}'.")

    return resolved


def _build_encounter_slots(raw_val, delimiter=";", enemy_dir="/Game/Data/GeneratedCombat/Enemies"):
    """Parses 'SlotIndex:EnemyAssetName' entries separated by delimiter into FSovereignEncounterSlot structs."""
    if not IN_UNREAL or not raw_val:
        return []

    slots_out = []
    entries = _split_delimited_items(raw_val, delimiter=delimiter)

    for entry in entries:
        if ":" not in entry:
            unreal.log_warning(f"⚠️ [UniversalImporter] Malformed EnemySlots entry '{entry}' (expected 'SlotIndex:EnemyAssetName').")
            continue

        slot_str, enemy_name = entry.split(":", 1)
        slot_str = slot_str.strip()
        enemy_name = enemy_name.strip().replace(".", "_")

        try:
            slot_idx = int(slot_str)
        except ValueError:
            unreal.log_warning(f"⚠️ [UniversalImporter] Invalid slot index '{slot_str}' in '{entry}'.")
            continue

        enemy_path = f"{enemy_dir}/{enemy_name}"
        enemy_asset = None
        if unreal.EditorAssetLibrary.does_asset_exist(enemy_path):
            enemy_asset = unreal.EditorAssetLibrary.load_asset(enemy_path)
        else:
            unreal.log_warning(f"⚠️ [UniversalImporter] Enemy asset not found at '{enemy_path}' for slot {slot_idx}.")

        slot_struct = unreal.SovereignEncounterSlot()
        slot_struct.set_editor_property("slot_index", slot_idx)
        if enemy_asset:
            slot_struct.set_editor_property("enemy_definition", enemy_asset)

        slots_out.append(slot_struct)

    return slots_out


def apply_property_value(target_obj, prop_name, raw_val, prop_type="String", delimiter=";", extra_meta=None):
    """
    Reflectively sets prop_name on target_obj using prop_type type coercion.
    """
    if not target_obj or not prop_name or raw_val is None:
        return False

    val_str = str(raw_val).strip()
    extra_meta = extra_meta or {}

    try:
        if prop_type.lower() in ["string", "str"]:
            target_obj.set_editor_property(prop_name, val_str)
            return True

        elif prop_type.lower() in ["text"]:
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

        elif prop_type.lower() in ["enum"]:
            enum_path = extra_meta.get("EnumClass", "")
            enum_val = _resolve_enum_value(enum_path, val_str)
            if enum_val is not None:
                target_obj.set_editor_property(prop_name, enum_val)
                return True
            else:
                unreal.log_warning(f"⚠️ [UniversalImporter] Could not resolve enum '{val_str}' ({enum_path}) for '{prop_name}'.")

        elif prop_type.lower() in ["skillarraybytag", "skillarray"]:
            search_paths = extra_meta.get("SkillSearchPaths", None)
            skill_refs = _resolve_skill_soft_refs(val_str, delimiter=delimiter, search_paths=search_paths)
            target_obj.set_editor_property(prop_name, skill_refs)
            return True

        elif prop_type.lower() in ["encounterslots"]:
            enemy_dir = extra_meta.get("EnemyAssetDirectory", "/Game/Data/GeneratedCombat/Enemies")
            slot_structs = _build_encounter_slots(val_str, delimiter=delimiter, enemy_dir=enemy_dir)
            target_obj.set_editor_property(prop_name, slot_structs)
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
                tags = _split_delimited_items(val_str, delimiter=delimiter)
                gt_list = []
                for t in tags:
                    try:
                        gt = unreal.GameplayTagsManager.get_gameplay_tags_manager().request_gameplay_tag(unreal.Name(t))
                        gt_list.append(gt)
                    except Exception:
                        try:
                            gt = unreal.GameplayTag()
                            gt_list.append(gt)
                        except Exception:
                            pass

                tag_container = unreal.GameplayTagContainer(gameplay_tags=gt_list)
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

    b_found = False
    for sdir in search_dirs:
        if b_found:
            break
        for name_var in name_variants:
            if b_found:
                break
            for prefix in ["SM_", ""]:
                if b_found:
                    break
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
                        if b_found:
                            break
        except Exception:
            pass

    b_found = False
    for sdir in search_dirs:
        if b_found:
            break
        for name_var in name_variants:
            if b_found:
                break
            for prefix in ["SK_", ""]:
                if b_found:
                    break
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

    b_found = False
    prefixes = ["MI_", "M_", "mi_", "m_", ""]
    suffixes = ["", "_MI", "_mi", "_Mat", "_mat", "_Material", "_material"]
    for sdir in search_dirs:
        if b_found:
            break
        for name_var in name_variants:
            if b_found:
                break
            for p in prefixes:
                if b_found:
                    break
                for s in suffixes:
                    if b_found:
                        break
                    mat_path = f"{sdir}/{p}{name_var}{s}"
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

    if not b_found:
        try:
            ar = unreal.AssetRegistryHelpers.get_asset_registry()
            assets = []
            for cls_name in ["MaterialInstanceConstant", "MaterialInstanceDynamic", "Material"]:
                try:
                    class_path = unreal.TopLevelAssetPath("/Script/Engine", cls_name)
                    res = ar.get_assets_by_class(class_path, True)
                    if res:
                        assets.extend(res)
                except Exception:
                    try:
                        res = ar.get_assets_by_class_name(unreal.Name(cls_name), True)
                        if res:
                            assets.extend(res)
                    except Exception:
                        pass

            for asset_data in assets:
                aname = str(asset_data.asset_name).lower()
                for v in name_variants:
                    v_lower = v.lower()
                    if (
                        aname == v_lower
                        or aname == f"mi_{v_lower}"
                        or aname == f"m_{v_lower}"
                        or aname == f"{v_lower}_mi"
                        or aname == f"{v_lower}_mat"
                        or aname == f"{v_lower}_material"
                    ):
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
                            if b_found:
                                break
                if b_found:
                    break
        except Exception:
            pass

    b_found = False
    for sdir in search_dirs:
        if b_found:
            break
        for name_var in name_variants:
            if b_found:
                break
            for prefix in ["T_", "UI_", ""]:
                if b_found:
                    break
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
        {"CSVColumn": "Name", "TargetProperty": "Name", "PropertyType": "String", "FallbackValue": "SampleAsset"},
        {"CSVColumn": "ParentClass", "TargetProperty": "ParentClass", "PropertyType": "String", "FallbackValue": ""},
        {"CSVColumn": "ActorClass", "TargetProperty": "ActorClass", "PropertyType": "String", "FallbackValue": ""},
    ]

    sample_row = {
        "Name": "Sample_Item_01",
        "ParentClass": "",
        "ActorClass": "",
    }

    for prop_name in dir(cdo):
        if prop_name.startswith("_") or prop_name in {"name", "outer", "class", "parent_class", "actor_class"}:
            continue
        try:
            val = cdo.get_editor_property(prop_name)
        except Exception:
            continue

        if callable(val):
            continue

        if isinstance(val, bool):
            ptype = "Bool"
            sample_val = "False"
        elif isinstance(val, int):
            ptype = "Int"
            sample_val = str(val)
        elif isinstance(val, float):
            ptype = "Float"
            sample_val = str(val)
        else:
            ptype = "String"
            sample_val = ""

        headers.append(prop_name)
        schema_rules.append({
            "CSVColumn": prop_name,
            "TargetProperty": prop_name,
            "PropertyType": ptype,
            "FallbackValue": sample_val,
        })
        sample_row[prop_name] = sample_val

    os.makedirs(os.path.dirname(output_csv_path), exist_ok=True)
    os.makedirs(os.path.dirname(output_schema_path), exist_ok=True)

    with open(output_csv_path, "w", newline="", encoding="utf-8") as f_csv:
        writer = csv.DictWriter(f_csv, fieldnames=headers)
        writer.writeheader()
        writer.writerow(sample_row)

    schema_obj = {
        "SchemaVersion": "1.0.0",
        "PluginName": "UniversalDataImporter",
        "TargetDataAssetClass": class_path,
        "TargetPackageDirectory": "/Game/Data/GeneratedAssets",
        "AutoBindArtAssets": True,
        "ArtSearchDirectories": ["/Game/Assets/External", "/Game/ArtVault", "/Game/Assets"],
        "ColumnToPropertyMap": schema_rules,
        "NestedComponentPropertyMap": [],
        "BlueprintSynthesisRules": {
            "bCreateMissingBlueprints": False,
            "DefaultParentClass": "/Script/Engine.Actor",
            "ParentClassColumn": "ParentClass",
            "CDOBindingProperty": "DataAssetRef",
            "DefaultInputMappingContext": "",
        },
    }

    with open(output_schema_path, "w", encoding="utf-8") as f_json:
        json.dump(schema_obj, f_json, indent=2)

    unreal.log(f"✅ [UniversalImporter] Exported template CSV '{output_csv_path}' and schema '{output_schema_path}' from '{class_path}'.")
    return True


def import_csv_with_schema(csv_path="", schema_path="", b_overwrite=True, b_auto_bind_art=True):
    """
    Imports rows from a CSV file using a JSON schema and creates/updates DataAsset packages.
    """
    if not IN_UNREAL:
        print("❌ [UniversalImporter] import_csv_with_schema requires running inside Unreal Editor.")
        return False

    default_schema = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "Schemas", "DefaultMappingSchema.json"))
    default_csv = os.path.abspath(os.path.join(current_dir, "..", "..", "Resources", "SampleTemplates", "SampleSpeciesTemplate.csv"))

    actual_schema_path = schema_path if (schema_path and os.path.exists(str(schema_path))) else default_schema
    schema = schema_parser.ImporterSchema.load_from_json(actual_schema_path)
    schema.auto_bind_art_assets = bool(b_auto_bind_art)

    actual_csv_path = csv_path if (csv_path and os.path.exists(str(csv_path))) else default_csv
    if not os.path.exists(actual_csv_path):
        unreal.log_error(f"❌ [UniversalImporter] CSV file not found at '{actual_csv_path}'.")
        return False

    with open(actual_csv_path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    unreal.log(f"🏛️ [UniversalImporter] Read {len(rows)} rows from CSV '{actual_csv_path}'")

    target_cls = unreal.load_class(None, schema.target_data_asset_class)
    if not target_cls:
        unreal.log_error(f"❌ Could not load target DataAsset class '{schema.target_data_asset_class}'")
        return False

    out_dir = schema.target_package_directory.rstrip("/")
    editor_asset_lib = unreal.EditorAssetLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    if not editor_asset_lib.does_directory_exist(out_dir):
        editor_asset_lib.make_directory(out_dir)

    created_count = 0
    updated_count = 0
    saved_packages = []

    for idx, row in enumerate(rows):
        raw_name = (row.get("Name") or row.get("RowName") or f"GeneratedAsset_{idx:02d}").strip()
        asset_name = "".join(ch if (ch.isalnum() or ch == "_") else "_" for ch in raw_name).strip("_") or f"GeneratedAsset_{idx:02d}"
        package_path = f"{out_dir}/{asset_name}"

        asset_existed = editor_asset_lib.does_asset_exist(package_path)
        if asset_existed and not b_overwrite:
            continue

        if asset_existed:
            data_asset = editor_asset_lib.load_asset(package_path)
            if data_asset and data_asset.get_class() != target_cls:
                editor_asset_lib.delete_asset(package_path)
                data_asset = None
                asset_existed = False
        else:
            data_asset = None

        if not data_asset:
            data_asset = asset_tools.create_asset(asset_name, out_dir, target_cls, None)

        if not data_asset:
            unreal.log_warning(f"⚠️ [UniversalImporter] Failed to create or load DataAsset '{package_path}'.")
            continue

        for rule in schema.column_to_property_map:
            if rule.target_property.lower() in {"name", "rowname"}:
                continue
            raw_val = row.get(rule.csv_column, None)
            if raw_val is None or str(raw_val).strip() == "":
                raw_val = rule.fallback_value
            if raw_val is not None and str(raw_val).strip() != "":
                apply_property_value(
                    target_obj=data_asset,
                    prop_name=rule.target_property,
                    raw_val=raw_val,
                    prop_type=rule.property_type,
                    delimiter=rule.delimiter,
                    extra_meta=rule.extra_meta,
                )

        if schema.auto_bind_art_assets:
            discover_and_bind_art_assets(data_asset, asset_name, schema, editor_asset_lib)

        actor_class_val = (row.get("ActorClass") or "").strip()
        parent_class_val = (row.get(schema.parent_class_column) or schema.default_parent_class).strip()
        if actor_class_val:
            synthesize_blueprint_actor(
                actor_class_str=actor_class_val,
                parent_class_str=parent_class_val,
                schema=schema,
                data_asset=data_asset,
                row=row,
                asset_tools=asset_tools,
                editor_asset_lib=editor_asset_lib,
            )

        data_asset.modify()
        editor_asset_lib.save_loaded_asset(data_asset)
        saved_packages.append(package_path)

        if asset_existed:
            updated_count += 1
        else:
            created_count += 1

    unreal.log(f"✅ [UniversalImporter] Complete! Created: {created_count} | Updated: {updated_count} assets in '{out_dir}'.")
    return True