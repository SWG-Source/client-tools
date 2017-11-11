using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Class to handle routine entering of input through a dialog box.
	/// </summary>
	public class SimpleTextForm : System.Windows.Forms.Form
	{
		#region automembers
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Label lblMain;
		private System.Windows.Forms.TextBox txtInput;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		/// <summary>
		/// Constructor to fully populate an instance
		/// </summary>
		/// <param name="title">Title of the message box</param>
		/// <param name="prompt">Prompt to display to user</param>
		/// <param name="requireInput">Is a valid string required to close the message box?</param>
		public SimpleTextForm( string title, string prompt, bool requireInput )
		{
			InitializeComponent();

			m_bRequireInput = requireInput;
			this.Text = title;
			lblMain.Text = prompt;
		}

		/// <summary>
		/// default constructor.
		/// </summary>
		public SimpleTextForm()
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

		#region public interface
		/// <summary>
		/// get the entered text
		/// </summary>
		/// <returns>the text string that was entered by the user</returns>
		public string getText()
		{
			return txtInput.Text;
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
			this.btnOK = new System.Windows.Forms.Button();
			this.btnCancel = new System.Windows.Forms.Button();
			this.lblMain = new System.Windows.Forms.Label();
			this.txtInput = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// btnOK
			// 
			this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.btnOK.Location = new System.Drawing.Point(240, 96);
			this.btnOK.Name = "btnOK";
			this.btnOK.TabIndex = 0;
			this.btnOK.Text = "OK";
			this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(40, 96);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 1;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			// 
			// lblMain
			// 
			this.lblMain.Location = new System.Drawing.Point(8, 8);
			this.lblMain.Name = "lblMain";
			this.lblMain.Size = new System.Drawing.Size(336, 40);
			this.lblMain.TabIndex = 2;
			this.lblMain.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// txtInput
			// 
			this.txtInput.Location = new System.Drawing.Point(24, 64);
			this.txtInput.Name = "txtInput";
			this.txtInput.Size = new System.Drawing.Size(304, 20);
			this.txtInput.TabIndex = 3;
			this.txtInput.Text = "";
			// 
			// SimpleTextForm
			// 
			this.AcceptButton = this.btnOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(352, 126);
			this.Controls.Add(this.txtInput);
			this.Controls.Add(this.lblMain);
			this.Controls.Add(this.btnCancel);
			this.Controls.Add(this.btnOK);
			this.Name = "SimpleTextForm";
			this.Text = "Input";
			this.ResumeLayout(false);

		}
		#endregion

		#region events
		private void btnOK_Click(object sender, System.EventArgs e)
		{
			if( ( txtInput.Text == "" ) && m_bRequireInput )
			{
				return; // nada.
			}
			this.Hide();		
		}

		private void btnCancel_Click(object sender, System.EventArgs e)
		{
			this.Hide();
		}
		#endregion

		#region members
		/// <summary>
		/// Does this dialog require input before it can be closed?
		/// </summary>
		protected bool m_bRequireInput;
		#endregion
	}
}
