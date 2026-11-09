# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-10-21
"""
Automated Sovereign Combat Data Importer Suite (Tickets CS-001 & CS-002)
------------------------------------------------------------------------
Reads dedicated CSV templates:
- DT_SovereignSkillsTemplate.csv -> Synthesizes USovereignSkillDefinition in /Game/Data/Skills/
- DT_SovereignClassesTemplate.csv -> Synthesizes USovereignClassDefinition in /Game/Data/Classes/
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

# Enum mapping for ESovereignSkillTargetPattern
TARGET_PATTERN_MAP = {
    "singletarget": 0,
    "frontrowsweep": 1,
    "columnpierce": 2,
    "fullmatrixaoe": 3,
    "selfonly": 4,
    "allallies": 5
}

# Enum mapping for ESovereignSkillScalingAttribute
SCALING_ATTR_MAP = {
    "strength": 0,
    "dexterity": 1,
    "intelligence": 2,
    "wisdom": 3,
    "constitution": 4
}


def sanitize_asset_name(name_str, prefix="DA_"):
    """Generates a clean asset name prefixed with DA_"""
    clean = name_str.replace("Skill_", "").replace("Class_", "")
    return f"{prefix}{clean}"


def resolve_csv_path(csv_path):
    """
    Robustly resolves path to CSV template across different working directories
    and Unreal Engine execution environments.
    """
    if not csv_path:
        return ""

    csv_path = csv_path.replace("\\", "/").strip("\"'")

    # 1. Direct path exists
    if os.path.exists(csv_path):
        return os.path.abspath(csv_path)

    script_dir = os.path.dirname(os.path.abspath(__file__))

    # 2. Candidate: relative to repo root derived from script location
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


def parse_csv_skill_rows(csv_path):
    """Parses Skills CSV and returns list of skill records."""
    resolved_path = resolve_csv_path(csv_path)
    if not os.path.exists(resolved_path):
        raise FileNotFoundError(f"Skills CSV file not found: {csv_path} (Resolved: {resolved_path})")

    records = []
    with open(resolved_path, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            raw_name = row.get("Name", "").strip()
            if not raw_name:
                continue

            record = {
                "RawName": raw_name,
                "AssetName": sanitize_asset_name(raw_name, "DA_"),
                "SkillTag": row.get("SkillTag", "").strip(),
                "DisplayName": row.get("DisplayName", "").strip(),
                "Description": row.get("Description", "").strip(),
                "TargetPattern": row.get("TargetPattern", "SingleTarget").strip(),
                "ScalingAttribute": row.get("ScalingAttribute", "Strength").strip(),
                "BasePower": float(row.get("BasePower", "20.0") or "20.0"),
                "StatMultiplier": float(row.get("StatMultiplier", "1.20") or "1.20"),
                "QiCost": float(row.get("QiCost", "0.0") or "0.0"),
                "CooldownTurns": int(float(row.get("CooldownTurns", "0") or "0")),
                "IconPath": row.get("IconPath", "").strip()
            }
            records.append(record)
    return records


def parse_csv_class_rows(csv_path):
    """Parses Classes CSV and returns list of class records."""
    resolved_path = resolve_csv_path(csv_path)
    if not os.path.exists(resolved_path):
        raise FileNotFoundError(f"Classes CSV file not found: {csv_path} (Resolved: {resolved_path})")

    records = []
    with open(resolved_path, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            raw_name = row.get("Name", "").strip()
            if not raw_name:
                continue

            # Parse PrerequisiteClasses (e.g. "Class.Combat.Warrior:5;Class.Combat.Rogue:5")
            raw_prereqs = row.get("PrerequisiteClasses", "").strip()
            prereqs = []
            if raw_prereqs:
                for entry in raw_prereqs.split(";"):
                    entry = entry.strip()
                    if not entry:
                        continue
                    if ":" in entry:
                        parts = entry.split(":")
                        prereqs.append({
                            "ClassTag": parts[0].strip(),
                            "Level": int(parts[1].strip())
                        })
                    else:
                        prereqs.append({
                            "ClassTag": entry,
                            "Level": 1
                        })

            # Parse GrantedSkills (e.g. "Skill.Combat.Warrior.Cleave,Skill.Combat.Cleric.Mend")
            raw_skills = row.get("GrantedSkills", "").strip()
            granted_skills = [s.strip() for s in raw_skills.split(",") if s.strip()]

            record = {
                "RawName": raw_name,
                "AssetName": sanitize_asset_name(raw_name, "DA_"),
                "ClassTag": row.get("ClassTag", "").strip(),
                "DisplayName": row.get("DisplayName", "").strip(),
                "Description": row.get("Description", "").strip(),
                "CPCost": int(float(row.get("CPCost", "1") or "1")),
                "StrengthMultiplier": float(row.get("StrengthMultiplier", "1.0") or "1.0"),
                "DexterityMultiplier": float(row.get("DexterityMultiplier", "1.0") or "1.0"),
                "ConstitutionMultiplier": float(row.get("ConstitutionMultiplier", "1.0") or "1.0"),
                "IntelligenceMultiplier": float(row.get("IntelligenceMultiplier", "1.0") or "1.0"),
                "WisdomMultiplier": float(row.get("WisdomMultiplier", "1.0") or "1.0"),
                "Prerequisites": prereqs,
                "ReqSTR": int(float(row.get("ReqSTR", "0") or "0")),
                "ReqDEX": int(float(row.get("ReqDEX", "0") or "0")),
                "ReqCON": int(float(row.get("ReqCON", "0") or "0")),
                "ReqINT": int(float(row.get("ReqINT", "0") or "0")),
                "ReqWIS": int(float(row.get("ReqWIS", "0") or "0")),
                "GrantedSkills": granted_skills
            }
            records.append(record)
    return records


def import_skills_to_assets(csv_path="AI_Nexus/Data/DT_SovereignSkillsTemplate.csv", output_dir="/Game/Data/Skills"):
    """
    Synthesizes USovereignSkillDefinition Primary DataAssets from DT_SovereignSkillsTemplate.csv.
    """
    records = parse_csv_skill_rows(csv_path)

    if not IN_ENGINE:
        summary = f"[Standalone Dry-Run] Successfully validated {len(records)} skill records in {csv_path}."
        print(summary)
        return summary

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    if not editor_asset_lib.does_directory_exist(output_dir):
        editor_asset_lib.make_directory(output_dir)

    skill_def_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignSkillDefinition")
    if not skill_def_class:
        err = "Failed: /Script/WispCPP7VR.SovereignSkillDefinition C++ class not found."
        unreal.log_error(err)
        return err

    imported_count = 0
    for rec in records:
        asset_name = rec["AssetName"]
        package_path = f"{output_dir}/{asset_name}"

        skill_asset = None
        if editor_asset_lib.does_asset_exist(package_path):
            skill_asset = editor_asset_lib.load_asset(package_path)
        else:
            factory = unreal.DataAssetFactory()
            try:
                factory.set_editor_property("data_asset_class", skill_def_class)
            except Exception:
                try:
                    factory.data_asset_class = skill_def_class
                except Exception:
                    pass
            skill_asset = asset_tools.create_asset(asset_name, output_dir, skill_def_class, factory)

        if not skill_asset:
            continue

        skill_asset.modify()

        # Set Identity & Display
        if rec["SkillTag"]:
            tag_struct = unreal.GameplayTag()
            tag_struct.tag_name = unreal.Name(rec["SkillTag"])
            skill_asset.set_editor_property("skill_tag", tag_struct)

        skill_asset.set_editor_property("skill_display_name", unreal.Text(rec["DisplayName"]))
        skill_asset.set_editor_property("skill_description", unreal.Text(rec["Description"]))

        # Set Target Pattern Enum
        pattern_str = rec["TargetPattern"].lower()
        if hasattr(unreal, "SovereignSkillTargetPattern"):
            if "front" in pattern_str:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.FRONT_ROW_SWEEP)
            elif "column" in pattern_str:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.COLUMN_PIERCE)
            elif "matrix" in pattern_str or "full" in pattern_str:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.FULL_MATRIX_AOE)
            elif "self" in pattern_str:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.SELF_ONLY)
            elif "allies" in pattern_str or "all" in pattern_str:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.ALL_ALLIES)
            else:
                skill_asset.set_editor_property("target_pattern", unreal.SovereignSkillTargetPattern.SINGLE_TARGET)

        # Set Scaling Attribute Enum
        scale_str = rec["ScalingAttribute"].lower()
        if hasattr(unreal, "SovereignSkillScalingAttribute"):
            if "dex" in scale_str:
                skill_asset.set_editor_property("scaling_attribute", unreal.SovereignSkillScalingAttribute.DEXTERITY)
            elif "int" in scale_str:
                skill_asset.set_editor_property("scaling_attribute", unreal.SovereignSkillScalingAttribute.INTELLIGENCE)
            elif "wis" in scale_str:
                skill_asset.set_editor_property("scaling_attribute", unreal.SovereignSkillScalingAttribute.WISDOM)
            elif "con" in scale_str:
                skill_asset.set_editor_property("scaling_attribute", unreal.SovereignSkillScalingAttribute.CONSTITUTION)
            else:
                skill_asset.set_editor_property("scaling_attribute", unreal.SovereignSkillScalingAttribute.STRENGTH)

        # Numerical Values
        skill_asset.set_editor_property("base_power", rec["BasePower"])
        skill_asset.set_editor_property("stat_multiplier", rec["StatMultiplier"])
        skill_asset.set_editor_property("qi_cost", rec["QiCost"])
        skill_asset.set_editor_property("cooldown_turns", rec["CooldownTurns"])

        # Soft Pointer to Icon
        if rec["IconPath"] and editor_asset_lib.does_asset_exist(rec["IconPath"]):
            icon_obj = editor_asset_lib.load_asset(rec["IconPath"])
            if icon_obj:
                skill_asset.set_editor_property("skill_icon", icon_obj)

        editor_asset_lib.save_loaded_asset(skill_asset)
        imported_count += 1

    summary = f"Synthesized {imported_count} USovereignSkillDefinition assets in {output_dir}."
    unreal.log(f"⚔️ Sovereign Combat: {summary}")
    return summary


def import_classes_to_assets(csv_path="AI_Nexus/Data/DT_SovereignClassesTemplate.csv", output_dir="/Game/Data/Classes", skills_dir="/Game/Data/Skills"):
    """
    Synthesizes USovereignClassDefinition Primary DataAssets from DT_SovereignClassesTemplate.csv.
    """
    records = parse_csv_class_rows(csv_path)

    if not IN_ENGINE:
        summary = f"[Standalone Dry-Run] Successfully validated {len(records)} class records in {csv_path}."
        print(summary)
        return summary

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    if not editor_asset_lib.does_directory_exist(output_dir):
        editor_asset_lib.make_directory(output_dir)

    class_def_class = unreal.load_class(None, "/Script/WispCPP7VR.SovereignClassDefinition")
    if not class_def_class:
        err = "Failed: /Script/WispCPP7VR.SovereignClassDefinition C++ class not found."
        unreal.log_error(err)
        return err

    prereq_struct = unreal.load_struct(None, "/Script/WispCPP7VR.SovereignClassPrerequisite")

    imported_count = 0
    for rec in records:
        asset_name = rec["AssetName"]
        package_path = f"{output_dir}/{asset_name}"

        class_asset = None
        if editor_asset_lib.does_asset_exist(package_path):
            class_asset = editor_asset_lib.load_asset(package_path)
        else:
            factory = unreal.DataAssetFactory()
            try:
                factory.set_editor_property("data_asset_class", class_def_class)
            except Exception:
                try:
                    factory.data_asset_class = class_def_class
                except Exception:
                    pass
            class_asset = asset_tools.create_asset(asset_name, output_dir, class_def_class, factory)

        if not class_asset:
            continue

        class_asset.modify()

        # Set Identity & Display
        if rec["ClassTag"]:
            tag_struct = unreal.GameplayTag()
            tag_struct.tag_name = unreal.Name(rec["ClassTag"])
            class_asset.set_editor_property("class_tag", tag_struct)

        class_asset.set_editor_property("class_display_name", unreal.Text(rec["DisplayName"]))
        class_asset.set_editor_property("class_description", unreal.Text(rec["Description"]))
        class_asset.set_editor_property("cp_cost", rec["CPCost"])

        # Set Multipliers
        class_asset.set_editor_property("strength_multiplier", rec["StrengthMultiplier"])
        class_asset.set_editor_property("dexterity_multiplier", rec["DexterityMultiplier"])
        class_asset.set_editor_property("constitution_multiplier", rec["ConstitutionMultiplier"])
        class_asset.set_editor_property("intelligence_multiplier", rec["IntelligenceMultiplier"])
        class_asset.set_editor_property("wisdom_multiplier", rec["WisdomMultiplier"])

        # Minimum Attribute Thresholds
        class_asset.set_editor_property("required_strength", rec["ReqSTR"])
        class_asset.set_editor_property("required_dexterity", rec["ReqDEX"])
        class_asset.set_editor_property("required_constitution", rec["ReqCON"])
        class_asset.set_editor_property("required_intelligence", rec["ReqINT"])
        class_asset.set_editor_property("required_wisdom", rec["ReqWIS"])

        # Build Prerequisites Array
        prereq_objs = []
        for p in rec["Prerequisites"]:
            pr_inst = unreal.SovereignClassPrerequisite() if hasattr(unreal, "SovereignClassPrerequisite") else None
            if pr_inst is not None:
                tag_st = unreal.GameplayTag()
                tag_st.tag_name = unreal.Name(p["ClassTag"])
                pr_inst.set_editor_property("required_class_tag", tag_st)
                pr_inst.set_editor_property("required_level", p["Level"])
                prereq_objs.append(pr_inst)

        if prereq_objs:
            class_asset.set_editor_property("prerequisite_classes", prereq_objs)

        # Build Granted Skills Soft Pointer Array
        granted_soft_ptrs = []
        for skill_ref in rec["GrantedSkills"]:
            skill_asset_name = sanitize_asset_name(skill_ref.split(".")[-1], "DA_")
            skill_path = f"{skills_dir}/{skill_asset_name}"
            if editor_asset_lib.does_asset_exist(skill_path):
                sk_obj = editor_asset_lib.load_asset(skill_path)
                if sk_obj:
                    granted_soft_ptrs.append(sk_obj)

        if granted_soft_ptrs:
            class_asset.set_editor_property("granted_skills", granted_soft_ptrs)

        editor_asset_lib.save_loaded_asset(class_asset)
        imported_count += 1

    summary = f"Synthesized {imported_count} USovereignClassDefinition assets in {output_dir}."
    unreal.log(f"🛡️ Sovereign Combat: {summary}")
    return summary


def import_skills(csv_path="AI_Nexus/Data/DT_SovereignSkillsTemplate.csv", output_dir="/Game/Data/Skills"):
    """User-facing wrapper for Skill importing."""
    return import_skills_to_assets(csv_path=csv_path, output_dir=output_dir)


def import_classes(csv_path="AI_Nexus/Data/DT_SovereignClassesTemplate.csv", output_dir="/Game/Data/Classes", skills_dir="/Game/Data/Skills"):
    """User-facing wrapper for Class importing."""
    return import_classes_to_assets(csv_path=csv_path, output_dir=output_dir, skills_dir=skills_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Sovereign Skills & Classes Deterministic Importer")
    parser.add_argument("--skills-csv", default="AI_Nexus/Data/DT_SovereignSkillsTemplate.csv", help="Path to Skills CSV")
    parser.add_argument("--classes-csv", default="AI_Nexus/Data/DT_SovereignClassesTemplate.csv", help="Path to Classes CSV")
    parser.add_argument("--skills-out", default="/Game/Data/Skills", help="Unreal destination folder for skills")
    parser.add_argument("--classes-out", default="/Game/Data/Classes", help="Unreal destination folder for classes")
    args = parser.parse_args()

    s_res = import_skills(args.skills_csv, args.skills_out)
    c_res = import_classes(args.classes_csv, args.classes_out, args.skills_out)
    print(s_res)
    print(c_res)
