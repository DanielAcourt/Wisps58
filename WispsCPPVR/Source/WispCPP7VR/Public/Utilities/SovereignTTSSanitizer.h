// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SovereignTTSSanitizer.generated.h"

/**
 * USovereignTTSSanitizer
 * C++ Fallback String Sanitizer and Sentence Chunker for Unreal Engine Flite TTS (AD-037).
 * Prevents buffer underruns and FliteTextToSpeechSubmixListener ensure failures by stripping
 * markdown syntax, escape sequences, symbols, and splitting text into chunks under 200 chars.
 */
UCLASS()
class WISPCPP7VR_API USovereignTTSSanitizer : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Converts mathematical, monetary, and common symbols into spoken English words.
     * Example: "$10.50" -> "10.50 dollars", "50%" -> "50 percent"
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|TTS")
    static FString ConvertSymbolsToSpokenWords(const FString& InText);

    /**
     * Strips markdown formatting (**bold**, *italic*, `code`, # headers, [links](url), // comments)
     * and escape codes.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|TTS")
    static FString StripMarkdownAndEscapes(const FString& InText);

    /**
     * Full sanitization pipeline: converts symbols and strips markdown/escapes.
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|TTS")
    static FString SanitizeTextForTTS(const FString& InText);

    /**
     * Splits sanitized text into clean sentence/clause chunks under MaxChars limit (default 200).
     */
    UFUNCTION(BlueprintCallable, Category = "Sovereign|TTS")
    static TArray<FString> ChunkTextForTTS(const FString& InText, int32 MaxChars = 200);

private:
    static TArray<FString> SplitOversizedSentence(const FString& Sentence, int32 MaxChars);
};
