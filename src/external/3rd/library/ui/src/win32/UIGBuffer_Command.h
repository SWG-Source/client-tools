#ifndef __UIGBUFFER_COMMAND_H__
#define __UIGBUFFER_COMMAND_H__

#include <map>
#include <vector>

class UIGBuffer_Command
{
public:
	explicit UIGBuffer_Command(UIWidget * widget);
	UIGBuffer_Command();
	~UIGBuffer_Command();

	enum Id
	{
		ID_Null,
		ID_Quad,
		ID_Widget,
		ID_Line,
		ID_Count
	};

	typedef std::multimap</*buffer*/ float, /*command instance*/ UIGBuffer_Command *> List;

	void Render(UICanvas &) const;

	void SetType(Id command);
	void SetPoints(UIPoint const & p1, UIPoint const & p2, UIPoint const & p3, UIPoint const & p4);
	void SetPoints(UIPoint const & p1, UIPoint const & p2);
	void SetColor(UIColor const & c1);
	void SetOpacity(float o1);

private:
	UIGBuffer_Command const & operator=(UIGBuffer_Command const & rhs);
	UIGBuffer_Command(UIGBuffer_Command const & command);

	void attach(UIWidget *);

private:
	typedef std::vector<UIPoint> PointVector;

	Id                    mCommand;
	UIWidget             *mWidget;
	PointVector           mPoints;
	UIColor               mColor;
	float                 mOpacity;
};

#endif // __UIGBUFFER_COMMAND_H__