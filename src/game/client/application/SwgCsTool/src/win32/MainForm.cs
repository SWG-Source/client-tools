using System;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Main form for the CS Tool.  Contains a tab control which contains the actual 'work' tabs.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		#region auto members
		private System.Windows.Forms.Panel pnlTreeview;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.TreeView treServers;
		private System.Windows.Forms.TextBox txtInput;
		private System.Windows.Forms.TextBox txtOutput;
		private System.Windows.Forms.TextBox txtPlayerName;
		private System.Windows.Forms.Label lblPlayerName;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button btnGetInfo;
		private System.Windows.Forms.Timer tmrRead;
		private System.Windows.Forms.TextBox txtOID;
		private System.Windows.Forms.TextBox txtCredits;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox txtLots;
		private System.Windows.Forms.TabPage PlayerTab;
		private SwgCsTool.PlayerTab PlayerTab1;
		private System.Windows.Forms.TabControl tabMain;
		private System.Windows.Forms.TabPage PlayerTabs;
		private SwgCsTool.PlayerTab PlayerTab2;
		private SwgCsTool.AdministrationTab tabAdmin2;
		private System.Windows.Forms.TabPage tabAdmin1;
		private SwgCsTool.AdministrationTab AdministrationTab1;
		private System.Windows.Forms.TabPage tabPage1;
		private SwgCsTool.ItemMovementTab ItemMovementTab1;
        private TabPage StructuresTab;
        private StructuresListTab StructuresListTab;
		private System.ComponentModel.IContainer components;
		#endregion

		#region constructors
		public MainForm( string loginaddress )
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
	
			m_aLoginServers = new ArrayList();
			b_initialized = false;
			// once we're connected, fire off a clusters message so we can
			// display a list of available clusters.
			addLS( loginaddress, true );

            
		}

        private void MainForm_Load(object sender, EventArgs e)
        {
            refreshServerList();

            // refreshing the server list will take care of updating the enabled/
            // disabled status of any of our controls, so we don't have to
            // worry about it here.
        }
		#endregion


		#region Dispose
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}
		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this.pnlTreeview = new System.Windows.Forms.Panel();
            this.treServers = new System.Windows.Forms.TreeView();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.txtInput = new System.Windows.Forms.TextBox();
            this.txtOutput = new System.Windows.Forms.TextBox();
            this.PlayerTab = new System.Windows.Forms.TabPage();
            this.label2 = new System.Windows.Forms.Label();
            this.txtCredits = new System.Windows.Forms.TextBox();
            this.btnGetInfo = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtOID = new System.Windows.Forms.TextBox();
            this.lblPlayerName = new System.Windows.Forms.Label();
            this.txtPlayerName = new System.Windows.Forms.TextBox();
            this.tmrRead = new System.Windows.Forms.Timer(this.components);
            this.label3 = new System.Windows.Forms.Label();
            this.txtLots = new System.Windows.Forms.TextBox();
            this.tabMain = new System.Windows.Forms.TabControl();
            this.PlayerTabs = new System.Windows.Forms.TabPage();
            this.PlayerTab2 = new SwgCsTool.PlayerTab();
            this.tabAdmin1 = new System.Windows.Forms.TabPage();
            this.AdministrationTab1 = new SwgCsTool.AdministrationTab();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.ItemMovementTab1 = new SwgCsTool.ItemMovementTab();
            this.StructuresTab = new System.Windows.Forms.TabPage();
            this.StructuresListTab = new SwgCsTool.StructuresListTab();
            this.PlayerTab1 = new SwgCsTool.PlayerTab();
            this.tabAdmin2 = new SwgCsTool.AdministrationTab();
            this.pnlTreeview.SuspendLayout();
            this.tabMain.SuspendLayout();
            this.PlayerTabs.SuspendLayout();
            this.tabAdmin1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.StructuresTab.SuspendLayout();
            this.SuspendLayout();
            // 
            // pnlTreeview
            // 
            this.pnlTreeview.Controls.Add(this.treServers);
            this.pnlTreeview.Dock = System.Windows.Forms.DockStyle.Left;
            this.pnlTreeview.Location = new System.Drawing.Point(0, 0);
            this.pnlTreeview.Name = "pnlTreeview";
            this.pnlTreeview.Size = new System.Drawing.Size(144, 558);
            this.pnlTreeview.TabIndex = 0;
            // 
            // treServers
            // 
            this.treServers.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.treServers.CheckBoxes = true;
            this.treServers.Location = new System.Drawing.Point(8, 8);
            this.treServers.Name = "treServers";
            this.treServers.Size = new System.Drawing.Size(136, 544);
            this.treServers.TabIndex = 0;
            this.treServers.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.treServers_AfterCheck);
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(144, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 558);
            this.splitter1.TabIndex = 1;
            this.splitter1.TabStop = false;
            // 
            // txtInput
            // 
            this.txtInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.txtInput.Location = new System.Drawing.Point(152, 536);
            this.txtInput.Name = "txtInput";
            this.txtInput.Size = new System.Drawing.Size(672, 20);
            this.txtInput.TabIndex = 2;
            this.txtInput.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtInput_KeyPress);
            // 
            // txtOutput
            // 
            this.txtOutput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.txtOutput.Location = new System.Drawing.Point(152, 456);
            this.txtOutput.Multiline = true;
            this.txtOutput.Name = "txtOutput";
            this.txtOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtOutput.Size = new System.Drawing.Size(672, 80);
            this.txtOutput.TabIndex = 3;
            // 
            // PlayerTab
            // 
            this.PlayerTab.Location = new System.Drawing.Point(0, 0);
            this.PlayerTab.Name = "PlayerTab";
            this.PlayerTab.Size = new System.Drawing.Size(200, 100);
            this.PlayerTab.TabIndex = 0;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(0, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 23);
            this.label2.TabIndex = 0;
            // 
            // txtCredits
            // 
            this.txtCredits.Location = new System.Drawing.Point(0, 0);
            this.txtCredits.Name = "txtCredits";
            this.txtCredits.Size = new System.Drawing.Size(100, 20);
            this.txtCredits.TabIndex = 0;
            // 
            // btnGetInfo
            // 
            this.btnGetInfo.Location = new System.Drawing.Point(0, 0);
            this.btnGetInfo.Name = "btnGetInfo";
            this.btnGetInfo.Size = new System.Drawing.Size(75, 23);
            this.btnGetInfo.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 23);
            this.label1.TabIndex = 0;
            // 
            // txtOID
            // 
            this.txtOID.Location = new System.Drawing.Point(0, 0);
            this.txtOID.Name = "txtOID";
            this.txtOID.Size = new System.Drawing.Size(100, 20);
            this.txtOID.TabIndex = 0;
            // 
            // lblPlayerName
            // 
            this.lblPlayerName.Location = new System.Drawing.Point(0, 0);
            this.lblPlayerName.Name = "lblPlayerName";
            this.lblPlayerName.Size = new System.Drawing.Size(100, 23);
            this.lblPlayerName.TabIndex = 0;
            // 
            // txtPlayerName
            // 
            this.txtPlayerName.Location = new System.Drawing.Point(0, 0);
            this.txtPlayerName.Name = "txtPlayerName";
            this.txtPlayerName.Size = new System.Drawing.Size(100, 20);
            this.txtPlayerName.TabIndex = 0;
            // 
            // tmrRead
            // 
            this.tmrRead.Enabled = true;
            this.tmrRead.Tick += new System.EventHandler(this.tmrRead_Tick);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(0, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(100, 23);
            this.label3.TabIndex = 0;
            // 
            // txtLots
            // 
            this.txtLots.Location = new System.Drawing.Point(0, 0);
            this.txtLots.Name = "txtLots";
            this.txtLots.Size = new System.Drawing.Size(100, 20);
            this.txtLots.TabIndex = 0;
            // 
            // tabMain
            // 
            this.tabMain.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabMain.Controls.Add(this.PlayerTabs);
            this.tabMain.Controls.Add(this.tabAdmin1);
            this.tabMain.Controls.Add(this.tabPage1);
            this.tabMain.Controls.Add(this.StructuresTab);
            this.tabMain.Location = new System.Drawing.Point(152, 8);
            this.tabMain.Name = "tabMain";
            this.tabMain.SelectedIndex = 0;
            this.tabMain.Size = new System.Drawing.Size(672, 440);
            this.tabMain.TabIndex = 4;
            // 
            // PlayerTabs
            // 
            this.PlayerTabs.Controls.Add(this.PlayerTab2);
            this.PlayerTabs.Location = new System.Drawing.Point(4, 22);
            this.PlayerTabs.Name = "PlayerTabs";
            this.PlayerTabs.Size = new System.Drawing.Size(664, 414);
            this.PlayerTabs.TabIndex = 0;
            this.PlayerTabs.Text = "PC Info";
            this.PlayerTabs.UseVisualStyleBackColor = true;
            // 
            // PlayerTab2
            // 
            this.PlayerTab2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.PlayerTab2.Location = new System.Drawing.Point(0, 0);
            this.PlayerTab2.Name = "PlayerTab2";
            this.PlayerTab2.Size = new System.Drawing.Size(664, 416);
            this.PlayerTab2.TabIndex = 0;
            // 
            // tabAdmin1
            // 
            this.tabAdmin1.Controls.Add(this.AdministrationTab1);
            this.tabAdmin1.Location = new System.Drawing.Point(4, 22);
            this.tabAdmin1.Name = "tabAdmin1";
            this.tabAdmin1.Size = new System.Drawing.Size(664, 414);
            this.tabAdmin1.TabIndex = 1;
            this.tabAdmin1.Text = "Administration";
            this.tabAdmin1.UseVisualStyleBackColor = true;
            // 
            // AdministrationTab1
            // 
            this.AdministrationTab1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.AdministrationTab1.Location = new System.Drawing.Point(0, 0);
            this.AdministrationTab1.Name = "AdministrationTab1";
            this.AdministrationTab1.Size = new System.Drawing.Size(664, 414);
            this.AdministrationTab1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.ItemMovementTab1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(664, 414);
            this.tabPage1.TabIndex = 2;
            this.tabPage1.Text = "Item Movement";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // ItemMovementTab1
            // 
            this.ItemMovementTab1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ItemMovementTab1.Location = new System.Drawing.Point(0, 0);
            this.ItemMovementTab1.Name = "ItemMovementTab1";
            this.ItemMovementTab1.Size = new System.Drawing.Size(664, 414);
            this.ItemMovementTab1.TabIndex = 0;
            // 
            // StructuresTab
            // 
            this.StructuresTab.Controls.Add(this.StructuresListTab);
            this.StructuresTab.Location = new System.Drawing.Point(4, 22);
            this.StructuresTab.Name = "StructuresTab";
            this.StructuresTab.Padding = new System.Windows.Forms.Padding(3);
            this.StructuresTab.Size = new System.Drawing.Size(664, 414);
            this.StructuresTab.TabIndex = 3;
            this.StructuresTab.Text = "Structures";
            this.StructuresTab.UseVisualStyleBackColor = true;
            // 
            // StructuresListTab
            // 
            this.StructuresListTab.AutoSize = true;
            this.StructuresListTab.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.StructuresListTab.Dock = System.Windows.Forms.DockStyle.Fill;
            this.StructuresListTab.Location = new System.Drawing.Point(3, 3);
            this.StructuresListTab.Name = "StructuresListTab";
            this.StructuresListTab.Size = new System.Drawing.Size(658, 408);
            this.StructuresListTab.TabIndex = 0;
            // 
            // PlayerTab1
            // 
            this.PlayerTab1.Location = new System.Drawing.Point(0, 0);
            this.PlayerTab1.Name = "PlayerTab1";
            this.PlayerTab1.Size = new System.Drawing.Size(400, 312);
            this.PlayerTab1.TabIndex = 0;
            // 
            // tabAdmin2
            // 
            this.tabAdmin2.Location = new System.Drawing.Point(4, 22);
            this.tabAdmin2.Name = "tabAdmin2";
            this.tabAdmin2.Size = new System.Drawing.Size(664, 414);
            this.tabAdmin2.TabIndex = 1;
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(832, 558);
            this.Controls.Add(this.tabMain);
            this.Controls.Add(this.txtOutput);
            this.Controls.Add(this.txtInput);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.pnlTreeview);
            this.Name = "MainForm";
            this.Text = "SWG CS Tool";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.pnlTreeview.ResumeLayout(false);
            this.tabMain.ResumeLayout(false);
            this.PlayerTabs.ResumeLayout(false);
            this.tabAdmin1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.StructuresTab.ResumeLayout(false);
            this.StructuresTab.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region PublicInterface
		public void addLS( string address, bool active )
		{
			LoginServerInfo info = new LoginServerInfo();
			info.name = address;
			info.active = active;
			m_aLoginServers.Add( info );
			if( active )
				m_currentLS = info;
		}

		public void addClusterToCurrentLS( string cluster )
		{
			addClusterToCurrentLS( cluster, true );
		}

		public void addClusterToCurrentLS( string cluster, bool refresh )
		{
			for( int i = 0; i < m_currentLS.clusters.Count; ++i )
			{
				if( ( ( ClusterInfo ) m_currentLS.clusters[ i ] ).name == cluster )
					return;
			}
			ClusterInfo info = new ClusterInfo();
			info.name = cluster;
			info.active = false;
			m_currentLS.clusters.Add( info );
			if ( refresh )
			{
				refreshServerList();
			}
		}

        public int getNumberOfSelectedServers() 
        {
            TreeNodeCollection tempnodes = treServers.Nodes[0].Nodes;
            int count = 0;
            foreach (TreeNode node in tempnodes )
            {
                if (node.Checked)
                    count++;
            }
            return count;
        }

        public void updateAllTabControls()
        {
            // iterate over the tab pages
            foreach(TabPage tab in tabMain.TabPages)
            {
                // there could theoretically be more than one BaseTab
                // on a TabPage, so it's best to do this as a foreach.
                // It also makes it easy to find the actual BaseTabs.
                foreach(BaseTab basetab in tab.Controls)
                {
                    basetab.updateEnabledControls();
                }

            }
        }

		public void refreshServerList()
		{
			treServers.Nodes.Clear();
			
			for( int i = 0; i < m_aLoginServers.Count; ++i )
			{
				LoginServerInfo ls = ( LoginServerInfo )m_aLoginServers[ i ];
				TreeNode node = new TreeNode( ls.name );
				node.Checked = ls.active;
				for( int j = 0; j < ls.clusters.Count; ++j )
				{
					ClusterInfo ci = ( ClusterInfo )ls.clusters[ j ];
					TreeNode clusternode = new TreeNode( ci.name );
					clusternode.Checked = ci.active;
					node.Nodes.Add( clusternode );
				}
				treServers.Nodes.Add( node );
			}
            treServers.ExpandAll();
            updateAllTabControls();
		}

		public void updateOutput( string message )
		{
			txtOutput.Text += message;
			txtOutput.Select( txtOutput.Text.Length, 0 );
			txtOutput.ScrollToCaret();
		}

		public string[] splitIntoLines( string message )
		{
			StringCollection strings = new StringCollection();
			int pos = 0;
			while( pos != -1 && pos < message.Length )
			{
				int newpos = message.IndexOf( "\r\n", pos );
				if( newpos == -1 )
				{
					if( pos < message.Length - 1 )
						strings.Add( message.Substring( pos ) );
					break;
				}
				else
				{
					strings.Add( message.Substring( pos , newpos - pos ) ) ;
					pos = newpos + 2;
				}
			}
			string[] returns = new string[ strings.Count ];
			for( int i = 0; i < strings.Count; ++i )
				returns[ i ] = strings[ i ];
			return returns;
		}
		#endregion

		#region PrivateFunctions
		private void parse( string message )
		{
			// first see if it's something we need to handle here.
			// for now:  Assume that anything starting with 'ls:' is something we might
			// POTENTIALLY need to deal with.

			if( message.Substring(0, 3 ) == "ls:" )
			{
				string lsmessage = message.Substring( 3 );
				// okay, let's figure out what's going on.

				Match match;
				Regex clusters_regex = new Regex( "Active clusters" );
				// list of active clusters.
				if( clusters_regex.Match( lsmessage ).Success )
				{				
					// read through the rest of the string, finding cluster names.
					
					string[] strings = splitIntoLines( lsmessage );
					for( int i = 1; i < strings.Length; ++i )
					{
						addClusterToCurrentLS( strings[ i ], false );
					}
                    refreshServerList();
				}
				// NOTE:  The next three are probably redundant, but best to make sure everything's
				// doing what we think.
				

				Regex deselect_regex = new Regex( @"Cluster ([^\s\.]+) removed from" );
				
				match = deselect_regex.Match( lsmessage );
				if( match.Success )
				{
					string cluster = match.Value;
					m_currentLS.deactivateCluster( cluster );
					refreshServerList();

				}

				// selected cluster.

				Regex select_regex = new Regex( @"Cluster ([^\s]) added to active" );
				match = select_regex.Match( lsmessage );
				if( match.Success )
				{
					string cluster = match.Value;
					m_currentLS.activateCluster( cluster );
					refreshServerList();
				}
				
				// deselected ALL clusters


				return; 
			}

			// otherwise, pass it to the active tab.
			else
			{
				// assume we can't have a : in the cluster name.
				int pos = message.IndexOf( ":" );
				if( pos == -1 ) // this should never, never, never be the case.
				{
					return;
				}
				string tabmessage = message.Substring( pos + 1, message.Length - ( pos + 1 ) );

				// Since tab controls want TabPages only (no subclasses), the way we
				// get around this is to add a single control to each tab page, derived
				// from the BaseTab class.  Here, what we're doing is finding this control
				// on the currently selected tab page (it's most likely the only one,
				// but we're iterating through the collection Just In Case.)
				TabPage tab_page = this.tabMain.TabPages[ this.tabMain.SelectedIndex ];
				foreach( Control control in tab_page.Controls )
				{
					BaseTab tab = control as BaseTab;
					if( tab != null )
					{
						// this is the one we want.
						tab.handleResponse( tabmessage );
                        tab.updateEnabledControls();

						// NOTE:  If we ever want more than one base tab on a page,
						// we'll have to remove the break.  It's an optimization right now
						// since I can't ever see the need for more than one.
						break;
					}
				}
			}
		}
		#endregion

		#region events
		private void txtInput_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			if( e.KeyChar == '\r' )
			{
				// send the message to the appropriate LS
				CsToolClass.getInstance().sendMessage( txtInput.Text );
				txtInput.Text = "";
			}
		}
		

		private void treServers_AfterCheck(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			// find out what was checked/unchecked
			string path = e.Node.FullPath;
			// count the slashes.
			string[] strings = path.Split( '\\' );
			int count = strings.Length;
			if( count == 1 ) // LS
			{
				// need to allow disconnection from an LS
			}
			else if( count == 2 ) // Cluster
			{
				if( e.Node.Checked )
				{
					string message = "select +" + strings[ 1 ];
					CsToolClass.getInstance().sendMessage( message );
				}
				else
				{
					string message = "select -" + strings[ 1 ];
					CsToolClass.getInstance().sendMessage( message );
				}
			}
			else if( count == 3 ) // Planet
			{
			}
            updateAllTabControls();
		}

		private void tmrRead_Tick(object sender, System.EventArgs e)
		{
			if( !b_initialized )
			{
				CsToolClass.getInstance().sendMessage( "clusters" );
				b_initialized = true;
			}
			// if we're connected, try and read from our connection.
			string message = CsToolClass.getInstance().getMessage();
			while( message != "" )
			{
				parse( message );
				updateOutput( message );
                message = CsToolClass.getInstance().getMessage();
			}
		}
		#endregion

        #region members
        protected LoginServerInfo m_currentLS;

		protected ArrayList m_aLoginServers;

		protected bool b_initialized;
		#endregion


	}
}
