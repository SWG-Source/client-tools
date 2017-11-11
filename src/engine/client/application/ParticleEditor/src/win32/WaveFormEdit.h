// ============================================================================
//
// WaveFormEdit.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef WAVEFORMEDIT_H
#define WAVEFORMEDIT_H

#include "BaseWaveFormEdit.h"
#include "sharedMath/WaveForm.h"

class PerformanceTimer;

//-----------------------------------------------------------------------------
class WaveFormFrame : public QFrame
{
	Q_OBJECT

public:

	WaveFormFrame(QWidget *parent, char const *name);
	virtual ~WaveFormFrame();

	virtual void  drawContents(QPainter *painter);
	virtual void  mouseMoveEvent(QMouseEvent *mouseEvent);

	//void          drawHandle(QPainter &painter, float const percent, int const handleSize);
	QPoint const &getMousePositionMoveLocal() const;
	QRect         getViewPortRectGlobal() const;
	QRect         getViewPortRectLocal() const;

signals:

	void signalDrawWaveForm(QPainter *);

protected:

	virtual void paintEvent(QPaintEvent *paintEvent);

private:

	QPoint            m_mousePositionMoveLocal;
	PerformanceTimer *m_performanceTimer;
	QPixmap           m_pixMap;
	float             m_timeElapsed;

	void drawCursorLineVertical(QPainter &painter) const;

private:

	// Disable

	WaveFormFrame(WaveFormFrame const &);
	WaveFormFrame &operator =(WaveFormFrame const &);
};

//-----------------------------------------------------------------------------
class WaveFormEdit : public BaseWaveFormEdit
{
	Q_OBJECT

public:

	class WaveFormEditControlPoint
	{
	public:

		explicit WaveFormEditControlPoint(WaveForm::ControlPointList::iterator const &iter);
		WaveFormEditControlPoint();

		bool isRandomSelected() const;

		WaveForm::ControlPointList::iterator m_controlPointIter;
		bool                                 m_randomMinSelected;
		bool                                 m_randomMaxSelected;
	};

	typedef WaveForm::ControlPointList          ControlPointList;
	typedef std::list<WaveFormEditControlPoint> ControlPointIteratorList;
	typedef std::pair<float, float>             FloatFloatPair;
	typedef std::list<FloatFloatPair>           PointList;

	WaveFormEdit(QWidget *parent, char const *name);
	~WaveFormEdit();

	void  setToDisplayTemporaryWaveForm(bool const b);
	void  swapToTemporaryWaveForm();
	void  setWaveForm(WaveForm const &waveForm);
	void  setDrawRandomHandles(bool const drawRandomHandles);

	static void  drawWaveForm(QPainter &painter, float const scale, float const max, float const min, WaveForm &waveForm, WaveFormEditControlPoint const &anchoredControlPoint, WaveFormEditControlPoint const &selectedControlPoint, WaveFormEditControlPoint const &mouseControlPoint, const int controlPointSize = -1, bool const drawRandomHandles = false);

signals:

	void         waveFormChanged(const WaveForm &, const bool);

private slots:

	void         slotAddControlPoint(int id);
	void         slotDeleteControlPoint(int id);
	void         slotDrawWaveForm(QPainter *painter);
	void         waveFormGenerationPushButtonPressed();
	void         controlsLineEditReturnPressed();
	void         setInterpolationType(int index);
	void         onSampleTypeComboBoxActivated(int index);

private:

	virtual void leaveEvent(QEvent *event);
	virtual void mousePressEvent(QMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
	virtual void keyPressEvent(QKeyEvent *keyEvent);
	virtual void keyReleaseEvent(QKeyEvent *keyEvent);
	virtual void wheelEvent(QWheelEvent *wheelEvent);

	WaveFormEditControlPoint   getControlPointAtPosition(QPoint const &point);

	static void                drawLinearWaveform(QPainter &painter, float const max, float const min, PointList const &pointList, QColor const &color, float const scale);
	static void                drawSplineWaveform(QPainter &painter, float const max, float const min, PointList const &pointList, QColor const &color, float const scale);
	static void                drawControlPoints(QPainter &painter, float const scale, float const max, float const min, WaveForm &waveForm, WaveFormEditControlPoint const &anchoredControlPoint, WaveFormEditControlPoint const &selectedControlPoint, WaveFormEditControlPoint const &mouseControlPoint, int const controlPointSize = -1, bool const drawRandomHandles = false);
	static void                drawHandle(QPainter &painter, QPoint const &point, int const halfWidth, QColor const &color);

	void                       drawLinearWaveform(QPainter &painter, PointList const &pointList, QColor const &color, float const scale);
	void                       drawSplineWaveform(QPainter &painter, PointList const &pointList, QColor const &color, float const scale);

	//void                       resizeRects();
	void                       setControlValues(WaveFormEditControlPoint const &iter);
	void                       calculateScale();
	void                       addControlPointToRemoveList(WaveFormEditControlPoint &iter);
	void                       emitWaveFormChanged(const WaveForm &waveForm, const bool forceUpdate);

	static int const          m_controlPointSize;
	QPoint                    m_mousePositionMoveLocal;
	//QPoint                   m_mousePressPositionLocal;
	QPoint                    m_mousePositionReleaseLocal;
	float                     m_minValue;
	float                     m_maxValue;
	QPopupMenu *              m_addControlPointPopupMenu;
	QPopupMenu *              m_deleteControlPointPopupMenu;
	QFrame *                  m_controlFrame;
	WaveForm                  m_waveForm;
	WaveForm                  m_temporaryWaveForm;
	ControlPointList *        m_addControlPointList;
	ControlPointIteratorList *m_removeControlPointList;
	WaveFormEditControlPoint  m_selectedControlPoint;      // The highlighted or dragged control point
	WaveFormEditControlPoint  m_anchoredControlPoint;      // The clicked control point
	WaveFormEditControlPoint  m_deleteControlPoint;        // The control point to delete
	WaveFormEditControlPoint  m_controlValuesControlPoint;
	bool                      m_spaceKeyDown;
	bool                      m_shadeMode;
	bool                      m_displayTemporaryWaveForm;
	int                       m_scaleIndex;
	static int const          m_scaleIndexMax;
	static int const          m_scaleIndexMin;
	float                     m_scale;
	bool                      m_rebuildOnRelease;
	bool                      m_drawRandomHandles;
	QPixmap                   m_pixMap;

private:

	// Disabled

	WaveFormEdit();
	WaveFormEdit(WaveFormEdit const &);
	WaveFormEdit &operator =(WaveFormEdit const &);
};

// ============================================================================

#endif // WAVEFORMEDIT_H