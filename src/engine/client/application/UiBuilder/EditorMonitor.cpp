#include "FirstUiBuilder.h"
#include "EditorMonitor.h"

// lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
void EditorMonitor::lock() {}

// unlock the dialog to allow redraws. lock and unlock are reference-count based to each call to lock
// must have a corresponding call to unlock.
void EditorMonitor::unlock(){}
void EditorMonitor::onEditReset(){}
void EditorMonitor::onEditInsertSubtree(UIBaseObject &, UIBaseObject *){}
void EditorMonitor::onEditRemoveSubtree(UIBaseObject &){}
void EditorMonitor::onEditMoveSubtree(UIBaseObject &, UIBaseObject *, UIBaseObject *){}
void EditorMonitor::onEditSetObjectProperty(UIBaseObject &, const char *){}
void EditorMonitor::onSelect(UIBaseObject &, bool){}


