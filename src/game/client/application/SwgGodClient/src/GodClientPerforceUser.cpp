// ======================================================================
//
// GodClientPerforce.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "GodClientPerforceUser.h"
#include "GodClientPerforce.h"

#include "errornum.h"

#include <qinputdialog.h>

//==============================================================================
// subclass of the PerforceAPI StrBuf class, to workaround a bug
//  in the destructor. We can't fix the bug because it's an external library
class StrBufFixed : public StrBuf
{
public:
	~StrBufFixed()
	{
		delete buffer;
		StringInit();
	}
};

// ======================================================================

GodClientPerforceUser::GodClientPerforceUser()
: ClientUser(),
  MessageDispatch::Emitter(),
  m_errorOccurred(false),
  m_lastError (0),
  m_filteredErrors (),
  m_return_value ()
{
}

//-----------------------------------------------------------------

void GodClientPerforceUser::HandleError( Error *err )
{
	if (err != NULL && err->Test())
	{
		if (err->GetGeneric() != EV_EMPTY)
		{
			StrBufFixed buf;

			m_errorOccurred = true;
			m_lastError = err->GetGeneric();

			err->Fmt( &buf );
			m_lastErrorText = buf.Text();

			WARNING( true, (m_lastErrorText.c_str()) );

			// test for filtered errors
			for (size_t i = 0; i < m_filteredErrors.size(); ++i)
			{
				if (m_lastError == m_filteredErrors[i])
					return;
			}
		}
	}

	ClientUser::HandleError(err);
}

//-----------------------------------------------------------------

void GodClientPerforceUser::OutputInfo(char level, char *data)
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

void GodClientPerforceUser::Prompt(const StrPtr& msg, StrBuf& rsp, int noEcho, Error* e)
{
	UNREF(e);

	bool ok;

	QString text = QInputDialog::getText(
		"Perforce Says", 
		msg.Text(), 
		noEcho ? QLineEdit::Password : QLineEdit::Normal, 
		QString::null,
		&ok);

	if (ok)
	{
		rsp = text;
	}
}

//-----------------------------------------------------------------

bool GodClientPerforceUser::runCommand (const char * command, const char * arg1)
{ 
	StringVector sv;

	if (arg1 != 0)
		sv.push_back (arg1);

	return runCommand (command, sv);
}

//-----------------------------------------------------------------

bool GodClientPerforceUser::runCommand (const char * command, const StringVector & args)
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
		const GodClientPerforce::CommandMessage cm (output);
		emitMessage (cm);
	}

	client.SetProg( "SWGGodClient" );
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
