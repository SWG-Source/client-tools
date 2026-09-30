#include "ClientMiles.h"
#include "private/failure_boundary.h"
#include "private/native_file_setter67.h"
#include <exception>
namespace ClientMiles {
void set_file_callbacks(FileOpenCallback open,FileCloseCallback close,
                        FileSeekCallback seek,FileReadCallback read) {
    try {
        ClientMilesPrivate52::requireFatalReporter();
        ClientMilesNativeFile67::set_file_callbacks(open,close,seek,read);
    } catch(const std::exception &error) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
    } catch(...) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,
                                  "non-standard exception in native file setter");
    }
}
}
