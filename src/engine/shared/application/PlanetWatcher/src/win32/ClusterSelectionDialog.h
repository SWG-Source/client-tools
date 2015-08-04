// ============================================================================
//
// ClusterSelectionDialog.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ClusterSelectionDialog_H
#define INCLUDED_ClusterSelectionDialog_H

#include "BaseClusterSelectionDialog.h"

#include <map>

class LoginConnection;
class LoginEnumCluster;
class LoginClusterStatus;
class QListViewItem;

struct LoginClusterStatus_ClusterData;

//-----------------------------------------------------------------------------
class ClusterSelectionDialog : public BaseClusterSelectionDialog
{
	Q_OBJECT

public:

	ClusterSelectionDialog(QWidget *parent, char const *name);
	~ClusterSelectionDialog();

	void onLoginMessage( const LoginEnumCluster &lec );
	void onLoginMessage( const LoginClusterStatus &lcs );
	void setStatusText( const char *text );
	
	int  getSelectedCluster() const;
	const LoginClusterStatus_ClusterData &getClusterData( int clusterId ) const;
	void loginServerConnectionStatusUpdate( bool connected );
	
	
protected:

	virtual void keyPressEvent(QKeyEvent *keyEvent);

private:

	LoginConnection *                m_loginConnection;

private slots:

//	void slotAcceptPushButtonClicked();
	void slotConnectButtonClicked();
	void slotConnectLoginServer();
	void slotCancelButtonClicked();
};

// ============================================================================

#endif // INCLUDED_ClusterSelectionDialog_H
