// ======================================================================
//
// Parser.h
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Parser_H
#define INCLUDED_Parser_H

// ======================================================================

class CStdioFile;

#if _MSC_VER < 1300
class CString;
#endif

class Parser
{
public:
	// -------------------------------------
	enum TokenType
	{
		tt_begin,
		tt_store,
		tt_end
	};

	typedef CString Token;
	typedef std::map<Token, TokenType> TokenMap;

	enum NodeFlag
	{
		nf_none,
		nf_default
	};

	typedef std::pair<CString /*data*/, NodeFlag /*flags*/> Node;
	typedef std::multimap<Token, Node> Nodes;


	// -------------------------------------
	Parser();
	~Parser();

	// -------------------------------------
	bool parse(CStdioFile & file);
	bool eof() const;
	bool error() const;

	void addNode(Token const & token, CString const & line, NodeFlag flag = nf_none); // RLS TODO: Add a user data value.
	Nodes const & getNodes() const;
	void getNodes(Token const & token, Nodes & nodes) const;
	void addToken(Token const & token, TokenType const & ttype);


	// -------------------------------------
	// Get data from a node.
	bool hasToken(Token const & tokenName) const;
	CString const & getTokenAsString(Token const & tokenName) const;
	int getTokenAsInt(Token const & tokenName) const;
	float getTokenAsFloat(Token const & tokenName) const;
	bool getTokenAsBool(Token const & tokenName ) const;

private:
	// -------------------------------------
	bool getNextLine(CStdioFile & file, CString & line);

	// -------------------------------------
	TokenType mNextTokenType;
	TokenMap mTokens;
	Nodes mNodes;

	bool mEof : 1, 
	     mError : 1;

	// -------------------------------------
	static int mLineCount;
};

// ----------------------------------------------------------------------

inline Parser::Nodes const & Parser::getNodes() const
{
	return mNodes;
}

// ----------------------------------------------------------------------

inline bool Parser::eof() const
{
	return mEof;
}

// ----------------------------------------------------------------------

inline bool Parser::error() const
{
	return mError;
}

// ======================================================================

#endif
