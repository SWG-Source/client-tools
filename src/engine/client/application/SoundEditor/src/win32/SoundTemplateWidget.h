// ============================================================================
//
// SoundTemplateWidget.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDTEMPLATEWIDGET_H
#define SOUNDTEMPLATEWIDGET_H

#include "clientAudio/Audio.h"
#include "BaseSoundTemplateWidget.h"
#include "SoundEditorUtility.h"
#include "clientAudio/Sound2dTemplate.h"

class Iff;
class QPopupMenu;
class SoundDebugInformationWidget;
class SoundTemplate;
class Sound3dTemplate;

//-----------------------------------------------------------------------------
class SoundTemplateWidgetListViewItem : public QListViewItem
{
public:

	SoundTemplateWidgetListViewItem(QListView *parent);

	int                getSampleBits() const;
	int                getSampleChannels() const;
	std::string const &getSamplePath() const;
	std::string        getSampleTreeFilePath() const;
	std::string const &getSampleFileType() const;
	float              getSampleTime() const;
	int                getSampleRate() const;
	int                getSampleSize() const;
	bool               isSampleCompressed() const;
	bool               isSampleStereo() const;
	void               setSamplePath(std::string const &path);

private:

	float       m_sampleTime;       // seconds
	int         m_sampleBits;
	int         m_sampleChannels;
	bool        m_sampleCompressed;
	std::string m_samplePath;
	std::string m_sampleFileType;
	int         m_sampleRate;
	int         m_sampleSize;
};

//-----------------------------------------------------------------------------
class SoundTemplateWidget : public BaseSoundTemplateWidget
{
	Q_OBJECT

public:

	SoundTemplateWidget(QWidget *parent, char const *name);
	virtual ~SoundTemplateWidget();

	void load(Iff &iff);
	void write(Iff &iff) const;

public slots:

	void onDistanceMinLineEditReturnPressed();
	void onSpatialityMethodComboBoxActivated(int index);
	void onFadeInMinLineEditReturnPressed();
	void onFadeInMaxLineEditReturnPressed();
	void onFadeInSampleRateComboBoxActivated(int index);
	void onFadeOutMinLineEditReturnPressed();
	void onFadeOutMaxLineEditReturnPressed();
	void onFadeOutSampleRateComboBoxActivated(int index);
	void onFileResetActionActivated();
	void onFileSaveActionActivated();
	void onFileSaveAsActionActivated();
	void onInfiniteLoopingCheckBoxClicked();
	void onLoopCountMinLineEditReturnPressed();
	void onLoopCountMaxLineEditReturnPressed();
	void onLoopDelayMinLineEditReturnPressed();
	void onLoopDelayMaxLineEditReturnPressed();
	void onLoopDelaySampleRateComboBoxActivated(int index);
	void onMouseButtonClicked(int button, QListViewItem *listViewItem, const QPoint &point, int column);
	void onPitchMinLineEditReturnPressed();
	void onPitchMaxLineEditReturnPressed();
	void onPitchInterpolationRateLineEditReturnPressed();
	void onPitchSampleRateComboBoxActivated(int index);
	void onPlayPushButtonClicked();
	void onPollSound();
	void onRestartPushButtonClicked();
	void onSoundsToPlayPopupMenuActivated(int index);
	void onStartDelayMinLineEditReturnPressed();
	void onStartDelayMaxLineEditReturnPressed();
	void onVolumeMinLineEditReturnPressed();
	void onVolumeMaxLineEditReturnPressed();
	void onVolumeInterpolationRateLineEditReturnPressed();
	void onVolumeSampleRateComboBoxActivated(int index);

signals:

protected:

	virtual void closeEvent(QCloseEvent *closeEvent);

private:

	enum SampleListPopUpMenu
	{
		SLPM_addSample,
		SLPM_removeSample
	};

	enum ListViewColumns
	{
		LVC_number,
		LVC_sample,
		LVC_size,
		LVC_time,
		LVC_type,
		LVC_bits,
		LVC_rate,
		LVC_channels
	};

	enum AttenuationMethod
	{
		AM_none,
		AM_2d,
		AM_3d
	};

	static SoundBoundFloat m_startDelayBound;
	static SoundBoundFloat m_fadeInBound;
	static SoundBoundInt   m_loopCountBound;
	static SoundBoundFloat m_loopDelayBound;
	static SoundBoundFloat m_fadeOutBound;
	static SoundBoundFloat m_volumeSampleRateBound;
	static SoundBoundInt   m_volumeBound;
	static SoundBoundFloat m_volumeInterpolationRateBound;
	static SoundBoundFloat m_pitchSampleRateBound;
	static SoundBoundFloat m_pitchBound;
	static SoundBoundFloat m_pitchInterpolationRateBound;
	static SoundBoundFloat m_distanceAtMaxVolumeBound;
	static int const       m_floatingPointPrecision;

	QListViewItem * m_selectedListViewItem;
	SoundId         m_soundId;
	QPopupMenu *    m_sampleListPlayPopupMenu;
	std::string     m_soundTemplateFileName;
	QWorkspace *    m_workSpaceParent;
	Sound2dTemplate m_currentSound2dTemplate;
	SoundTemplate const *m_cachedSoundTemplate;

	void addSample();
	void addSample(std::string const &path);
	bool isSound3d() const;
	void playSoundTemplate();
	void removeSample();
	void reNumberSamples();
	void resetSoundTemplate();
	void validateAll();
	bool verifySampleListAll3d();
	void write(Sound2dTemplate &sound2dTemplate) const;
	void write(Sound3dTemplate &sound3dTemplate) const;
	bool save();
	bool saveAs();

	virtual void setCaption(QString const &caption);

	// Disabled

	SoundTemplateWidget();
	SoundTemplateWidget(SoundTemplateWidget const &);
	SoundTemplateWidget &operator =(SoundTemplateWidget const &);
};

// ============================================================================

#endif // SOUNDTEMPLATEWIDGET_H
