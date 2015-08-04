#ifndef __UISCRIPTENGINE_H__
#define __UISCRIPTENGINE_H__

#include "UITypes.h"

class UIBaseObject;

//----------------------------------------------------------------------

typedef ui_stdvector<std::string>::fwd TokenVector;
typedef ui_stdlist<TokenVector>::fwd   TokenVectorList;

//----------------------------------------------------------------------

struct ExpressionTree
{
public:

									ExpressionTree();
								 ~ExpressionTree();

								
	std::string                   value;

	ExpressionTree *parent;
	ExpressionTree *lhs;
	ExpressionTree *rhs;
};

//----------------------------------------------------------------------

class UIScriptEngine
{
public:

	bool Execute( const Unicode::String &Script,       UIBaseObject *Context, std::string &ErrorMessage );
	virtual bool Execute( const std::string &Script, UIBaseObject *Context, std::string &ErrorMessage );

private:

	virtual bool Tokenize( const std::string &inScript, TokenVectorList &outTokenizedScript, std::string &ErrorMessage );
	virtual bool BuildExpressionTree( const TokenVector &inTokenizedScript, ExpressionTree *&outExpressionTree, long LineNumber, std::string &ErrorMessage );
	virtual bool Evaluate( ExpressionTree *ExpressionTree, UIBaseObject *Context, long LineNumber, std::string &ErrorMessage );
	
	virtual long GetBinaryOperatorPrecedence( const std::string &Token );
	virtual bool IsUnaryOperator( const std::string &Token );

	virtual bool EvaluationHook( ExpressionTree *ExpressionTree, UIBaseObject *Context );
	virtual bool AssignmentHook( ExpressionTree *LeftHandSide, ExpressionTree *RightHandSide );
};

//----------------------------------------------------------------------


#endif // __UISCRIPTENGINE_H__
