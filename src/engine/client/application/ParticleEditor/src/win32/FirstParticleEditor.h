// ============================================================================
//
// FirstParticleEditor.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#pragma warning (disable:4275) // non dll-interface class 'QPtrVector<class QConnectionList>' used as base for dll-interface class 'QSignalVec'
#pragma warning (disable:4800) // 'unsigned char' : forcing value to bool 'true' or 'false' (performance warning)

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/FirstSharedFoundation.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "StringId.h"
#include "sharedFile/Iff.h"
#include "ParticleEditorUtility.h"

//// commonly used standard includes
//
//#include <cassert>
//#include <ctime>
//#include <limits>
//#include <cstdlib>
//
//// commonly used STL
//
//#include <algorithm>
//#include <list>
//#include <vector>

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
#include <qfontmetrics.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h.>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qsettings.h>
#include <qslider.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwidget.h>
#include <qworkspace.h>
