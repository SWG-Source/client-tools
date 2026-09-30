#ifndef CLIENT_MILES_NATIVE_FILE_SETTER67_H
#define CLIENT_MILES_NATIVE_FILE_SETTER67_H
#include "../ClientMiles.h"
namespace ClientMilesNativeFile67 {
void set_file_callbacks(ClientMiles::FileOpenCallback,ClientMiles::FileCloseCallback,
                        ClientMiles::FileSeekCallback,ClientMiles::FileReadCallback);
}
#endif
