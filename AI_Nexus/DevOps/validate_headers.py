#!/usr/bin/env python3
# Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25
"""
Automated "0.36 Standard" Header Validation & Repair Script.

Scans governed files (.cpp, .h, .py, .md) across the project directory or specific file,
validating presence and accuracy of the standardized copyright header:
// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.[x]. Licensed under GPLv3 (See LICENSE). Last Updated: YYYY-MM-DD
"""

import os
import re
import sys
import argparse
from datetime import datetime

HEADER_PATTERN = re.compile(
    r'^(?://|#)\s*Copyright\s+\(c\)\s+2013-202\d\s+Daniel\s+Acourt\.\s+Version\s+36\.4\.\d+\.\s+Licensed\s+under\s+GPLv3\s+\(See\s+LICENSE\)\.\s+Last\s+Updated:\s+\d{4}-\d{2}-\d{2}'
)

EXCLUDE_DIRS = {
    '.git', '.vs', 'Intermediate', 'Binaries', 'Saved', 'DerivedDataCache',
    'Build', '__pycache__', 'node_modules', 'venv', 'env'
}

GOVERNED_EXTENSIONS = {'.cpp', '.h', '.py', '.md'}

def generate_header(comment_prefix: str) -> str:
    today = datetime.now().strftime("%Y-%m-%d")
    return f"{comment_prefix} Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: {today}"

def check_or_fix_file(filepath: str, fix: bool = False) -> bool:
    ext = os.path.splitext(filepath)[1].lower()
    if ext not in GOVERNED_EXTENSIONS:
        return True

    comment_prefix = '#' if ext in ('.py', '.sh') else '//'

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"[ERROR] Could not read {filepath}: {e}")
        return False

    if not lines:
        return True

    first_line = lines[0].strip()

    # Handle python shebang on line 1
    target_line_idx = 0
    if ext == '.py' and first_line.startswith('#!'):
        if len(lines) > 1:
            target_line_idx = 1
            first_line = lines[1].strip()

    if HEADER_PATTERN.match(first_line):
        return True

    if not fix:
        print(f"[FAIL] Invalid or missing header in: {filepath}")
        return False

    new_header = generate_header(comment_prefix) + '\n'
    if target_line_idx == 0:
        if HEADER_PATTERN.match(first_line) or 'Copyright' in first_line:
            lines[0] = new_header
        else:
            lines.insert(0, new_header)
    else:
        if len(lines) > 1 and ('Copyright' in lines[1]):
            lines[1] = new_header
        else:
            lines.insert(1, new_header)

    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(lines)
        print(f"[FIXED] Applied standard header to: {filepath}")
        return True
    except Exception as e:
        print(f"[ERROR] Could not write to {filepath}: {e}")
        return False

def scan_repository(target_path: str, fix: bool = False) -> tuple:
    total_scanned = 0
    passed = 0
    failed = 0

    if os.path.isfile(target_path):
        ext = os.path.splitext(target_path)[1].lower()
        if ext in GOVERNED_EXTENSIONS:
            total_scanned += 1
            if check_or_fix_file(target_path, fix=fix):
                passed += 1
            else:
                failed += 1
        return total_scanned, passed, failed

    for root, dirs, files in os.walk(target_path):
        dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext in GOVERNED_EXTENSIONS:
                filepath = os.path.join(root, file)
                total_scanned += 1
                if check_or_fix_file(filepath, fix=fix):
                    passed += 1
                else:
                    failed += 1

    return total_scanned, passed, failed

def main():
    parser = argparse.ArgumentParser(description='Validate or repair 0.36 Standard copyright headers.')
    parser.add_argument('--fix', action='store_true', help='Automatically apply or repair missing headers.')
    parser.add_argument('--path', default='.', help='Root directory or file to scan.')
    args = parser.parse_args()

    print(f"Scanning '{args.path}' for governed files ({', '.join(GOVERNED_EXTENSIONS)})...")
    total, passed, failed = scan_repository(args.path, fix=args.fix)

    print(f"\n--- Header Validation Summary ---")
    print(f"Total Governed Files Scanned: {total}")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")

    if failed > 0 and not args.fix:
        print("\nRun 'python AI_Nexus/DevOps/validate_headers.py --fix' to auto-repair non-compliant files.")
        sys.exit(1)
    else:
        print("\nAll scanned headers are compliant!")
        sys.exit(0)

if __name__ == '__main__':
    main()
