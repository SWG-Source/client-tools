using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Tab to handle administrative tasks.
	/// </summary>
	public class AdministrationTab : SwgCsTool.BaseTab
	{
		#region automembers
		private System.Windows.Forms.TextBox txtPlayerName;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtPlayerID;
		private System.Windows.Forms.Button btnLookup;
		private System.Windows.Forms.Button btnFreeze;
		private System.Windows.Forms.Button btnUnfreeze;
        private Button btnRename;
        private ComboBox cmbDestination;
        private Button btnMove;
		private System.ComponentModel.IContainer components = null;
		#endregion

		#region constructors
		public AdministrationTab()
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			associate( "character id", txtPlayerID );
            activationRequirement("oneserver", txtPlayerName);
            activationRequirement("oneserver", btnLookup);
            activationRequirement("oneserver", btnFreeze);
            activationRequirement("oneserver", btnUnfreeze);
            activationRequirement("oneserver", btnRename);
            activationRequirement("txtPlayerID", btnFreeze);
            activationRequirement("txtPlayerID", btnUnfreeze);
            activationRequirement("txtPlayerID", btnRename);

            // load destinations
            m_locationDictionary = new Dictionary<string, string>();
            try
            {
                StreamReader file = System.IO.File.OpenText("cstool_locations.txt");
                string entry;
                while ((entry = file.ReadLine()) != null)
                {
                    entry = entry.Trim();
                    // skip blanks and comments.
                    if (entry == "" || entry.StartsWith("#"))
                        continue;
                    string[] pieces = entry.Split("=".ToCharArray());
                    if (pieces.Length != 2)
                        continue; // invalid entry.
                    string description = pieces[0].Trim();
                    string location = pieces[1].Trim();

                    cmbDestination.Items.Add(description);
                    m_locationDictionary[description] = location;
                }
            }
            catch
            {
            }
		}
		#endregion

		#region dispose
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

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.txtPlayerName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.txtPlayerID = new System.Windows.Forms.TextBox();
            this.btnLookup = new System.Windows.Forms.Button();
            this.btnFreeze = new System.Windows.Forms.Button();
            this.btnUnfreeze = new System.Windows.Forms.Button();
            this.btnRename = new System.Windows.Forms.Button();
            this.cmbDestination = new System.Windows.Forms.ComboBox();
            this.btnMove = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtPlayerName
            // 
            this.txtPlayerName.Location = new System.Drawing.Point(8, 24);
            this.txtPlayerName.Name = "txtPlayerName";
            this.txtPlayerName.Size = new System.Drawing.Size(100, 20);
            this.txtPlayerName.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 16);
            this.label1.TabIndex = 1;
            this.label1.Text = "Character Name";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(216, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 16);
            this.label2.TabIndex = 3;
            this.label2.Text = "Character ID";
            // 
            // txtPlayerID
            // 
            this.txtPlayerID.Location = new System.Drawing.Point(216, 24);
            this.txtPlayerID.Name = "txtPlayerID";
            this.txtPlayerID.ReadOnly = true;
            this.txtPlayerID.Size = new System.Drawing.Size(100, 20);
            this.txtPlayerID.TabIndex = 2;
            // 
            // btnLookup
            // 
            this.btnLookup.Location = new System.Drawing.Point(120, 24);
            this.btnLookup.Name = "btnLookup";
            this.btnLookup.Size = new System.Drawing.Size(75, 23);
            this.btnLookup.TabIndex = 4;
            this.btnLookup.Text = "Lookup!";
            this.btnLookup.Click += new System.EventHandler(this.btnLookup_Click);
            // 
            // btnFreeze
            // 
            this.btnFreeze.Location = new System.Drawing.Point(8, 54);
            this.btnFreeze.Name = "btnFreeze";
            this.btnFreeze.Size = new System.Drawing.Size(75, 23);
            this.btnFreeze.TabIndex = 5;
            this.btnFreeze.Text = "Freeze";
            this.btnFreeze.Click += new System.EventHandler(this.btnFreeze_Click);
            // 
            // btnUnfreeze
            // 
            this.btnUnfreeze.Location = new System.Drawing.Point(89, 54);
            this.btnUnfreeze.Name = "btnUnfreeze";
            this.btnUnfreeze.Size = new System.Drawing.Size(75, 23);
            this.btnUnfreeze.TabIndex = 6;
            this.btnUnfreeze.Text = "Unfreeze";
            this.btnUnfreeze.Click += new System.EventHandler(this.btnUnfreeze_Click);
            // 
            // btnRename
            // 
            this.btnRename.Location = new System.Drawing.Point(8, 83);
            this.btnRename.Name = "btnRename";
            this.btnRename.Size = new System.Drawing.Size(75, 23);
            this.btnRename.TabIndex = 7;
            this.btnRename.Text = "Rename";
            this.btnRename.UseVisualStyleBackColor = true;
            this.btnRename.Click += new System.EventHandler(this.btnRename_Click);
            // 
            // cmbDestination
            // 
            this.cmbDestination.FormattingEnabled = true;
            this.cmbDestination.Location = new System.Drawing.Point(86, 112);
            this.cmbDestination.Name = "cmbDestination";
            this.cmbDestination.Size = new System.Drawing.Size(289, 21);
            this.cmbDestination.TabIndex = 8;
            // 
            // btnMove
            // 
            this.btnMove.Location = new System.Drawing.Point(8, 112);
            this.btnMove.Name = "btnMove";
            this.btnMove.Size = new System.Drawing.Size(75, 23);
            this.btnMove.TabIndex = 9;
            this.btnMove.Text = "Move";
            this.btnMove.UseVisualStyleBackColor = true;
            this.btnMove.Click += new System.EventHandler(this.btnMove_Click);
            // 
            // AdministrationTab
            // 
            this.Controls.Add(this.btnMove);
            this.Controls.Add(this.cmbDestination);
            this.Controls.Add(this.btnRename);
            this.Controls.Add(this.btnUnfreeze);
            this.Controls.Add(this.btnFreeze);
            this.Controls.Add(this.btnLookup);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPlayerID);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtPlayerName);
            this.Name = "AdministrationTab";
            this.Size = new System.Drawing.Size(400, 256);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

        #region overrides
        public override void handleResponse(string response)
        {
            if( response.StartsWith("Could not rename player, name in use"))
            {
                MessageBox.Show("Could not rename player, name was in use.", "Rename error");
                return;
            }
            if(response.StartsWith("Rename player complete:"))
            {
                MessageBox.Show("Rename player in process.  Note: The renamed character will not be accessible through the tool until the next save cycle", "Note");
                return;
            }
            if(response.StartsWith("Unable to warp player"))
            {
                MessageBox.Show("Character is in save queue.  Please log character in or wait until save cycle is complete to move character", "Move player failed");
            }
            base.handleResponse(response);
        }
        #endregion

        #region events
        private void btnLookup_Click(object sender, System.EventArgs e)
		{
			if( txtPlayerName.Text != "" )
			{
				CsToolClass.getInstance().sendMessage( "get_player_id " + txtPlayerName.Text );
			}
		}

		private void btnFreeze_Click(object sender, System.EventArgs e)
		{
			if( txtPlayerID.Text != "" )
			{
				CsToolClass.getInstance().sendMessage( "freeze " + txtPlayerID.Text );
			}
		}

		private void btnUnfreeze_Click(object sender, System.EventArgs e)
		{
			if( txtPlayerID.Text != "" )
			{
				CsToolClass.getInstance().sendMessage( "unfreeze " + txtPlayerID.Text );
			}		
		}

        private void btnRename_Click(object sender, EventArgs e)
        {
            SimpleTextForm request = new SimpleTextForm("Rename Character", "Please enter the new name for the character", true);
            DialogResult result = request.ShowDialog();
            if (result == DialogResult.OK)
            {
                CsToolClass.getInstance().sendMessage("rename_player " + txtPlayerName.Text + " " + request.getText() + " verify");
            }

        }

		#endregion

        #region members
        Dictionary<string, string> m_locationDictionary;
        #endregion

        private void btnMove_Click(object sender, EventArgs e)
        {
            if (txtPlayerID.Text == "")
                return; // no id entered.
            string dest = cmbDestination.Text;
            if(m_locationDictionary.ContainsKey(cmbDestination.Text))
            {
                dest = m_locationDictionary[dest];
            }

            // check number of arguments.

            string[] args = dest.Split(" ".ToCharArray());
            if (args.GetLength(0) < 4)
            {
                MessageBox.Show("Error, not enough parameters for move function!", "Error");
                return;
            }
            string command = "warp_player " + txtPlayerID.Text + " " + dest;
            CsToolClass.getInstance().sendMessage(command);
        }
    }
}

