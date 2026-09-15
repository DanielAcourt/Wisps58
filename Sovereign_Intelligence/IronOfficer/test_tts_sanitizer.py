# Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).
"""
Unit tests for TTS Audio Guardrails & Flite Subsystem Sanitizer (AD-037).
Tests markdown stripping, escape sequence removal, symbol expansion, and sentence chunking (< 200 chars).
"""

import unittest
import sys
import os

# Ensure local module import path
sys.path.insert(0, os.path.dirname(__file__))

import tts_sanitizer


class TestTTSSanitizer(unittest.TestCase):

    def test_convert_symbols_to_spoken_words(self):
        text = "Cost is $10.50 with a 50% discount at 100°C for item #1 & item #2."
        converted = tts_sanitizer.convert_symbols_to_spoken_words(text)
        self.assertIn("10.50 dollars", converted)
        self.assertIn("50 percent", converted)
        self.assertIn("100 degrees Celsius", converted)
        self.assertIn("number 1", converted)
        self.assertIn("and", converted)

    def test_strip_markdown_and_escapes(self):
        raw_text = "**Warning!** This is *italic* and `code` with [link](http://example.com). // Comment here\nNew line \\_ escaped."
        stripped = tts_sanitizer.strip_markdown_and_escapes(raw_text)
        self.assertNotIn("**", stripped)
        self.assertNotIn("*", stripped)
        self.assertNotIn("`", stripped)
        self.assertNotIn("// Comment", stripped)
        self.assertNotIn("\n", stripped)
        self.assertIn("Warning!", stripped)
        self.assertIn("link", stripped)

    def test_asterisks_and_class_identifiers(self):
        sample = "there are *3* antelope. According to current world manifest, class `BP_Antelope_C` registered: BP_Antelope_C_5."
        sanitized = tts_sanitizer.sanitize_text(sample)
        self.assertNotIn("*", sanitized)
        self.assertNotIn("`", sanitized)
        self.assertNotIn("_", sanitized)
        self.assertIn("there are 3 antelope", sanitized)
        self.assertIn("BP Antelope C", sanitized)
        self.assertIn("BP Antelope C 5", sanitized)

    def test_user_reported_antelope_response(self):
        sample = (
            "According to the current world manifest, there are **3** entities of class `BP_Antelope_C` registered:\n\n"
            "1.  `BP_Antelope_C_1`\n"
            "2.  `BP_Antelope_C_3`\n"
            "3.  `BP_Antelope_C_5`"
        )
        sanitized = tts_sanitizer.sanitize_text(sample)
        self.assertNotIn("*", sanitized)
        self.assertNotIn("`", sanitized)
        self.assertNotIn("_", sanitized)
        self.assertIn("there are 3 entities", sanitized)

    def test_chunk_text_under_200_chars(self):
        long_paragraph = (
            "The Iron Knight stands vigilant over the simulation grid. "
            "Telemetry signals indicate zero anomalies across all active sectors. "
            "A total of $500.00 was allocated for submix processing with 100% reliability. "
            "Everything is operating nominally under standard protocol parameters."
        )
        chunks = tts_sanitizer.chunk_text(long_paragraph, max_chars=120)
        self.assertTrue(len(chunks) > 1)
        for chunk in chunks:
            self.assertLessEqual(len(chunk), 120, f"Chunk exceeded max_chars: '{chunk}'")

    def test_oversized_single_sentence(self):
        oversized = (
            "This is an exceptionally long sentence that does not contain standard sentence-ending punctuation, "
            "but it has several clauses separated by commas, which should allow the sanitizer to break it down into smaller, "
            "more manageable natural speech audio chunks for Flite TTS without triggering buffer underruns."
        )
        chunks = tts_sanitizer.chunk_text(oversized, max_chars=100)
        self.assertTrue(len(chunks) >= 2)
        for chunk in chunks:
            self.assertLessEqual(len(chunk), 100, f"Chunk exceeded max_chars: '{chunk}'")

    def test_sanitize_and_chunk_convenience(self):
        text = "**Status:** 100% operational with $0 error count."
        result = tts_sanitizer.sanitize_and_chunk(text)
        self.assertIn("100 percent", result["sanitized_text"])
        self.assertIn("0 dollars", result["sanitized_text"])
        self.assertTrue(isinstance(result["chunks"], list))
        self.assertEqual(result["chunk_count"], len(result["chunks"]))


if __name__ == "__main__":
    unittest.main()
