// ======================================================================
//
// FilterManager.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FilterManager_H
#define INCLUDED_FilterManager_H

// ======================================================================

class Object;

// ======================================================================

/**
 * This class is used to manage various object "filters" that other parts of the app can run against.
 * It maintains user-set criteria (i.e. "filter objects further an 100 meters from my avatar), and runs
 * given objects against it.
 */
class FilterManager : public MessageDispatch::Emitter, public MessageDispatch::Receiver
{
public:
	/**
	 * Virtual base class for all filter types.  It only stores activation state.
	 */
	class Filter
	{
		public:
			Filter();
			virtual ~Filter() {}
			void setOn(bool b);
			virtual bool validate(const Object& obj) = 0;
		protected:
			bool   m_isOn;
	};

	/**
	 * This class represents a filter of all object within a certain distance from the avatar.  It can
	 * can represent both minimum and maximum distances (i.e. within 50m of my avatar but at least 10m from my avatar)
	 */
	class RadiusFilter : public Filter
	{
		public:
			RadiusFilter();
			void setMinThreshold(real minThreshold);
			void setMaxThreshold(real maxThreshold);
			void clearMinThreshold();
			void clearMaxThreshold();
			virtual bool validate(const Object& obj);
		private:
			real m_minThreshold;
			real m_maxThreshold;
			bool m_minThresholdUsed;
			bool m_maxThresholdUsed;
	};

	/**
	 * This class represents a filter of objects by their NetworkId.  It supports a range of objects, with checks at
	 * either end (i.e. objectId's between 50 and 100)
	 */
	class NetworkIdFilter : public Filter
	{
		public:
			NetworkIdFilter();
			void setMinNetworkId(unsigned int minNetworkId);
			void setMaxNetworkId(unsigned int maxNetworkId);
			void clearMinNetworkId();
			void clearMaxNetworkId();
			virtual bool validate(const Object& obj);
		private:
			unsigned int  m_minNetworkId;
			unsigned int  m_maxNetworkId;
			bool          m_minNetworkIdUsed;
			bool          m_maxNetworkIdUsed;
	};


public:
	static void install();
	static void remove();
	static RadiusFilter&     getRadiusFilter();
	static NetworkIdFilter&  getNetworkIdFilter();
	///The heart of the class given an object, test it against all filters and return whether it passed all active filters
	static bool             validate(const Object& obj);

private:
	static bool             ms_installed;
	static NetworkIdFilter  ms_networkIdFilter;
	static RadiusFilter     ms_radiusFilter;
};

// ======================================================================

inline FilterManager::Filter::Filter()
: m_isOn(false)
{}

// ======================================================================

inline void FilterManager::Filter::setOn(bool b)
{
	m_isOn = b;
}

// ======================================================================

inline FilterManager::RadiusFilter::RadiusFilter()
: Filter(),
	m_minThreshold(0.0f),
  m_maxThreshold(0.0f),
  m_minThresholdUsed(false),
  m_maxThresholdUsed(false)
{}

// ======================================================================

inline void FilterManager::RadiusFilter::setMinThreshold(real minThreshold)
{
	m_minThreshold = minThreshold;
	m_minThresholdUsed = true;
}

// ======================================================================

inline void FilterManager::RadiusFilter::setMaxThreshold(real maxThreshold)
{
	m_maxThreshold = maxThreshold;
	m_maxThresholdUsed = true;
}

// ======================================================================

/**
 * Clear out the value from the filter (since an unassigned value is one we don't want to filter against)
 */
inline void FilterManager::RadiusFilter::clearMinThreshold()
{
	m_minThreshold     = 0.0f;
	m_minThresholdUsed = false;
}

// ======================================================================

/**
 * Clear out the value from the filter (since an unassigned value is one we don't want to filter against)
 */
inline void FilterManager::RadiusFilter::clearMaxThreshold()
{
	m_maxThreshold     = 0.0f;
	m_maxThresholdUsed = false;
}

// ======================================================================

inline FilterManager::NetworkIdFilter::NetworkIdFilter()
: Filter(),
  m_minNetworkId(0),
  m_maxNetworkId(0),
  m_minNetworkIdUsed(false),
  m_maxNetworkIdUsed(false)
{
}

// ======================================================================

inline void FilterManager::NetworkIdFilter::setMinNetworkId(unsigned int minNetworkId)
{
	m_minNetworkId = minNetworkId;
	m_minNetworkIdUsed = true;
}

// ======================================================================

inline void FilterManager::NetworkIdFilter::setMaxNetworkId(unsigned int maxNetworkId)
{
	m_maxNetworkId = maxNetworkId;
	m_maxNetworkIdUsed = true;
}

// ======================================================================

/**
 * Clear out the value from the filter (since an unassigned value is one we don't want to filter against)
 */
inline void FilterManager::NetworkIdFilter::clearMinNetworkId()
{
	m_minNetworkId     = 0;
	m_minNetworkIdUsed = false;
}

// ======================================================================

/**
 * Clear out the value from the filter (since an unassigned value is one we don't want to filter against)
 */
inline void FilterManager::NetworkIdFilter::clearMaxNetworkId()
{
	m_maxNetworkId     = 0;
	m_maxNetworkIdUsed = false;
}

// ======================================================================

/**
 * Fuction required to meet the SOE "static manager class" idiom, doesn't happen to do anything
 */
inline void FilterManager::install()
{
	DEBUG_FATAL(ms_installed, ("FilterManager already installed"));
	ms_installed = true;
}

// ======================================================================

/**
 * Fuction required to meet the SOE "static manager class" idiom, doesn't happen to do anything
 */
inline void FilterManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("FilterManager not installed"));
	ms_installed = false;
}

// ======================================================================

inline FilterManager::RadiusFilter& FilterManager::getRadiusFilter()
{
	DEBUG_FATAL(!ms_installed, ("FilterManager not installed"));
	return ms_radiusFilter;
}

// ======================================================================

inline FilterManager::NetworkIdFilter& FilterManager::getNetworkIdFilter()
{
	DEBUG_FATAL(!ms_installed, ("FilterManager not installed"));
	return ms_networkIdFilter;
}

// ======================================================================

#endif



