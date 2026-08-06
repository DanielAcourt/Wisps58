# Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7-Knight-AAS-RAG. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-06-28
"""
Sovereign RAG: Simple, Modular, Level-Aware Retrieval Engine
Zero VRAM footprint, running in CPU memory to preserve the GTX 5090 for active inference.
Maps knowledge to SSoT Levels 0 to 4.
"""

import os
import re
import json
import math
import logging
from typing import Dict, Any, List, Tuple, Set

logger = logging.getLogger("AAS.SovereignRAG")

STOPWORDS = {
    "a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "arent", "as", "at",
    "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", "cant", "cannot", "could",
    "did", "didnt", "do", "does", "doesnt", "doing", "dont", "down", "during", "each", "few", "for", "from", "further",
    "had", "hadnt", "has", "hasnt", "have", "havent", "having", "he", "hed", "hell", "hes", "her", "here", "heres",
    "hers", "herself", "him", "himself", "his", "how", "hows", "i", "id", "ill", "im", "ive", "if", "in", "into",
    "is", "isnt", "it", "its", "itself", "lets", "me", "more", "most", "mustnt", "my", "myself", "no", "nor", "not",
    "of", "off", "on", "once", "only", "or", "other", "ought", "our", "ours", "ourselves", "out", "over", "own",
    "same", "shant", "she", "shed", "shell", "shes", "should", "shouldnt", "so", "some", "such", "than", "that",
    "thats", "the", "their", "theirs", "them", "themselves", "then", "there", "theres", "these", "they", "theyd",
    "theyll", "theyre", "theyve", "this", "those", "through", "to", "too", "under", "until", "up", "very", "was",
    "wasnt", "we", "wed", "well", "were", "weve", "werent", "what", "whats", "when", "whens", "where", "wheres",
    "which", "while", "who", "whos", "whom", "why", "whys", "with", "wont", "would", "wouldnt", "you", "youd",
    "youll", "youre", "youve", "your", "yours", "yourself", "yourselves"
}

def determine_ssot_level(relative_path: str) -> int:
    """Categorizes a filepath into its Sovereign SSoT Level (0 to 4)."""
    path_lower = relative_path.replace("\\", "/").lower()

    # Specific exceptions first
    if "archivist_theta" in path_lower:
        return 0
    if "jules" in path_lower or "identity.json" in path_lower or "aas_protocol" in path_lower:
        return 2

    # Standard directories
    if "timeline/" in path_lower:
        return 0
    if "protocols/" in path_lower or "docs/" in path_lower:
        return 1
    if "identity/" in path_lower:
        return 2
    if "research/" in path_lower or "admin/" in path_lower:
        return 3
    if "devops/" in path_lower or "memories/" in path_lower:
        return 4

    return 4  # Default to Level 4 (Execution/DevOps)

def tokenize(text: str) -> List[str]:
    """Tokenizes text, keeping alphanumeric strings and discarding stopwords."""
    text_clean = re.sub(r'[^a-zA-Z0-9_\-]', ' ', text.lower())
    words = text_clean.split()
    return [w for w in words if w not in STOPWORDS and len(w) > 1]

class SovereignRAG:
    def __init__(self, repo_root: str, chunk_size_words: int = 250):
        self.repo_root = os.path.abspath(repo_root)
        self.chunk_size_words = chunk_size_words
        self.chunks: List[Dict[str, Any]] = []
        self.vocab: Set[str] = set()
        self.df: Dict[str, int] = {}  # Document Frequency for IDF
        self.num_chunks = 0

    def clear(self):
        self.chunks = []
        self.vocab = set()
        self.df = {}
        self.num_chunks = 0

    def build_index(self, directories: List[str]):
        """Recursively scans designated directories and indexes files."""
        self.clear()
        indexed_files_count = 0

        for d in directories:
            # Resolve directory absolute path relative to repo_root
            abs_dir = os.path.abspath(os.path.join(self.repo_root, d))
            if not os.path.exists(abs_dir):
                logger.warning(f"RAG: Index directory does not exist: {d}")
                continue

            for root, _, filenames in os.walk(abs_dir):
                # Skip .git, cache, and build directories
                if any(p in root.replace("\\", "/") for p in [".git", "__pycache__", "Environment"]):
                    continue
                for f in filenames:
                    if f.endswith((".md", ".txt", ".json", ".csv")):
                        filepath = os.path.join(root, f)
                        try:
                            self.index_file(filepath)
                            indexed_files_count += 1
                        except Exception as e:
                            logger.error(f"RAG: Failed to index file {filepath}: {e}")

        self.num_chunks = len(self.chunks)
        self.compute_global_statistics()
        logger.info(f"RAG: Finished indexing. Total files: {indexed_files_count}, Total chunks: {self.num_chunks}")

    def index_file(self, filepath: str):
        """Chunks and registers a single file's content with Level mappings."""
        try:
            rel_path = os.path.relpath(filepath, self.repo_root)
        except ValueError:
            # Handle Windows cross-drive path relativity
            rel_path = filepath
        level = determine_ssot_level(rel_path)

        with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()

        # Handle different chunking models based on file format
        if filepath.endswith(".json"):
            # For JSON, chunk by top-level keys if it's a dict
            try:
                data = json.loads(content)
                if isinstance(data, dict):
                    for k, v in data.items():
                        chunk_text = f"Key: {k}\n{json.dumps(v, indent=2)}"
                        self.add_chunk(rel_path, level, f"JSON Section: {k}", chunk_text)
                else:
                    self.chunk_and_add_text(rel_path, level, "JSON Raw", content)
            except Exception:
                self.chunk_and_add_text(rel_path, level, "JSON Raw", content)
        elif filepath.endswith(".md"):
            # Chunk MD files by headers to preserve semantic boundaries
            sections = re.split(r'^(#+\s+.*)$', content, flags=re.MULTILINE)
            current_header = "Intro"
            current_text = []

            for section in sections:
                if section.strip().startswith("#"):
                    if current_text:
                        self.chunk_and_add_text(rel_path, level, current_header, "\n".join(current_text))
                    current_header = section.strip()
                    current_text = []
                else:
                    current_text.append(section)

            if current_text:
                self.chunk_and_add_text(rel_path, level, current_header, "\n".join(current_text))
        else:
            # For txt, csv, etc. chunk by paragraph
            self.chunk_and_add_text(rel_path, level, "File Content", content)

    def chunk_and_add_text(self, rel_path: str, level: int, header: str, text: str):
        """Splits a large block of text into configured word limit chunks and registers them."""
        words = text.split()
        if not words:
            return

        chunk_size = self.chunk_size_words
        # Simple sliding chunking with a 10% overlap
        step = int(chunk_size * 0.9)
        if step <= 0:
            step = chunk_size

        for i in range(0, len(words), step):
            chunk_words = words[i:i + chunk_size]
            chunk_text = " ".join(chunk_words)
            self.add_chunk(rel_path, level, header, chunk_text)

    def add_chunk(self, rel_path: str, level: int, header: str, text: str):
        """Calculates term frequencies for a chunk and adds it to the list."""
        if not text.strip():
            return

        tokens = tokenize(text)
        if not tokens:
            return

        # Calculate local term frequency (TF)
        tf: Dict[str, float] = {}
        for token in tokens:
            tf[token] = tf.get(token, 0.0) + 1.0

        # Store TF as normalized term frequency
        total_tokens = len(tokens)
        for token in tf:
            tf[token] = tf[token] / total_tokens
            self.vocab.add(token)

        self.chunks.append({
            "path": rel_path.replace("\\", "/"),
            "level": level,
            "header": header,
            "text": text,
            "tf": tf,
            "tokens": set(tokens)
        })

    def compute_global_statistics(self):
        """Computes global Document Frequency (DF) across all chunks."""
        self.df = {}
        for chunk in self.chunks:
            for token in chunk["tf"]:
                self.df[token] = self.df.get(token, 0) + 1

    def search(self, query: str, max_chunks: int = 3, threshold: float = 0.05) -> List[Tuple[Dict[str, Any], float]]:
        """
        Performs TF-IDF similarity search over chunks with SSoT Level Priority Boosting.
        """
        if self.num_chunks == 0:
            return []

        query_tokens = tokenize(query)
        if not query_tokens:
            return []

        # Calculate IDF for query tokens
        # IDF = ln(1 + (Total Chunks / Chunk DF))
        query_tfidf: Dict[str, float] = {}
        for token in set(query_tokens):
            if token in self.df:
                idf = math.log(1.0 + (self.num_chunks / (self.df[token])))
                # Local tf in query is simply count/total
                tf = query_tokens.count(token) / len(query_tokens)
                query_tfidf[token] = tf * idf

        # Calculate Query Magnitude for Cosine Similarity
        query_mag = math.sqrt(sum(val ** 2 for val in query_tfidf.values()))

        # Determine level boosting targets based on query tokens
        boosted_levels = self.detect_intent_levels(query_tokens)

        scored_chunks: List[Tuple[Dict[str, Any], float]] = []

        for chunk in self.chunks:
            # Compute cosine similarity
            similarity = 0.0
            if query_mag > 0:
                dot_product = 0.0
                chunk_mag_sq = 0.0

                # Calculate TF-IDF representation for chunk active tokens
                chunk_tfidf: Dict[str, float] = {}
                for token, tf in chunk["tf"].items():
                    if token in self.df:
                        idf = math.log(1.0 + (self.num_chunks / (self.df[token])))
                        chunk_tfidf[token] = tf * idf
                        chunk_mag_sq += chunk_tfidf[token] ** 2

                for token, q_val in query_tfidf.items():
                    if token in chunk_tfidf:
                        dot_product += q_val * chunk_tfidf[token]

                chunk_mag = math.sqrt(chunk_mag_sq)
                if chunk_mag > 0:
                    similarity = dot_product / (query_mag * chunk_mag)

            # [AD-003 Hardening] Fallback substring and fused-term matching (e.g. 'ainexus' -> 'ai_nexus', 'SaveEnitity' -> 'SaveEntity')
            if similarity == 0.0:
                raw_text_lower = chunk["text"].lower()
                raw_path_lower = chunk["path"].lower()
                raw_header_lower = chunk["header"].lower()

                # Expand camelCase into separate terms (e.g., 'SaveEnitityComponent' -> 'Save Enitity Component')
                expanded_query = re.sub(r'([a-z0-9])([A-Z])', r'\1 \2', query)
                query_words = [w.strip().lower() for w in re.split(r'[^a-zA-Z0-9]', expanded_query) if len(w.strip()) > 2]

                if query_words:
                    match_count = 0
                    for qw in query_words:
                        # Direct or substring check on text, path, or headers
                        if qw in raw_text_lower or qw in raw_path_lower or qw in raw_header_lower:
                            match_count += 1
                        # Substring match on stripped/fused terms (e.g., 'ainexus' matches 'ai_nexus')
                        elif qw.replace("_", "").replace("-", "") in raw_text_lower.replace("_", "").replace("-", "") or \
                             qw.replace("_", "").replace("-", "") in raw_path_lower.replace("_", "").replace("-", ""):
                            match_count += 1
                        # Fuzzy matching for minor suffix/prefix typo variations (e.g., matching 75% of string)
                        else:
                            qw_len = len(qw)
                            if qw_len > 4:
                                prefix = qw[:int(qw_len * 0.75)]
                                if prefix in raw_text_lower or prefix in raw_path_lower:
                                    match_count += 1

                    if match_count > 0:
                        # Normalize similarity based on matched terms ratio
                        similarity = 0.15 * (match_count / len(query_words))

            # Apply Level-Based Boosting
            if similarity > 0 and chunk["level"] in boosted_levels:
                similarity *= 1.5  # Apply 50% Level Priority Boost

            if similarity >= threshold:
                scored_chunks.append((chunk, similarity))

        # Sort by similarity score descending
        scored_chunks.sort(key=lambda x: x[1], reverse=True)
        return scored_chunks[:max_chunks]

    def detect_intent_levels(self, query_tokens: List[str]) -> Set[int]:
        """Maps query keywords to target SSoT Levels for priority boosting."""
        boosted_levels: Set[int] = set()

        # Keywords mapping directly to SSoT levels
        level_keywords = {
            0: ["timeline", "history", "archivist", "theta", "mission", "report", "milestones", "record", "archive", "log"],
            1: ["sop", "rules", "rulebook", "protocol", "agents", "command", "handshake", "setup", "constraint", "savesystem"],
            2: ["jules", "persona", "aas", "arbitrate", "identity", "schema", "credential", "precedence", "viability", "vss_breakdown"],
            3: ["psta", "mathematics", "formula", "health", "score", "dimension", "coherence", "coherence", "unweighted", "uncertainty", "epistemic", "severance", "kernel"],
            4: ["backlog", "sprint", "ticket", "devops", "implementation", "checklist", "verification", "crash", "bug", "terminal", "wisp", "unreal"]
        }

        for level, keywords in level_keywords.items():
            for kw in keywords:
                if kw in query_tokens:
                    boosted_levels.add(level)

        return boosted_levels
