// ============================================================================
//
// ColorRampEdit.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef COLORRAMPEDIT_H
#define COLORRAMPEDIT_H

#include "BaseColorRampWidget.h"
#include "clientParticle/ColorRamp.h"
#include "sharedMath/WaveForm.h"

#include <qframe.h>
#include <qpixmap.h>
#include <qpopupmenu.h>

class PerformanceTimer;

//-----------------------------------------------------------------------------
class ColorRampFrame : public QFrame
{
	Q_OBJECT

public:

	ColorRampFrame(QWidget *parent, char const *name);
	virtual ~ColorRampFrame();

	virtual void  drawContents(QPainter *painter);
	virtual void  mouseMoveEvent(QMouseEvent *mouseEvent);

	void          drawHandle(QPainter &painter, float const percent, int const handleSize);
	void          drawMarkerLine(QPainter &painter, float const percent, int const length);
	void          drawPercent(QPainter &painter, float const percent, int const decimals, bool const hasBackground = false);
	QPoint const &getMousePositionMoveLocal() const;
	QRect         getViewPortRectGlobal() const;
	QRect         getViewPortRectLocal() const;

signals:

	void signalDrawColorRamp(QPainter *);

protected:

	virtual void paintEvent(QPaintEvent *paintEvent);

private:

	void drawMouseMarker(QPainter &painter);

	QPoint            m_mousePositionMoveLocal;
	PerformanceTimer *m_performanceTimer;
	QPixmap           m_pixMap;
	float             m_timeElapsed;

private:

	// Disable

	ColorRampFrame(ColorRampFrame const &);
	ColorRampFrame &operator =(ColorRampFrame const &);
};

//-----------------------------------------------------------------------------
class ColorRampEdit : public BaseColorRampWidget
{
	Q_OBJECT

public:

	typedef ColorRamp::ControlPointList           ControlPointList;
	typedef std::list<ControlPointList::iterator> ControlPointIteratorList;

public:

	ColorRampEdit(QWidget *parent, char const *name = "");
	~ColorRampEdit();

	void             setColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm);
	ColorRamp const &getColorRamp() const;
	WaveForm const & getAlphaWaveForm() const;

	static void  drawColorRamp(QPainter &painter, ColorRamp const &colorRamp, WaveForm const &alphaWaveForm);

signals:

	void         colorRampChanged(const ColorRamp &, const WaveForm &, const bool);

private slots:

	// slots

	//void         paint();
	void         slotAddControlPoint(int id);
	void         slotDeleteControlPoint(int id);
	void         slotDrawColorRamp(QPainter *painter);
	void         slotColorTableClicked(int row, int col, int button, const QPoint &mousePos);
	void         slotColorTableDoubleClicked(int row, int col, int button, const QPoint &mousePos);
	void         slotColorTableValueChanged(int row, int col);
	void         slotColorInterpolationTypeComboBoxActivated(int index);
	void         slotColorSampleTypeComboBoxActivated(int index);

private:

	// virtuals

	//virtual void paintEvent(QPaintEvent *paintEvent);
	virtual void leaveEvent(QEvent *event);
	virtual void mousePressEvent(QMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
	virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);

private:

	void                       addControlPointToRemoveList(ControlPointList::iterator &iter);
	void                       drawColorHandles(QPainter &painter);
	ControlPointList::iterator getControlPoint(QPoint const &pointGlobal);
	void                       openColorDialog();
	void                       emitColorRampChanged(ColorRamp const &colorRamp, WaveForm const &waveForm, const bool forceRebuild);
	void                       setColorTable(ColorRamp const &colorRamp);

	ColorRamp                  m_colorRamp;
	WaveForm                   m_alphaWaveForm;
	static const int           m_handleSize;
	ControlPointList::iterator m_selectedControlPoint;       // The highlighted or dragged control point
	ControlPointList::iterator m_anchoredControlPoint;       // The double clicked control point
	ControlPointList::iterator m_deleteControlPoint;         // The control point to delete
	QPoint                     m_mousePositionMoveLocal;
	QPoint                     m_mousePositionReleaseLocal;
	QPopupMenu *               m_addControlPointPopupMenu;
	QPopupMenu *               m_deleteControlPointPopupMenu;
	ControlPointList *         m_addControlPointList;
	ControlPointIteratorList * m_removeControlPointList;
	bool                       m_rebuildOnRelease;

private:

	// Disabled

	ColorRampEdit();
	ColorRampEdit(ColorRampEdit const &);
	ColorRampEdit &operator =(ColorRampEdit const &);
};

// ============================================================================

#endif // COLORRAMPEDIT_H