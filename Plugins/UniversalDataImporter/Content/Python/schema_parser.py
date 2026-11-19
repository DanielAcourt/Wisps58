# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer: Agnostic Schema Parser
"""
Agnostic JSON Schema Parser for Unreal Engine 5.8+ Universal Data Importer.
Validates, loads, and normalizes mapping schema configurations without project dependencies.
"""

import os
import json


class MappingPropertyRule:
    def __init__(
        self,
        csv_column,
        target_property,
        property_type="String",
        fallback_value=None,
        delimiter=",",
        extra_meta=None,
    ):
        self.csv_column = str(csv_column).strip()
        self.target_property = str(target_property).strip()
        self.property_type = str(property_type).strip()
        self.fallback_value = fallback_value
        self.delimiter = delimiter
        self.extra_meta = dict(extra_meta) if isinstance(extra_meta, dict) else {}


class ComponentPropertyRule:
    def __init__(self, component_class, csv_column, target_property, property_type="String", fallback_value=None):
        self.component_class = str(component_class).strip()
        self.csv_column = str(csv_column).strip()
        self.target_property = str(target_property).strip()
        self.property_type = str(property_type).strip()
        self.fallback_value = fallback_value


class ImporterSchema:
    def __init__(self):
        self.schema_version = "1.0.0"
        self.plugin_name = "UniversalDataImporter"
        self.target_data_asset_class = "/Script/Engine.DataAsset"
        self.target_package_directory = "/Game/Data/GeneratedAssets"
        self.auto_bind_art_assets = True
        self.art_search_directories = ["/Game/Assets/External", "/Game/ArtVault", "/Game/Assets"]
        self.column_to_property_map = []
        self.nested_component_property_map = []
        self.b_create_missing_blueprints = True
        self.default_parent_class = "/Script/Engine.Actor"
        self.parent_class_column = "ParentClass"
        self.cdo_binding_property = "DataAssetRef"
        self.default_input_mapping_context = ""

    @classmethod
    def load_from_json(cls, schema_filepath):
        """Loads and parses a JSON schema file, returning an ImporterSchema instance."""
        schema = cls()
        if not schema_filepath or not os.path.exists(schema_filepath):
            print(f"⚠️ [SchemaParser] Schema file not found at path: {schema_filepath}. Using defaults.")
            return schema

        try:
            with open(schema_filepath, "r", encoding="utf-8") as f:
                data = json.load(f)

            schema.schema_version = data.get("SchemaVersion", "1.0.0")
            schema.plugin_name = data.get("PluginName", "UniversalDataImporter")
            schema.target_data_asset_class = data.get("TargetDataAssetClass", "/Script/Engine.DataAsset")
            schema.target_package_directory = data.get("TargetPackageDirectory", "/Game/Data/GeneratedAssets")
            schema.auto_bind_art_assets = bool(data.get("AutoBindArtAssets", True))
            schema.art_search_directories = data.get("ArtSearchDirectories", ["/Game/Assets/External", "/Game/Assets"])

            known_rule_keys = {"CSVColumn", "TargetProperty", "PropertyType", "FallbackValue", "Delimiter"}
            schema.column_to_property_map = []
            for item in data.get("ColumnToPropertyMap", []):
                extra = {k: v for k, v in item.items() if k not in known_rule_keys}
                rule = MappingPropertyRule(
                    csv_column=item.get("CSVColumn", ""),
                    target_property=item.get("TargetProperty", ""),
                    property_type=item.get("PropertyType", "String"),
                    fallback_value=item.get("FallbackValue", None),
                    delimiter=item.get("Delimiter", ","),
                    extra_meta=extra,
                )
                if rule.csv_column and rule.target_property:
                    schema.column_to_property_map.append(rule)

            schema.nested_component_property_map = []
            for item in data.get("NestedComponentPropertyMap", []):
                rule = ComponentPropertyRule(
                    component_class=item.get("ComponentClass", "/Script/Engine.ActorComponent"),
                    csv_column=item.get("CSVColumn", ""),
                    target_property=item.get("TargetProperty", ""),
                    property_type=item.get("PropertyType", "String"),
                    fallback_value=item.get("FallbackValue", None),
                )
                if rule.csv_column and rule.target_property:
                    schema.nested_component_property_map.append(rule)

            bp_rules = data.get("BlueprintSynthesisRules", {})
            schema.b_create_missing_blueprints = bool(bp_rules.get("bCreateMissingBlueprints", True))
            schema.default_parent_class = bp_rules.get("DefaultParentClass", "/Script/Engine.Actor")
            schema.parent_class_column = bp_rules.get("ParentClassColumn", "ParentClass")
            schema.cdo_binding_property = bp_rules.get("CDOBindingProperty", "DataAssetRef")
            schema.default_input_mapping_context = bp_rules.get("DefaultInputMappingContext", "")

            print(
                f"✅ [SchemaParser] Successfully loaded schema '{schema_filepath}' "
                f"({len(schema.column_to_property_map)} column rules, {len(schema.nested_component_property_map)} component rules)"
            )
            return schema

        except Exception as e:
            print(f"❌ [SchemaParser] Failed to parse schema file '{schema_filepath}': {e}")
            return schema