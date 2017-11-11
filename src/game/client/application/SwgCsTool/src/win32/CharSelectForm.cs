using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Form to handle selecting a character from a list.
	/// </summary>
	public class CharSelectForm : System.Windows.Forms.Form
	{
		#region automembers
		private System.Windows.Forms.ListBox lstNames;
		private System.Windows.Forms.Button btnOk;
		private System.Windows.Forms.Button btnCancel;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		public CharSelectForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
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
			this.lstNames = new System.Windows.Forms.ListBox();
			this.btnOk = new System.Windows.Forms.Button();
			this.btnCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// lstNames
			// 
			this.lstNames.Location = new System.Drawing.Point(8, 8);
			this.lstNames.Name = "lstNames";
			this.lstNames.Size = new System.Drawing.Size(224, 264);
			this.lstNames.TabIndex = 0;
			// 
			// btnOk
			// 
			this.btnOk.Location = new System.Drawing.Point(160, 280);
			this.btnOk.Name = "btnOk";
			this.btnOk.TabIndex = 1;
			this.btnOk.Text = "OK";
			this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(8, 280);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 2;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			// 
			// CharSelectForm
			// 
			this.AcceptButton = this.btnOk;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(240, 310);
			this.Controls.Add(this.btnCancel);
			this.Controls.Add(this.btnOk);
			this.Controls.Add(this.lstNames);
			this.Name = "CharSelectForm";
			this.Text = "Select a character";
			this.ResumeLayout(false);

		}
		#endregion

		#region public interface
		public void addChar( string character, int id )
		{
			lstNames.Items.Add( character );
		}
		#endregion

		#region events
		private void btnOk_Click(object sender, System.EventArgs e)
		{
			if( lstNames.SelectedIndex != -1 )
			{
				// send a message to get info
				CsToolClass.getInstance().sendMessage( "get_pc_info " + lstNames.SelectedItem.ToString() );
			}
			this.Hide();
		}

		private void btnCancel_Click(object sender, System.EventArgs e)
		{
			this.Hide();
		}
		#endregion
	}
}
