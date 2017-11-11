// ============================================================================
//
// FirstSoundEditor.h
// copyright Sony Online Entertainment
//
// ============================================================================

#pragma warning (disable:4275) // non dll-interface class 'QPtrVector<class QConnectionList>' used as base for dll-interface class 'QSignalVec'
#pragma warning (disable:4800) // 'unsigned char' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning (disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning (disable:4510) // default constructor could not be generated
#pragma warning (disable:4610) // user defined constructor required

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/FirstSharedFoundation.h"
//#include "sharedMessageDispatch/Emitter.h"
//#include "sharedMessageDispatch/Message.h"
//#include "sharedMessageDispatch/Receiver.h"
//#include "StringId.h"
//#include "sharedFile/Iff.h"

// commonly used standard includes

#include <cassert>
#include <ctime>
#include <dinput.h>
#include <limits>
#include <stdlib.h>

// commonly used STL

#include <algorithm>
#include <list>
#include <vector>
#include <string>

// commonly used Qt

#include <qaction.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qdial.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qfont.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h.>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qsettings.h>
#include <qslider.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwidget.h>
#include <qworkspace.h>
