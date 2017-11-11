// ============================================================================
//
// WaveFormEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "WaveFormEdit.h"
#include "WaveFormEdit.moc"

#include "sharedDebug/PerformanceTimer.h"
#include "sharedMath/CatmullRomSpline.h"
#include "RectHelper.h"

int const WaveFormEdit::m_scaleIndexMax = 16;
int const WaveFormEdit::m_scaleIndexMin = 0;

///////////////////////////////////////////////////////////////////////////////
//
// WaveFormFrame
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
WaveFormFrame::WaveFormFrame(QWidget *parent, char const *name)
 : QFrame(parent, name)
 , m_mousePositionMoveLocal(QPoint(0, 0))
 , m_pixMap()
 , m_performanceTimer(NULL)
 , m_timeElapsed(0.0f)
{
	setBackgroundMode(Qt::NoBackground);
	setMouseTracking(true);
	//setFrameStyle(QFrame::Raised | StyledPanel);
	setFrameStyle(QFrame::NoFrame);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_performanceTimer = new PerformanceTimer;
	m_performanceTimer->start();
}

//-----------------------------------------------------------------------------
WaveFormFrame::~WaveFormFrame()
{
	delete m_performanceTimer;
}

//-----------------------------------------------------------------------------
void WaveFormFrame::paintEvent(QPaintEvent *)
{
	m_performanceTimer->stop();
	m_timeElapsed += m_performanceTimer->getElapsedTime();
	m_performanceTimer->start();
	
	float const hertz = (1.0f / 24.0f);
	
	if (m_timeElapsed > hertz)
	{
		m_timeElapsed -= hertz * (m_timeElapsed / hertz);

		QPainter painter;

		drawFrame(&painter);
		drawContents(&painter);
	}
}

//-----------------------------------------------------------------------------
void WaveFormFrame::drawContents(QPainter *painter)
{
	if (painter)
	{
		if ((m_pixMap.width() != contentsRect().width()) ||
		    (m_pixMap.height() != contentsRect().height()))
		{
			m_pixMap.resize(contentsRect().width(), contentsRect().height());
		}

		if (painter->isActive())
		{
			painter->end();
		}

		if (painter->begin(&m_pixMap))
		{
			// Clear the background

			painter->fillRect(m_pixMap.rect(), QBrush(backgroundColor()));

			// Set the area to draw the color ramp in

			QRect viewPortRect(getViewPortRectLocal());

			painter->setViewport(viewPortRect);
			painter->setWindow(QRect(0, 0, painter->viewport().width(), painter->viewport().height()));

			// Draw the color ramp onto the pixmap

			emit signalDrawWaveForm(painter);

			// Draw the vertical cursor line

			drawCursorLineVertical(*painter);

			// Blit the pixmap onto this widget

			bitBlt(this, contentsRect().left(), contentsRect().top(), &m_pixMap);

			painter->end();
		}
	}
}

//-----------------------------------------------------------------------------
QRect WaveFormFrame::getViewPortRectLocal() const
{
	int const marginTopBottom = 16;
	int const marginLeft = 68;
	int const marginRight = 16;

	return QRect(QPoint(contentsRect().x() + marginLeft, contentsRect().y() + marginTopBottom), QSize(contentsRect().width() - marginLeft - marginRight, contentsRect().height() - marginTopBottom * 2));
}

//-----------------------------------------------------------------------------
QRect WaveFormFrame::getViewPortRectGlobal() const
{
	return QRect(mapToGlobal(getViewPortRectLocal().topLeft()), mapToGlobal(getViewPortRectLocal().bottomRight()));
}

//-----------------------------------------------------------------------------
void WaveFormFrame::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent)
	{
		m_mousePositionMoveLocal = mouseEvent->pos();

		// Force a repaint

		update();

		mouseEvent->ignore();
	}
}

// Returns the mouse position within this widget
//-----------------------------------------------------------------------------
QPoint const &WaveFormFrame::getMousePositionMoveLocal() const
{
	return m_mousePositionMoveLocal;
}

//-----------------------------------------------------------------------------
void WaveFormFrame::drawCursorLineVertical(QPainter &painter) const
{
	// Draw the vertical cursor line

	QRect destRect(painter.window());

	int const positionX = m_mousePositionMoveLocal.x() - painter.viewport().left();

	if (positionX >= destRect.left() && positionX < destRect.right())
	{
		painter.setPen(QColor("yellow"));
		painter.drawLine(positionX, 0, positionX, height());
	}
}

// ============================================================================
//
// WaveFormEdit::WaveFormEditControlPoint
//
// ============================================================================

//-----------------------------------------------------------------------------
WaveFormEdit::WaveFormEditControlPoint::WaveFormEditControlPoint(WaveForm::ControlPointList::iterator const &iter)
 : m_controlPointIter(iter)
 , m_randomMinSelected(false)
 , m_randomMaxSelected(false)
{
}

//-----------------------------------------------------------------------------
WaveFormEdit::WaveFormEditControlPoint::WaveFormEditControlPoint()
 : m_controlPointIter()
 , m_randomMinSelected(false)
 , m_randomMaxSelected(false)
{
}

//-----------------------------------------------------------------------------
bool WaveFormEdit::WaveFormEditControlPoint::isRandomSelected() const
{
	return (m_randomMinSelected || m_randomMaxSelected);
}

// ============================================================================
//
// WaveFormEdit
//
// ============================================================================

int const WaveFormEdit::m_controlPointSize = 8; // keep this an even number plz

//-----------------------------------------------------------------------------
WaveFormEdit::WaveFormEdit(QWidget *parent, char const *name)
 : BaseWaveFormEdit(parent, name)
 , m_minValue(-1.0f)
 , m_maxValue(1.0f)
 , m_shadeMode(true)
 , m_displayTemporaryWaveForm(false)
 , m_scaleIndex(m_scaleIndexMin)
 , m_scale(0.0f)
 , m_drawRandomHandles(false)
 , m_controlValuesControlPoint()
{
	calculateScale();

	//setWFlags(Qt::WStyle_Customize | Qt::WStyle_NoBorder);
	//setFixedWidth(512);
	//setFixedHeight(76);
	//setBaseSize(512, 64);
	setMouseTracking(true);

	QFontMetrics metrics(fontMetrics());

	//resizeRects();

	// Put in four control points to test with.

	m_waveForm.insert(WaveFormControlPoint(0.0f, 0.0f, 0.0f, 0.0f));
	m_waveForm.insert(WaveFormControlPoint(1.0f, 0.0f, 0.0f, 0.0f));
	m_selectedControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorEnd());
	m_deleteControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorEnd());
	m_anchoredControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorBegin());
	m_addControlPointList = new ControlPointList;
	m_removeControlPointList = new ControlPointIteratorList;

	// Add control point popup menu

	m_addControlPointPopupMenu = new QPopupMenu(this);
	connect(m_addControlPointPopupMenu, SIGNAL(activated(int)), SLOT(slotAddControlPoint(int)));
	m_addControlPointPopupMenu->insertItem("Add Control Point");
	m_addControlPointPopupMenu->setCheckable(false);

	// Delete control point popup menu

	m_deleteControlPointPopupMenu = new QPopupMenu(this);
	connect(m_deleteControlPointPopupMenu, SIGNAL(activated(int)), SLOT(slotDeleteControlPoint(int)));
	m_deleteControlPointPopupMenu->insertItem("Delete Control Point");
	m_deleteControlPointPopupMenu->setCheckable(false);

	connect(m_sampleTypeComboBox, SIGNAL(activated(int)), SLOT(onSampleTypeComboBoxActivated(int)));
	connect(m_interpolationTypeComboBox, SIGNAL(activated(int)), SLOT(setInterpolationType(int)));
	connect(m_percentLineEdit, SIGNAL(returnPressed()), SLOT(controlsLineEditReturnPressed()));
	connect(m_maxRandomLineEdit, SIGNAL(returnPressed()), SLOT(controlsLineEditReturnPressed()));
	connect(m_valueLineEdit, SIGNAL(returnPressed()), SLOT(controlsLineEditReturnPressed()));
	connect(m_minRandomLineEdit, SIGNAL(returnPressed()), SLOT(controlsLineEditReturnPressed()));
	connect(m_waveFormGeneratorPushButton, SIGNAL(pressed()), SLOT(waveFormGenerationPushButtonPressed()));
	
	connect(m_waveFormFrame, SIGNAL(signalDrawWaveForm(QPainter *)), this, SLOT(slotDrawWaveForm(QPainter *)));

	// Set validators to restrict input for line inputs

	QIntValidator *intValidator = new QIntValidator(this);
	m_percentLineEdit->setValidator(intValidator);

	QDoubleValidator *doubleValidator = new QDoubleValidator(this);
	m_valueLineEdit->setValidator(doubleValidator);
	m_maxRandomLineEdit->setValidator(doubleValidator);
	m_minRandomLineEdit->setValidator(doubleValidator);

	// Set the initial control values
	
	setControlValues(m_anchoredControlPoint);

	// Hide a bunch of controls for now

	m_waveFormGeneratorPushButton->hide();
	m_loadWaveFormPushButton->hide();
	m_saveWaveFormPushButton->hide();
}

//-----------------------------------------------------------------------------
WaveFormEdit::~WaveFormEdit()
{
	delete m_addControlPointList;
	delete m_removeControlPointList;
}

////-----------------------------------------------------------------------------
//void WaveFormEdit::resizeRects()
//{
//	QRect boundingRect(rect());
//
//	m_rectOffsetX = m_controlsHBoxLayout->minimumSize().width();
//
//	int rLeft = m_rectOffsetX;
//	int rTop = 0;
//	int rWidth = 64;
//	int rHeight = height();
//	m_minMaxRect = QRect(rLeft, rTop, rWidth, rHeight);
//
//	rLeft = m_minMaxRect.right();
//	rTop = 0;
//	rWidth = width() - m_rectOffsetX - m_minMaxRect.width();
//	rHeight = 16;
//	m_horizontalRect = QRect(rLeft, rTop, rWidth, rHeight);
//
//	int const hSpace = 8;
//	int const vSpace = 16;
//
//	rLeft = m_minMaxRect.right();
//	rTop = m_horizontalRect.bottom() + vSpace;
//	rWidth = width() - m_rectOffsetX - m_minMaxRect.width() - hSpace * 3;
//	rHeight = boundingRect.bottom() - m_horizontalRect.bottom() - vSpace * 2;
//	m_splineRect = QRect(rLeft, rTop, rWidth, rHeight);
//}

//-----------------------------------------------------------------------------
void WaveFormEdit::slotAddControlPoint(int)
{
	float const releasePercentX = RectHelper::getPercentageInRectX(m_waveFormFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionReleaseLocal));
	float const releasePercentY = RectHelper::getPercentageInRectY(m_waveFormFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionReleaseLocal));

	m_addControlPointList->push_back(WaveFormControlPoint(releasePercentX, m_maxValue - releasePercentY * (m_maxValue - m_minValue), 0.0f, 0.0f));

	// Add any new control points

	for (; !m_addControlPointList->empty(); m_addControlPointList->pop_back())
	{
		m_waveForm.insert(m_addControlPointList->back());

		// Recalculate the min and max values

		m_waveForm.calculateMinMax(m_maxValue, m_minValue);

		emitWaveFormChanged(m_waveForm, true);
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::slotDeleteControlPoint(int)
{
	// Add the control point to the remove list

	addControlPointToRemoveList(m_deleteControlPoint);

	// Invalidate the delete control point

	m_deleteControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorEnd());

	// Remove any old control points

	for (; !m_removeControlPointList->empty(); m_removeControlPointList->pop_front())
	{
		WaveFormEditControlPoint waveFormEditControlPoint = *m_removeControlPointList->begin();

		m_waveForm.remove(waveFormEditControlPoint.m_controlPointIter);

		// Recalculate the min and max values

		m_waveForm.calculateMinMax(m_maxValue, m_minValue);

		emitWaveFormChanged(m_waveForm, true);
	}
}

//-----------------------------------------------------------------------------
WaveFormEdit::WaveFormEditControlPoint WaveFormEdit::getControlPointAtPosition(QPoint const &point)
{
	WaveFormEditControlPoint result(m_waveForm.getIteratorEnd());

	QPoint pointGlobal(mapToGlobal(point));

	// See if the press was inside any of the control points

	for (ControlPointList::iterator iter = m_waveForm.getIteratorBegin(); iter != m_waveForm.getIteratorEnd(); ++iter)
	{
		float const xPercent = iter->getPercent();
		float yPercent = (m_maxValue - iter->getValue()) / (m_maxValue - m_minValue);

		QPoint controlPointPosition(RectHelper::getPointInRect(m_waveFormFrame->getViewPortRectGlobal(), xPercent, yPercent));

		// Create a rect around this point

		QRect controlPointRect(controlPointPosition.x() - m_controlPointSize, controlPointPosition.y() - m_controlPointSize, m_controlPointSize * 2, m_controlPointSize * 2);

		if (controlPointRect.contains(pointGlobal) && !m_drawRandomHandles)
		{
			result.m_controlPointIter = iter;
			break;
		}
		else
		{
			// See if the random min control point is selected

			yPercent = (m_maxValue - (iter->getValue() + iter->getRandomMax())) / (m_maxValue - m_minValue);
			controlPointPosition = QPoint(RectHelper::getPointInRect(m_waveFormFrame->getViewPortRectGlobal(), xPercent, yPercent));

			if (m_drawRandomHandles)
			{
				controlPointRect = QRect(controlPointPosition.x() - m_controlPointSize, controlPointPosition.y() - m_controlPointSize, m_controlPointSize * 2, m_controlPointSize);
			}
			else
			{
				controlPointRect = QRect(controlPointPosition.x() - m_controlPointSize, controlPointPosition.y() - m_controlPointSize, m_controlPointSize * 2, m_controlPointSize * 2);
			}

			if (controlPointRect.contains(pointGlobal))
			{
				result.m_controlPointIter = iter;
				result.m_randomMaxSelected = true;
				break;
			}

			// See if the random max control point is selected

			yPercent = (m_maxValue - (iter->getValue() - iter->getRandomMin())) / (m_maxValue - m_minValue);
			controlPointPosition = QPoint(RectHelper::getPointInRect(m_waveFormFrame->getViewPortRectGlobal(), xPercent, yPercent));
			
			if (m_drawRandomHandles)
			{
				controlPointRect = QRect(controlPointPosition.x() - m_controlPointSize, controlPointPosition.y(), m_controlPointSize * 2, m_controlPointSize);
			}
			else
			{
				controlPointRect = QRect(controlPointPosition.x() - m_controlPointSize, controlPointPosition.y() - m_controlPointSize, m_controlPointSize * 2, m_controlPointSize * 2);
			}

			if (controlPointRect.contains(pointGlobal))
			{
				result.m_controlPointIter = iter;
				result.m_randomMinSelected = true;
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void WaveFormEdit::keyPressEvent(QKeyEvent *keyEvent)
{
	keyEvent->ignore();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::keyReleaseEvent(QKeyEvent *keyEvent)
{
	keyEvent->ignore();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::wheelEvent(QWheelEvent *wheelEvent)
{
	if (wheelEvent->delta() > 0)
	{
		--m_scaleIndex;
	}
	else if (wheelEvent->delta() < 0)
	{
		++m_scaleIndex;
	}

	// Clamp the scale

	if (m_scaleIndex > m_scaleIndexMax)
	{
		m_scaleIndex = m_scaleIndexMax;
	}
	else if (m_scaleIndex < m_scaleIndexMin)
	{
		m_scaleIndex = m_scaleIndexMin;
	}

	char text[256];
	sprintf(text, "%d", m_scaleIndex);
	setCaption(text);

	calculateScale();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::mousePressEvent(QMouseEvent *mouseEvent)
{
	// Find the control point to select if the left button was pressed

	if (mouseEvent->button() == Qt::LeftButton)
	{
  		m_selectedControlPoint = getControlPointAtPosition(mouseEvent->pos());
	}

	// Find the control point to be deleted if the right button was pressed

	if (mouseEvent->button() == Qt::RightButton)
	{
		m_deleteControlPoint = getControlPointAtPosition(mouseEvent->pos());
	}

	// Set the anchored control point as the just selected control point so
	// the user can modify the values from the line edits
	
	if (m_selectedControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd())
	{
		m_anchoredControlPoint = m_selectedControlPoint;
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		// Save the mouse release position

		m_mousePositionReleaseLocal = mouseEvent->pos();

		// If the release occured inside the spline rect, and the button was the
		// right mouse button, then pop up the add control point button

		if (mouseEvent->button() == Qt::RightButton)
		{
			// Make sure nothing happens when the begin and end control point are
			// clicked

			WaveFormEditControlPoint waveFormEditControlPoint = getControlPointAtPosition(mouseEvent->pos());

			if ((waveFormEditControlPoint.m_controlPointIter != m_waveForm.getIteratorBegin()) && 
				(waveFormEditControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd()) && 
				 (waveFormEditControlPoint.m_controlPointIter == m_deleteControlPoint.m_controlPointIter) &&
				 (++waveFormEditControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd()))
			{
				m_deleteControlPointPopupMenu->exec(mouseEvent->globalPos());
			}
			else if (waveFormEditControlPoint.m_controlPointIter == m_waveForm.getIteratorEnd())
			{
				if (m_waveFormFrame->getViewPortRectGlobal().contains(mouseEvent->globalPos()))
				{
					m_addControlPointPopupMenu->exec(mouseEvent->globalPos());
				}
			}
		}

		// Set the control values

		setControlValues(m_anchoredControlPoint);

		// Clear the selected control point

		m_selectedControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorEnd());

		// Recalculate the min and max values

		m_waveForm.calculateMinMax(m_maxValue, m_minValue);

		// See if we need to cause a rebuild

		if (m_rebuildOnRelease)
		{
			m_rebuildOnRelease = false;

			emitWaveFormChanged(m_waveForm, true);
		}
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	m_mousePositionMoveLocal = mouseEvent->pos();

	if (m_selectedControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd())
	{
		// Get the new x position

		float percentX = RectHelper::getPercentageInRectX(m_waveFormFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionMoveLocal));

		percentX = (percentX > 1.0f) ? 1.0f : percentX;
		percentX = (percentX < 0.0f) ? 0.0f : percentX;

		// Get the current random positions

		WaveFormControlPoint newControlPoint;

		if (m_selectedControlPoint.isRandomSelected())
		{
			float const percentY = RectHelper::getPercentageInRectY(m_waveFormFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionMoveLocal));
			float const difference = (m_maxValue - m_minValue) * m_scale;

			if (m_selectedControlPoint.m_randomMinSelected)
			{
				float value = m_selectedControlPoint.m_controlPointIter->getValue() - ((m_maxValue * m_scale) - (percentY * difference));

				if (value < 0.0f)
				{
					value = 0.0f;
				}

				newControlPoint = WaveFormControlPoint(m_selectedControlPoint.m_controlPointIter->getPercent(), m_selectedControlPoint.m_controlPointIter->getValue(), m_selectedControlPoint.m_controlPointIter->getRandomMax(), value);
			}
			else
			{
				float value = ((m_maxValue * m_scale) - (percentY * difference)) - m_selectedControlPoint.m_controlPointIter->getValue();

				if (value < 0.0f)
				{
					value = 0.0f;
				}

				newControlPoint = WaveFormControlPoint(m_selectedControlPoint.m_controlPointIter->getPercent(), m_selectedControlPoint.m_controlPointIter->getValue(), value, m_selectedControlPoint.m_controlPointIter->getRandomMin());
			}
		}
		else
		{
			float const percentY = RectHelper::getPercentageInRectY(m_waveFormFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionMoveLocal));
			float const difference = (m_maxValue - m_minValue) * m_scale;
			float const value = (m_maxValue * m_scale) - (percentY * difference);

			newControlPoint = WaveFormControlPoint(percentX, value, m_selectedControlPoint.m_controlPointIter->getRandomMax(), m_selectedControlPoint.m_controlPointIter->getRandomMin());
		}

		// Set the new control point values

		m_waveForm.setControlPoint(m_selectedControlPoint.m_controlPointIter, newControlPoint);

		// Set the control value

		setControlValues(m_selectedControlPoint);

		// Signal the waveform changed but a rebuild is not necessary since the mouse
		// is still in movement

		emitWaveFormChanged(m_waveForm, false);

		// Notify that a rebuild needs to occur when the mouse is released

		m_rebuildOnRelease = true;
	}
	else
	{
		WaveFormEditControlPoint currentControlPoint(getControlPointAtPosition(m_mousePositionMoveLocal));

		if (currentControlPoint.m_controlPointIter != m_controlValuesControlPoint.m_controlPointIter)
		{
			if (currentControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd())
			{
				if (currentControlPoint.m_controlPointIter != m_anchoredControlPoint.m_controlPointIter)
				{
					m_controlValuesControlPoint = currentControlPoint;

					setControlValues(m_controlValuesControlPoint);
				}
			}
			else if (currentControlPoint.m_controlPointIter != m_anchoredControlPoint.m_controlPointIter)
			{
				m_controlValuesControlPoint = currentControlPoint;

				setControlValues(m_anchoredControlPoint);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawControlPoints(QPainter &painter, float const scale, float const max, float const min, WaveForm &waveForm, WaveFormEditControlPoint const &anchoredControlPoint, WaveFormEditControlPoint const &selectedControlPoint, WaveFormEditControlPoint const &mouseControlPoint, int const controlPointSize, bool const drawRandomHandles)
{
	UNREF(scale);

	QRect destRect(painter.window());

	painter.setPen(Qt::SolidLine);

	int renderControlPointSize = (controlPointSize < 0) ? m_controlPointSize : controlPointSize;

	for (ControlPointList::iterator iter = waveForm.getIteratorBegin(); iter != waveForm.getIteratorEnd(); ++iter)
	{
		//int x = m_splineRect.left() + iter->first * m_splineRect.width();
		//int y = m_horizontalRect.height();
		//
		//if (m_maxValue - m_minValue != 0.0f)
		//{
		//	y += ((m_maxValue - iter->second.m_value) / (m_maxValue - m_minValue)) * m_splineRect.height();
		//}

		QPoint currentPoint(RectHelper::getPointInRect(destRect, iter->getPercent(), iter->getValue(), min, max));

		// Draw the anchored control point

		if (anchoredControlPoint.m_controlPointIter == iter)
		{
			drawHandle(painter, currentPoint, renderControlPointSize, QColor("yellow"));
		}

		// Draw the selected control point

		if ((selectedControlPoint.m_controlPointIter == iter) || ((selectedControlPoint.m_controlPointIter == waveForm.getIteratorEnd()) && (mouseControlPoint.m_controlPointIter == iter)))
		{
			if (mouseControlPoint.m_randomMinSelected || selectedControlPoint.m_randomMinSelected)
			{
				QPoint randomPoint(RectHelper::getPointInRect(destRect, iter->getPercent(), iter->getValue() - iter->getRandomMin(), min, max));
				drawHandle(painter, randomPoint, renderControlPointSize, QColor("white"));
			}
			else if (mouseControlPoint.m_randomMaxSelected || selectedControlPoint.m_randomMaxSelected)
			{
				QPoint randomPoint(RectHelper::getPointInRect(destRect, iter->getPercent(), iter->getValue() + iter->getRandomMax(), min, max));
				drawHandle(painter, randomPoint, renderControlPointSize, QColor("white"));
			}
			else
			{
				drawHandle(painter, currentPoint, renderControlPointSize, QColor("red"));
			}
		}

		QPoint maxRandomPoint(RectHelper::getPointInRect(destRect, iter->getPercent(), iter->getValue() + iter->getRandomMax(), min, max));
		QPoint minRandomPoint(RectHelper::getPointInRect(destRect, iter->getPercent(), iter->getValue() - iter->getRandomMin(), min, max));

		// Draw the random control point handles

		if (controlPointSize == -1)
		{
			drawHandle(painter, QPoint(maxRandomPoint.x(), maxRandomPoint.y()), renderControlPointSize / 2, QColor("darkgray"));
			drawHandle(painter, QPoint(minRandomPoint.x(), minRandomPoint.y()), renderControlPointSize / 2, QColor("darkgray"));
		}

		// Draw the small control point handle

		drawHandle(painter, currentPoint, renderControlPointSize / 2, QColor("black"));

		// Draw the random control point handles

		if (drawRandomHandles)
		{
			int const halfWidth = renderControlPointSize;
			painter.fillRect(maxRandomPoint.x() - halfWidth + 1, maxRandomPoint.y() - halfWidth + 1, halfWidth * 2, halfWidth, QColor("green"));
			painter.fillRect(minRandomPoint.x() - halfWidth + 1, minRandomPoint.y(), halfWidth * 2, halfWidth, QColor("blue"));
		}
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::leaveEvent(QEvent *)
{
	update();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawLinearWaveform(QPainter &painter, PointList const &pointList, QColor const &color, float const scale)
{
	drawLinearWaveform(painter, m_maxValue, m_minValue, pointList, color, scale);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawSplineWaveform(QPainter &painter, PointList const &pointList, QColor const &color, float const scale)
{
	drawSplineWaveform(painter, m_maxValue, m_minValue, pointList, color, scale);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawLinearWaveform(QPainter & painter, float const max, float const min, PointList const & pointList, QColor const &color, float const scale)
{
	UNREF(scale);

	QRect destRect(painter.window());

	painter.setPen(color);
	PointList::const_iterator current = pointList.begin();
	PointList::const_iterator previous = current;
	QPoint currentPoint(0, 0);
	QPoint previousPoint(0, 0);

	for (; current != pointList.end(); ++current)
	{
		if (current == pointList.begin())
		{
			continue;
		}

		QPoint currentAveragePoint(RectHelper::getPointInRect(destRect, current->first, current->second, min, max));
		QPoint previousAveragePoint(RectHelper::getPointInRect(destRect, previous->first, previous->second, min, max));
		QPointArray pointArray(4);

		// Draw the control point line

		painter.drawLine(previousAveragePoint.x(), previousAveragePoint.y(), currentAveragePoint.x(), currentAveragePoint.y());

		// Save the previous control point

		previous = current;
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawSplineWaveform(QPainter & painter, float const max, float const min, PointList const & pointList, QColor const &color, float const scale)
{
	UNREF(scale);

	QRect destRect(painter.window());

	int listCount = pointList.size();

	if (listCount >= 3)
	{	
		painter.setPen(color);
		PointList::const_iterator iter = pointList.begin();
		PointList::const_iterator c1 = iter;
		PointList::const_iterator c2 = iter;
		++iter;
		PointList::const_iterator c3 = iter;
		++iter;
		PointList::const_iterator c4 = iter;

		int j = 0;
		int count = listCount - 1;

		// See if the current control point is on the same vertical as the previous
		// or next control point

		for (; j < count; ++j)
		{
			QPoint p1(RectHelper::getPointInRect(destRect, c1->first, c1->second, min, max));
			QPoint p2(RectHelper::getPointInRect(destRect, c2->first, c2->second, min, max));
			QPoint p3(RectHelper::getPointInRect(destRect, c3->first, c3->second, min, max));
			QPoint p4(RectHelper::getPointInRect(destRect, c4->first, c4->second, min, max));

			int controlPointCount = 14;

			float aResultX;
			float aResultY;

			float bResultX;
			float bResultY;

			CatmullRomSpline::getCatmullRomSplinePoint(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), 0.0f, bResultX, bResultY);

			float const tDelta = 1.0f / static_cast<float>(controlPointCount);
			float t = tDelta;

			for (int i = 0; i < controlPointCount; ++i)
			{
				CatmullRomSpline::getCatmullRomSplinePoint(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), t, aResultX, aResultY);

				float const xPrev = static_cast<float>(RectHelper::getPointInRect(destRect, c2->first, 0.0f).x());
				float const xNext = static_cast<float>(RectHelper::getPointInRect(destRect, c3->first, 0.0f).x());

				if (aResultX < xPrev)
				{
					aResultX = xPrev;
				}
				else if (aResultX > xNext)
				{
					aResultX = xNext;
				}

				painter.drawLine(static_cast<int>(aResultX), static_cast<int>(aResultY), static_cast<int>(bResultX), static_cast<int>(bResultY));

				bResultX = aResultX;
				bResultY = aResultY;
				t += tDelta;
			}

			c1 = c2;
			c2 = c3;
			c3 = c4;

			if (iter != pointList.end())
			{
				++iter;

				if (iter != pointList.end())
				{
					c4 = iter;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::waveFormGenerationPushButtonPressed()
{
}

//-----------------------------------------------------------------------------
void WaveFormEdit::setControlValues(WaveFormEditControlPoint const &waveFormEditControlPoint)
{
	if (waveFormEditControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd())
	{
		char text[256];
		sprintf(text, "%d", static_cast<int>(waveFormEditControlPoint.m_controlPointIter->getPercent() * 100.0f));
		m_percentLineEdit->setText(text);

		sprintf(text, "%.3f", waveFormEditControlPoint.m_controlPointIter->getValue());
		waveFormEditControlPoint.m_controlPointIter->setValue(QString(text).toFloat());
		m_valueLineEdit->setText(text);

		sprintf(text, "%.3f", waveFormEditControlPoint.m_controlPointIter->getRandomMax());
		waveFormEditControlPoint.m_controlPointIter->setRandomMax(QString(text).toFloat());
		m_maxRandomLineEdit->setText(text);

		sprintf(text, "%.3f", waveFormEditControlPoint.m_controlPointIter->getRandomMin());
		waveFormEditControlPoint.m_controlPointIter->setRandomMin(QString(text).toFloat());
		m_minRandomLineEdit->setText(text);
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::setInterpolationType(int index)
{
	switch (index)
	{
		case 0:
			{
				m_waveForm.setInterpolationType(WaveForm::IT_linear);
			}
			break;
		case 1:
			{
				m_waveForm.setInterpolationType(WaveForm::IT_spline);
			}
			break;
	}

	emitWaveFormChanged(m_waveForm, true);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::onSampleTypeComboBoxActivated(int index)
{
	switch (index)
	{
		case 0:
			{
				m_waveForm.setSampleType(WaveForm::ST_initial);
			}
			break;
		case 1:
			{
				m_waveForm.setSampleType(WaveForm::ST_continuous);
			}
			break;
	}

	emitWaveFormChanged(m_waveForm, true);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::emitWaveFormChanged(const WaveForm &waveForm, const bool forceUpdate)
{
	// Redraw the waveform

	m_waveFormFrame->update();

	// Signal the waveform changed

	emit waveFormChanged(waveForm, forceUpdate);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::controlsLineEditReturnPressed()
{
	float const percent = ParticleEditorUtility::clamp(ParticleEditorUtility::getFloat(m_percentLineEdit) / 100.0f, 1.0f, 0.0f);
	float const value = ParticleEditorUtility::getFloat(m_valueLineEdit);
	float const maxRandom = ParticleEditorUtility::clampMin(ParticleEditorUtility::getFloat(m_maxRandomLineEdit), 0.0f);
	float const minRandom = ParticleEditorUtility::clampMin(ParticleEditorUtility::getFloat(m_minRandomLineEdit), 0.0f);

	m_waveForm.setControlPoint(m_anchoredControlPoint.m_controlPointIter, WaveFormControlPoint(percent, value, maxRandom, minRandom));

	// Recalculate the min and max values

	m_waveForm.calculateMinMax(m_maxValue, m_minValue);

	// Reformat the controls text

	setControlValues(m_anchoredControlPoint);

	emitWaveFormChanged(m_waveForm, true);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::setToDisplayTemporaryWaveForm(bool const b)
{
	m_displayTemporaryWaveForm = b;

	// Recalculate the min and max values

	if (m_displayTemporaryWaveForm)
	{
		m_temporaryWaveForm.calculateMinMax(m_maxValue, m_minValue);
	}
	else
	{
		m_waveForm.calculateMinMax(m_maxValue, m_minValue);
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::swapToTemporaryWaveForm()
{
	// Copy the list data

	m_waveForm = m_temporaryWaveForm;

	// Clear the temporary list

	m_temporaryWaveForm.clear();

	// Disable the temporary flag

	setToDisplayTemporaryWaveForm(false);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawWaveForm(QPainter &painter, float const scale, float const max, float const min, WaveForm &waveForm, WaveFormEditControlPoint const &anchoredControlPoint, WaveFormEditControlPoint const &selectedControlPoint, WaveFormEditControlPoint const &mouseControlPoint, const int controlPointSize, bool const drawRandomHandles)
{
	char text[256];
	int const space = 7;

	// Modify the size of the rect to allow space for the min and max text

	//int const widthForText = 32;
	//QRect waveFormRect(rect.left() + widthForText, rect.top(), rect.width() - widthForText, rect.height());
	QRect destRect(painter.window());

	// Draw the min and max values

	painter.setPen(QColor(128, 128, 128));
	sprintf(text, "%.3f", max * scale);
	painter.drawText(destRect.left() - painter.fontMetrics().width(text) - space, RectHelper::getPointInRect(destRect, 0.0f, 0.0f).y() + painter.fontMetrics().height() / 2, text);

	sprintf(text, "%.3f", min * scale);
	painter.drawText(destRect.left() - painter.fontMetrics().width(text) - space, RectHelper::getPointInRect(destRect, 0.0f, 1.0f).y() + painter.fontMetrics().height() / 2, text);

	painter.setPen(QColor(0, 0, 0));
	sprintf(text, "%.3f", max);
	painter.drawText(destRect.left() - painter.fontMetrics().width(text) - space, RectHelper::getPointInRect(destRect, 0.0f, 0.0f).y() + painter.fontMetrics().height() / 2, text);

	sprintf(text, "%.3f", min);
	painter.drawText(destRect.left() - painter.fontMetrics().width(text) - space, RectHelper::getPointInRect(destRect, 0.0f, 1.0f).y() + painter.fontMetrics().height() / 2, text);

	// Draw a line across the top and bottom of the wave form

	painter.setPen(QColor(0, 0, 0));
	painter.drawLine(destRect.left(), destRect.top(), destRect.right(), destRect.top());
	painter.drawLine(destRect.left(), destRect.bottom() + 1, destRect.right(), destRect.bottom() + 1);

	// Create the point list for the current control points and the 
	// the random min and max control points

	PointList currentPointList;
	PointList randomMaxPointList;
	PointList randomMinPointList;

	for (ControlPointList::iterator current = waveForm.getIteratorBegin(); current != waveForm.getIteratorEnd(); ++current)
	{
		currentPointList.push_back(FloatFloatPair(current->getPercent(), current->getValue()));
		randomMaxPointList.push_back(FloatFloatPair(current->getPercent(), current->getValue() + current->getRandomMax()));
		randomMinPointList.push_back(FloatFloatPair(current->getPercent(), current->getValue() - current->getRandomMin()));
	}

	// Draw the lines between the control points

	switch (waveForm.getInterpolationType())
	{
		case WaveForm::IT_linear:
			{
				drawLinearWaveform(painter, max, min, randomMaxPointList, QColor("orange"), scale);
				drawLinearWaveform(painter, max, min, randomMinPointList, QColor("orange"), scale);
				drawLinearWaveform(painter, max, min, currentPointList, QColor("black"), scale);
			}
			break;
		case WaveForm::IT_spline:
			{
				int const listCount = waveForm.getControlPointCount();

				if (listCount <= 2)
				{
					drawLinearWaveform(painter, max, min, randomMaxPointList, QColor("orange"), scale);
					drawLinearWaveform(painter, max, min, randomMinPointList, QColor("orange"), scale);
					drawLinearWaveform(painter, max, min, currentPointList, QColor("black"), scale);
				}
				else
				{
					drawSplineWaveform(painter, max, min, randomMaxPointList, QColor("orange"), scale);
					drawSplineWaveform(painter, max, min, randomMinPointList, QColor("orange"), scale);
					drawSplineWaveform(painter, max, min, currentPointList, QColor("black"), scale);
				}
			}
			break;
		default:
		{
			DEBUG_FATAL(true, ("Unexpected waveform type."));
		}
	}

	// Draw the 25, 50, 75 percent vertical lines

	int const color = 128;
	QPen pen(QColor(color, color, color), 0, Qt::DotLine);
	painter.setPen(pen);

	QPoint a(RectHelper::getPointInRect(destRect, 0.25f, 0.0f));
	QPoint b(RectHelper::getPointInRect(destRect, 0.25f, 1.0f));
	painter.drawLine(a, b);

	a = QPoint(RectHelper::getPointInRect(destRect, 0.5f, 0.0f));
	b = QPoint(RectHelper::getPointInRect(destRect, 0.5f, 1.0f));
	painter.drawLine(a, b);

	a = QPoint(RectHelper::getPointInRect(destRect, 0.75f, 0.0f));
	b = QPoint(RectHelper::getPointInRect(destRect, 0.75f, 1.0f));
	painter.drawLine(a, b);

	// Draw a dashed line for the zero line

	QPoint baseLine(RectHelper::getPointInRect(destRect, 0.0f, max / (max - min)));

	if (destRect.contains(baseLine.x(), baseLine.y()))
	{
		painter.drawLine(destRect.left(), baseLine.y(), destRect.right(), baseLine.y());
	}

	// Draw the spline area control point boxes

	drawControlPoints(painter, scale, max, min, waveForm, anchoredControlPoint, selectedControlPoint, mouseControlPoint, controlPointSize, drawRandomHandles);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::setWaveForm(WaveForm const &waveForm)
{
	// Change the caption to the current attribute

#ifdef _DEBUG
	setCaption(waveForm.getName().c_str());
#endif // _DEBUG

	// Copy the wave form
	
	m_waveForm = waveForm;
	
	// Disable the temporary control point list
	
	m_temporaryWaveForm.clear();
	
	setToDisplayTemporaryWaveForm(false);

	// Interpolation Type

	m_interpolationTypeComboBox->setCurrentItem(static_cast<int>(m_waveForm.getInterpolationType()));

	// Sample Mode

	m_sampleTypeComboBox->setCurrentItem(static_cast<int>(m_waveForm.getSampleType()));

	// Set the anchored control point to the first control point

	m_anchoredControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorBegin());
	m_selectedControlPoint = WaveFormEditControlPoint(m_waveForm.getIteratorEnd());

	// Update the control values

	setControlValues(m_anchoredControlPoint);

	// Reset the scale

	m_scaleIndex = m_scaleIndexMin;

	calculateScale();

	// Redraw the waveform

	m_waveFormFrame->update();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::drawHandle(QPainter &painter, QPoint const &point, int const halfWidth, QColor const &color)
{
	painter.fillRect(point.x() - halfWidth + 1, point.y() - halfWidth + 1, halfWidth * 2, halfWidth * 2, color);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::calculateScale()
{
	// This is fine since m_scaleMin is always 0

	m_scale = 1.0f + static_cast<float>(m_scaleIndex) / static_cast<float>(m_scaleIndexMax);
}

//-----------------------------------------------------------------------------
void WaveFormEdit::addControlPointToRemoveList(WaveFormEditControlPoint &waveFormControlPoint)
{
	// Make sure the control point is not already in the list and make sure
	// the control point is not the start or end control point

	bool validToRemove = true;

	WaveFormEditControlPoint nextWaveFormControlPoint(waveFormControlPoint);

	if ((waveFormControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd()) &&
	    (waveFormControlPoint.m_controlPointIter != m_waveForm.getIteratorBegin()) &&
		 (++nextWaveFormControlPoint.m_controlPointIter != m_waveForm.getIteratorEnd()))
	{
		ControlPointIteratorList::iterator controlPointIter = m_removeControlPointList->begin();

		for (; controlPointIter != m_removeControlPointList->end(); ++controlPointIter)
		{
			if (controlPointIter->m_controlPointIter == waveFormControlPoint.m_controlPointIter)
			{
				validToRemove = false;
				break;
			}
		}
	}
	else
	{
		validToRemove = false;
	}
	
	if (validToRemove)
	{
		m_removeControlPointList->push_back(waveFormControlPoint);
	}
}

//-----------------------------------------------------------------------------
void WaveFormEdit::setDrawRandomHandles(bool const drawRandomHandles)
{
	m_drawRandomHandles = drawRandomHandles;

	// Redraw the color ramp

	m_waveFormFrame->update();
}

//-----------------------------------------------------------------------------
void WaveFormEdit::slotDrawWaveForm(QPainter *painter)
{
	// Draw the waveform

	WaveFormEditControlPoint mouseWaveFormEditControlPoint(getControlPointAtPosition(m_mousePositionMoveLocal));

	drawWaveForm(*painter, m_scale, m_maxValue, m_minValue, m_displayTemporaryWaveForm ? m_temporaryWaveForm : m_waveForm, m_anchoredControlPoint, m_selectedControlPoint, mouseWaveFormEditControlPoint, -1, m_drawRandomHandles);

	//// Draw the scale meter
	//
	//const int scaleMeterWidth = 10;
	//const int scaleMeterGap = 6;
	//const int w = scaleMeterWidth;
	//const int h = static_cast<int>((m_scale - 1.0f) * static_cast<float>(boundingRect.height() - scaleMeterGap * 2));
	//const int left = boundingRect.right() - scaleMeterWidth - scaleMeterGap;
	//const int top = boundingRect.bottom() - h - scaleMeterGap;
	//
	//painter.fillRect(left - 2, boundingRect.top() + scaleMeterGap - 3, w + 4, boundingRect.height() - scaleMeterGap * 2 + 4, QColor(180, 180, 180));
	//painter.fillRect(left, top, w, h, QColor("orange"));
}

///////////////////////////////////////////////////////////////////////////////
