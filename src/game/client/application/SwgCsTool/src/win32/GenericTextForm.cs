using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Generic form to display a chunk of text.
	/// </summary>
	public class GenericTextForm : System.Windows.Forms.Form
	{
		#region automembers
		private System.Windows.Forms.Button btnDone;
		private System.Windows.Forms.TextBox txtDisplay;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		public GenericTextForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		public GenericTextForm( string textToShow )
		{
			InitializeComponent();
			txtDisplay.Text = textToShow;
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
			this.txtDisplay = new System.Windows.Forms.TextBox();
			this.btnDone = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// txtDisplay
			// 
			this.txtDisplay.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.txtDisplay.Location = new System.Drawing.Point(8, 8);
			this.txtDisplay.Multiline = true;
			this.txtDisplay.Name = "txtDisplay";
			this.txtDisplay.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtDisplay.Size = new System.Drawing.Size(276, 188);
			this.txtDisplay.TabIndex = 0;
			this.txtDisplay.Text = "";
			// 
			// btnDone
			// 
			this.btnDone.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.btnDone.Location = new System.Drawing.Point(110, 204);
			this.btnDone.Name = "btnDone";
			this.btnDone.TabIndex = 1;
			this.btnDone.Text = "Done";
			this.btnDone.Click += new System.EventHandler(this.btnDone_Click);
			// 
			// GenericTextForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(288, 238);
			this.Controls.Add(this.btnDone);
			this.Controls.Add(this.txtDisplay);
			this.Name = "GenericTextForm";
			this.Text = "GenericTextForm";
			this.ResumeLayout(false);

		}
		#endregion

		#region events
		private void btnDone_Click(object sender, System.EventArgs e)
		{
			this.Hide();
		}
		#endregion
	}
}
