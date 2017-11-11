// ============================================================================
//
// AttributeEditor.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef ATTRIBUTEEDITOR_H
#define ATTRIBUTEEDITOR_H

class ColorRamp;
class ColorRampEdit;
class WaveForm;
class WaveFormEdit;

//-----------------------------------------------------------------------------
class AttributeEditor : public QWidget
{
	Q_OBJECT

public:

	AttributeEditor(QWidget *parent, char const *name, WFlags flags = 0);

	void reset();

signals:

	void waveFormChanged(const WaveForm &, const bool);
	void colorRampChanged(const ColorRamp &, const WaveForm &, const bool);

public slots:

	void onWaveFormChanged(const WaveForm &, const bool forceRebuild);
	void onColorRampChanged(const ColorRamp &, const WaveForm &, const bool forceRebuild);
	void setWaveForm(const WaveForm &);
	void setColorRamp(const ColorRamp &, const WaveForm &);
	void slotUnDefine();

protected:

	virtual void keyPressEvent(QKeyEvent *keyEvent);
	virtual void keyReleaseEvent(QKeyEvent *keyEvent);

private:

	virtual void paintEvent(QPaintEvent * paintEvent);

	WaveFormEdit * m_waveFormEdit;
	ColorRampEdit *m_colorRampEdit;
};

// ============================================================================

#endif // ATTRIBUTEEDITOR_H