# sovereign-art-sync

> **Dynamic Local Art Asset Synchronizer & Auto-Discovery Engine for Unreal Engine 5**

`sovereign-art-sync` is an open-source tool that solves repository bloat in Unreal Engine 5 projects. It decouples heavy binary `.uasset` files (meshes, high-res textures, audio) from Git source control while preserving level layouts, C++ source code, and Blueprint DNA.

---

## 🚀 Key Features

* **Zero Repository Bloat:** Keeps your Git repo under 50 MB by storing binary assets locally or on external storage (`Content/ArtVault/`).
* **Dynamic Project Auto-Detection:** Automatically discovers your `.uproject` file without requiring hardcoded project paths.
* **Vault Auto-Discovery:** Automatically detects new asset subdirectories dropped into `Content/ArtVault/` and updates `asset_manifest.json` on the fly.
* **1-Click Sync:** Includes `sync_art_assets.bat` for instant single-click synchronization on Windows.
* **Incremental Syncing:** Only copies new or modified `.uasset` binary files based on timestamp checks.

---

## 📁 Repository Structure

When extracted into your Unreal Engine project root:

```
[YourUE5ProjectRoot]/
├── YourGame.uproject
├── asset_manifest.json       # Asset package manifest mapping vault folders to Content/
├── sync_art_assets.bat       # 1-Click Windows execution script
├── sync_art_assets.py        # Python sync engine
├── Content/
│   ├── ArtVault/             # [LOCAL ONLY] Drop raw binary art assets here
│   └── Assets/External/      # [LOCAL ONLY] Auto-synced UE5 content target directory
└── ...
```

---

## 🛠️ Quick Start Guide

### 1. Installation
Extract `sovereign-art-sync` into your Unreal Engine 5 project root directory (where your `.uproject` file lives).

### 2. Configure `.gitignore`
Add the rules from `sample.gitignore` to your project `.gitignore`:

```gitignore
Content/ArtVault/
Content/Assets/External/
*.uasset.bak
*.umap.bak
```

### 3. Add Assets & Sync
1. Drop your `.uasset` folders (e.g. `Meshes`, `Textures`, `Audio`) into `Content/ArtVault/`.
2. Double-click **`sync_art_assets.bat`**.
3. Open Unreal Engine 5! Your assets will appear under `Content -> Assets -> External -> ArtVault/`.

For complete operational details and team workflows, read [USER_GUIDE.md](USER_GUIDE.md).

---

## 📜 License

Licensed under the [MIT License](LICENSE).
