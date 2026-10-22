# Copyright (c) 2026 Sovereign Framework Architecture Team. All rights reserved.
# Universal Data Importer: Agnostic Schema Parser
"""
Agnostic JSON Schema Parser for Unreal Engine 5.8+ Universal Data Importer.
Validates, loads, and normalizes mapping schema configurations without project dependencies.
"""

import os
import json

class MappingPropertyRule:
    def __init__(self, csv_column, target_property, property_type="String", fallback_value=None, delimiter=","):
        self.csv_column = str(csv_column).strip()
        self.target_property = str(target_property).strip()
        self.property_type = str(property_type).strip()  # String, Text, Int32, Float, Bool, GameplayTag, GameplayTagContainer, SoftClassPath, SoftObjectPath
        self.fallback_value = fallback_value
        self.delimiter = delimiter

class ComponentPropertyRule:
    def __init__(self, component_class, csv_column, target_property, property_type="String", fallback_value=None):
        self.component_class = str(component_class).strip()  # e.g. "/Script/Engine.ActorComponent" or "ActorComponent"
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
        self.column_to_property_map = []  # List of MappingPropertyRule
        self.nested_component_property_map = []  # List of ComponentPropertyRule
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
            with open(schema_filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)

            schema.schema_version = data.get("SchemaVersion", "1.0.0")
            schema.plugin_name = data.get("PluginName", "UniversalDataImporter")
            schema.target_data_asset_class = data.get("TargetDataAssetClass", "/Script/Engine.DataAsset")
            schema.target_package_directory = data.get("TargetPackageDirectory", "/Game/Data/GeneratedAssets")
            schema.auto_bind_art_assets = bool(data.get("AutoBindArtAssets", True))
            schema.art_search_directories = data.get("ArtSearchDirectories", ["/Game/Assets/External", "/Game/Assets"])

            # Load ColumnToPropertyMap
            schema.column_to_property_map = []
            for item in data.get("ColumnToPropertyMap", []):
                rule = MappingPropertyRule(
                    csv_column=item.get("CSVColumn", ""),
                    target_property=item.get("TargetProperty", ""),
                    property_type=item.get("PropertyType", "String"),
                    fallback_value=item.get("FallbackValue", None),
                    delimiter=item.get("Delimiter", ",")
                )
                if rule.csv_column and rule.target_property:
                    schema.column_to_property_map.append(rule)

            # Load NestedComponentPropertyMap
            schema.nested_component_property_map = []
            for item in data.get("NestedComponentPropertyMap", []):
                rule = ComponentPropertyRule(
                    component_class=item.get("ComponentClass", "/Script/Engine.ActorComponent"),
                    csv_column=item.get("CSVColumn", ""),
                    target_property=item.get("TargetProperty", ""),
                    property_type=item.get("PropertyType", "String"),
                    fallback_value=item.get("FallbackValue", None)
                )
                if rule.csv_column and rule.target_property:
                    schema.nested_component_property_map.append(rule)

            # Load BlueprintSynthesisRules
            bp_rules = data.get("BlueprintSynthesisRules", {})
            schema.b_create_missing_blueprints = bool(bp_rules.get("bCreateMissingBlueprints", True))
            schema.default_parent_class = bp_rules.get("DefaultParentClass", "/Script/Engine.Actor")
            schema.parent_class_column = bp_rules.get("ParentClassColumn", "ParentClass")
            schema.cdo_binding_property = bp_rules.get("CDOBindingProperty", "DataAssetRef")
            schema.default_input_mapping_context = bp_rules.get("DefaultInputMappingContext", "")

            print(f"✅ [SchemaParser] Successfully loaded schema '{schema_filepath}' ({len(schema.column_to_property_map)} column rules, {len(schema.nested_component_property_map)} component rules)")
            return schema

        except Exception as e:
            print(f"❌ [SchemaParser] Failed to parse schema file '{schema_filepath}': {e}")
            return schema
