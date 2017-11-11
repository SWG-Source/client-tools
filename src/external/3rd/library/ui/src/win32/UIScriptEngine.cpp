#include "_precompile.h"

#include "UIBaseObject.h"
#include "UIMemoryBlockManager.h"
#include "UIScriptEngine.h"
#include "UIUtils.h"

#include <cassert>
#include <cstdio>
#include <list>
#include <map>
#include <vector>

//----------------------------------------------------------------------

namespace UIScriptEngineNamespace
{
	UIScriptEngine *gUIScriptEngine = 0;
	typedef std::multimap<UIBaseObject * /*context*/, UINarrowString /*script*/> ActiveScriptMap;
	ActiveScriptMap ms_ActiveScriptMap;
}

using namespace UIScriptEngineNamespace;

//----------------------------------------------------------------------

bool UIScriptEngine::Execute( const UINarrowString &Script, UIBaseObject *Context, UINarrowString &ErrorMessage )
{
	bool success = true;

	ActiveScriptMap::const_iterator itPreviousScript = ms_ActiveScriptMap.find(Context);
	bool const isScriptActive = itPreviousScript != ms_ActiveScriptMap.end() && itPreviousScript->second == Script;
	
	UI_DEBUG_REPORT_LOG_PRINT(isScriptActive, ("UIManager::ExecuteScript(%s, %0xd) script is already executing.", Script.c_str(), Context));

	if (isScriptActive)
	{
		success = false;
	}
	else
	{
		ActiveScriptMap::iterator const itCurrentScript = ms_ActiveScriptMap.insert(std::make_pair(Context, Script));

		TokenVectorList	 theTokenizedScript;
		long             LineNumber = 1;
		
		if( Tokenize( Script, theTokenizedScript, ErrorMessage ) )
		{
			for( TokenVectorList::iterator i = theTokenizedScript.begin(); i != theTokenizedScript.end(); ++i )
			{
				ExpressionTree *theExpressionTree = 0;
				
				if( !BuildExpressionTree( *i, theExpressionTree, LineNumber, ErrorMessage ) )
				{
					success = false;
					break;
				}
				
				if( !Evaluate( theExpressionTree, Context, LineNumber, ErrorMessage ) )
				{
					success = false;
					delete theExpressionTree;
					break;
				}
				
				delete theExpressionTree;
				++LineNumber;
			}
		}
		else
		{
			success = false;
		}
		
		UI_DEBUG_REPORT_LOG_PRINT(!ErrorMessage.empty(), ("UIManager::ExecuteScript(%s, %0xd) reported: %s.", Script.c_str(), Context, ErrorMessage.c_str()));
		ms_ActiveScriptMap.erase(itCurrentScript);
	}

	return success;
}

bool UIScriptEngine::Tokenize( const UINarrowString &inScript, TokenVectorList &outTokenizedScript, UINarrowString &ErrorString )
{
	static const char *OperatorCharacters = "()+-*/&|!=?";

	TokenVector	CurrentTokenVector;
	UINarrowString		CurrentToken;
	char				QuoteCharacter = 0;
	long                LineNumber     = 1;
	char                CommentCharacter = '#';
	bool                inComment = false;

	for( UINarrowString::const_iterator i = inScript.begin(); i != inScript.end(); ++i )
	{
		if( *i == '\n' )
		{
			if( QuoteCharacter )
			{
				char Buffer[512];
				sprintf( Buffer, "Line %d: Unclosed quotation.", LineNumber );
				ErrorString = Buffer;
				return false;
			}

			if( !CurrentToken.empty() )
			{
				CurrentTokenVector.push_back( CurrentToken );
				CurrentToken.erase();
			}

			if( !CurrentTokenVector.empty() )
			{
				outTokenizedScript.push_back( CurrentTokenVector );
				CurrentTokenVector.clear();
			}

			inComment = false;
			++LineNumber;
			continue;
		}

		if(inComment)
			continue;
		
		if( *i == CommentCharacter)
		{
			inComment = true;
			continue;
		}
			
		if( QuoteCharacter )
		{
			if( *i == QuoteCharacter )
			{				
				CurrentTokenVector.push_back( CurrentToken );
				CurrentToken.erase();
				QuoteCharacter = 0;
			}
			else
			{
				CurrentToken.append( 1, *i );
			}
			continue;
		}

		if( isspace( *i ) )
		{
			if( !CurrentToken.empty() )
			{
				CurrentTokenVector.push_back( CurrentToken );
				CurrentToken.erase();
			}
			continue;
		}

		if( CurrentToken.size() > 0 )
		{
			if( strchr( OperatorCharacters, *i ) )
			{
				// *i is a special character, we may change tokens
				if( *i == '&' )
				{
					if( (CurrentToken.size() != 1) || CurrentToken[0] != '&' )
					{
						CurrentTokenVector.push_back( CurrentToken );
						CurrentToken.erase();
					}				
				}
				else if( *i == '|' )
				{
					if( (CurrentToken.size() != 1) || CurrentToken[0] != '|' )
					{
						CurrentTokenVector.push_back( CurrentToken );
						CurrentToken.erase();
					}	
				}
				else if( *i == '=' )
				{
					if( (CurrentToken.size() != 1) || (CurrentToken[0] != '=' && CurrentToken[0] != '!'))
					{
						CurrentTokenVector.push_back( CurrentToken );
						CurrentToken.erase();
					}	
				}
				else
				{
					CurrentTokenVector.push_back( CurrentToken );
					CurrentToken.erase();
				}
			}
			else if( strchr( OperatorCharacters, CurrentToken[0] ) )
			{
				// CurrentToken is an operator, we change tokens
				CurrentTokenVector.push_back( CurrentToken );
				CurrentToken.erase();
			}
		}
		
		if( (*i == '\'') || (*i == '\"') )
		{
			QuoteCharacter = *i;
			continue;
		}
		
		CurrentToken.append( 1, *i );
	}

	if( QuoteCharacter )
	{
		char Buffer[512];
		sprintf( Buffer, "Line %d: Unclosed quotation.", LineNumber );
		ErrorString = Buffer;
		return false;
	}

	if( !CurrentToken.empty() )
	{
		CurrentTokenVector.push_back( CurrentToken );
		CurrentToken.erase();
	}

	if( !CurrentTokenVector.empty() )
	{
		outTokenizedScript.push_back( CurrentTokenVector );
		CurrentTokenVector.clear();
	}

	return true;
}

bool UIScriptEngine::BuildExpressionTree( const TokenVector &inTokenizedScript, ExpressionTree *&outExpressionTree, long LineNumber, UINarrowString &ErrorMessage )
{

	ExpressionTree *CurrentRoot     = 0;

	for( TokenVector::const_iterator i = inTokenizedScript.begin(); i != inTokenizedScript.end(); ++i )
	{
		const char *s = i->c_str();

		if( !_stricmp( s, "(" ) )
		{
			TokenVector	SubExpression;
			long				ParenLevel = 0;

			++i;

			while( i != inTokenizedScript.end() )
			{
				s = i->c_str();

				if( !_stricmp( s, ")" ) )
				{
					if( ParenLevel == 0 )
						break;
					else
						--ParenLevel;
				}
				else if( !_stricmp( s, "(" ) )
					++ParenLevel;

				SubExpression.push_back( *i );
				++i;
			}

			if( ParenLevel != 0 )
			{
				char Buffer[256];
				sprintf( Buffer, "Line %d: Unmatched '('.", LineNumber );
				ErrorMessage = Buffer;
				return false;
			}
			
			ExpressionTree *NewChild;

			BuildExpressionTree( SubExpression, NewChild, LineNumber, ErrorMessage );

			if( !CurrentRoot )
			{
				CurrentRoot = NewChild;
			}
			else
			{
				if( !CurrentRoot->lhs )
				{
					CurrentRoot->lhs = NewChild;
					NewChild->parent = CurrentRoot;
				}
				else
				{
					ExpressionTree *RightMostChild = CurrentRoot;

					while( RightMostChild->rhs )
						RightMostChild = RightMostChild->rhs;

					RightMostChild->rhs = NewChild;
					NewChild->parent = RightMostChild;
				}
			}

			outExpressionTree = CurrentRoot;
			return true;
		}

		if( !_stricmp( s, ")" ) )
		{
			char Buffer[256];
			sprintf( Buffer, "Line %d: Unmatched ')'.", LineNumber );
			ErrorMessage = Buffer;
			return false;
		}

		if( !CurrentRoot )
		{	
			CurrentRoot	= new ExpressionTree;
			CurrentRoot->value = *i;
		}			
		else
		{
			ExpressionTree *RightMostChild = CurrentRoot;

			while( RightMostChild->rhs )
				RightMostChild = RightMostChild->rhs;

			if( IsUnaryOperator( RightMostChild->value ) )
			{
				if( RightMostChild->lhs )
				{
					ExpressionTree *NewChild = new ExpressionTree;

					NewChild->value  = *i;
					NewChild->lhs    = RightMostChild;
					NewChild->parent = RightMostChild->parent;

					if( NewChild->parent )
						NewChild->parent->rhs = NewChild;
					else
						CurrentRoot = NewChild;

					RightMostChild->parent = NewChild;
				}
				else
				{					
					RightMostChild->lhs         = new ExpressionTree;
					RightMostChild->lhs->parent = RightMostChild;
					RightMostChild->lhs->value  = *i;
				}
			}
			//-- binary operator
			else
			{
				ExpressionTree * p = 0;

				int const precedence = GetBinaryOperatorPrecedence(*i);

				//-- non-operators get put at the rightmost side
				if (precedence > 0)
				{
					p = CurrentRoot;
					while (p)
					{
						if (precedence > GetBinaryOperatorPrecedence(p->value))
						{
							ExpressionTree * NewTree = new ExpressionTree;

							NewTree->lhs = p;
							NewTree->value = *i;
							NewTree->parent = p->parent;
							if (p->parent)
								p->parent->rhs = NewTree;
							p->parent = NewTree;

							if (p == CurrentRoot)
								CurrentRoot = NewTree;

							break;
						}

						p = p->rhs;
					}
				}

				//-- data and fallthrough goes to end
				if (!p)
				{
					ExpressionTree * NewTree = new ExpressionTree;
					NewTree->value = *i;
					RightMostChild->rhs = NewTree;
					NewTree->parent = RightMostChild;
				}
			}
		}
	}

	outExpressionTree = CurrentRoot;
	return true;
}

bool UIScriptEngine::Evaluate( ExpressionTree *e, UIBaseObject *Context, long LineNumber, UINarrowString &ErrorMessage )
{
	if( e )
	{
		const char *s = e->value.c_str();

		bool lhsProcessed = false;

		if( !_stricmp( s, "?" ) )
		{
			// Conditional, do lhs first
			if( e->lhs && e->rhs )
			{
				if( !Evaluate( e->lhs, Context, LineNumber, ErrorMessage ) )
					return false;

				bool result = false;
				if (!UIUtils::ParseBoolean(e->lhs->value, result) || !result)
					return true;
			}
			else
				return false;

			lhsProcessed = true;
		}

		if( e->rhs )
		{
			if( !Evaluate( e->rhs, Context, LineNumber, ErrorMessage ) )
				return false;
		}

		if (lhsProcessed)
			return true;

		if( !_stricmp( s, "=" ) )
		{
			// Assignment - binary, lhs is not evaluated first
			if( e->lhs && e->rhs )
			{
				if( !Context || !Context->SetProperty( UILowerString (e->lhs->value), UIUnicode::narrowToWide (e->rhs->value) ) )
				{
					AssignmentHook( e->lhs, e->rhs );
				}
				e->value = e->rhs->value;
			}
			return true;
		}

		if( e->lhs )
		{
			if( !Evaluate( e->lhs, Context, LineNumber, ErrorMessage ) )
				return false;
		}		

		if( !_stricmp( s, "==" ) )
		{
			// String equality - binary
			if( e->lhs && e->rhs )
			{
				bool res = _stricmp( e->lhs->value.c_str(), e->rhs->value.c_str() ) == 0;
				UIUtils::FormatBoolean( e->value, res );
			}
		}
		else if( !_stricmp( s, "!=" ) )
		{
			// String inequality - binary
			if( e->lhs && e->rhs )
			{
				bool res = _stricmp( e->lhs->value.c_str(), e->rhs->value.c_str() ) != 0;
				UIUtils::FormatBoolean( e->value, res );
			}
		}
		else if( !_stricmp( s, "<" ) )
		{
			// Numerical less - binary
			if( e->lhs && e->rhs )
			{
				float lhs = (float)atof( e->lhs->value.c_str() );
				float rhs = (float)atof( e->rhs->value.c_str() );

				UIUtils::FormatBoolean( e->value, lhs < rhs );
			}
		}
		else if( !_stricmp( s, "<=" ) )
		{
			// Numerical less or equals - binary
			if( e->lhs && e->rhs )
			{
				float lhs = (float)atof( e->lhs->value.c_str() );
				float rhs = (float)atof( e->rhs->value.c_str() );

				UIUtils::FormatBoolean( e->value, lhs <= rhs );
			}
		}
		else if( !_stricmp( s, ">" ) )
		{
			// Numerical greater - binary
			if( e->lhs && e->rhs )
			{
				float lhs = (float)atof( e->lhs->value.c_str() );
				float rhs = (float)atof( e->rhs->value.c_str() );

				UIUtils::FormatBoolean( e->value, lhs > rhs );
			}
		}
		else if( !_stricmp( s, ">=" ) )
		{
			// Numerical greater or equals - binary
			if( e->lhs && e->rhs )
			{
				float lhs = (float)atof( e->lhs->value.c_str() );
				float rhs = (float)atof( e->rhs->value.c_str() );

				UIUtils::FormatBoolean( e->value, lhs >= rhs );
			}
		}
		else if( !_stricmp( s, "&&" ) )
		{
			// Logical AND - binary
			if( e->lhs && e->rhs )
			{
				bool rhs, lhs;

				if( !UIUtils::ParseBoolean( e->lhs->value, lhs ) )
					lhs = false;

				if( !UIUtils::ParseBoolean( e->rhs->value, rhs ) )
					rhs = false;

				UIUtils::FormatBoolean( e->value, lhs && rhs );
			}
		}
		else if( !_stricmp( s, "||" ) )
		{
			// Logical OR - binary
			if( e->lhs && e->rhs )
			{
				bool rhs, lhs;

				if( !UIUtils::ParseBoolean( e->lhs->value, lhs ) )
					lhs = false;

				if( !UIUtils::ParseBoolean( e->rhs->value, rhs ) )
					rhs = false;

				UIUtils::FormatBoolean( e->value, lhs || rhs );
			}
		}
		else if( !_stricmp( s, "!" ) )
		{
			// Logical NOT - unary
			if( e->lhs )
			{
				bool lhs;

				if( !UIUtils::ParseBoolean( e->lhs->value, lhs ) )
					lhs = false;

				UIUtils::FormatBoolean( e->value, !lhs );
			}
		}
		else if( !_stricmp( s, "+" ) )
		{
			// Addition - binary
			if( e->lhs && e->rhs )
			{
				float rhs, lhs;

				if( !UIUtils::ParseFloat( e->lhs->value, lhs ) )
					lhs = 0;

				if( !UIUtils::ParseFloat( e->rhs->value, rhs ) )
					rhs = 0;

				UIUtils::FormatFloat( e->value, lhs + rhs );
			}
		}
		else if( !_stricmp( s, "-" ) )
		{
			// Subraction - binary
			if( e->lhs && e->rhs )
			{
				float rhs, lhs;

				if( !UIUtils::ParseFloat( e->lhs->value, lhs ) )
					lhs = 0;

				if( !UIUtils::ParseFloat( e->rhs->value, rhs ) )
					rhs = 0;

				UIUtils::FormatFloat( e->value, lhs - rhs );
			}
		}
		else if( !_stricmp( s, "*" ) )
		{
			// Multiplication - binary
			if( e->lhs && e->rhs )
			{
				float rhs, lhs;

				if( !UIUtils::ParseFloat( e->lhs->value, lhs ) )
					lhs = 0;

				if( !UIUtils::ParseFloat( e->rhs->value, rhs ) )
					rhs = 0;

				UIUtils::FormatFloat( e->value, lhs * rhs );
			}
		}
		else if( !_stricmp( s, "/" ) )
		{
			// Division - binary
			if( e->lhs && e->rhs )
			{
				float rhs, lhs;

				if( !UIUtils::ParseFloat( e->lhs->value, lhs ) )
					lhs = 0;

				if( !UIUtils::ParseFloat( e->rhs->value, rhs ) )
					rhs = 0;

				if( rhs != 0 )
					UIUtils::FormatFloat( e->value, lhs / rhs );
				else
					e->value = "0";
			}
		}
		else if( !_stricmp( s, "^" ) )
		{
			// Maximum - binary
			if( e->lhs && e->rhs )
			{
				float rhs, lhs;

				if( !UIUtils::ParseFloat( e->lhs->value, lhs ) )
					lhs = 0;

				if( !UIUtils::ParseFloat( e->rhs->value, rhs ) )
					rhs = 0;

				UIUtils::FormatFloat( e->value, (lhs > rhs) ? lhs : rhs );
			}
		}
		else if( Context )
		{
			UINarrowString ValueCopy = e->value;

			UIString valueResult;

			if( !Context->GetProperty( UILowerString (ValueCopy), valueResult) || !e->parent )
			{
				EvaluationHook( e, Context );
			}
			else
			{
				e->value = UIUnicode::wideToNarrow (valueResult);
			}

		}
	}
	return true;
}

/**
* GetBinaryOperatorPrecedence returns a relative value for operator/operand placement in the expression tree
* Lower value means higher logical precendence
*/

long UIScriptEngine::GetBinaryOperatorPrecedence( const UINarrowString &Token )
{
	const char *s = Token.c_str();

	// It's a bug for this to be different from C operator precidence

	if( !_stricmp( s, "^" ) )
		return 1;
	if( !_stricmp( s, "*" ) )
		return 1;
	if( !_stricmp( s, "/" ) )
		return 1;
	
	if( !_stricmp( s, "+" ) )
		return 2;
	if( !_stricmp( s, "-" ) )
		return 2;
	
	if( !_stricmp( s, "<" ) )
		return 3;
	if( !_stricmp( s, "<=" ) )
		return 3;
	if( !_stricmp( s, ">" ) )
		return 3;
	if( !_stricmp( s, ">=" ) )
		return 3;

	if( !_stricmp( s, "==" ) )
		return 4;
	if( !_stricmp( s, "!=" ) )
		return 4;

	if( !_stricmp( s, "&&" ) )
		return 5;
	
	if( !_stricmp( s, "||" ) )
		return 6;

	if( !_stricmp( s, "=" ) )
		return 7;
	if( !_stricmp( s, "?" ) )
		return 8;

	return 0;
}

bool UIScriptEngine::IsUnaryOperator( const UINarrowString &Token )
{
	const char *s = Token.c_str();

	if( !_stricmp( s, "!" ) )
		return true;
	
	return false;
}

bool UIScriptEngine::EvaluationHook( ExpressionTree *, UIBaseObject * )
{
	return true;
}

bool UIScriptEngine::AssignmentHook( ExpressionTree *, ExpressionTree * )
{
	return true;
}

ExpressionTree::ExpressionTree( void )
{
	lhs    = 0;
	rhs    = 0;
	parent = 0;
}

ExpressionTree::~ExpressionTree( void )
{
	delete lhs;
	delete rhs;
}

//----------------------------------------------------------------------

bool UIScriptEngine::Execute (const Unicode::String &Script, UIBaseObject *Context, std::string &ErrorMessage )
{
	return Execute (Unicode::wideToNarrow (Script), Context, ErrorMessage);
}
