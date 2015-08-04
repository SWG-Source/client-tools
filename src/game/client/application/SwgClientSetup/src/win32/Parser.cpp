// ======================================================================
//
// Parser.cpp
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "Parser.h"
#include <wchar.h>


// ======================================================================

namespace ParserNamespace
{
	// -----------------------------------
	CString nullString;

	// ------------------------------------
	Parser::Token tokenLower(Parser::Token const & token)
	{
		Parser::Token lowerToken(token);
		lowerToken.MakeLower();
		return lowerToken;
	}

	// -----------------------------------
	Parser::TokenMap::const_iterator findToken(CString const & lineOriginal, Parser::TokenMap const & tokens)
	{
		CString line(lineOriginal);
		line.MakeLower();

		Parser::TokenMap::const_iterator itToken = tokens.begin();

		for (;itToken != tokens.end(); ++itToken) 
		{
			// Get the token.
			Parser::Token const & token = itToken->first;

			// See if we find something.
			int const tokenIndex = line.Find(token);

			// Check to see if the token index 0.
			if (tokenIndex == 0)
			{
				return itToken;
			}
		}

		return tokens.end();
	}
	
	// -----------------------------------
	void advanceLinePastToken(CString & originalLine, Parser::Token const & token)
	{
		// Case insensitive stuff.
		CString line(originalLine);
		line.MakeLower();

		// See if we find something.
		int const tokenIndex = line.Find(token);
		
		// Check to see if the token index 0.
		if (tokenIndex >= 0)
		{
			originalLine.Delete(tokenIndex, token.GetLength());
			trimString(originalLine);
		}
	}

	// -----------------------------------
	void printError(CString codeFile, int codeMode, int codeLine, CString scriptFile, int scriptLine, CString scriptToken, int scriptNextTokenType)
	{
		CString errorString;

		errorString.Format(_T("Parse Error Detected.\nCode:   File %s, Mode %d, Line %d\nScript: File %s, Line %d, Token %s, NextTokenType %d\n"), 
								codeFile.GetBuffer(0), codeMode, codeLine,
								scriptFile.GetBuffer(0), scriptLine, scriptToken.GetBuffer(0), scriptNextTokenType);

		MessageBox(NULL, errorString, _T("Parser Error"), MB_OK | MB_ICONERROR);
	}
}

// ======================================================================

using namespace ParserNamespace;

// ----------------------------------------------------------------------

int Parser::mLineCount = -1;

// ----------------------------------------------------------------------

#define postError() { mError = true; printError(_T(__FILE__), static_cast<int>(tokenType), __LINE__, file.GetFileName(), mLineCount, token, static_cast<int>(mNextTokenType));}

// ----------------------------------------------------------------------

Parser::Parser() : mNextTokenType(tt_begin), mTokens(), mNodes(), mEof(false), mError(false)
{
}

// ----------------------------------------------------------------------

Parser::~Parser()
{
}

// ----------------------------------------------------------------------

bool Parser::parse(CStdioFile & file)
{
	// Temporary flags.
	bool done = false;
	bool parsedText = false;
	CString line;

	// Reset persistent data.
	mError = false;
	mNextTokenType = tt_begin;
	mNodes.clear();

	// Parse text.
	while (!done && !mError && getNextLine(file, line))
	{
		mLineCount++;

		// Clean up whitespace.
		trimString(line);

		// Do not parse empty lines.
		if (!line.GetLength())
		{
			continue;
		}

		// Look for comments.
		int const commentIndex = line.Find(_T("#"));
		if (commentIndex >= 0) 
		{
			// Text after comments are ignored.
			line.SetAt(commentIndex, 0);
		}

		// Find the token!
		TokenMap::const_iterator itToken = findToken(line, mTokens);
		if (itToken != mTokens.end())
		{
			// Get token and type.
			Token const & token = itToken->first;
			TokenType const & tokenType = itToken->second;

			// Parse based on token type.
			switch(tokenType) 
			{
			// -----------------------------------
			case tt_begin:
				if (tt_begin == mNextTokenType) 
				{
					mNextTokenType = tt_store;
				}
				else
				{
					postError();
				}
				break;

			// -----------------------------------
			case tt_store:
				if (tt_store == mNextTokenType) 
				{
					advanceLinePastToken(line, token);
					addNode(token, line, iswpunct(line.GetAt(0)) ? Parser::nf_default : Parser::nf_none);
					parsedText = true;
				}
				else
				{
					postError();
				}
				break;

			// -----------------------------------
			case tt_end:
				if (tt_store == mNextTokenType) 
				{
					done = true;
					mNextTokenType = tt_end;
				}
				else
				{
					postError();
				}
				break;

			default:
				postError();
			}
		}
	}

	return !mError && parsedText;
}

// ----------------------------------------------------------------------

void Parser::addToken(Token const & token, TokenType const & ttype)
{
	mTokens.insert(std::make_pair(tokenLower(token), ttype));
}

// ----------------------------------------------------------------------

void Parser::addNode(Token const & token, CString const & line, NodeFlag flag)
{
	CString trimmedLine(line);

	trimString(trimmedLine);

	mNodes.insert(std::make_pair(tokenLower(token), Node(trimmedLine, flag)));
}

// ----------------------------------------------------------------------

CString const & Parser::getTokenAsString(Token const & tokenName) const
{
	// Find the first instance.
	Nodes::const_iterator const itNode = mNodes.find(tokenLower(tokenName));
	
	if (itNode != mNodes.end())
	{
		Node const & node = itNode->second;
		
		return node.first;
	}
	
	return nullString;
}

// ----------------------------------------------------------------------

int Parser::getTokenAsInt(CString const & tokenName) const
{
	CString & data = const_cast<CString &>(getTokenAsString(tokenName));
	return static_cast<int>(_wtol(data.GetBuffer(0)));
}

// ----------------------------------------------------------------------

bool Parser::getTokenAsBool(Token const & tokenName) const
{
	CString boolString = getTokenAsString(tokenName);
	boolString.MakeLower();

	trimString(boolString);

	bool isTrue = !boolString.IsEmpty() && (boolString.Find(_T("t")) == 0);
	return isTrue || getTokenAsInt(tokenName) != 0;
}

// ----------------------------------------------------------------------

float Parser::getTokenAsFloat(CString const & tokenName) const
{
	CString & data = const_cast<CString &>(getTokenAsString(tokenName));
	return static_cast<float>(wcstod(data.GetBuffer(0), NULL));
}

// ----------------------------------------------------------------------

bool Parser::getNextLine(CStdioFile & file, CString & line)
{
	mEof = !file.ReadString(line);
	return !mEof;
}

// ----------------------------------------------------------------------

void Parser::getNodes(Token const & token, Nodes & nodes) const
{
	std::pair<Nodes::const_iterator, Nodes::const_iterator> foundNodes = mNodes.equal_range(tokenLower(token));
	
	for (Nodes::const_iterator itNode = foundNodes.first; itNode != foundNodes.second; ++itNode) 
	{
		nodes.insert(*itNode);
	}
}

// ----------------------------------------------------------------------

bool Parser::hasToken(Token const & tokenName) const
{
	Nodes::const_iterator const itNode = mNodes.find(tokenLower(tokenName));
	return itNode != mNodes.end();
}