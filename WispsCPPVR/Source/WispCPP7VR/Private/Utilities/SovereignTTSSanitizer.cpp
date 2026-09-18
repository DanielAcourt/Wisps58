// Copyright (c) 2013-2026 Daniel Acourt. Version 37.0.0. Licensed under GPLv3 (See LICENSE).

#include "Utilities/SovereignTTSSanitizer.h"
#include "Internationalization/Regex.h"

FString USovereignTTSSanitizer::ConvertSymbolsToSpokenWords(const FString& InText)
{
    if (InText.IsEmpty())
    {
        return FString();
    }

    FString Text = InText;

    // Currency ($10 -> 10 dollars, $10.50 -> 10.50 dollars)
    {
        const FRegexPattern Pattern(TEXT("\\$(\\d+(?:\\.\\d+)?)"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            FString Val = Matcher.GetCaptureGroup(1);
            Result += Val + TEXT(" dollars");
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Temperature (°C -> degrees Celsius, °F -> degrees Fahrenheit, ° -> degrees)
    Text = Text.Replace(TEXT("°C"), TEXT(" degrees Celsius"));
    Text = Text.Replace(TEXT("°F"), TEXT(" degrees Fahrenheit"));
    Text = Text.Replace(TEXT("°"), TEXT(" degrees"));

    // Percentage (50% -> 50 percent)
    {
        const FRegexPattern Pattern(TEXT("(\\d+(?:\\.\\d+)?)\\s*%"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            FString Val = Matcher.GetCaptureGroup(1);
            Result += Val + TEXT(" percent");
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Number symbol (#1 -> number 1)
    {
        const FRegexPattern Pattern(TEXT("#(\\d+)"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            FString Val = Matcher.GetCaptureGroup(1);
            Result += TEXT("number ") + Val;
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Common symbols
    Text = Text.Replace(TEXT("&"), TEXT(" and "));
    Text = Text.Replace(TEXT("@"), TEXT(" at "));
    Text = Text.Replace(TEXT("+"), TEXT(" plus "));
    Text = Text.Replace(TEXT("="), TEXT(" equals "));

    return Text;
}

FString USovereignTTSSanitizer::StripMarkdownAndEscapes(const FString& InText)
{
    if (InText.IsEmpty())
    {
        return FString();
    }

    FString Text = InText;

    // Strip C-style comments (// comment)
    {
        const FRegexPattern Pattern(TEXT("//.*$"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Strip markdown bold (**text** -> text)
    {
        const FRegexPattern Pattern(TEXT("\\*\\*([^*]+)\\*\\*"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            Result += Matcher.GetCaptureGroup(1);
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Strip markdown italics (*text* -> text)
    {
        const FRegexPattern Pattern(TEXT("\\*([^*]+)\\*"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            Result += Matcher.GetCaptureGroup(1);
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Strip inline code (`text` -> text)
    {
        const FRegexPattern Pattern(TEXT("`([^`]+)`"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            Result += Matcher.GetCaptureGroup(1);
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Strip markdown links ([text](url) -> text)
    {
        const FRegexPattern Pattern(TEXT("\\[([^\\]]+)\\]\\([^\\)]+\\)"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            Result += Matcher.GetCaptureGroup(1);
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result;
    }

    // Replace underscores in identifiers (BP_Antelope_C_5 -> BP Antelope C 5)
    {
        const FRegexPattern Pattern(TEXT("([A-Za-z0-9])_([A-Za-z0-9])"));
        while (Text.Contains(TEXT("_")))
        {
            FRegexMatcher Matcher(Pattern, Text);
            FString Result;
            int32 LastPos = 0;
            bool bFoundMatch = false;
            while (Matcher.FindNext())
            {
                bFoundMatch = true;
                int32 MatchStart = Matcher.GetMatchBeginning();
                int32 MatchEnd = Matcher.GetMatchEnding();
                Result += Text.Mid(LastPos, MatchStart - LastPos);
                Result += Matcher.GetCaptureGroup(1) + TEXT(" ") + Matcher.GetCaptureGroup(2);
                LastPos = MatchEnd;
            }
            Result += Text.Mid(LastPos);
            if (!bFoundMatch || Result == Text)
            {
                break;
            }
            Text = Result;
        }
    }

    // Strip escapes (\_ -> _, \* -> *, etc.)
    Text = Text.Replace(TEXT("\\_"), TEXT(" "));
    Text = Text.Replace(TEXT("\\*"), TEXT(" "));
    Text = Text.Replace(TEXT("\\#"), TEXT(" "));
    Text = Text.Replace(TEXT("\\`"), TEXT(" "));

    // Replace newlines and tabs with spaces
    Text = Text.Replace(TEXT("\r\n"), TEXT(" "));
    Text = Text.Replace(TEXT("\n"), TEXT(" "));
    Text = Text.Replace(TEXT("\r"), TEXT(" "));
    Text = Text.Replace(TEXT("\t"), TEXT(" "));

    // Final cleanup pass: strip ANY remaining asterisks, backticks, underscores, or unpronounceable characters
    TArray<FString> BadChars = { TEXT("*"), TEXT("`"), TEXT("_"), TEXT("~"), TEXT("^"), TEXT("|"), TEXT("{"), TEXT("}"), TEXT("<"), TEXT(">"), TEXT("["), TEXT("]"), TEXT("\\") };
    for (const FString& BadChar : BadChars)
    {
        Text = Text.Replace(*BadChar, TEXT(" "));
    }

    // Collapse whitespace
    {
        const FRegexPattern Pattern(TEXT("\\s+"));
        FRegexMatcher Matcher(Pattern, Text);
        FString Result;
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            Result += Text.Mid(LastPos, MatchStart - LastPos);
            Result += TEXT(" ");
            LastPos = MatchEnd;
        }
        Result += Text.Mid(LastPos);
        Text = Result.TrimStartAndEnd();
    }

    return Text;
}

FString USovereignTTSSanitizer::SanitizeTextForTTS(const FString& InText)
{
    FString Converted = ConvertSymbolsToSpokenWords(InText);
    return StripMarkdownAndEscapes(Converted);
}

TArray<FString> USovereignTTSSanitizer::ChunkTextForTTS(const FString& InText, int32 MaxChars)
{
    TArray<FString> Chunks;
    FString Sanitized = SanitizeTextForTTS(InText);

    if (Sanitized.IsEmpty())
    {
        return Chunks;
    }

    if (Sanitized.Len() <= MaxChars)
    {
        Chunks.Add(Sanitized);
        return Chunks;
    }

    // Split into sentences on punctuation (. ! ? ;)
    TArray<FString> RawSentences;
    {
        const FRegexPattern Pattern(TEXT("(?<=[.!?;])\\s+"));
        FRegexMatcher Matcher(Pattern, Sanitized);
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            FString Sent = Sanitized.Mid(LastPos, MatchStart - LastPos).TrimStartAndEnd();
            if (!Sent.IsEmpty())
            {
                RawSentences.Add(Sent);
            }
            LastPos = MatchEnd;
        }
        FString Remainder = Sanitized.Mid(LastPos).TrimStartAndEnd();
        if (!Remainder.IsEmpty())
        {
            RawSentences.Add(Remainder);
        }
    }

    FString CurrentChunk;

    for (const FString& Sentence : RawSentences)
    {
        TArray<FString> SubSentences = (Sentence.Len() > MaxChars) ? SplitOversizedSentence(Sentence, MaxChars) : TArray<FString>{ Sentence };

        for (const FString& Sub : SubSentences)
        {
            if (CurrentChunk.IsEmpty())
            {
                CurrentChunk = Sub;
            }
            else if (CurrentChunk.Len() + 1 + Sub.Len() <= MaxChars)
            {
                CurrentChunk += TEXT(" ") + Sub;
            }
            else
            {
                Chunks.Add(CurrentChunk.TrimStartAndEnd());
                CurrentChunk = Sub;
            }
        }
    }

    if (!CurrentChunk.IsEmpty())
    {
        Chunks.Add(CurrentChunk.TrimStartAndEnd());
    }

    return Chunks;
}

TArray<FString> USovereignTTSSanitizer::SplitOversizedSentence(const FString& Sentence, int32 MaxChars)
{
    TArray<FString> ClauseChunks;

    if (Sentence.Len() <= MaxChars)
    {
        ClauseChunks.Add(Sentence);
        return ClauseChunks;
    }

    // Split on clauses (, : -)
    TArray<FString> Clauses;
    {
        const FRegexPattern Pattern(TEXT("(?<=[,:\\-])\\s+"));
        FRegexMatcher Matcher(Pattern, Sentence);
        int32 LastPos = 0;
        while (Matcher.FindNext())
        {
            int32 MatchStart = Matcher.GetMatchBeginning();
            int32 MatchEnd = Matcher.GetMatchEnding();
            FString Clause = Sentence.Mid(LastPos, MatchStart - LastPos).TrimStartAndEnd();
            if (!Clause.IsEmpty())
            {
                Clauses.Add(Clause);
            }
            LastPos = MatchEnd;
        }
        FString Remainder = Sentence.Mid(LastPos).TrimStartAndEnd();
        if (!Remainder.IsEmpty())
        {
            Clauses.Add(Remainder);
        }
    }

    FString Current;

    for (const FString& Clause : Clauses)
    {
        if (Clause.Len() > MaxChars)
        {
            // Word-level fallback for long clauses
            TArray<FString> Words;
            Clause.ParseIntoArray(Words, TEXT(" "), true);
            FString WordChunk;

            for (const FString& Word : Words)
            {
                if (WordChunk.IsEmpty())
                {
                    WordChunk = Word;
                }
                else if (WordChunk.Len() + 1 + Word.Len() <= MaxChars)
                {
                    WordChunk += TEXT(" ") + Word;
                }
                else
                {
                    ClauseChunks.Add(WordChunk.TrimStartAndEnd());
                    WordChunk = Word;
                }
            }

            if (!WordChunk.IsEmpty())
            {
                ClauseChunks.Add(WordChunk.TrimStartAndEnd());
            }
        }
        else
        {
            if (Current.IsEmpty())
            {
                Current = Clause;
            }
            else if (Current.Len() + 1 + Clause.Len() <= MaxChars)
            {
                Current += TEXT(" ") + Clause;
            }
            else
            {
                ClauseChunks.Add(Current.TrimStartAndEnd());
                Current = Clause;
            }
        }
    }

    if (!Current.IsEmpty())
    {
        ClauseChunks.Add(Current.TrimStartAndEnd());
    }

    return ClauseChunks;
}
