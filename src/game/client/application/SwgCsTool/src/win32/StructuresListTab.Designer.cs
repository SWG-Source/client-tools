namespace SwgCsTool
{
    partial class StructuresListTab
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.Label label1;
            System.Windows.Forms.Label label2;
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            this.txtCharacterName = new System.Windows.Forms.TextBox();
            this.txtCharacterId = new System.Windows.Forms.TextBox();
            this.btnLookup = new System.Windows.Forms.Button();
            this.btnGetStructures = new System.Windows.Forms.Button();
            this.grdItems = new System.Windows.Forms.DataGridView();
            this.colStructureId = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colTemplate = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colScene = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colLocation = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colDeleted = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.mnuViewObjvars = new System.Windows.Forms.ToolStripMenuItem();
            this.chkHideDeleted = new System.Windows.Forms.CheckBox();
            label1 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.grdItems)).BeginInit();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(3, 6);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(84, 13);
            label1.TabIndex = 3;
            label1.Text = "Character Name";
            // 
            // label2
            // 
            label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(365, 6);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(67, 13);
            label2.TabIndex = 3;
            label2.Text = "Character ID";
            // 
            // txtCharacterName
            // 
            this.txtCharacterName.Location = new System.Drawing.Point(88, 3);
            this.txtCharacterName.Name = "txtCharacterName";
            this.txtCharacterName.Size = new System.Drawing.Size(100, 20);
            this.txtCharacterName.TabIndex = 0;
            // 
            // txtCharacterId
            // 
            this.txtCharacterId.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.txtCharacterId.Location = new System.Drawing.Point(438, 3);
            this.txtCharacterId.Name = "txtCharacterId";
            this.txtCharacterId.ReadOnly = true;
            this.txtCharacterId.Size = new System.Drawing.Size(100, 20);
            this.txtCharacterId.TabIndex = 1;
            // 
            // btnLookup
            // 
            this.btnLookup.Location = new System.Drawing.Point(194, 1);
            this.btnLookup.Name = "btnLookup";
            this.btnLookup.Size = new System.Drawing.Size(75, 23);
            this.btnLookup.TabIndex = 2;
            this.btnLookup.Text = "Lookup";
            this.btnLookup.UseVisualStyleBackColor = true;
            this.btnLookup.Click += new System.EventHandler(this.btnLookup_Click);
            // 
            // btnGetStructures
            // 
            this.btnGetStructures.Location = new System.Drawing.Point(62, 30);
            this.btnGetStructures.Name = "btnGetStructures";
            this.btnGetStructures.Size = new System.Drawing.Size(96, 23);
            this.btnGetStructures.TabIndex = 5;
            this.btnGetStructures.Text = "Get Structures";
            this.btnGetStructures.UseVisualStyleBackColor = true;
            this.btnGetStructures.Click += new System.EventHandler(this.btnGetStructures_Click);
            // 
            // grdItems
            // 
            this.grdItems.AllowUserToAddRows = false;
            this.grdItems.AllowUserToDeleteRows = false;
            this.grdItems.AllowUserToOrderColumns = true;
            dataGridViewCellStyle1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.grdItems.AlternatingRowsDefaultCellStyle = dataGridViewCellStyle1;
            this.grdItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.grdItems.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.grdItems.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.grdItems.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colStructureId,
            this.colTemplate,
            this.colScene,
            this.colLocation,
            this.colDeleted});
            this.grdItems.ContextMenuStrip = this.contextMenuStrip1;
            this.grdItems.Location = new System.Drawing.Point(6, 59);
            this.grdItems.MultiSelect = false;
            this.grdItems.Name = "grdItems";
            this.grdItems.ReadOnly = true;
            this.grdItems.RowHeadersWidth = 5;
            this.grdItems.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.grdItems.Size = new System.Drawing.Size(560, 312);
            this.grdItems.TabIndex = 6;
            // 
            // colStructureId
            // 
            this.colStructureId.DataPropertyName = "Id";
            this.colStructureId.HeaderText = "Structure ID";
            this.colStructureId.Name = "colStructureId";
            this.colStructureId.ReadOnly = true;
            // 
            // colTemplate
            // 
            this.colTemplate.DataPropertyName = "template";
            this.colTemplate.FillWeight = 200F;
            this.colTemplate.HeaderText = "Template";
            this.colTemplate.Name = "colTemplate";
            this.colTemplate.ReadOnly = true;
            // 
            // colScene
            // 
            this.colScene.DataPropertyName = "scene";
            this.colScene.HeaderText = "Scene";
            this.colScene.Name = "colScene";
            this.colScene.ReadOnly = true;
            // 
            // colLocation
            // 
            this.colLocation.DataPropertyName = "location";
            this.colLocation.FillWeight = 120F;
            this.colLocation.HeaderText = "Location";
            this.colLocation.Name = "colLocation";
            this.colLocation.ReadOnly = true;
            // 
            // colDeleted
            // 
            this.colDeleted.DataPropertyName = "deleted";
            this.colDeleted.FillWeight = 50F;
            this.colDeleted.HeaderText = "Deleted";
            this.colDeleted.Name = "colDeleted";
            this.colDeleted.ReadOnly = true;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuViewObjvars});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(138, 26);
            // 
            // mnuViewObjvars
            // 
            this.mnuViewObjvars.Name = "mnuViewObjvars";
            this.mnuViewObjvars.Size = new System.Drawing.Size(137, 22);
            this.mnuViewObjvars.Text = "View Objvars";
            this.mnuViewObjvars.Click += new System.EventHandler(this.mnuViewObjvars_Click);
            // 
            // chkHideDeleted
            // 
            this.chkHideDeleted.AutoSize = true;
            this.chkHideDeleted.Location = new System.Drawing.Point(312, 34);
            this.chkHideDeleted.Name = "chkHideDeleted";
            this.chkHideDeleted.Size = new System.Drawing.Size(88, 17);
            this.chkHideDeleted.TabIndex = 7;
            this.chkHideDeleted.Text = "Hide Deleted";
            this.chkHideDeleted.UseVisualStyleBackColor = true;
            // 
            // StructuresListTab
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.Controls.Add(this.grdItems);
            this.Controls.Add(this.chkHideDeleted);
            this.Controls.Add(label2);
            this.Controls.Add(this.btnGetStructures);
            this.Controls.Add(label1);
            this.Controls.Add(this.btnLookup);
            this.Controls.Add(this.txtCharacterId);
            this.Controls.Add(this.txtCharacterName);
            this.Name = "StructuresListTab";
            this.Size = new System.Drawing.Size(569, 374);
            ((System.ComponentModel.ISupportInitialize)(this.grdItems)).EndInit();
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtCharacterName;
        private System.Windows.Forms.TextBox txtCharacterId;
        private System.Windows.Forms.Button btnLookup;
        private System.Windows.Forms.Button btnGetStructures;
        private System.Windows.Forms.DataGridView grdItems;
        private System.Windows.Forms.DataGridViewTextBoxColumn colStructureId;
        private System.Windows.Forms.DataGridViewTextBoxColumn colTemplate;
        private System.Windows.Forms.DataGridViewTextBoxColumn colScene;
        private System.Windows.Forms.DataGridViewTextBoxColumn colLocation;
        private System.Windows.Forms.DataGridViewCheckBoxColumn colDeleted;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem mnuViewObjvars;
        private System.Windows.Forms.CheckBox chkHideDeleted;
    }
}
