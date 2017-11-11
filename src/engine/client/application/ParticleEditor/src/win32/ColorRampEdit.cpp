// ============================================================================
//
// ColorRampEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ColorRampEdit.h"
#include "ColorRampEdit.moc"

#include "sharedDebug/PerformanceTimer.h"
#include "RectHelper.h"

///////////////////////////////////////////////////////////////////////////////
//
// TableItemInt
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
class TableItemInt : public QTableItem
{
public:
	
	TableItemInt(QTable *table, EditType editType, const QString &text, int const min, int const max);

	virtual QWidget *createEditor() const;
	virtual void     setContentFromEditor(QWidget *widget);

private:

	int m_min;
	int m_max;
};

//-----------------------------------------------------------------------------
TableItemInt::TableItemInt(QTable *table, EditType editType, const QString &text, int const min, int const max)
 : QTableItem(table, editType, text)
 , m_min(min)
 , m_max(max)
{
}

//-----------------------------------------------------------------------------
QWidget *TableItemInt::createEditor() const
{
	QLineEdit *lineEdit = new QLineEdit(table()->viewport(), "QLineEdit");

	lineEdit->setValidator(new QIntValidator(m_min, m_max, table()->viewport(), "QIntValidator"));
	lineEdit->setText(text());

	return lineEdit;
}

//-----------------------------------------------------------------------------
void TableItemInt::setContentFromEditor(QWidget *widget)
{
	QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(widget);

	if (lineEdit)
	{
		// See if the result is valid

		ParticleEditorUtility::validateLineEditInt(lineEdit, ParticleBoundInt(m_max, m_min));

		// Set the final result

		setText(lineEdit->text());
	}
	else
	{
		DEBUG_FATAL(true, ("TableItemInt::setContentFromEditor() - Invalid widget"));
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// TableItemColor
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
class TableItemColor : public QTableItem
{
public:

	TableItemColor(QTable *table, EditType editType, int const red, int const green, int const blue);

	virtual void paint(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect, bool selected);

	int getRed() const;
	int getGreen() const;
	int getBlue() const;

private:

	int m_red;
	int m_green;
	int m_blue;

};

//-----------------------------------------------------------------------------
TableItemColor::TableItemColor(QTable *table, EditType editType, int const red, int const green, int const blue)
 : QTableItem(table, editType, "")
 , m_red(red)
 , m_green(green)
 , m_blue(blue)
{
}

//-----------------------------------------------------------------------------
void TableItemColor::paint(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect, bool selected)
{
	UNREF(colorGroup);
	UNREF(selected);

	painter->fillRect(QRect(0, 0, rect.width(), rect.height()), QBrush(QColor(m_red, m_green, m_blue)));
}

//-----------------------------------------------------------------------------
int TableItemColor::getRed() const
{
	return m_red;
}

//-----------------------------------------------------------------------------
int TableItemColor::getGreen() const
{
	return m_green;
}

//-----------------------------------------------------------------------------
int TableItemColor::getBlue() const
{
	return m_blue;
}

///////////////////////////////////////////////////////////////////////////////
//
// ColorRampFrame
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ColorRampFrame::ColorRampFrame(QWidget *parent, char const *name)
 : QFrame(parent, name)
 , m_mousePositionMoveLocal(0, 0)
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
ColorRampFrame::~ColorRampFrame()
{
	delete m_performanceTimer;
}

//-----------------------------------------------------------------------------
void ColorRampFrame::paintEvent(QPaintEvent *)
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
void ColorRampFrame::drawContents(QPainter *painter)
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
			//painter->eraseRect(m_pixMap.rect());

			// Set the area to draw the color ramp in

			QRect viewPortRect(getViewPortRectLocal());

			painter->setViewport(viewPortRect);
			painter->setWindow(QRect(0, 0, painter->viewport().width(), painter->viewport().height()));

			// Draw the color ramp onto the pixmap

			emit signalDrawColorRamp(painter);

			// Draw the reference lines
			
			int const markerLineLength = 5;
			painter->setPen(QColor(96, 96, 96));
			drawMarkerLine(*painter, 0.25f, markerLineLength);
			drawMarkerLine(*painter, 0.5f, markerLineLength);
			drawMarkerLine(*painter, 0.75f, markerLineLength);

			// Draw the mouse marker
			
			drawMouseMarker(*painter);

			// Blit the pixmap onto this widget

			bitBlt(this, contentsRect().left(), contentsRect().top(), &m_pixMap);

			painter->end();
		}
	}
}

//-----------------------------------------------------------------------------
QRect ColorRampFrame::getViewPortRectLocal() const
{
	int const marginTopBottom = 30;
	int const marginLeftRight = 20;

	return QRect(QPoint(contentsRect().x() + marginLeftRight, contentsRect().y() + marginTopBottom), QSize(contentsRect().width() - marginLeftRight * 2, contentsRect().height() - marginTopBottom * 2));
}

//-----------------------------------------------------------------------------
QRect ColorRampFrame::getViewPortRectGlobal() const
{
	return QRect(mapToGlobal(getViewPortRectLocal().topLeft()), mapToGlobal(getViewPortRectLocal().bottomRight()));
}

//-----------------------------------------------------------------------------
void ColorRampFrame::mouseMoveEvent(QMouseEvent *mouseEvent)
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
QPoint const &ColorRampFrame::getMousePositionMoveLocal() const
{
	return m_mousePositionMoveLocal;
}

//-----------------------------------------------------------------------------
void ColorRampFrame::drawMouseMarker(QPainter &painter)
{
	UNREF(painter);

	QRect destRect(painter.window());

	// Draw the mouse position above the color ramp

	QPoint positionMapped(RectHelper::mapToRect(painter.viewport(), m_mousePositionMoveLocal));

	if (rect().contains(m_mousePositionMoveLocal) &&
	    (positionMapped.x() >= destRect.left()) &&
	    (positionMapped.x() <= destRect.right()))
	{
		int const x = positionMapped.x();
		int const y = -2;
		int const arrowSize = 10;
	
		QPointArray pointArray(3);
		pointArray.setPoint(0, x, y);
		pointArray.setPoint(1, x - arrowSize / 2, y - arrowSize);
		pointArray.setPoint(2, x + arrowSize / 2, y - arrowSize);
	
		painter.setPen("red");
		painter.setBrush(QBrush(QColor("black"), Qt::SolidPattern));
		painter.drawPolygon(pointArray);
	
		int const destRectWidth = destRect.width();
		float const percent = static_cast<float>(x) / static_cast<float>(destRectWidth - 1);
		
		painter.setPen(QColor(96, 96, 96));
		drawPercent(painter, percent, 0, true);
	}
}

//-----------------------------------------------------------------------------
void ColorRampFrame::drawPercent(QPainter &painter, float const percent, int const decimals, bool const hasBackground)
{
	DEBUG_FATAL((percent < 0.0f), ("ColorRampFrame::drawPercent() - percent(%f) < 0", percent));
	DEBUG_FATAL((percent > 1.0f), ("ColorRampFrame::drawPercent() - percent(%f) > 1", percent));
	DEBUG_FATAL((decimals < 0.0f), ("ColorRampFrame::drawPercent() - decimals(%d) < 0", decimals));

	QRect destRect(painter.window());

	// Draw a text percent

	char text[256];
	
	if (decimals > 0)
	{
		sprintf(text, "%.*f", decimals, percent * 100.0f);
	}
	else
	{
		sprintf(text, "%d", static_cast<int>(percent * 100.0f));
	}
	
	int const textWidth = fontMetrics().width(text);
	int const textHeight = fontMetrics().height();
	int const x = RectHelper::getPointInRect(destRect, percent, 0.0f).x();

	if (hasBackground)
	{
		QRect backgroundRect(QPoint(x - textWidth, destRect.top() - 14 - textHeight), QSize(textWidth * 2, textHeight));
		painter.fillRect(backgroundRect, QBrush(backgroundColor()));
	}

	painter.setPen(QColor("black"));
	painter.drawText(x - textWidth / 2, destRect.top() - 14, text);
}

//-----------------------------------------------------------------------------
void ColorRampFrame::drawMarkerLine(QPainter &painter, float const percent, int const length)
{
	DEBUG_FATAL((length < 0.0f), ("ColorRampFrame::drawMarkerLine() - length(%d) < 0", length));
	DEBUG_FATAL((percent < 0.0f), ("ColorRampFrame::drawMarkerLine() - percent(%f) < 0", percent));
	DEBUG_FATAL((percent > 1.0f), ("ColorRampFrame::drawMarkerLine() - percent(%f) > 1", percent));
	
	QRect destRect(painter.window());

	int const x = RectHelper::getPointInRect(destRect, percent, 0.0f).x();
	
	painter.drawLine(x, destRect.top() - 1, x, destRect.top() - 1 - length);
}

//-----------------------------------------------------------------------------
void ColorRampFrame::drawHandle(QPainter &painter, float const percent, int const handleSize)
{
	DEBUG_FATAL((percent < 0.0f), ("ColorRampFrame::drawHandle() - percent(%f) < 0", percent));
	DEBUG_FATAL((percent > 1.0f), ("ColorRampFrame::drawHandle() - percent(%f) > 1", percent));

	QRect destRect(painter.window());
	QPoint point(static_cast<float>(destRect.width() - 1) * percent, destRect.height() + handleSize);

	QPointArray pointArray(5);
	pointArray.setPoint(0, point.x() - handleSize, point.y() + handleSize);
	pointArray.setPoint(1, point.x() - handleSize, point.y() + handleSize * 2);
	pointArray.setPoint(2, point.x() + handleSize, point.y() + handleSize * 2);
	pointArray.setPoint(3, point.x() + handleSize, point.y() + handleSize - 1);
	pointArray.setPoint(4, point.x(), point.y() - handleSize);

	painter.setPen("red");
	painter.setBrush(QBrush(QColor("black"), Qt::SolidPattern));
	painter.drawPolygon(pointArray);
}

///////////////////////////////////////////////////////////////////////////////
//
// ColorRampEdit
//
///////////////////////////////////////////////////////////////////////////////

const int ColorRampEdit::m_handleSize = 7;

//-----------------------------------------------------------------------------
ColorRampEdit::ColorRampEdit(QWidget *parent, char const *name)
 : BaseColorRampWidget(parent, name)
 , m_addControlPointPopupMenu(NULL)
 , m_deleteControlPointPopupMenu(NULL)
 , m_addControlPointList(NULL)
 , m_removeControlPointList(NULL)
 , m_rebuildOnRelease(false)
{
	// Setup the color table

	m_colorTable->horizontalHeader()->setClickEnabled(false);
	m_colorTable->horizontalHeader()->setResizeEnabled(false);
	m_colorTable->horizontalHeader()->setMovingEnabled(false);
	m_colorTable->horizontalHeader()->setStretchEnabled(false);

	m_colorTable->verticalHeader()->setClickEnabled(false);
	m_colorTable->verticalHeader()->setResizeEnabled(false);
	m_colorTable->verticalHeader()->setMovingEnabled(false);
	m_colorTable->verticalHeader()->setStretchEnabled(false);

	m_colorTable->setColumnWidth(0, 30);
	m_colorTable->setColumnWidth(1, 30);
	m_colorTable->setColumnWidth(2, 30);
	m_colorTable->setColumnWidth(3, 30);
	m_colorTable->setColumnWidth(4, 50);
	m_colorTable->setSelectionMode(QTable::NoSelection);
	m_colorTable->setRowMovingEnabled(false);
	m_colorTable->setColumnMovingEnabled(false);
	m_colorTable->setColumnReadOnly(4, true);
	m_colorTable->setDragEnabled(false);
	m_colorTable->setMouseTracking(true);
	m_colorTable->setNumCols(5);
	m_colorTable->setShowGrid(true);
	m_colorTable->setSorting(false);

	// Setup connections

	connect(m_colorTable, SIGNAL(clicked(int, int, int, const QPoint &)), this, SLOT(slotColorTableClicked(int, int, int, const QPoint &)));
	connect(m_colorTable, SIGNAL(doubleClicked(int, int, int, const QPoint &)), this, SLOT(slotColorTableDoubleClicked(int, int, int, const QPoint &)));
	connect(m_colorTable, SIGNAL(valueChanged(int, int)), this, SLOT(slotColorTableValueChanged(int, int)));
	connect(m_colorInterpolationTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotColorInterpolationTypeComboBoxActivated(int)));
	connect(m_colorSampleTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotColorSampleTypeComboBoxActivated(int)));
	connect(m_colorRampFrame, SIGNAL(signalDrawColorRamp(QPainter *)), this, SLOT(slotDrawColorRamp(QPainter *)));

	// Select nothing

	m_selectedControlPoint = m_colorRamp.getIteratorEnd();
	m_anchoredControlPoint = m_colorRamp.getIteratorBegin();
	m_deleteControlPoint = m_colorRamp.getIteratorEnd();

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

	// Setup the add remove lists

	m_addControlPointList = new ControlPointList();
	m_removeControlPointList = new ControlPointIteratorList();

	// Initialize some random values

	m_colorRamp.randomize();
}

//-----------------------------------------------------------------------------
ColorRampEdit::~ColorRampEdit()
{
	delete m_addControlPointList;
	delete m_removeControlPointList;
}

//-----------------------------------------------------------------------------
void ColorRampEdit::mousePressEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		// Find the control point to select if the left button was pressed

		if (mouseEvent->button() == Qt::LeftButton)
		{
			m_selectedControlPoint = getControlPoint(mouseEvent->globalPos());
		}

		// Find the control point to be deleted if the right button was pressed

		if (mouseEvent->button() == Qt::RightButton)
		{
			m_deleteControlPoint = getControlPoint(mouseEvent->globalPos());
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		// Save the mouse release position

		m_mousePositionReleaseLocal = mouseEvent->pos();

		// See if we need to add or remove any control points

		if (mouseEvent->button() == Qt::RightButton)
		{
			// Make sure no delete happens when the begin and end control point are
			// clicked

			ControlPointList::iterator current = getControlPoint(mouseEvent->globalPos());

			if (m_colorRampFrame->getViewPortRectGlobal().contains(mapToGlobal(mouseEvent->pos())) &&
			    (current == m_colorRamp.getIteratorEnd()))
			{
				m_addControlPointPopupMenu->exec(mouseEvent->globalPos());
			}
			else if (current != m_colorRamp.getIteratorBegin())
			{
				if (current != m_colorRamp.getIteratorEnd())
				{
					if (current == m_deleteControlPoint)
					{
						if (++current != m_colorRamp.getIteratorEnd())
						{
							m_deleteControlPointPopupMenu->exec(mouseEvent->globalPos());
						}
					}
				}
			}
		}

		// Select nothing

		m_selectedControlPoint = m_colorRamp.getIteratorEnd();

		// Signal things changed

		if (m_rebuildOnRelease)
		{
			m_rebuildOnRelease = false;

			emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		// Set the mouse position

		m_mousePositionMoveLocal = mouseEvent->pos();
		
		ControlPointList::const_iterator temp = m_selectedControlPoint;

		if ((m_selectedControlPoint != m_colorRamp.getIteratorEnd()) &&
			(m_selectedControlPoint != m_colorRamp.getIteratorBegin()) &&
			(++temp != m_colorRamp.getIteratorEnd()))
		{
			if (m_colorRampFrame->rect().width() > 0)
			{
				// Some handle is selected so lets move it

				float const percent = (m_colorRampFrame->getViewPortRectGlobal().width() < 0) ? 0.0f : (static_cast<float>(mouseEvent->globalPos().x() - m_colorRampFrame->getViewPortRectGlobal().left()))  / (static_cast<float>(m_colorRampFrame->getViewPortRectGlobal().width() - 1));
				float const red = m_selectedControlPoint->getRed();
				float const green = m_selectedControlPoint->getGreen();
				float const blue = m_selectedControlPoint->getBlue();

				ColorRampControlPoint controlPoint(percent, red, green, blue);
				m_colorRamp.setControlPoint(m_selectedControlPoint, controlPoint);

				// Let things know the color ramp changed but a rebuild is not
				// necessary because the mouse is still in movement

				emitColorRampChanged(m_colorRamp, m_alphaWaveForm, false);
			}

			// Notify that a rebuild needs to occur when the mouse is released

			m_rebuildOnRelease = true;
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		if (mouseEvent->button() == Qt::LeftButton)
		{
			m_anchoredControlPoint = getControlPoint(mouseEvent->globalPos());

			if (m_anchoredControlPoint != m_colorRamp.getIteratorEnd())
			{
				openColorDialog();
			}
		}
	}
}

//-----------------------------------------------------------------------------
ColorRampEdit::ControlPointList::iterator ColorRampEdit::getControlPoint(QPoint const &pointGlobal)
{
	QRect colorRampRectGlobal(m_colorRampFrame->getViewPortRectGlobal());

	ControlPointList::iterator result = m_colorRamp.getIteratorEnd();
	ControlPointList::iterator iterControlPointList = m_colorRamp.getIteratorBegin();

	for (; iterControlPointList != m_colorRamp.getIteratorEnd(); ++iterControlPointList)
	{
		int const x = colorRampRectGlobal.left() + static_cast<int>(static_cast<float>(colorRampRectGlobal.width() - 1) * iterControlPointList->getPercent());
		int const y = colorRampRectGlobal.bottom() + m_handleSize * 2;

		QRect handleRect(x - m_handleSize, y - m_handleSize, m_handleSize * 2, m_handleSize * 3);

		if (handleRect.contains(pointGlobal))
		{
			result = iterControlPointList;

			// Special case, if the first control point is selected, see if there
			// is any other control points on top of it, so that you can move a 
			// control point on top of it and move it back off again

			if (iterControlPointList != m_colorRamp.getIteratorBegin())
			{
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void ColorRampEdit::drawColorRamp(QPainter &painter, ColorRamp const &colorRamp, WaveForm const &alphaWaveForm)
{
	UNREF(alphaWaveForm);

	QRect destRect(painter.window());

	// Draw the color ramp itself
	
	if ((destRect.top() < destRect.bottom()) && (destRect.left() < destRect.right()))
	{
		if (colorRamp.getControlPointCount() >= 2)
		{
			for (int x = 0; x < destRect.width(); ++x)
			{
				float const percent = static_cast<float>(x) / static_cast<float>(destRect.width() - 1);
	
				float red;
				float green;
				float blue;
	
				colorRamp.getColorAtPercent(percent, red, green, blue);
				painter.setPen(QColor(static_cast<int>(255.0f * red), static_cast<int>(255.0f * green), static_cast<int>(255.0f * blue)));
				painter.drawLine(destRect.left() + x, destRect.top(), destRect.left() + x, destRect.bottom());
			}
		}
	}

	// Draw a hairline border around it
	
	painter.setPen(QColor("black"));
	painter.drawRect(destRect);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::setColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm)
{
	// Set the new color ramp and waveform

	m_colorRamp = colorRamp;
	m_alphaWaveForm = alphaWaveForm;

	// Setup the color table

	setColorTable(m_colorRamp);

	// Interpolation Type

	switch (m_colorRamp.getInterpolationType())
	{
		case ColorRamp::IT_linear:
			{
				m_colorInterpolationTypeComboBox->setCurrentItem(0);
			}
			break;
		case ColorRamp::IT_spline:
			{
				m_colorInterpolationTypeComboBox->setCurrentItem(1);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ColorRampEdit::setColorRamp() - Invalid color ramp interpolation type specified."));
			}
			break;
	}
	
	// Sample Type

	switch (m_colorRamp.getSampleType())
	{
		case ColorRamp::ST_all:
			{
				m_colorSampleTypeComboBox->setCurrentItem(0);
			}
			break;
		case ColorRamp::ST_single:
			{
				m_colorSampleTypeComboBox->setCurrentItem(1);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ColorRampEdit::setColorRamp() - Invalid color ramp sample type specified."));
			}
			break;
	}

	// Redraw the color ramp

	m_colorRampFrame->update();
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotAddControlPoint(int)
{
	// Get the mouse release x position percentage in the color ramp

	float const releasePercentX = RectHelper::getPercentageInRectX(m_colorRampFrame->getViewPortRectGlobal(), mapToGlobal(m_mousePositionReleaseLocal));

	// Add the new control point
	
	setUpdatesEnabled(false);
	m_anchoredControlPoint = m_colorRamp.insert(ColorRampControlPoint(releasePercentX, 1.0f, 1.0f, 1.0f));
	DEBUG_FATAL((m_anchoredControlPoint == m_colorRamp.getIteratorEnd()), ("ColorRampEdit::slotAddControlPoint() - Invalid added control point."));
	setUpdatesEnabled(true);

	// Let the user select a new color for the added control point

	openColorDialog();
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotDeleteControlPoint(int)
{
	// Add the control point to the remove list

	addControlPointToRemoveList(m_deleteControlPoint);

	// Invalidate the delete control point

	m_deleteControlPoint = m_colorRamp.getIteratorEnd();

	setUpdatesEnabled(false);

	// Remove any old control points
	
	for (; !m_removeControlPointList->empty(); m_removeControlPointList->pop_front())
	{
		ControlPointList::iterator current = *m_removeControlPointList->begin();
	
		m_colorRamp.remove(current);
	}
	
	setUpdatesEnabled(true);

	emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::addControlPointToRemoveList(ControlPointList::iterator &iter)
{
	// Make sure the control point is not already in the list and make sure
	// the control point is not the start or end control point

	bool validToRemove = true;

	ControlPointList::iterator nextIter(iter);

	if ((iter != m_colorRamp.getIteratorEnd()) &&
	    (iter != m_colorRamp.getIteratorBegin()) &&
		 (++nextIter != m_colorRamp.getIteratorEnd()))
	{
		ControlPointIteratorList::iterator current = m_removeControlPointList->begin();

		for (; current != m_removeControlPointList->end(); ++current)
		{
			if ((*current) == iter)
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
		m_removeControlPointList->push_back(iter);
	}
}

//-----------------------------------------------------------------------------
ColorRamp const &ColorRampEdit::getColorRamp() const
{
	return m_colorRamp;
}

//-----------------------------------------------------------------------------
WaveForm const &ColorRampEdit::getAlphaWaveForm() const
{
	return m_alphaWaveForm;
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotDrawColorRamp(QPainter *painter)
{
	// Draw the color ramp and stuff

	drawColorRamp(*painter, m_colorRamp, m_alphaWaveForm);

	// Draw the color handles
	
	drawColorHandles(*painter);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::drawColorHandles(QPainter &painter)
{
	// Draw the color handle positions
	
	for (ControlPointList::iterator current = m_colorRamp.getIteratorBegin(); current != m_colorRamp.getIteratorEnd(); ++current)
	{
		if (m_selectedControlPoint == current)
		{
			painter.setPen(QColor("red"));
		}
		else
		{
			painter.setPen(QColor("black"));
		}
	
		const int markerLineLength = 11;

		m_colorRampFrame->drawHandle(painter, current->getPercent(), m_handleSize);
		m_colorRampFrame->drawMarkerLine(painter, current->getPercent(), markerLineLength);
		//m_colorRampFrame->drawPercent(painter, current->getPercent(), ((current->getPercent() >= 1.0f) || (current->getPercent() <= 0.0f)) ? 0 : 1);
		m_colorRampFrame->drawPercent(painter, current->getPercent(), 0);
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotColorTableClicked(int row, int col, int button, const QPoint &mousePos)
{
	UNREF(col);

	if (button == Qt::RightButton)
	{
		if ((row > 0) && (row < (m_colorTable->numRows() - 1)))
		{
			// Find the control point to delete

			ControlPointList::iterator iterControlPointList = m_colorRamp.getIteratorBegin();
			int index = 0;

			for (; iterControlPointList != m_colorRamp.getIteratorEnd(); ++iterControlPointList)
			{
				if (index == row)
				{
					// Set the control point to delete
					
					m_deleteControlPoint = iterControlPointList;

					break;
				}

				++index;
			}

			// Delete the control point

			m_deleteControlPointPopupMenu->exec(m_colorTable->mapToGlobal(mousePos));
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotColorTableDoubleClicked(int row, int col, int button, const QPoint &mousePos)
{
	UNREF(mousePos);

	if (button == Qt::LeftButton)
	{
		if (col == 4)
		{
			// Set the anchored control point

			ControlPointList::iterator iterControlPointList = m_colorRamp.getIteratorBegin();
			int index = 0;

			for (; iterControlPointList != m_colorRamp.getIteratorEnd(); ++iterControlPointList)
			{
				if (index == row)
				{
					// Set the control point to delete
					
					m_anchoredControlPoint = iterControlPointList;

					break;
				}

				++index;
			}

			// Open the color dialog

			openColorDialog();
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::openColorDialog()
{
	// Redraw the color ramp

	m_colorRampFrame->update();

	// Open a color dialog

	int const initialRed = static_cast<int>(m_anchoredControlPoint->getRed() * 255.0f);
	int const initialGreen = static_cast<int>(m_anchoredControlPoint->getGreen() * 255.0f);
	int const initialBlue = static_cast<int>(m_anchoredControlPoint->getBlue() * 255.0f);

	QColor resultColor(QColorDialog::getColor(QColor(initialRed, initialGreen, initialBlue), this, "QColorDialog"));

	if (resultColor.isValid())
	{
		float const percent = m_anchoredControlPoint->getPercent();
		float const red = (static_cast<float>(resultColor.red())) / 255.0f;
		float const green = (static_cast<float>(resultColor.green())) / 255.0f;
		float const blue = (static_cast<float>(resultColor.blue())) / 255.0f;

		*m_anchoredControlPoint = ColorRampControlPoint(percent, red, green, blue);
	}

	emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::leaveEvent(QEvent *event)
{
	UNREF(event);

	// Invalidate the mouse position

	m_mousePositionMoveLocal = QPoint(-1, -1);

	// Force a repaint

	m_colorRampFrame->update();
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotColorTableValueChanged(int row, int col)
{
	if (col <= 3)
	{
		// Find the control point to update

		ControlPointList::iterator iterControlPointList = m_colorRamp.getIteratorBegin();
		int index = 0;

		for (; iterControlPointList != m_colorRamp.getIteratorEnd(); ++iterControlPointList)
		{
			if (index == row)
			{
				// Get the new control point values

				float const percent = atof(m_colorTable->text(index, 0)) / 100.0f;
				float const red = atof(m_colorTable->text(index, 1)) / 255.0f;
				float const green = atof(m_colorTable->text(index, 2)) / 255.0f;
				float const blue = atof(m_colorTable->text(index, 3)) / 255.0f;

				// Make sure the values actually changed

				if ((iterControlPointList->getPercent() != percent) ||
				    (iterControlPointList->getRed() != red) ||
					(iterControlPointList->getGreen() != green) ||
					(iterControlPointList->getBlue() != blue))
				{
					m_colorRamp.setControlPoint(iterControlPointList, ColorRampControlPoint(percent, red, green, blue));

					emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
				}

				break;
			}

			++index;
		}
	}
}

//-----------------------------------------------------------------------------
void ColorRampEdit::emitColorRampChanged(ColorRamp const &colorRamp, WaveForm const &waveForm, const bool forceRebuild)
{
	if (forceRebuild)
	{
		// Setup the color table

		setColorTable(colorRamp);
	}

	// Redraw the color ramp

	m_colorRampFrame->update();

	// Signal that the color of a control point changed

	emit colorRampChanged(colorRamp, waveForm, forceRebuild);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotColorInterpolationTypeComboBoxActivated(int index)
{
	switch (index)
	{
		case 0:
			{
				m_colorRamp.setInterpolationType(ColorRamp::IT_linear);
			}
			break;
		case 1:
			{
				m_colorRamp.setInterpolationType(ColorRamp::IT_spline);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ColorRampEdit::slotColorInterpolationTypeComboBoxActivated() - Invalid color ramp interpolation type specified."));
			}
			break;
	}

	emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::slotColorSampleTypeComboBoxActivated(int index)
{
	switch (index)
	{
		case 0:
			{
				m_colorRamp.setSampleType(ColorRamp::ST_all);
			}
			break;
		case 1:
			{
				m_colorRamp.setSampleType(ColorRamp::ST_single);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ColorRampEdit::slotColorSampleTypeComboBoxActivated() - Invalid color ramp sample type specified."));
			}
			break;
	}

	emitColorRampChanged(m_colorRamp, m_alphaWaveForm, true);
}

//-----------------------------------------------------------------------------
void ColorRampEdit::setColorTable(ColorRamp const &colorRamp)
{
	// Setup the table

	m_colorTable->setNumRows(colorRamp.getControlPointCount());

	ColorRamp::ControlPointList::const_iterator iterColorRampControlPoint = colorRamp.getIteratorBegin();

	int row = 0;
	for (; iterColorRampControlPoint != colorRamp.getIteratorEnd(); ++iterColorRampControlPoint)
	{
		int const percent = static_cast<int>(iterColorRampControlPoint->getPercent() * 100.0f);
		int const red = static_cast<int>(iterColorRampControlPoint->getRed() * 255.0f);
		int const green = static_cast<int>(iterColorRampControlPoint->getGreen() * 255.0f);
		int const blue = static_cast<int>(iterColorRampControlPoint->getBlue() * 255.0f);

		m_colorTable->setItem(row, 0, new TableItemInt(m_colorTable, QTableItem::WhenCurrent, QString::number(percent), 0, 100));
		m_colorTable->setItem(row, 1, new TableItemInt(m_colorTable, QTableItem::WhenCurrent, QString::number(red), 0, 255));
		m_colorTable->setItem(row, 2, new TableItemInt(m_colorTable, QTableItem::WhenCurrent, QString::number(green), 0, 255));
		m_colorTable->setItem(row, 3, new TableItemInt(m_colorTable, QTableItem::WhenCurrent, QString::number(blue), 0, 255));
		m_colorTable->setItem(row, 4, new TableItemColor(m_colorTable, QTableItem::Never, red, green, blue));
		++row;
	}
}

// ============================================================================
