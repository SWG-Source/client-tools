using System;
using System.Collections;
using System.Collections.Specialized;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Main class to handle communication with the SWG server.  Also contains the main execution thread.
	/// </summary>
	public class CsToolClass
	{
		#region singleton methods
		public static CsToolClass getInstance()
		{
			return sm_instance;
		}

		public static void install()
		{
			sm_instance = new CsToolClass();
		}

		protected CsToolClass()
		{
            m_buffer = "";
			m_bIsConnected = false;
		}
		#endregion

		#region public interface
		public void disconnectLS()
		{
			m_client.Close();
			m_bIsConnected = false;
		}

		public bool installClient( string address, int port )
		{
			try
			{
				m_client = new TcpClient( address, port );
				m_bIsConnected = true;
				return true;
			}
			catch( SocketException e )
			{
				// something bad happened.
				MessageBox.Show( e.Message );
				return false;
			}
		}

		public bool isConnected()
		{
			return m_bIsConnected;
		}

		public void sendMessage( string message )
		{
            // clear extra cr/lfs at the end, to prevent extra server passing
            message = message.Replace("\r", "");
            message = message.Replace("\n", "");

			message = message + "\r\n";
			if( isConnected() )
			{
				NetworkStream stream = client.GetStream();
				Byte[] data = System.Text.Encoding.ASCII.GetBytes( message );
				if( stream.CanWrite )
				{
					try
					{
						stream.Write( data, 0, data.Length );

					}
					catch( Exception e )
					{
						string ignoreError = e.Message;
						// mark the connection as dead.
						disconnectLS();
					}
				}
				else
				{
					// warn here.
				}
			}

		}


		public string getMessage()
		{
			if( isConnected() )
			{
				Byte[] data = new Byte[ 256 ];

				string message = string.Empty;

				NetworkStream stream = client.GetStream();

				if( stream.CanRead )
				{
					while( stream.DataAvailable )
					{
						
						int num = stream.Read( data, 0, data.Length );
						message = message + System.Text.Encoding.ASCII.GetString( data, 0, num );
					}
				}

				// we seem to be getting extra nulls in our input stream, so get rid of them.

				message = message.Replace( "\0", "" );
				if( message.Length > 0 )
				{
                    m_buffer = m_buffer + message;
                }
                if( m_buffer.Length > 0 )
                {
					int pos;
					
					const string escapeSequence = "\r\n*\r\n";

					// see if we can find our escape sequence.
					if( ( pos = m_buffer.IndexOf( escapeSequence ) ) != -1 )
					{
						message = m_buffer.Substring( 0, pos );
						if( m_buffer.Length > pos + escapeSequence.Length )
						{
							m_buffer = m_buffer.Substring( pos + escapeSequence.Length );
						}
						else
						{
							m_buffer = "";
						}
						return message;
					}
				}
			}
			return "";
		}
		#endregion

		#region properties
		public TcpClient client
		{
			get
			{
				return m_client;
			}
			// no set access.  Use installClient()
		}
		#endregion

		#region members
		protected string m_buffer;
		protected bool m_bIsConnected;
		protected TcpClient m_client;
		protected static CsToolClass sm_instance;
		#endregion

		#region static thread
		// Main execution thread.

		[STAThread]
		static void Main() 
		{	
			CsToolClass.install();

			// Show the login form.
			LoginForm login = new LoginForm();
			DialogResult res = login.ShowDialog();

			if( res == DialogResult.Cancel )
			{
				// no validation, so exit.
				return;
			}

			// store the login info, and connect to the LS
			if ( !CsToolClass.getInstance().installClient( login.getAddress(), 10666 ) )
			{
				return; // die.
			}

			CsToolClass.getInstance().sendMessage( "login " + login.userName+ " " + login.password );

			// wait for a response.

			string response;
            int timeWaited = 0;
            while( true )
			{
				Thread.Sleep( 100 );
				response = CsToolClass.getInstance().getMessage();
				if( response != "" )
					break;
                timeWaited += 100;
                if (timeWaited > 5000)
                {
                    MessageBox.Show("Error - no response from server.", "Could not log in");
                    return;
                }
			}

			// see if we were successful.
			if( response.StartsWith( "Logged in successfully" )	)
			{
				// fire off the main form.

				MainForm main = new MainForm( login.getAddress() );
				main.ShowDialog();
				
			}
			else
			{
				MessageBox.Show( "Error!  Could not log in.  Please check name/password and try again.", "Login error" );
			}

		}
		#endregion
	}
}
