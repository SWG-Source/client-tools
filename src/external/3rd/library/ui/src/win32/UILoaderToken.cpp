#include "_precompile.h"

#include "UILoaderToken.h"
#include <cassert>
#include <cstdio>

#pragma warning (disable:4702)

static const char * const BadEOF       = "Encountered unexpected end of file";
static const char * const BadChar      = "Encountered unexpected character '%c'";
static const char * const BadCharKnown = "Encountered unexpected character '%c', expected '%c'";

inline bool EatWhitespace( const UINarrowString &Data, UINarrowString::const_iterator &i )
{
	for( ; i != Data.end(); ++i )
	{
		if( !isspace( *i ) )
			break;		
	}

	if( i == Data.end() )
		return false;

	return true;
}

inline bool EatWhitespace( const UINarrowString &Data, UINarrowString::const_iterator &i, UINarrowString &ParseError )
{
	for( ; i != Data.end(); ++i )
	{
		if( !isspace( *i ) )
			break;		
	}

	if( i == Data.end() )
	{
		ParseError = BadEOF;
		return false;
	}

	return true;
}

inline bool EatCharacter( const UINarrowString &Data, UINarrowString::const_iterator &i, UINarrowString &ParseError )
{
	if( ++i == Data.end() )
	{
		ParseError = BadEOF;
		return false;
	}
	return true;
}

inline bool EatCharacterAndTrailingWhitespace( const UINarrowString &Data, UINarrowString::const_iterator &i, UINarrowString &ParseError )
{
	if( !EatCharacter( Data, i, ParseError ) )
		return false;
	if( !EatWhitespace( Data, i, ParseError ) )
		return false;

	return true;
}		



bool UILoaderToken::CreateFromXML( const UINarrowString &Data, UINarrowString::const_iterator &i, UINarrowString &ParseError )
{
	enum
	{
		InHeader,
		InAttributeName,
		InAttributeValue,
	}
	CurrentState;

	UINarrowString::value_type StringDelimiter = 0;

	AttributeName.clear ();
	AttributeValue.clear ();

	Header.erase();
	Attributes.clear();
	IsContainer     = false;
	IsEndContainer  = false;
	IsData          = false;
	
	if( !EatWhitespace( Data, i ) )	// If we run out of string here we're ok, so don't set parse error
		return false;

	if( *i != '<' )
	{
		IsData = true;

		for( ; (*i != '<') && (i != Data.end()); ++i )
		{
			if( *i == '\\' )
			{
				++i;

				if( i == Data.end() )
					return true;
			}

			Header += *i;
		}
		
		return true;
	}

	if( !EatCharacter( Data, i, ParseError ) )
		return false;

	if( *i == '/' )
	{
		IsEndContainer = true;

		if( !EatCharacter( Data, i, ParseError ) )
			return false;
	}
	else if( (*i == '!') || (*i == '?') )
	{
		// Eat anything of these forms

		for( ; (*i != '>') && (i != Data.end()); ++i )
			{}

		if( i != Data.end() )
			++i;

		return false;
	}

	if( !EatWhitespace( Data, i, ParseError ) )
		return false;

	CurrentState = InHeader;

	const size_t SCRATCH_SIZE = 255;
	char scratch[SCRATCH_SIZE+1];

	for ( ; ; )
	{
		switch( CurrentState )
		{
			case InHeader:
				if( isalnum( *i ) )
				{
					Header += *i;

					if( !EatCharacter( Data, i, ParseError ) )
						return false;
				}
				else if( isspace( *i ) )
				{
					if( !EatWhitespace( Data, i, ParseError ) )
						return false;

					if( *i == '>' )
					{
						if( !IsEndContainer )
							IsContainer = true;

						// Valid for i to be Data.end() after this, so we don't use EatCharacter
						++i;
						return true;
					}
					else if( *i == '/' )
					{
						if( IsEndContainer )
						{
							_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '>');
							ParseError = scratch;
							return false;
						}

						if( !EatCharacter( Data, i, ParseError ) )
							return false;

						if( *i == '>' )
						{						
							IsContainer = false;

							// Valid for i to be Data.end() after this, so we don't use EatCharacter
							++i;
							return true;
						}
						else
						{
							_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '/');
							ParseError = scratch;
							return false;
						}
					}
					CurrentState = InAttributeName;
				}
				else if( *i == '>' )
				{
					if( !IsEndContainer )
						IsContainer = true;

					// Valid for i to be Data.end() after this, so we don't use EatCharacter
					++i;
					return true;
				}
				else if( *i == '/' )
				{
					if( IsEndContainer )
					{
						_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '>');
						ParseError = scratch;
						return false;
					}

					if( !EatCharacter( Data, i, ParseError ) )
						return false;

					if( *i == '>' )
					{
						IsContainer = false;

						// Valid for i to be Data.end() after this, so we don't use EatCharacter
						++i;
						return true;
					}
					else
					{
						_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '/');
						ParseError = scratch;
						return false;
					}
				}
				else
				{
					_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '>');
					ParseError = scratch;
					return false;
				}
				break;

			case InAttributeName:

				if( isalnum( *i ) || (*i == '.') || (*i == '_') || (*i == '+'))
				{
					if( IsEndContainer )
					{
						_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '>');
						ParseError = scratch;
						return false;
					}

					AttributeName += static_cast<char> (*i);

					if( !EatCharacter( Data, i, ParseError ) )
						return false;
				}
				else
				{
					if( !EatWhitespace( Data, i, ParseError ) )
						return false;

					if( *i != '=' )
					{
						_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '=');
						ParseError = scratch;
						return false;
					}

					if( !EatCharacterAndTrailingWhitespace( Data, i, ParseError ) )
						return false;

					if( *i == '\"' )
						StringDelimiter = '\"';
					else if( *i == '\'' )
						StringDelimiter = '\'';
					else
						StringDelimiter = '\0';

					if( StringDelimiter != '\0' )
					{
						if( !EatCharacter( Data, i, ParseError ) )
							return false;
					}
					CurrentState = InAttributeValue;
				}
				break;

			case InAttributeValue:
			{
				bool EndOfValue;

				if( *i == '\\' )
				{
					if( !EatCharacter(Data, i, ParseError) )
						return false;

					AttributeValue += *i;

					if( !EatCharacter(Data, i, ParseError) )
						return false;

					break;
				}

				if( StringDelimiter == '\0' )
					EndOfValue = (*i == '>') || (*i == '/') || (isspace( *i ) != 0);
				else
					EndOfValue = *i == StringDelimiter;

				if( EndOfValue )
				{
					Attributes[UILowerString (AttributeName)] = AttributeValue;

					AttributeName.erase();
					AttributeValue.erase();

					if( (*i != '>') && (*i != '/') && !EatCharacterAndTrailingWhitespace( Data, i, ParseError ) )
						return false;

					if( *i == '>' )
					{
						IsContainer = true;
						// Valid for i to be Data.end() after this, so we don't use EatCharacter 
						++i;
						return true;
					}
					else if( *i == '/' )
					{
						if( !EatCharacter( Data, i, ParseError ) )
							return false;

						if( *i == '>' )
						{
							IsContainer = false;

							// Valid for i to be Data.end() after this, so we don't use EatCharacter
							++i;
							return true;
						}
						else
						{
							_snprintf(scratch, SCRATCH_SIZE, BadCharKnown, *i, '>');
							ParseError = scratch;
							return false;
						}
					}
					CurrentState = InAttributeName;
				}
				else
				{
					AttributeValue += *i;
					
					if( !EatCharacter( Data, i, ParseError ) )
						return false;
				}
				break;
			}
			default:
				assert(false);
				break;
		}
	}
}

//----------------------------------------------------------------------

void UILoaderToken::WriteToString( UINarrowString &Out ) const
{
	if( IsData )
		Out.append(Header.begin (), Header.end ());
	else if( IsEndContainer )
	{
		Out.append("</");
		Out.append(Header.begin (), Header.end ());
		Out.append(">");
	}
	else
	{
		Out.append("<");
		Out.append(Header.begin (), Header.end ());
		Out.append(" ");
		
		for( UIStringMap::const_iterator i = Attributes.begin(); i != Attributes.end(); ++i )
		{
			if( i != Attributes.begin() )
				Out.append(" ");

			Out.append(i->first.get ());
			Out.append(" = '");
			Out.append(UIUnicode::wideToNarrow (i->second));
			Out.append("'");
		}

		if( IsContainer )
			Out.append(">");
		else
			Out.append("/>");
	}
}

//----------------------------------------------------------------------
