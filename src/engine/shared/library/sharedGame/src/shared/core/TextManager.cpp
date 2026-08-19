// ============================================================================
//
// TextManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/TextManager.h"

#include "LocalizationManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/TextIterator.h"
#include "sharedMath/PackedRgb.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"
#include "utf8.h"

#include <map>
#include <vector>

// ============================================================================
//
// TextManagerNamespace
//
// ============================================================================

// ----------------------------------------------------------------------------
namespace TextManagerNamespace
{
	typedef std::map<Unicode::String, bool> CussWords;         // string/allow sub-string match
	typedef std::vector<Unicode::String>    UnicodeStringVector;

	bool      s_installed = false;
	CussWords s_cussWords;

	void getAppropriateWord(Unicode::String &text);
	void tokenize(const Unicode::String &source, UnicodeStringVector &target);
	void getFilterLetter(Unicode::String &filterLetter);
	bool isExactMatch(Unicode::String const &text);
	bool isAlpha(Unicode::unicode_char_t const character);
	void checkText(Unicode::String &text, int const startIndex, int const endIndex);
}

using namespace TextManagerNamespace;

// ----------------------------------------------------------------------------
bool TextManagerNamespace::isExactMatch(Unicode::String const &text)
{
	bool result = false;
	Unicode::String lowerText(Unicode::toLower(text));
	CussWords::iterator iterCussWords = s_cussWords.find(lowerText);

	if (iterCussWords != s_cussWords.end())
	{
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::getAppropriateWord(Unicode::String &text)
{
	if (isExactMatch(text))
	{
		// An exact word match was found, plug the bad word with symbols

		unsigned int const letterCount = text.size();
		Unicode::String filterLetter;

		// Found a bad word, replace the word

		for (unsigned int i = 0; i < letterCount; ++i)
		{
			getFilterLetter(filterLetter);

			text[i] = filterLetter[0];
		}
	}
	else
	{
		// See if we can find a non-exact match, but a match by removing
		// all non-alphabet letters and condensing the result into a word

		Unicode::String alphabetString;
		alphabetString.reserve(text.size());
		int index = 0;

		for (unsigned int i = 0; i < text.size(); ++i)
		{
			if (isAlpha(text[i]))
			{
				alphabetString.push_back(text[i]);
				++index;
			}
		}

		if (isExactMatch(alphabetString))
		{
			// Now we found an exact match so plug all the alpha letters

			Unicode::String filterLetter;

			for (unsigned int i = 0; i < text.size(); ++i)
			{
				if (isAlpha(text[i]))
				{
					getFilterLetter(filterLetter);

					text[i] = filterLetter[0];
				}
			}
		}
		else
		{
			// Sub-string search time for words that are listed for sub-string searches

			Unicode::String lowerText(Unicode::toLower(text));
			// Use size_t to match basic_string::find/npos. The old code used
			// `unsigned int`, which on x64 truncates npos (8 bytes) to
			// 0xFFFFFFFF (4 bytes), making "is not found" checks always
			// false and causing out-of-bounds writes to text[index+i].
			size_t findStartPosition = 0;
			size_t const textSize = text.size();

			// Hard backstop on outer iterations - if we ever fail to make forward
			// progress (e.g. due to a zero-length cuss word or any other reason)
			// we must NOT loop forever. Two passes per character is more than
			// enough since each successful pass advances findStartPosition.
			size_t const maxOuterIterations = (textSize + 1) * 2 + 16;
			size_t outerIterations = 0;

			for (;;)
			{
				if (++outerIterations > maxOuterIterations)
					break;

				if (findStartPosition >= textSize)
					break;

				bool done = true;
				CussWords::iterator iterCussWords = s_cussWords.begin();

				for (; iterCussWords != s_cussWords.end(); ++iterCussWords)
				{
					Unicode::String const &cussWord = iterCussWords->first;
					bool const allowSubStringSearch = iterCussWords->second;

					if (!allowSubStringSearch)
					{
						continue;
					}

					// Skip empty cuss words - find() returns findStartPosition for
					// an empty needle which would infinite-loop here.
					if (cussWord.empty())
					{
						continue;
					}

					size_t index = lowerText.find(cussWord, findStartPosition);

					if (index != Unicode::String::npos)
					{
						// Defensive: index must lie within text. Both lowerText and
						// text should be the same size (toLower preserves length)
						// but never trust that on x64 after the historic truncation
						// bugs we've found.
						if (index >= textSize)
						{
							findStartPosition = textSize;
							done = true;
							break;
						}

						findStartPosition = index;

						// Found a bad word, replace the word

						size_t const letterCount = cussWord.size();
						Unicode::String filterLetter;

						// Cap the replacement at the actual remaining text size.
						// Avoid underflow if index > textSize (shouldn't happen given
						// the guard above, but belt-and-suspenders).
						size_t const remaining = (textSize > index) ? (textSize - index) : 0;
						size_t const upperLimit = (letterCount < remaining) ? letterCount : remaining;
						for (size_t i = 0; i < upperLimit; ++i)
						{
							// Hard bounds check to prevent any oob write.
							if ((index + i) >= textSize)
								break;

							getFilterLetter(filterLetter);

							if (!filterLetter.empty())
								text[index + i] = filterLetter[0];
						}

						// Advance findStartPosition past this match by at least 1
						// so we always make forward progress, even when letterCount
						// was zero or upperLimit was clipped.
						findStartPosition = index + (upperLimit > 0 ? upperLimit : 1);

						// We are not done with a clean run so start over in case
						// there are multiple cuss words in the same string

						done = false;
						break;
					}
				}

				if (done)
				{
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::getFilterLetter(Unicode::String &filterLetter)
{
	int number = rand() % 7;

	switch (number)
	{
		default:
		case 1: { filterLetter = Unicode::narrowToWide("@"); } break;
		case 2: { filterLetter = Unicode::narrowToWide("#"); } break;
		case 3: { filterLetter = Unicode::narrowToWide("$"); } break;
		case 4: { filterLetter = Unicode::narrowToWide("%"); } break;
		case 5: { filterLetter = Unicode::narrowToWide("&"); } break;
		case 6: { filterLetter = Unicode::narrowToWide("*"); } break;
	}
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::tokenize(const Unicode::String &source, UnicodeStringVector &target)
{
	UnicodeStringVector unicodeStringVector;

	IGNORE_RETURN(Unicode::tokenize(source, unicodeStringVector));

	target = unicodeStringVector;
}

// ----------------------------------------------------------------------------
bool TextManagerNamespace::isAlpha(Unicode::unicode_char_t const character)
{
	bool result = false;

	if (   (character <= 255)
		&& isalpha(character))
	{
		result = true;
	}

	return result;
}

// ----------------------------------------------------------------------------
void TextManagerNamespace::checkText(Unicode::String &text, int const startIndex, int const endIndex)
{
	// Defensive bounds checks: avoid std::out_of_range from substr/operator[].
	if (startIndex < 0 || endIndex <= startIndex || static_cast<size_t>(endIndex) > text.size())
		return;

	int const length = endIndex - startIndex;
	Unicode::String subString(text.substr(startIndex, length));

	getAppropriateWord(subString);

	// Put the fixed text back into the string - cap the loop at the subString size
	// in case getAppropriateWord shrank it.
	int const upper = std::min<int>(length, static_cast<int>(subString.size()));
	for (int index = 0; index < upper; ++index)
	{
		text[startIndex + index] = subString[index];
	}
}

// ============================================================================
//
// TextManager
//
// ============================================================================

// ----------------------------------------------------------------------------
void TextManager::install()
{
	InstallTimer const installTimer("TextManager::install");

	DEBUG_FATAL(s_installed,("TextManager already installed.\n"));

	// Cuss words

	char const profanityFile[] = "datatables/chat/profanity_filter.iff";
	Iff profanityFilterIff;

	if (profanityFilterIff.open(profanityFile, true))
	{
		DataTable dataTable;

		dataTable.load(profanityFilterIff);

		int const rowCount = dataTable.getNumRows();

		s_cussWords.clear();

		// Add all the words to the list in lowercase

		for (int index = 0; index < rowCount; ++index)
		{
			std::string const &word = dataTable.getStringValue(0, index);
			bool const allowSubStringMatch = (dataTable.getIntValue(1, index) != 0);

			Unicode::UTF16 wordBuf[50]{};
			Unicode::UTF8_convertToUTF16(const_cast<char *>(word.c_str()), wordBuf, std::size(wordBuf));

			Unicode::String unicodeWord(reinterpret_cast<const char16_t *>(wordBuf));

			s_cussWords.insert(std::make_pair(Unicode::toLower(unicodeWord), allowSubStringMatch));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load the profanity filter data table: %s", profanityFile));
	}

	ExitChain::add(remove,"TextManager::remove");
	s_installed = true;
}

// ----------------------------------------------------------------------------
void TextManager::remove()
{
	s_cussWords.clear();

	s_installed = false;
}

// Filter the text for any profanity.
// ----------------------------------------------------------------------------
Unicode::String TextManager::filterText(Unicode::String const &text)
{
	Unicode::String result(text);
	Unicode::String::const_iterator iterText = text.begin();
	// Use -1 as the "unset" sentinel for the int index. The old code stored
	// Unicode::String::npos in an int, which is fine on x86 (npos==0xFFFFFFFF)
	// but on x64 (npos==0xFFFFFFFFFFFFFFFF) the value truncates to -1 in int,
	// and the subsequent unsigned-vs-size_t compare `static_cast<unsigned>(-1) ==
	// npos` is FALSE because 0x00000000FFFFFFFF != 0xFFFFFFFFFFFFFFFF. That broke
	// the "is unset" check entirely on x64 and let stale indices pass into
	// substr()/operator[] causing std::out_of_range in checkText.
	int startIndex = -1;
	int currentIndex = 0;

	{
		// Walk through the text finding sections of alpha characters.
		// Each set of alpha characters is checked for cuss words, all
		// non-alphabet is skipped and preserved.

		for (; iterText != text.end(); ++iterText)
		{
			Unicode::unicode_char_t const character = (*iterText);

			if (startIndex < 0 && isAlpha(character))
			{
				startIndex = currentIndex;
			}
			else if (startIndex >= 0 && !isAlpha(character))
			{
				checkText(result, startIndex, currentIndex);

				startIndex = -1;
			}

			++currentIndex;
		}

		// Possibly check the last word

		if (startIndex >= 0)
		{
			checkText(result, startIndex, currentIndex);
		}
	}

	{
		// Walk through the text finding sections between spaces.
		// Each set of text between spaces is checked for cuss words,
		// all non-alphabet is skipped and preserved.

		iterText = text.begin();
		startIndex = -1;
		currentIndex = 0;

		for (; iterText != text.end(); ++iterText)
		{
			Unicode::unicode_char_t const character = (*iterText);

			if (startIndex < 0 && character != static_cast<Unicode::unicode_char_t>(' '))
			{
				startIndex = currentIndex;
			}
			else if (startIndex >= 0 && character == static_cast<Unicode::unicode_char_t>(' '))
			{
				checkText(result, startIndex, currentIndex);

				startIndex = -1;
			}

			++currentIndex;
		}

		// Possibly check the last word

		if (startIndex >= 0)
		{
			checkText(result, startIndex, currentIndex);
		}
	}

	return result;
}

// ----------------------------------------------------------------------------
bool TextManager::isAppropriateText(Unicode::String const &text)
{
	bool result = false;
	Unicode::String filteredText(filterText(text));

	// If the text looks clean so far, check the text again with color
	// codes removed.

	if (filteredText == text)
	{
		result = true;

		TextIterator textIterator(filteredText);
		Unicode::String colorCodeRemovedText(textIterator.getPrintableText());

		// Make sure there was some color codes removed before we waste time
		// trying to filter it

		if (filteredText.size() != colorCodeRemovedText.size())
		{
			Unicode::String colorCodeRemovedFilteredText(filterText(colorCodeRemovedText));

			if (colorCodeRemovedText != colorCodeRemovedFilteredText)
			{
				result = false;
			}
		}
	}

	return result;
}

// ============================================================================
