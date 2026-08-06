@echo off
:: Copyright (c) 2013-2025 Daniel Acourt. Version 36.4.7. Licensed under GPLv3 (See LICENSE).
:: 07 Architect: Model Parity Auditor
:: Purpose: Audits the bridge vs. system models to ensure targeting alignment (Technical Pillar).

echo [07] Commencing Model Parity Audit...
echo.

echo --- 1. CLI PERSPECTIVE (Ollama Environment) ---
ollama list
echo.

echo --- 2. API PERSPECTIVE (REST Interface) ---
curl -s http://127.0.0.1:11434/api/tags | python -c "import sys, json; print('\n'.join([m['name'] for m in json.load(sys.stdin)['models']]))"
echo.

echo --- 3. ENVIRONMENT CHECK ---
echo OLLAMA_MODELS = %OLLAMA_MODELS%
echo.

echo [07] If lists 1 and 2 do not match, restart Ollama and the Bridge.
echo.
pause
