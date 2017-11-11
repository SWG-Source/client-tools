using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// This form displays deleted items and allows the user to undelete them.
	/// </summary>
	public class DeletedItemsForm : System.Windows.Forms.Form
	{
		#region inner classes
		private class itemInfo
		{
			public string itemname;
			public Int64 itemid;
		}
		#endregion

		#region automembers
		private System.Windows.Forms.ListBox lstItems;
		private System.Windows.Forms.Button btnOk;
		private System.Windows.Forms.Button btnCancel;
        private CheckBox chkMoveItems;
        private Button btnPrev;
        private Button btnNext;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		public DeletedItemsForm(string characterId)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			m_items = new ArrayList();
            m_characterId = characterId;
            m_pageNum = 0;
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
            this.lstItems = new System.Windows.Forms.ListBox();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.chkMoveItems = new System.Windows.Forms.CheckBox();
            this.btnPrev = new System.Windows.Forms.Button();
            this.btnNext = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lstItems
            // 
            this.lstItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lstItems.Location = new System.Drawing.Point(8, 8);
            this.lstItems.Name = "lstItems";
            this.lstItems.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple;
            this.lstItems.Size = new System.Drawing.Size(262, 225);
            this.lstItems.TabIndex = 0;
            // 
            // btnOk
            // 
            this.btnOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOk.Location = new System.Drawing.Point(195, 297);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 1;
            this.btnOk.Text = "OK";
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(12, 297);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 2;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // chkMoveItems
            // 
            this.chkMoveItems.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.chkMoveItems.AutoSize = true;
            this.chkMoveItems.Location = new System.Drawing.Point(45, 274);
            this.chkMoveItems.Name = "chkMoveItems";
            this.chkMoveItems.Size = new System.Drawing.Size(194, 17);
            this.chkMoveItems.TabIndex = 3;
            this.chkMoveItems.Text = "Move undeleted items to inventory?";
            this.chkMoveItems.UseVisualStyleBackColor = true;
            // 
            // btnPrev
            // 
            this.btnPrev.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnPrev.Location = new System.Drawing.Point(12, 245);
            this.btnPrev.Name = "btnPrev";
            this.btnPrev.Size = new System.Drawing.Size(75, 23);
            this.btnPrev.TabIndex = 4;
            this.btnPrev.Text = "Previous";
            this.btnPrev.UseVisualStyleBackColor = true;
            this.btnPrev.Click += new System.EventHandler(this.btnPrev_Click);
            // 
            // btnNext
            // 
            this.btnNext.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnNext.Location = new System.Drawing.Point(195, 245);
            this.btnNext.Name = "btnNext";
            this.btnNext.Size = new System.Drawing.Size(75, 23);
            this.btnNext.TabIndex = 5;
            this.btnNext.Text = "Next";
            this.btnNext.UseVisualStyleBackColor = true;
            this.btnNext.Click += new System.EventHandler(this.btnNext_Click);
            // 
            // DeletedItemsForm
            // 
            this.AcceptButton = this.btnOk;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(282, 332);
            this.Controls.Add(this.btnNext);
            this.Controls.Add(this.btnPrev);
            this.Controls.Add(this.chkMoveItems);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.lstItems);
            this.Name = "DeletedItemsForm";
            this.Text = "Select an item to undelete";
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region public interface
		public void addItem( string item, Int64 id )
		{
			itemInfo info = new itemInfo();
			info.itemid = id;
			info.itemname = item;
			m_items.Add( info );
			lstItems.Items.Add( item );
		}

        public void clearItems()
        {
            lstItems.Items.Clear();
            m_items.Clear();
        }

        public void updateButtons()
        {
            btnPrev.Enabled = m_pageNum > 0;
            btnNext.Enabled = m_items.Count == 20;
        }
		#endregion

		#region events
		private void btnOk_Click(object sender, System.EventArgs e)
		{
			if( lstItems.SelectedIndex != -1 )
			{
                foreach (int index in lstItems.SelectedIndices)
                {
                    // send a message to get info
                    CsToolClass.getInstance().sendMessage("undelete_item " + m_characterId + " " + (((itemInfo)m_items.ToArray()[index]).itemid) + (chkMoveItems.Checked ? " move" : " nomove"));
                }
			}
			this.Hide();
		}

		private void btnCancel_Click(object sender, System.EventArgs e)
		{
			this.Hide();
		}

        private void btnPrev_Click(object sender, EventArgs e)
        {
            if (m_pageNum >= 1)
            {
                m_pageNum--;
                CsToolClass.getInstance().sendMessage("get_deleted_items " + m_characterId + " " + m_pageNum);
            }
        }

        private void btnNext_Click(object sender, EventArgs e)
        {
            m_pageNum++;
            CsToolClass.getInstance().sendMessage("get_deleted_items " + m_characterId + " " + m_pageNum);
        }
		#endregion

		#region members
		protected System.Collections.ArrayList m_items;
        protected string m_characterId;
        protected int m_pageNum;
		#endregion
	}
}
