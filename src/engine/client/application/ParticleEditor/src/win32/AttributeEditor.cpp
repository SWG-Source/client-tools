// ============================================================================
//
// AttributeEditor.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "AttributeEditor.h"
#include "AttributeEditor.moc"

#include "ColorRampEdit.h"
#include "WaveFormEdit.h"

// ============================================================================
//
// AttributeEditor
//
// ============================================================================

//-----------------------------------------------------------------------------
AttributeEditor::AttributeEditor(QWidget *parent, char const *name, WFlags flags)
 : QWidget(parent, name, flags)
{
	setCaption("Attribute Editor");

	setFocusPolicy(QWidget::ClickFocus);

	m_waveFormEdit = new WaveFormEdit(this, "WaveFormEdit");
	m_waveFormEdit->move(0, 0);

	m_colorRampEdit = new ColorRampEdit(this, "ColorRampEdit");
	m_colorRampEdit->move(0, 0);

	// Hookup the connections
	
	connect(m_waveFormEdit, SIGNAL(waveFormChanged(const WaveForm &, const bool)), this, SLOT(onWaveFormChanged(const WaveForm &, const bool)));
	connect(m_colorRampEdit, SIGNAL(colorRampChanged(const ColorRamp &, const WaveForm &, const bool)), this, SLOT(onColorRampChanged(const ColorRamp &, const WaveForm &, const bool)));

	// Set the minimum size to the color ramps specifications

	setMinimumSize(m_colorRampEdit->minimumSize());

	reset();
}

//-----------------------------------------------------------------------------
void AttributeEditor::slotUnDefine()
{
	reset();
}

//-----------------------------------------------------------------------------
void AttributeEditor::reset()
{
	setCaption("No Attribute Selected");

	m_waveFormEdit->hide();
	m_colorRampEdit->hide();
}

//-----------------------------------------------------------------------------
void AttributeEditor::paintEvent(QPaintEvent * paintEvent)
{
	UNREF(paintEvent);

	if (!m_waveFormEdit->isVisible() &&
	    !m_colorRampEdit->isVisible())
	{
		QPainter painter;
		QFontMetrics fm(fontMetrics());
		QPixmap pm(width(), height());

		painter.begin(&pm);

		// Paint the background color

		painter.fillRect(rect(), QBrush(paletteBackgroundColor()));

		// Mention that no attribute is selected

		painter.setPen(QColor("black"));
		QString text("No editable attribute is selected.");
		painter.drawText(width() / 2 - fm.width(text) / 2, height() / 2, text);

		// Draw the buffer to the widget

		bitBlt(this, 0, 0, &pm);
	}

	// Resize the contained widgets

	m_waveFormEdit->resize(size());
	m_colorRampEdit->resize(size());
}

//-----------------------------------------------------------------------------
void AttributeEditor::onWaveFormChanged(const WaveForm &waveForm, const bool forceRebuild)
{
	emit waveFormChanged(waveForm, forceRebuild);
}

//-----------------------------------------------------------------------------
void AttributeEditor::onColorRampChanged(const ColorRamp &colorRamp, const WaveForm &alphaWaveForm, const bool forceRebuild)
{
	emit colorRampChanged(colorRamp, alphaWaveForm, forceRebuild);
}

//-----------------------------------------------------------------------------
void AttributeEditor::setWaveForm(const WaveForm &waveForm)
{
	m_colorRampEdit->hide();
	m_waveFormEdit->setWaveForm(waveForm);
#ifdef _DEBUG
	setCaption(waveForm.getName().c_str());
#endif // _DEBUG
	m_waveFormEdit->show();
}

//-----------------------------------------------------------------------------
void AttributeEditor::setColorRamp(const ColorRamp &colorRamp, const WaveForm &alphaWaveForm)
{
	m_waveFormEdit->hide();
	m_colorRampEdit->setColorRamp(colorRamp, alphaWaveForm);
#ifdef _DEBUG
	setCaption(colorRamp.getName().c_str());
#endif // _DEBUG
	m_colorRampEdit->show();
}

//-----------------------------------------------------------------------------
void AttributeEditor::keyPressEvent(QKeyEvent *keyEvent)
{
	if ((keyEvent->key() == Qt::Key_Control) && !keyEvent->isAutoRepeat())
	{
		m_waveFormEdit->setDrawRandomHandles(true);

		keyEvent->accept();
	}
	else
	{
		keyEvent->ignore();
	}
}

//-----------------------------------------------------------------------------
void AttributeEditor::keyReleaseEvent(QKeyEvent *keyEvent)
{
	if ((keyEvent->key() == Qt::Key_Control) && !keyEvent->isAutoRepeat())
	{
		m_waveFormEdit->setDrawRandomHandles(false);

		keyEvent->accept();
	}
	else
	{
		keyEvent->ignore();
	}
}

// ============================================================================