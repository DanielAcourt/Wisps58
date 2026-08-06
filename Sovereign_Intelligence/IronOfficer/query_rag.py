# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7-Knight-AAS-RAG. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
"""
Sovereign Level-Aware RAG Interactive Query Sandbox.
Allows the Lead to test search keywords and observe SSoT Level classification and priority boosting.
"""

import sys
import os

BASE_DIR = os.path.dirname(__file__)
sys.path.append(BASE_DIR)

from rag import SovereignRAG

def main():
    repo_root = os.path.abspath(os.path.join(BASE_DIR, "..", ".."))
    print(f"[07] Initializing Sovereign RAG on root: {repo_root}")

    rag = SovereignRAG(repo_root)
    rag.build_index(["AI_Nexus"])

    print("\n" + "="*70)
    print("     🏛️  SOVEREIGN LEVEL-AWARE HIERARCHICAL RAG QUERY SANDBOX  🏛️")
    print("     Enter search queries to inspect SSoT Level mapping and scoring.")
    print("     Type 'exit' or 'quit' to close the sandbox.")
    print("="*70 + "\n")

    while True:
        try:
            query = input("Sovereign RAG Search > ").strip()
            if query.lower() in ("exit", "quit"):
                print("\n[07] Closing sandbox. May the pillars stand strong. 🫡")
                break
            if not query:
                continue

            results = rag.search(query, max_chunks=3, threshold=0.01)
            if not results:
                print("❌ No matching chunks found above the similarity threshold.\n")
                continue

            print(f"\n🔍 Retrieved {len(results)} chunks:")
            for i, (chunk, score) in enumerate(results):
                print(f"\n🌟 MATCH {i+1} | Score: {score:.4f}")
                print(f"   ├─ SSoT Level: {chunk['level']}")
                print(f"   ├─ File Path:  {chunk['path']}")
                print(f"   └─ Section:    {chunk['header']}")
                print("   " + "─"*50)

                # Format text snippet nicely
                text_lines = chunk['text'].strip().split('\n')
                snippet = "\n".join([f"      {line}" for line in text_lines[:6]])
                if len(text_lines) > 6:
                    snippet += "\n      ..."
                print(snippet)
            print("\n" + "="*70 + "\n")

        except (KeyboardInterrupt, EOFError):
            print("\n\n[07] Closing sandbox. 🫡")
            break

if __name__ == "__main__":
    main()
