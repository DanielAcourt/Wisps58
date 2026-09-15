# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
TTS Audio Guardrails & Flite Subsystem Sanitizer Module (AD-037)

Provides string sanitization and sentence chunking to prevent Unreal Engine
Flite TTS buffer underruns and submix listener ensure failures caused by markdown
syntax, escape sequences, symbols, or long text strings (> 200 chars).
"""

import re
from typing import List, Dict, Any


def convert_symbols_to_spoken_words(text: str) -> str:
    """
    Converts mathematical, monetary, and common symbols into spoken English words.
    Examples:
        '$10' -> '10 dollars'
        '50%' -> '50 percent'
        '100°C' -> '100 degrees Celsius'
        '&' -> 'and'
    """
    if not text:
        return ""

    # Currency ($10 -> 10 dollars, $10.50 -> 10.50 dollars)
    text = re.sub(r'\$(\d+(?:\.\d+)?)', r'\1 dollars', text)

    # Temperature units (°C, °F, °)
    text = re.sub(r'°C\b', ' degrees Celsius', text)
    text = re.sub(r'°F\b', ' degrees Fahrenheit', text)
    text = re.sub(r'°', ' degrees', text)

    # Percentage (50% -> 50 percent)
    text = re.sub(r'(\d+(?:\.\d+)?)\s*%', r'\1 percent', text)

    # Number symbol (#1 -> number 1)
    text = re.sub(r'#(\d+)', r'number \1', text)

    # Standalone or spaced symbol replacements
    replacements = [
        (r'\s*&\s*', ' and '),
        (r'\s*@\s*', ' at '),
        (r'\s*\+\s*', ' plus '),
        (r'\s*=\s*', ' equals '),
    ]

    for pattern, replacement in replacements:
        text = re.sub(pattern, replacement, text)

    return text


def strip_markdown_and_escapes(text: str) -> str:
    """
    Strips markdown syntax (**bold**, *italic*, `code`, # headers, [links](url), // comments)
    and escape codes.
    """
    if not text:
        return ""

    # Strip code blocks ```...``` and inline code `...`
    text = re.sub(r'```[\s\S]*?```', '', text)
    text = re.sub(r'`([^`]+)`', r'\1', text)

    # Strip markdown links [text](url) -> text
    text = re.sub(r'\[([^\]]+)\]\([^\)]+\)', r'\1', text)

    # Strip bold and italics (**text**, *text*, __text__, _text_)
    text = re.sub(r'\*\*([^*]+)\*\*', r'\1', text)
    text = re.sub(r'\*([^*]+)\*', r'\1', text)
    text = re.sub(r'__([^_]+)__', r'\1', text)
    text = re.sub(r'_([^_]+)_', r'\1', text)

    # Strip headers (# Header -> Header)
    text = re.sub(r'^\s*#+\s*', '', text, flags=re.MULTILINE)

    # Strip blockquotes (> quote -> quote)
    text = re.sub(r'^\s*>\s*', '', text, flags=re.MULTILINE)

    # Strip list bullets (- item, * item, 1. item)
    text = re.sub(r'^\s*[\*\-\+]\s+', '', text, flags=re.MULTILINE)
    text = re.sub(r'^\s*\d+\.\s+', '', text, flags=re.MULTILINE)

    # Strip C-style comments (// comment)
    text = re.sub(r'//.*$', '', text, flags=re.MULTILINE)

    # Strip escape backslashes (\_, \*, \n, etc.)
    text = re.sub(r'\\([_*\\[\](){}#+.!-])', r'\1', text)

    # Convert newlines and tabs to spaces
    text = re.sub(r'[\r\n\t]+', ' ', text)

    # Strip unpronounceable bracket/pipe/tilde symbols
    text = re.sub(r'[~^|{}<>[\]\\]', ' ', text)

    # Collapse multiple spaces into one
    text = re.sub(r'\s+', ' ', text).strip()

    return text


def sanitize_text(text: str) -> str:
    """
    Full sanitization pipeline: converts symbols to spoken words, strips markdown/escapes,
    and returns clean, natural speech text.
    """
    if not text:
        return ""
    text_converted = convert_symbols_to_spoken_words(text)
    sanitized = strip_markdown_and_escapes(text_converted)
    return sanitized


def chunk_text(text: str, max_chars: int = 200) -> List[str]:
    """
    Splits sanitized text into clean, natural sentence chunks under max_chars limit (default 200).
    Splits first on sentence boundaries (. ! ? ;), then on clauses (, : -), then on word boundaries.
    """
    sanitized = sanitize_text(text)
    if not sanitized:
        return []

    if len(sanitized) <= max_chars:
        return [sanitized]

    # Split into raw sentences while preserving punctuation
    sentence_pattern = r'(?<=[.!?;])\s+'
    raw_sentences = [s.strip() for s in re.split(sentence_pattern, sanitized) if s.strip()]

    chunks: List[str] = []
    current_chunk = ""

    for sentence in raw_sentences:
        # Handle oversized sentences (> max_chars) by splitting on clauses or words
        sub_sentences = _split_oversized_sentence(sentence, max_chars) if len(sentence) > max_chars else [sentence]

        for sub in sub_sentences:
            if not current_chunk:
                current_chunk = sub
            elif len(current_chunk) + 1 + len(sub) <= max_chars:
                current_chunk += " " + sub
            else:
                chunks.append(current_chunk.strip())
                current_chunk = sub

    if current_chunk:
        chunks.append(current_chunk.strip())

    return chunks


def _split_oversized_sentence(sentence: str, max_chars: int) -> List[str]:
    """
    Helper function to decompose sentences longer than max_chars into clauses or word groups.
    """
    if len(sentence) <= max_chars:
        return [sentence]

    # Split on clauses (, : - —)
    clause_pattern = r'(?<=[,:\-—])\s+'
    clauses = [c.strip() for c in re.split(clause_pattern, sentence) if c.strip()]

    clause_chunks: List[str] = []
    current = ""

    for clause in clauses:
        if len(clause) > max_chars:
            # Word-level fallback for extremely long clauses
            words = clause.split()
            word_chunk = ""
            for word in words:
                if not word_chunk:
                    word_chunk = word
                elif len(word_chunk) + 1 + len(word) <= max_chars:
                    word_chunk += " " + word
                else:
                    clause_chunks.append(word_chunk.strip())
                    word_chunk = word
            if word_chunk:
                clause_chunks.append(word_chunk.strip())
        else:
            if not current:
                current = clause
            elif len(current) + 1 + len(clause) <= max_chars:
                current += " " + clause
            else:
                clause_chunks.append(current.strip())
                current = clause

    if current:
        clause_chunks.append(current.strip())

    return clause_chunks


def sanitize_and_chunk(text: str, max_chars: int = 200) -> Dict[str, Any]:
    """
    Convenience function returning full sanitized text and chunked array.
    """
    sanitized = sanitize_text(text)
    chunks = chunk_text(text, max_chars=max_chars)
    return {
        "sanitized_text": sanitized,
        "chunks": chunks,
        "chunk_count": len(chunks)
    }
