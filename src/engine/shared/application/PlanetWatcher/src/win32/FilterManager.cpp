// ============================================================================
//
// FilterManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "FilterManager.h"
#include "PlanetWatcherUtility.h"

#include <string>

// ============================================================================
//
// FilterManager
//
// To add a new filter, define the class below in step1, then add the filter to the list in step2.
//
// ============================================================================

namespace FilterManagerNamespace
{
	std::string const cms_conditionLessThan           ("Less Than");
	std::string const cms_conditionGreaterThan        ("Greater Than");
	std::string const cms_conditionLessThanOrEqual    ("Less Than Or Equal");
	std::string const cms_conditionGreaterThanOrEqual ("Greater Than Or Equal");
	std::string const cms_conditionIs                 ("Is");
	std::string const cms_conditionIsNot              ("Is Not");

	std::string const cms_emptyString                 = "";

	bool ms_installed = false;

	//--------------

	class Filter
	{
	public:
		//need to be overridden in derived classes
		virtual void getConditions(std::vector<std::string> & conditions) const = 0;
		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const = 0;
		virtual std::string const & getValidationFailedMessage() {return ms_validationFailedMessage;}

		virtual bool validateValue(std::string const & valueStr) const
		{
			if(ms_validValues.empty())
				return true;
			return (std::find(ms_validValues.begin(), ms_validValues.end(), valueStr) != ms_validValues.end());
		}
		std::string const & getFilterTypeName() const {return ms_filterName;}
		void setValidValues(std::vector<std::string> const & validValues) {ms_validValues = validValues;}
	protected:
		Filter(std::string const & filterName, std::string const & validationFailedMessage) : ms_filterName(filterName), ms_validationFailedMessage(validationFailedMessage)
		{}
	protected:
		std::string ms_filterName;
		std::string ms_validationFailedMessage;
		std::vector<std::string> ms_validValues;
	private:
		//disabled
		Filter();
		Filter(Filter const &);
		Filter &operator =(Filter const &);
	};

	//--------------

	class LevelFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionLessThan);
			conditions.push_back(cms_conditionGreaterThan);
			conditions.push_back(cms_conditionLessThanOrEqual);
			conditions.push_back(cms_conditionGreaterThanOrEqual);
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			int const level = object.getLevel();
			int const value = atoi(valueStr.c_str());

			if(condition == cms_conditionIs)
			{
				return level == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return level != value;
			}
			else if(condition == cms_conditionGreaterThan)
			{
				return level > value;
			}
			else if(condition == cms_conditionLessThan)
			{
				return level < value;
			}
			else if(condition == cms_conditionGreaterThanOrEqual)
			{
				return level >= value;
			}
			else if(condition == cms_conditionLessThanOrEqual)
			{
				return level <= value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			//TODO handle non-ints
			int val = atoi(valueStr.c_str());
			if(val < 0 || val > 80)
				return false;
			else
				return true;
		}

		LevelFilter() : Filter("Level", "Value must be an int [0, 80]")
		{}

	private:
		//disabled
		LevelFilter(LevelFilter const &);
		LevelFilter &operator =(LevelFilter const &);
	};

	//--------------

	class WorldXFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionLessThan);
			conditions.push_back(cms_conditionGreaterThan);
			conditions.push_back(cms_conditionLessThanOrEqual);
			conditions.push_back(cms_conditionGreaterThanOrEqual);
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			int const worldX = object.getWorldX();
			int const value = atoi(valueStr.c_str());

			if(condition == cms_conditionIs)
			{
				return worldX == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return worldX != value;
			}
			else if(condition == cms_conditionGreaterThan)
			{
				return worldX > value;
			}
			else if(condition == cms_conditionLessThan)
			{
				return worldX < value;
			}
			else if(condition == cms_conditionGreaterThanOrEqual)
			{
				return worldX >= value;
			}
			else if(condition == cms_conditionLessThanOrEqual)
			{
				return worldX <= value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			//TODO handle non-ints
			int val = atoi(valueStr.c_str());
			if(val < -8000 || val > 8000)
				return false;
			else
				return true;
		}

		WorldXFilter() : Filter("WorldX", "Value must be an int [-8000, 8000]")
		{}

	private:
		//disabled
		WorldXFilter(WorldXFilter const &);
		WorldXFilter &operator =(WorldXFilter const &);
	};

	//--------------

	class WorldZFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionLessThan);
			conditions.push_back(cms_conditionGreaterThan);
			conditions.push_back(cms_conditionLessThanOrEqual);
			conditions.push_back(cms_conditionGreaterThanOrEqual);
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			int const worldZ = object.getWorldZ();
			int const value = atoi(valueStr.c_str());

			if(condition == cms_conditionIs)
			{
				return worldZ == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return worldZ != value;
			}
			else if(condition == cms_conditionGreaterThan)
			{
				return worldZ > value;
			}
			else if(condition == cms_conditionLessThan)
			{
				return worldZ < value;
			}
			else if(condition == cms_conditionGreaterThanOrEqual)
			{
				return worldZ >= value;
			}
			else if(condition == cms_conditionLessThanOrEqual)
			{
				return worldZ <= value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			//TODO handle non-ints
			int val = atoi(valueStr.c_str());
			if(val < -8000 || val > 8000)
				return false;
			else
				return true;
		}

		WorldZFilter() : Filter("WorldZ", "Value must be an int [-8000, 8000]")
		{}

	private:
		//disabled
		WorldZFilter(WorldZFilter const &);
		WorldZFilter &operator =(WorldZFilter const &);
	};

	//--------------

	class ServerIdFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			uint32 const serverId = object.getGameServerId();
			uint32 const value = static_cast<uint32>(atoi(valueStr.c_str()));

			if(condition == cms_conditionIs)
			{
				return serverId == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return serverId != value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		ServerIdFilter() : Filter("ServerId", "Please select one of the existing serverIds.")
		{}

		virtual bool validateValue(std::string const & valueStr) const
		{
			return Filter::validateValue(valueStr);
		}

	private:
		//disabled
		ServerIdFilter(ServerIdFilter const &);
		ServerIdFilter &operator =(ServerIdFilter const &);
	};

	//--------------

	class HibernationFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			bool const hibernating = object.getHibernating();
			bool const value = (valueStr == "true");

			if(condition == cms_conditionIs)
			{
				return hibernating == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return hibernating != value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			if(valueStr == "true" || valueStr == "false")
				return true;
			else
				return false;
		}

		HibernationFilter() : Filter("Hibernating", "Must be \"true\" or \"false\".")
		{}

	private:
		//disabled
		HibernationFilter(HibernationFilter const &);
		HibernationFilter &operator =(HibernationFilter const &);
	};

	//--------------

	class AiActivityFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			std::string const & aiActivity = object.getAiActivityString();
			
			if(condition == cms_conditionIs)
			{
				return aiActivity == valueStr;
			}
			else if(condition == cms_conditionIsNot)
			{
				return aiActivity != valueStr;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			return Filter::validateValue(valueStr);
		}

		virtual std::string const & getValidationFailedMessage()
		{
			ms_validationFailedMessage = "Must be a valid AI Activity type.  Valid values are:\n";
			for(std::vector<std::string>::const_iterator i = ms_validValues.begin(); i != ms_validValues.end(); ++i)
			{
				ms_validationFailedMessage += *i + "\n";
			}
			return ms_validationFailedMessage;
		}

		AiActivityFilter() : Filter("AI Activity", "")
		{}

	private:
		//disabled
		AiActivityFilter(AiActivityFilter const &);
		AiActivityFilter &operator =(AiActivityFilter const &);
	};

	//--------------

	class CrcFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			std::string const & crc = object.getTemplateCrcHexString();
			
			if(condition == cms_conditionIs)
			{
				return crc == valueStr;
			}
			else if(condition == cms_conditionIsNot)
			{
				return crc != valueStr;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const &) const
		{
			return true;
		}

		virtual std::string const & getValidationFailedMessage()
		{
			ms_validationFailedMessage.clear();
			return ms_validationFailedMessage;
		}

		CrcFilter() : Filter("Crc", "")
		{}

	private:
		//disabled
		CrcFilter(CrcFilter const &);
		CrcFilter &operator =(CrcFilter const &);
	};

	//--------------

	class TagFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			std::string const & tag = PlanetWatcherUtility::getObjectTypeName(object.getObjectTypeTag());
			
			if(condition == cms_conditionIs)
			{
				return tag == valueStr;
			}
			else if(condition == cms_conditionIsNot)
			{
				return tag != valueStr;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			return Filter::validateValue(valueStr);
		}

		virtual std::string const & getValidationFailedMessage()
		{
			ms_validationFailedMessage = "Must be a valid Tag type.  Valid values are:\n";
			for(std::vector<std::string>::const_iterator i = ms_validValues.begin(); i != ms_validValues.end(); ++i)
			{
				ms_validationFailedMessage += *i + "\n";
			}
			return ms_validationFailedMessage;
		}

		TagFilter() : Filter("Tag", "")
		{}

	private:
		//disabled
		TagFilter(TagFilter const &);
		TagFilter &operator =(TagFilter const &);
	};

	//--------------

	class StaticFilter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			conditions.push_back(cms_conditionIs);
			conditions.push_back(cms_conditionIsNot);
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			bool const isStatic = (object.getCreationType() == 1);
			bool const value = (valueStr == "true");

			if(condition == cms_conditionIs)
			{
				return isStatic == value;
			}
			else if(condition == cms_conditionIsNot)
			{
				return isStatic != value;
			}
			else
			{
				DEBUG_FATAL(true, ("unknown condition"));
			}
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			if(valueStr == "true" || valueStr == "false")
				return true;
			else
				return false;
		}

		StaticFilter() : Filter("Static", "Must be \"true\" or \"false\".")
		{}

	private:
		//disabled
		StaticFilter(StaticFilter const &);
		StaticFilter &operator =(StaticFilter const &);
	};

//step1 add a new filter class here from the template given.
/*
	//--------------

	class <Something>Filter : public Filter
	{
	public:
		virtual void getConditions(std::vector<std::string> & conditions) const
		{
			conditions.clear();
			//TODO push_bask the valid condition types here
		}

		virtual bool passes(std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object) const
		{
			UNREF(condition);
			UNREF(value);
			UNREF(object);
			//TODO test the condition here against the object
			return true;
		}

		virtual bool validateValue(std::string const & valueStr) const
		{
			UNREF(valueStr);
			//TODO fill this out
			return Filter::validateValue(valueStr);
		}

		<Something>Filter() : Filter("<Something>", "<Something>")
		{}

	private:
		//disabled
		<Something>Filter(<Something>Filter const &);
		<Something>Filter &operator =(<Something>Filter const &);
	};
*/
//end step1

	//--------------

	std::map<std::string, Filter *> ms_filters;
};

using namespace FilterManagerNamespace;

// ============================================================================

void FilterManager::install()
{
	ms_installed = true;

	ms_filters.clear();
	LevelFilter * const levelFilter = new LevelFilter();
	ms_filters[levelFilter->getFilterTypeName()] = levelFilter;

	WorldXFilter * const worldXFilter = new WorldXFilter();
	ms_filters[worldXFilter->getFilterTypeName()] = worldXFilter;

	WorldZFilter * const worldZFilter = new WorldZFilter();
	ms_filters[worldZFilter->getFilterTypeName()] = worldZFilter;

	ServerIdFilter * const serverIdFilter = new ServerIdFilter();
	ms_filters[serverIdFilter->getFilterTypeName()] = serverIdFilter;

	HibernationFilter * const hibernationFilter = new HibernationFilter();
	ms_filters[hibernationFilter->getFilterTypeName()] = hibernationFilter;

	AiActivityFilter * const aiActivityFilter = new AiActivityFilter();
	ms_filters[aiActivityFilter->getFilterTypeName()] = aiActivityFilter;

	CrcFilter * const crcFilter = new CrcFilter();
	ms_filters[crcFilter->getFilterTypeName()] = crcFilter;

	TagFilter * const tagFilter = new TagFilter();
	ms_filters[tagFilter->getFilterTypeName()] = tagFilter;

	StaticFilter * const staticFilter = new StaticFilter();
	ms_filters[staticFilter->getFilterTypeName()] = staticFilter;

//step2: add filter here
/*
	<Something>Filter * const <something>Filter = new <Something>Filter();
	ms_filters[<something>Filter->getFilterTypeName()] = <something>Filter;
*/
//end step2
}

//-----------------------------------------------------------------------------

void FilterManager::remove()
{
	for(std::map<std::string, Filter *>::iterator i = ms_filters.begin(); i != ms_filters.end(); ++i)
	{
		delete (i->second);
	}
	ms_filters.clear();

	ms_installed = false;
}

//-----------------------------------------------------------------------------

void FilterManager::getFilterTypes(std::vector<std::string> & filterTypes)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	filterTypes.clear();
	for(std::map<std::string, Filter *>::const_iterator i = ms_filters.begin(); i != ms_filters.end(); ++i)
	{
		filterTypes.push_back(i->first);
	}
}

//-----------------------------------------------------------------------------

void FilterManager::getConditions(std::vector<std::string> & conditions, std::string const & filterType)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	conditions.clear();
	std::map<std::string, Filter *>::const_iterator const i = ms_filters.find(filterType);
	if(i != ms_filters.end())
	{
		Filter const * const filter = i->second;
		if(filter)
			filter->getConditions(conditions);
	}
	else
	{
		DEBUG_FATAL(true, ("Filter not found"));
	}
}

//-----------------------------------------------------------------------------

bool FilterManager::passesFilter(std::string const & filterType, std::string const & condition, std::string const & valueStr, PlanetWatcherRenderer::MiniMapObject const & object)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	std::map<std::string, Filter *>::const_iterator const i = ms_filters.find(filterType);
	if(i != ms_filters.end())
	{
		Filter const * const filter = i->second;
		if(filter)
			return filter->passes(condition, valueStr, object);
	}
	else
	{
		DEBUG_FATAL(true, ("Filter not found"));
	}
	return true;
}

//-----------------------------------------------------------------------------

bool FilterManager::validateValue(std::string const & filterType, std::string const & value)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	std::map<std::string, Filter *>::const_iterator const i = ms_filters.find(filterType);
	if(i != ms_filters.end())
	{
		Filter const * const filter = i->second;
		if(filter)
			return filter->validateValue(value);
	}
	else
	{
		DEBUG_FATAL(true, ("Filter not found"));
	}
	return true;
}

//-----------------------------------------------------------------------------

std::string const & FilterManager::getValidationFailedMessage(std::string const & filterType)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	std::map<std::string, Filter *>::const_iterator const i = ms_filters.find(filterType);
	if(i != ms_filters.end())
	{
		Filter * const filter = i->second;
		if(filter)
			return filter->getValidationFailedMessage();
	}
	else
	{
		DEBUG_FATAL(true, ("Filter not found"));
	}
	return cms_emptyString;
}

//-----------------------------------------------------------------------------

void FilterManager::setValidValues(std::string const & filterType, stdvector<std::string>::fwd const & validValues)
{
	DEBUG_FATAL(!ms_installed, ("Not installed"));

	std::map<std::string, Filter *>::const_iterator const i = ms_filters.find(filterType);
	if(i != ms_filters.end())
	{
		Filter * const filter = i->second;
		if(filter)
			filter->setValidValues(validValues);
	}
	else
	{
		DEBUG_FATAL(true, ("Filter not found"));
	}
}

// ============================================================================
