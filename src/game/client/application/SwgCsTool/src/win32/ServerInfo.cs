using System.Collections;
using System.Collections.Specialized;
using System.Windows.Forms;


namespace SwgCsTool
{
	/// <summary>
	/// information for a server cluster.
	/// </summary>
	public class ClusterInfo
	{
		#region constructors
		public ClusterInfo()
		{
			m_aGameServers = new ArrayList();
		}
		#endregion

		#region properties
		public bool active
		{
			get
			{
				return m_bActive;
			}
			set
			{
				m_bActive = value;
			}
		}

		public string name
		{
			get
			{
				return m_sName;
			}
			set
			{
				m_sName = value;
			}
		}

		public ArrayList gameservers
		{
			get
			{
				return m_aGameServers;
			}
			// no set access
		}
		#endregion

		#region members
		protected bool m_bActive;
		protected string m_sName;
		protected ArrayList m_aGameServers;
		#endregion
	}

	/// <summary>
	/// information on a single login server.
	/// </summary>
	public class LoginServerInfo
	{
		#region constructors
		public LoginServerInfo()
		{
			m_aClusters = new ArrayList();
		}
		#endregion

		#region properties
		public string name
		{
			get
			{
				return m_sName;
			}
			set
			{
				m_sName = value;
			}
		
		}
		
		public bool active
		{
			get
			{
				return m_bActive;
			}
			set
			{
				m_bActive = value;
			}
		}

		public ArrayList clusters
		{
			get
			{
				return m_aClusters;
			}
			// no set access.		
		}
		#endregion

		#region public interface
		public void activateCluster( string cluster )
		{
			for( int i = 0; i < m_aClusters.Count; ++i )
			{
				ClusterInfo info = ( ClusterInfo ) m_aClusters[ i ];
				if( info.name == cluster )
				{
					info.active = true;
				}
			}
		}

		public void deactivateCluster( string cluster )
		{
			for( int i = 0; i < m_aClusters.Count; ++i )
			{
				ClusterInfo info = ( ClusterInfo ) m_aClusters[ i ];
				if( info.name == cluster )
				{
					info.active = false;
				}
			}
		}
		#endregion

		#region members
		protected string m_sName;
		protected bool m_bActive;
		protected ArrayList m_aClusters;
		#endregion
	}

}