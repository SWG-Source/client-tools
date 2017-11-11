using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;

namespace SwgCsTool
{
	/// <summary>
	/// Pop-up form to allow viewing and editing of objvars
	/// </summary>
	public class ObjVarsForm : System.Windows.Forms.Form
	{
		#region auto members
		private System.Windows.Forms.ListBox lstObjvars;
		private System.Windows.Forms.TextBox txtObjvarValue;
		private System.Windows.Forms.Button btnModify;
		private System.Windows.Forms.Button btnOK;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Button btnDelete;
		private System.Windows.Forms.Button btnAdd;
		#endregion

		#region constructors
		public ObjVarsForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			m_entries = new System.Collections.ArrayList();
		}
		#endregion

		#region public interface
		public void populate( string id, string response )
		{
			// clear existing entries.
			m_entries.Clear();

			m_id = id;
			// parse up the response string.

			string[] lines = response.Split( "\r".ToCharArray() );
			for( int i = 1; i < lines.Length; ++i )
			{
				string line = lines[ i ];
				line = line.Trim();

				string[] words = line.Split( ":".ToCharArray() );
				if( words.Length < 2 )
					continue;

				// first word should be the key.
				// everything after that is the value.
				string key = words[ 0 ];
				string val = "";
				for( int j = 1; j < words.Length; ++j )
				{
					val = val + words[ j ];
					val = val + ":";
				}
				val = val.TrimEnd( ":".ToCharArray() );
                
                // handle arrays of strings.  They're multiline, so we need to grab lines until we get to the end
                if (val.EndsWith("[["))
                {
                    val = "";
                    bool b_continue = true;
                    List<String> list = new List<string>();
                    do
                    {
                        if (++i < lines.Length)
                        {
                            string extra = lines[i].Trim(); ;
                            if (extra.StartsWith("]]"))
                            {
                                // we're done.                            
                                b_continue = false;
                            }
                            else
                            {
                                list.Add(extra);
                            }
                        }
                        // this shouldn't happen, unless we've got a malformed packet
                        else
                        {
                            b_continue = false;
                        }

                    } while (b_continue);
                    val = String.Join("|", list.ToArray());
                }
				// create an objvar entry.
				ObjvarEntry entry = new ObjvarEntry( key, val );
				// add it to our list.
				m_entries.Add( entry );
			}

			// repopulate our controls.
			updateData();
		}
		#endregion

		#region protected interface

		protected void updateData()
		{
			lstObjvars.BeginUpdate();
			int index = lstObjvars.SelectedIndex;
			txtObjvarValue.Text = "";			
			lstObjvars.Items.Clear();
			foreach( ObjvarEntry entry in m_entries )
			{
				lstObjvars.Items.Add( entry.name );
			}
			// possible that we deleted the last index.
			if( index >= lstObjvars.Items.Count )
			{
				index -= 1;
			}
			lstObjvars.SelectedIndex = index;
			lstObjvars.EndUpdate();
		}

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
            this.lstObjvars = new System.Windows.Forms.ListBox();
            this.txtObjvarValue = new System.Windows.Forms.TextBox();
            this.btnModify = new System.Windows.Forms.Button();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnDelete = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lstObjvars
            // 
            this.lstObjvars.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lstObjvars.Location = new System.Drawing.Point(16, 16);
            this.lstObjvars.Name = "lstObjvars";
            this.lstObjvars.Size = new System.Drawing.Size(120, 95);
            this.lstObjvars.TabIndex = 0;
            this.lstObjvars.SelectedIndexChanged += new System.EventHandler(this.indexChanged);
            // 
            // txtObjvarValue
            // 
            this.txtObjvarValue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.txtObjvarValue.Location = new System.Drawing.Point(152, 16);
            this.txtObjvarValue.Name = "txtObjvarValue";
            this.txtObjvarValue.ReadOnly = true;
            this.txtObjvarValue.Size = new System.Drawing.Size(168, 20);
            this.txtObjvarValue.TabIndex = 1;
            // 
            // btnModify
            // 
            this.btnModify.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnModify.Location = new System.Drawing.Point(240, 40);
            this.btnModify.Name = "btnModify";
            this.btnModify.Size = new System.Drawing.Size(75, 23);
            this.btnModify.TabIndex = 2;
            this.btnModify.Text = "Modify";
            this.btnModify.Click += new System.EventHandler(this.btnModify_Click);
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.Location = new System.Drawing.Point(240, 104);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 3;
            this.btnOK.Text = "Done";
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnDelete
            // 
            this.btnDelete.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnDelete.Location = new System.Drawing.Point(152, 40);
            this.btnDelete.Name = "btnDelete";
            this.btnDelete.Size = new System.Drawing.Size(75, 23);
            this.btnDelete.TabIndex = 4;
            this.btnDelete.Text = "Delete";
            this.btnDelete.Click += new System.EventHandler(this.btnDelete_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnAdd.Location = new System.Drawing.Point(200, 72);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 5;
            this.btnAdd.Text = "Add Objvar";
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // ObjVarsForm
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(328, 134);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.btnDelete);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.btnModify);
            this.Controls.Add(this.txtObjvarValue);
            this.Controls.Add(this.lstObjvars);
            this.Name = "ObjVarsForm";
            this.Text = "View/Modify ObjVars";
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region events
		private void btnOK_Click(object sender, System.EventArgs e)
		{
			this.Hide();
		}

		private void btnModify_Click(object sender, System.EventArgs e)
		{
			// get the new value.
			SimpleTextForm request = new SimpleTextForm( "Modify Objvar value", "Please enter the new value.", true );
			DialogResult result = request.ShowDialog();
			if( result == DialogResult.OK )
			{
				CsToolClass.getInstance().sendMessage( "set_objvar " +  m_id + " " + ( ( ObjvarEntry )m_entries[ lstObjvars.SelectedIndex ] ).name + " " + request.getText() );
			}
			CsToolClass.getInstance().sendMessage( "list_objvars " + m_id );
		}

		private void btnDelete_Click(object sender, System.EventArgs e)
		{
			// Make sure we want to do this.
			DialogResult res = MessageBox.Show( "Are you sure you want to delete the " + ( ( ObjvarEntry )m_entries[ lstObjvars.SelectedIndex ] ).name + " objvar?", "Delete Objvar", System.Windows.Forms.MessageBoxButtons.YesNo );
			if( res == DialogResult.Yes )
			{
				CsToolClass.getInstance().sendMessage( "remove_objvar " + m_id + " " + ( ( ObjvarEntry )m_entries[ lstObjvars.SelectedIndex ] ).name );
				CsToolClass.getInstance().sendMessage( "list_objvars " + m_id );
			}

		}

		private void btnAdd_Click(object sender, System.EventArgs e)
		{
			// hrm.  how to do this.  new form?  Two MBoxs?
			string objvarname;
			string objvarvalue;

			SimpleTextForm newfrm = new SimpleTextForm( "Objvar name", "Enter the objvar name", true );
			newfrm.ShowDialog();
			objvarname = newfrm.getText();
			if( objvarname == "" )
				return;

			newfrm = new SimpleTextForm( "Objvar value", "Enter the value for the new objvar", true );
			newfrm.ShowDialog();
			objvarvalue = newfrm.getText();

			CsToolClass.getInstance().sendMessage( "set_objvar " + m_id + " " + objvarname + " " + objvarvalue );
			CsToolClass.getInstance().sendMessage( "list_objvars " + m_id );
		}

		private void indexChanged(object sender, System.EventArgs e)
		{
			if( lstObjvars.SelectedIndex > m_entries.Count )
				return; // that's odd.

			txtObjvarValue.Text = ( ( ObjvarEntry )m_entries[ lstObjvars.SelectedIndex ] ).value;
		}
		#endregion

		#region members

		protected System.Collections.ArrayList m_entries;
		protected string m_id; // storing as a string is easier.

		#endregion

	}


	// name value pair for objvars.  I do wish that C# supported generics in this ver!
	public class ObjvarEntry
	{
		#region constructors
		public ObjvarEntry( string name, string value )
		{
			m_name = name;
			m_value = value;
		}
		#endregion

		#region properties
		public string name
		{
			get
			{
				return m_name;
			}
			set
			{
				m_name = value;
			}
		}

		public string value
		{
			get
			{
				return m_value;
			}
			set
			{
				m_value = value;
			}
		}
		#endregion

		#region members
		protected string m_name;
		protected string m_value;
		#endregion
	}
}
