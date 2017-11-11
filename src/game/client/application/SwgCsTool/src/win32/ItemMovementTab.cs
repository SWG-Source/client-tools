using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Tab to handle item movement.
	/// </summary>
	public class ItemMovementTab : SwgCsTool.BaseTab
	{
		#region automembers
		private System.Windows.Forms.Button btnGetItems;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtFromName;
		private System.Windows.Forms.TextBox txtToID;
		private System.Windows.Forms.TextBox txtToName;
		private System.Windows.Forms.Button btnGetToID;
		private System.Windows.Forms.Button btnMoveItems;
		private System.Windows.Forms.ListBox lstItems;
		private System.Windows.Forms.Label label2;
		private System.ComponentModel.IContainer components = null;

		#endregion

		#region constructors
		public ItemMovementTab()
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call
			associate( "character id", txtToID );
			associate( "item", lstItems );

            activationRequirement("oneserver", txtFromName);
            activationRequirement("oneserver", txtToName);

            activationRequirement("oneserver", btnMoveItems);
            activationRequirement("oneserver", btnGetItems);
            
            activationRequirement("txtToID", btnMoveItems);
            activationRequirement("txtFromName", btnMoveItems);
            activationRequirement("txtFromName", btnGetItems);
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

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.lstItems = new System.Windows.Forms.ListBox();
            this.txtFromName = new System.Windows.Forms.TextBox();
            this.txtToName = new System.Windows.Forms.TextBox();
            this.btnGetItems = new System.Windows.Forms.Button();
            this.btnGetToID = new System.Windows.Forms.Button();
            this.txtToID = new System.Windows.Forms.TextBox();
            this.btnMoveItems = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // lstItems
            // 
            this.lstItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lstItems.Location = new System.Drawing.Point(232, 8);
            this.lstItems.Name = "lstItems";
            this.lstItems.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple;
            this.lstItems.Size = new System.Drawing.Size(328, 303);
            this.lstItems.TabIndex = 0;
            // 
            // txtFromName
            // 
            this.txtFromName.Location = new System.Drawing.Point(16, 24);
            this.txtFromName.Name = "txtFromName";
            this.txtFromName.Size = new System.Drawing.Size(100, 20);
            this.txtFromName.TabIndex = 1;
            // 
            // txtToName
            // 
            this.txtToName.Location = new System.Drawing.Point(16, 104);
            this.txtToName.Name = "txtToName";
            this.txtToName.Size = new System.Drawing.Size(100, 20);
            this.txtToName.TabIndex = 2;
            // 
            // btnGetItems
            // 
            this.btnGetItems.Location = new System.Drawing.Point(16, 48);
            this.btnGetItems.Name = "btnGetItems";
            this.btnGetItems.Size = new System.Drawing.Size(75, 23);
            this.btnGetItems.TabIndex = 3;
            this.btnGetItems.Text = "Get Items";
            this.btnGetItems.Click += new System.EventHandler(this.btnGetItems_Click);
            // 
            // btnGetToID
            // 
            this.btnGetToID.Location = new System.Drawing.Point(16, 136);
            this.btnGetToID.Name = "btnGetToID";
            this.btnGetToID.Size = new System.Drawing.Size(75, 23);
            this.btnGetToID.TabIndex = 4;
            this.btnGetToID.Text = "Get ID";
            this.btnGetToID.Click += new System.EventHandler(this.btnGetToID_Click);
            // 
            // txtToID
            // 
            this.txtToID.Location = new System.Drawing.Point(120, 104);
            this.txtToID.Name = "txtToID";
            this.txtToID.ReadOnly = true;
            this.txtToID.Size = new System.Drawing.Size(100, 20);
            this.txtToID.TabIndex = 5;
            // 
            // btnMoveItems
            // 
            this.btnMoveItems.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnMoveItems.Location = new System.Drawing.Point(480, 320);
            this.btnMoveItems.Name = "btnMoveItems";
            this.btnMoveItems.Size = new System.Drawing.Size(75, 23);
            this.btnMoveItems.TabIndex = 6;
            this.btnMoveItems.Text = "Move Items";
            this.btnMoveItems.Click += new System.EventHandler(this.btnMoveItems_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 16);
            this.label1.TabIndex = 7;
            this.label1.Text = "From Player";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(16, 88);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 16);
            this.label2.TabIndex = 8;
            this.label2.Text = "To Player";
            // 
            // ItemMovementTab
            // 
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnMoveItems);
            this.Controls.Add(this.txtToID);
            this.Controls.Add(this.btnGetToID);
            this.Controls.Add(this.btnGetItems);
            this.Controls.Add(this.txtToName);
            this.Controls.Add(this.txtFromName);
            this.Controls.Add(this.lstItems);
            this.Name = "ItemMovementTab";
            this.Size = new System.Drawing.Size(568, 360);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region private interface
		private void setName( string name )
		{
			m_charName = name;
		}

		private void getItems()
		{
			lstItems.Items.Clear();
			CsToolClass.getInstance().sendMessage( "get_player_items " + m_charName );
		}
		#endregion

        #region overrides

        public override void handleResponse(string response)
        {
            if( response.StartsWith( "Moving object" ) )
            {
                MessageBox.Show( "Item movement in process.  The item will appear the next time the player logs in after a save cycle.", "Item movement" );
                return;
            }
            base.handleResponse(response);
        }
        #endregion

        #region events
        private void btnGetItems_Click(object sender, System.EventArgs e)
		{
			setName( txtFromName.Text );
			getItems();
		}

		private void btnGetToID_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "get_player_id " + txtToName.Text );
		}

		private void btnMoveItems_Click(object sender, System.EventArgs e)
		{
			for( int i = 0; i < lstItems.SelectedIndices.Count; ++i )
			{
				string item_id;
				int pos;
				int lastpos;
				string temp;
				temp = lstItems.Items[ lstItems.SelectedIndices[ i ] ].ToString();
				pos = temp.LastIndexOf( "(" );
				lastpos = temp.LastIndexOf( ")" );
				if( pos < 0 || lastpos < 0 )
					continue;
				pos++;
				int length = lastpos - pos;
				item_id = temp.Substring( pos, length );

				CsToolClass.getInstance().sendMessage( "move_object " + txtToID.Text + " " + item_id );
			}
			getItems();
		}
		#endregion

		#region members
		private string m_charName;
		#endregion
	}
}

