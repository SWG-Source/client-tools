// ======================================================================
//
// TemplateEditorPerforceUser.cpp
// Sony Online Entertainment
//
// ======================================================================

#include "FirstTemplateEditor.h"
#include "TemplateEditorPerforceUser.h"
#include "TemplateEditorPerforce.h"

// ======================================================================

TemplateEditorPerforceUser::TemplateEditorPerforceUser()
: ClientUser(),
  MessageDispatch::Emitter(),
  m_errorOccurred(false),
  m_lastError (0),
  m_filteredErrors (),
  m_return_value ()
{
}

//-----------------------------------------------------------------

void TemplateEditorPerforceUser::HandleError( Error *err )
{
	if (err != NULL && err->Test())
	{
		err->Report ();
		m_errorOccurred = true;
		m_lastError = err->GetGeneric();

		// test for filtered errors

		for (size_t i = 0; i < m_filteredErrors.size(); ++i)
		{
			if (m_lastError == m_filteredErrors[i])
				return;
		}
	}
	ClientUser::HandleError(err);
}

//-----------------------------------------------------------------

void TemplateEditorPerforceUser::OutputInfo(char level, char *data)
{
	switch(level)
	{
	default:
	case '0': 
		break;
		
	case '1':
		m_return_value += ("... ");
		break;
		
	case '2':
		m_return_value += ("... ");
		break;
	}
	m_return_value += data;
} //lint !e818 "data could be const", no it can't - that would change the signature of a virtual function

//-----------------------------------------------------------------

bool TemplateEditorPerforceUser::runCommand (const char * command, const char * arg1)
{ 
	StringVector sv;
	sv.push_back (arg1);

	return runCommand (command, sv);
}

//-----------------------------------------------------------------

bool TemplateEditorPerforceUser::runCommand (const char * command, const StringVector & args)
{
	ClientApi client;
	Error e;
	
	// Connect to Perforce server
	client.Init( &e );
	if (e.Test())
	{
		return false;
	}
	
	// check out the template file
	char ** argv = new char * [args.size () + 1];

	size_t num = 0;
	{
		for (StringVector::const_iterator it = args.begin (); it != args.end (); ++it)
		{
			argv [num++] = DuplicateString ((*it).c_str ());
		}
	}

	//-- emit the message to listeners
	{
		std::string output = command;
		for (StringVector::const_iterator it = args.begin (); it != args.end (); ++it)
		{
			IGNORE_RETURN (output.append (1, ' '));
			output += *it;
		}
		const TemplateEditorPerforce::CommandMessage cm (output);
		emitMessage (cm);
	}

	client.SetArgv (static_cast<int>(num), argv);
	client.Run( command, this );
	IGNORE_RETURN (client.Final (&e));

	for (size_t i = 0; i < num; ++i)
	{
		delete[] argv [i];
	}

	delete[] argv;
	
	if (errorOccurred())
		return false;
	
	return true;
}

//-----------------------------------------------------------------
