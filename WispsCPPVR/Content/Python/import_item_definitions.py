# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-07
"""
Automated Sovereign Item Definition & Fragment Synthesizer (B-070)
Reads DT_SovereignSpeciesTemplate.csv and auto-generates USovereignItemDefinition
Primary DataAssets with composable fragments in /Game/Data/ItemAssets/.
Supports both in-engine Unreal Engine execution and standalone offline auditing.
"""

import os
import csv
import sys
import argparse

try:
    import unreal
    IN_ENGINE = True
except ImportError:
    IN_ENGINE = False


def sanitize_asset_name(name_str):
    """Generates a clean asset name prefixed with DA_"""
    clean_name = name_str.replace("Species_", "").replace("Object_", "")
    return f"DA_{clean_name}"


def resolve_csv_path(csv_path="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv"):
    """
    Robustly resolves the path to the CSV file across different working directories
    and Unreal Engine execution environments.
    """
    if not csv_path:
        csv_path = "AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv"

    csv_path = csv_path.replace("\\", "/").strip("\"'")

    # 1. Direct path exists
    if os.path.exists(csv_path):
        return os.path.abspath(csv_path)

    script_dir = os.path.dirname(os.path.abspath(__file__))

    # 2. Candidate: relative to repo root derived from script location
    # Script is in <RepoRoot>/WispsCPPVR/Content/Python/
    repo_root = os.path.abspath(os.path.join(script_dir, "..", "..", ".."))
    candidate_repo = os.path.join(repo_root, csv_path)
    if os.path.exists(candidate_repo):
        return candidate_repo

    # 3. Candidate: relative to unreal project dir
    if IN_ENGINE:
        try:
            proj_dir = unreal.Paths.project_dir()
            candidate_ue_proj = os.path.abspath(os.path.join(proj_dir, "..", csv_path))
            if os.path.exists(candidate_ue_proj):
                return candidate_ue_proj
            candidate_ue_direct = os.path.abspath(os.path.join(proj_dir, csv_path))
            if os.path.exists(candidate_ue_direct):
                return candidate_ue_direct
        except Exception:
            pass

    # 4. Fallback search by filename
    base_name = os.path.basename(csv_path)
    for search_root in [repo_root, script_dir]:
        for root, _, files in os.walk(search_root):
            if base_name in files:
                return os.path.join(root, base_name)

    return csv_path


def parse_csv_item_rows(csv_path):
    """Parses CSV file and extracts item configuration records."""
    resolved_path = resolve_csv_path(csv_path)
    if not os.path.exists(resolved_path):
        raise FileNotFoundError(f"CSV file not found: {csv_path} (Resolved: {resolved_path})")

    records = []
    with open(resolved_path, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            raw_name = row.get("Name", "").strip()
            if not raw_name:
                continue

            item_tag = row.get("ItemTag", "").strip()
            display_name = row.get("DisplayName", "").strip()
            weight = float(row.get("Weight", "1.0") or "1.0")
            max_hunger = float(row.get("MaxHunger", "0.0") or "0.0")
            max_hydration = float(row.get("MaxHydration", "0.0") or "0.0")
            qi_give = float(row.get("QiGive", "0.0") or "0.0")
            qi_take = float(row.get("QiTake", "0.0") or "0.0")
            qi_radius = float(row.get("QiAuraRadius", "0.0") or "0.0")

            str_xp = float(row.get("StrengthXP", "0.0") or "0.0")
            dex_xp = float(row.get("DexterityXP", "0.0") or "0.0")
            con_xp = float(row.get("ConstitutionXP", "0.0") or "0.0")
            int_xp = float(row.get("IntelligenceXP", "0.0") or "0.0")
            wis_xp = float(row.get("WisdomXP", "0.0") or "0.0")
            cha_xp = float(row.get("CharismaXP", "0.0") or "0.0")
            lck_xp = float(row.get("LuckXP", "0.0") or "0.0")

            record = {
                "RawName": raw_name,
                "AssetName": sanitize_asset_name(raw_name),
                "ItemTag": item_tag,
                "DisplayName": display_name,
                "Weight": weight,
                "MaxHunger": max_hunger,
                "MaxHydration": max_hydration,
                "QiGive": qi_give,
                "QiTake": qi_take,
                "QiAuraRadius": qi_radius,
                "StrengthXP": str_xp,
                "DexterityXP": dex_xp,
                "ConstitutionXP": con_xp,
                "IntelligenceXP": int_xp,
                "WisdomXP": wis_xp,
                "CharismaXP": cha_xp,
                "LuckXP": lck_xp,
                "Traits": row.get("SpeciesTraits", "").strip()
            }
            records.append(record)

    return records


def determine_required_fragments(record):
    """
    Evaluates item record attributes and returns list of required fragment types and payloads.
    Decision Matrix:
    - UIData: Always attached.
    - Weight: Attached if Weight > 0.
    - Consumable: Attached if MaxHunger > 0, MaxHydration > 0, or any XP > 0.
    - Spiritual: Attached if QiGive > 0 or QiTake > 0.
    """
    fragments = []

    # 1. UI Data Fragment
    fragments.append({
        "Type": "USovereignItemFragment_UIData",
        "DisplayName": record["DisplayName"],
        "Description": f"{record['DisplayName']} ({record['ItemTag']})"
    })

    # 2. Weight Fragment
    if record["Weight"] > 0.0:
        fragments.append({
            "Type": "USovereignItemFragment_Weight",
            "UnitWeight": record["Weight"]
        })

    # 3. Consumable Fragment
    has_metabolic = record["MaxHunger"] > 0.0 or record["MaxHydration"] > 0.0
    has_xp = (record["StrengthXP"] > 0.0 or record["DexterityXP"] > 0.0 or
              record["ConstitutionXP"] > 0.0 or record["IntelligenceXP"] > 0.0 or
              record["WisdomXP"] > 0.0 or record["CharismaXP"] > 0.0 or record["LuckXP"] > 0.0)

    if has_metabolic or has_xp:
        fragments.append({
            "Type": "USovereignItemFragment_Consumable",
            "HungerRestore": record["MaxHunger"],
            "HydrationRestore": record["MaxHydration"],
            "StrengthXP": record["StrengthXP"],
            "DexterityXP": record["DexterityXP"],
            "ConstitutionXP": record["ConstitutionXP"],
            "IntelligenceXP": record["IntelligenceXP"],
            "WisdomXP": record["WisdomXP"],
            "CharismaXP": record["CharismaXP"],
            "LuckXP": record["LuckXP"]
        })

    # 4. Spiritual Fragment
    if record["QiGive"] > 0.0 or record["QiTake"] > 0.0:
        fragments.append({
            "Type": "USovereignItemFragment_Spiritual",
            "QiGive": record["QiGive"],
            "QiTake": record["QiTake"],
            "QiAuraRadiusBonus": record["QiAuraRadius"]
        })

    return fragments


def import_item_definitions_to_assets(csv_path="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv", output_dir="/Game/Data/ItemAssets"):
    """
    In-Engine synthesis of USovereignItemDefinition Primary DataAssets with attached fragments.
    """
    if not IN_ENGINE:
        records = parse_csv_item_rows(csv_path)
        total_frags = sum(len(determine_required_fragments(r)) for r in records)
        return f"[Standalone Dry-Run] Parsed {len(records)} records with {total_frags} total fragments."

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    if not editor_asset_lib.does_directory_exist(output_dir):
        editor_asset_lib.make_directory(output_dir)

    records = parse_csv_item_rows(csv_path)
    imported_count = 0

    item_def_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignItemDefinition")
    if not item_def_class:
        unreal.log_error("Could not load /Script/WispCPP7VR.SovereignItemDefinition C++ class.")
        return "Failed: SovereignItemDefinition C++ class not found."

    frag_ui_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignItemFragment_UIData")
    frag_weight_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignItemFragment_Weight")
    frag_consumable_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignItemFragment_Consumable")
    frag_spiritual_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignItemFragment_Spiritual")

    for rec in records:
        asset_name = rec["AssetName"]
        package_path = f"{output_dir}/{asset_name}"

        # Load or create DataAsset
        item_asset = None
        if editor_asset_lib.does_asset_exist(package_path):
            item_asset = editor_asset_lib.load_asset(package_path)
        else:
            factory = unreal.DataAssetFactory()
            try:
                factory.set_editor_property("data_asset_class", item_def_class)
            except Exception:
                try:
                    factory.data_asset_class = item_def_class
                except Exception:
                    pass
            item_asset = asset_tools.create_asset(asset_name, output_dir, item_def_class, factory)

        if not item_asset:
            continue

        item_asset.modify()

        # Set Core Properties
        item_asset.set_editor_property("display_name", unreal.Text(rec["DisplayName"]))
        if rec["ItemTag"]:
            tag_struct = unreal.GameplayTag()
            tag_struct.tag_name = unreal.Name(rec["ItemTag"])
            item_asset.set_editor_property("item_tag", tag_struct)

        # Build Fragments Array
        req_frags = determine_required_fragments(rec)
        new_fragments = []

        for f_info in req_frags:
            f_type = f_info["Type"]
            if f_type == "USovereignItemFragment_UIData" and frag_ui_class:
                u_frag = unreal.new_object(frag_ui_class, item_asset)
                u_frag.set_editor_property("description", unreal.Text(f_info["Description"]))
                new_fragments.append(u_frag)
            elif f_type == "USovereignItemFragment_Weight" and frag_weight_class:
                w_frag = unreal.new_object(frag_weight_class, item_asset)
                w_frag.set_editor_property("unit_weight", f_info["UnitWeight"])
                new_fragments.append(w_frag)
            elif f_type == "USovereignItemFragment_Consumable" and frag_consumable_class:
                c_frag = unreal.new_object(frag_consumable_class, item_asset)
                c_frag.set_editor_property("hunger_restore", f_info["HungerRestore"])
                c_frag.set_editor_property("hydration_restore", f_info["HydrationRestore"])
                c_frag.set_editor_property("strength_xp", f_info["StrengthXP"])
                c_frag.set_editor_property("dexterity_xp", f_info["DexterityXP"])
                c_frag.set_editor_property("constitution_xp", f_info["ConstitutionXP"])
                c_frag.set_editor_property("intelligence_xp", f_info["IntelligenceXP"])
                c_frag.set_editor_property("wisdom_xp", f_info["WisdomXP"])
                c_frag.set_editor_property("charisma_xp", f_info["CharismaXP"])
                c_frag.set_editor_property("luck_xp", f_info["LuckXP"])
                new_fragments.append(c_frag)
            elif f_type == "USovereignItemFragment_Spiritual" and frag_spiritual_class:
                s_frag = unreal.new_object(frag_spiritual_class, item_asset)
                s_frag.set_editor_property("qi_give", f_info["QiGive"])
                s_frag.set_editor_property("qi_take", f_info["QiTake"])
                s_frag.set_editor_property("qi_aura_radius_bonus", f_info["QiAuraRadiusBonus"])
                new_fragments.append(s_frag)

        item_asset.set_editor_property("fragments", new_fragments)
        editor_asset_lib.save_loaded_asset(item_asset)
        imported_count += 1

    summary = f"Synthesized {imported_count} USovereignItemDefinition assets in {output_dir}."
    unreal.log(f"🏛️ Sovereign Framework: {summary}")
    return summary


def execute_item_importer_ui(csv_path="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv", output_dir="/Game/Data/ItemAssets"):
    """UI execution wrapper returning a status message for EUW Text Blocks."""
    try:
        csv_path = csv_path.replace("\\", "/").strip("\"'")
        return import_item_definitions_to_assets(csv_path=csv_path, output_dir=output_dir)
    except Exception as e:
        err_msg = f"Item Importer Error: {str(e)}"
        if IN_ENGINE:
            unreal.log_error(err_msg)
        return err_msg


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Sovereign Item Definition & Fragment Synthesizer")
    parser.add_argument("--csv", default="AI_Nexus/Data/DT_SovereignSpeciesTemplate.csv", help="Path to CSV template")
    parser.add_argument("--out", default="/Game/Data/ItemAssets", help="Unreal destination folder")
    args = parser.parse_args()

    result = execute_item_importer_ui(args.csv, args.out)
    print(result)
