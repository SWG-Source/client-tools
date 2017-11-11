// ======================================================================
//
// VariableControl.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_VariableControl_h
#define INCLUDED_VariableControl_h

// ======================================================================

class CWnd;
class VariableElement;

// ======================================================================

class VariableControl
{
public:

	virtual ~VariableControl() {}

	const VariableElement &getVariableElement() const;
	VariableElement       &getVariableElement();

	virtual int            setupDimensions(int startX, int startY, int endX) = 0;
	virtual bool           saveControlToVariable(const CWnd *actionSource) = 0;
	virtual void           loadControlFromVariable() = 0;

protected:

	explicit VariableControl(VariableElement &variableElement);

private:

	VariableElement &m_variableElement;

private:
	// disabled
	VariableControl();
	VariableControl(const VariableControl&);
	VariableControl &operator =(const VariableControl&);
};

// ======================================================================

inline VariableControl::VariableControl(VariableElement &variableElement)
:	m_variableElement(variableElement)
{
}

// ----------------------------------------------------------------------

inline const VariableElement &VariableControl::getVariableElement() const
{
	return m_variableElement;
}

// ----------------------------------------------------------------------

inline VariableElement &VariableControl::getVariableElement()
{
	return m_variableElement; //lint !e1536 // exposing low access member
}

// ======================================================================

#endif
