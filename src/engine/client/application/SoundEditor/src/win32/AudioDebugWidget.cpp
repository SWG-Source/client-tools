// ============================================================================
//
// AudioDebugWidget.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "AudioDebugWidget.h"
#include "AudioDebugWidget.moc"

#include "clientAudio/Audio.h"

// ============================================================================
//
// AudioDebugWidget
//
// ============================================================================

//-----------------------------------------------------------------------------
AudioDebugWidget::AudioDebugWidget(QWidget *parent, char const *name, WFlags flags)
 : BaseAudioDebugWidget(parent, name, flags)
{
	// Disable sorting

	m_debugListView->setSorting(-1);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(timerTimout()));
	timer->start(1000 / 4);
}

//-----------------------------------------------------------------------------
void AudioDebugWidget::timerTimout()
{
	int rowCount = 0;

	QString text;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "3D Provider"));
	}
	text.sprintf("%s", Audio::getCurrent3dProvider());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Room Type"));
	}
	text.sprintf("%s", Audio::getRoomTypeString());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Cached Sample Count"));
	}
	text.sprintf("%d", Audio::getCachedSampleCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Sound Count"));
	}
	text.sprintf("%d", Audio::getSoundCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Sample 2d Count"));
	}
	text.sprintf("%d", Audio::getSample2dCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Sample 3d Count"));
	}
	text.sprintf("%d", Audio::getSample3dCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Sample Stream Count"));
	}
	text.sprintf("%d", Audio::getSampleStreamCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Digital CPU Percent"));
	}
	text.sprintf("%d", Audio::getDigitalCpuPercent());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Digital Latency"));
	}
	text.sprintf("%d", Audio::getDigitalLatency());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Cache Hit Count"));
	}
	text.sprintf("%d", Audio::getCacheHitCount());
	setText(rowCount, 1, text);
	++rowCount;

	if (m_debugListView->childCount() <= rowCount)
	{
		m_debugListView->insertItem(new QListViewItem(m_debugListView, "Cache Miss Count"));
	}
	text.sprintf("%d", Audio::getCacheMissCount());
	setText(rowCount, 1, text);
	++rowCount;
}

//-----------------------------------------------------------------------------
void AudioDebugWidget::setText(int const row, int const column, QString const &text)
{
	QListViewItem *child = m_debugListView->firstChild();

	int currentRow = 0;

	while (child != NULL)
	{
		if (currentRow == ((m_debugListView->childCount() - 1) - row))
		{
			child->setText(column, text);
			break;
		}

		child = child->nextSibling();
		++currentRow;
	}
}

// ============================================================================
