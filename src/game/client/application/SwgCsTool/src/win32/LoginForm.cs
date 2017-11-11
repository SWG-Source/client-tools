using System;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Form to get login information.
	/// </summary>
	public class LoginForm : System.Windows.Forms.Form
	{
		#region automembers
		private System.Windows.Forms.TextBox txtUsername;
		private System.Windows.Forms.Label lblUsername;
		private System.Windows.Forms.TextBox txtPassword;
		private System.Windows.Forms.Label lblPassword;
		private System.Windows.Forms.ComboBox cmbLoginAddress;
		private System.Windows.Forms.Label lblCluster;
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Button btnCancel;
		private System.Collections.Specialized.StringDictionary m_dctDisplayServers;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		public LoginForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_dctDisplayServers = new System.Collections.Specialized.StringDictionary();

			// ADD LOGIN CLUSTERS HERE
            addDisplayServer("TC2", "sdkswg-16-01.starwarsgalaxies.net");
			addDisplayServer( "Test", "sdkswg-12-01.starwarsgalaxies.net" );
            addDisplayServer("TC5", "ablswg-12-01.starwarsgalaxies.net");
            addDisplayServer("Live", "sdkswg-01-01.starwarsgalaxies.net");
		}


		#endregion

		#region public interface
		public string getAddress()
		{
			return m_address;
		}
		#endregion

        #region private interface
        
        private void addDisplayServer( string displayText, string serverAddress )
        {
            m_dctDisplayServers.Add(displayText, serverAddress);
            cmbLoginAddress.Items.Add(displayText);
        }
        
        #endregion

        #region properties
        public string userName
		{
			get
			{
				return txtUsername.Text;
			}
		}
		
		public string password
		{
			get
			{
				return txtPassword.Text;
			}
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
				if(components != null)
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
            this.txtUsername = new System.Windows.Forms.TextBox();
            this.lblUsername = new System.Windows.Forms.Label();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.lblPassword = new System.Windows.Forms.Label();
            this.cmbLoginAddress = new System.Windows.Forms.ComboBox();
            this.lblCluster = new System.Windows.Forms.Label();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtUsername
            // 
            this.txtUsername.Location = new System.Drawing.Point(88, 16);
            this.txtUsername.Name = "txtUsername";
            this.txtUsername.Size = new System.Drawing.Size(424, 20);
            this.txtUsername.TabIndex = 0;
            // 
            // lblUsername
            // 
            this.lblUsername.Location = new System.Drawing.Point(8, 16);
            this.lblUsername.Name = "lblUsername";
            this.lblUsername.Size = new System.Drawing.Size(72, 23);
            this.lblUsername.TabIndex = 1;
            this.lblUsername.Text = "Username";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(88, 40);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(424, 20);
            this.txtPassword.TabIndex = 2;
            // 
            // lblPassword
            // 
            this.lblPassword.Location = new System.Drawing.Point(8, 40);
            this.lblPassword.Name = "lblPassword";
            this.lblPassword.Size = new System.Drawing.Size(72, 23);
            this.lblPassword.TabIndex = 3;
            this.lblPassword.Text = "Password";
            // 
            // cmbLoginAddress
            // 
            this.cmbLoginAddress.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Append;
            this.cmbLoginAddress.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.cmbLoginAddress.Location = new System.Drawing.Point(88, 64);
            this.cmbLoginAddress.Name = "cmbLoginAddress";
            this.cmbLoginAddress.Size = new System.Drawing.Size(424, 21);
            this.cmbLoginAddress.TabIndex = 4;
            // 
            // lblCluster
            // 
            this.lblCluster.Location = new System.Drawing.Point(8, 64);
            this.lblCluster.Name = "lblCluster";
            this.lblCluster.Size = new System.Drawing.Size(72, 23);
            this.lblCluster.TabIndex = 5;
            this.lblCluster.Text = "Cluster";
            // 
            // btnOK
            // 
            this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOK.Location = new System.Drawing.Point(440, 96);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 6;
            this.btnOK.Text = "Login";
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(88, 96);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 7;
            this.btnCancel.Text = "Cancel";
            // 
            // LoginForm
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(528, 134);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.lblCluster);
            this.Controls.Add(this.cmbLoginAddress);
            this.Controls.Add(this.lblPassword);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.lblUsername);
            this.Controls.Add(this.txtUsername);
            this.Name = "LoginForm";
            this.Text = "SWG CS Tool Login";
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region events

		private void btnOK_Click(object sender, System.EventArgs e)
		{
			m_address = cmbLoginAddress.Text;
			foreach( DictionaryEntry entry in m_dctDisplayServers )
			{
				string text = entry.Key.ToString();
                // do this comparison because entry into the string dictionary
                // blasts case to lower.
				if( text.Equals(m_address, StringComparison.OrdinalIgnoreCase ))
				{
					m_address = entry.Value.ToString();
				}
			}
		}
		#endregion

		#region members
		protected string m_address;
		protected bool b_recursiveprotect;
		#endregion
	}
}
