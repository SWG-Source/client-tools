using System;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.IO;

namespace SwgCsTool
{
	/// <summary>
	/// Tab to allow viewing and editing of player data
	/// </summary>
	public class PlayerTab : SwgCsTool.BaseTab
	{
		#region automembers
		private System.Windows.Forms.TextBox txtPlayerName;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button btnLookup;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtCredits;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox txtLotsAvail;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtFaction;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Button btnBankUpdate;
		private System.Windows.Forms.TextBox txtBankCredits;
		private System.Windows.Forms.GroupBox grpAttribs;
		private System.Windows.Forms.TextBox txtAction;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox txtConstitution;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.TextBox txtHealth;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.TextBox txtMind;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TextBox txtStamina;
		private System.Windows.Forms.Label label11;
		private System.Windows.Forms.TextBox txtWillpower;
		private System.Windows.Forms.TextBox txtPVP;
		private System.Windows.Forms.ListBox cmbSkills;
		private System.Windows.Forms.Label label12;
		private System.Windows.Forms.Label label13;
		private System.Windows.Forms.ListBox lstExperience;
		private System.Windows.Forms.Label label14;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem mnuDeleteItem;
		private System.Windows.Forms.ListBox lstItems;
		private System.Windows.Forms.Label label15;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.Splitter splitter2;
		private System.Windows.Forms.Panel panel4;
		private System.Windows.Forms.TextBox txtAccount;
		private System.Windows.Forms.Label label16;
		private System.Windows.Forms.Button btnLookupChars;
		private System.Windows.Forms.Label label17;
		private System.Windows.Forms.TextBox txtCharacterId;
		private System.Windows.Forms.Button btnDeletedItems;
		private System.Windows.Forms.Button btnObjVars;
		private System.Windows.Forms.Button btnDump;
		private System.Windows.Forms.Button btnLookupID;
		private System.Windows.Forms.ContextMenu contextMenu;
		private System.Windows.Forms.MenuItem ContextItemInfo;
		private System.Windows.Forms.MenuItem ContextMenuItemObjvars;
        private ComboBox cmbItems;
        private Button btnGiveItem;
        private Button btnLotsAdjust;
        private Button btnLoadCharacter;
		private System.ComponentModel.IContainer components = null;
		#endregion

		#region constructors
		public PlayerTab()
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();
			
			associate( "lots available", txtLotsAvail );
			associate( "credits", txtCredits );
			associate( "bank credits", txtBankCredits );
			associate( "health", txtHealth );
			associate( "mind", txtMind );
			associate( "constitution", txtConstitution );
			associate( "willpower", txtWillpower );
			associate( "stamina", txtStamina );
			associate( "action", txtAction );
			associate( "character id", txtCharacterId );
			associate( "account id", txtAccount );

            // requirements for controls to be enabled.
            activationRequirement("oneserver", txtPlayerName);
            activationRequirement("oneserver", btnLookup);

            activationRequirement("oneserver", btnLotsAdjust);
            activationRequirement("oneserver", btnBankUpdate);
            activationRequirement("oneserver", btnObjVars);
            activationRequirement("oneserver", btnDeletedItems);
            activationRequirement("oneserver", btnGiveItem);
            activationRequirement("oneserver", btnLookupChars);
            activationRequirement("oneserver", btnDump);
            activationRequirement("oneserver", btnLookupID);
            activationRequirement("oneserver", btnLoadCharacter);

            activationRequirement("txtCharacterId", btnLotsAdjust);
            activationRequirement("txtCharacterId", btnBankUpdate);
            activationRequirement("txtCharacterId", btnObjVars);
            activationRequirement("txtCharacterId", btnDeletedItems);
            activationRequirement("txtCharacterId", btnGiveItem);
            activationRequirement("txtCharacterId", btnLookupChars);
            activationRequirement("txtCharacterId", btnDump);
            activationRequirement("txtCharacterId", btnLoadCharacter);
            activationRequirement("txtAccount", btnLoadCharacter);


            // load up our combobox.
            try
            {
                StreamReader file = System.IO.File.OpenText("static_items.txt");
                string entry;
                while ((entry = file.ReadLine()) != null)
                {
                    // skip blanks and comments.
                    if (entry == "" || entry.StartsWith("#"))
                        continue;
                    cmbItems.Items.Add(entry);
                }
            }
            catch
            {
                // it's okay if we don't find the file.
            }

            // list of items we don't want to display.

            m_nonDisplayItems = new System.Collections.Generic.List<string>();

            m_nonDisplayItems.Add("hair_name:hair");
            m_nonDisplayItems.Add("item_n:datapad");
            m_nonDisplayItems.Add("item_n:bank");
            m_nonDisplayItems.Add("object/player/player.iff");
            m_nonDisplayItems.Add("item_n:inventory");
            m_nonDisplayItems.Add("item_n:mission_bag");
            m_nonDisplayItems.Add("weapon_name:default_weapon");
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

		#region public interface

        public void getCharacterData()
        {
            CsToolClass.getInstance().sendMessage("get_pc_info " + txtPlayerName.Text);
            clearData();
        }

		public void clearData()
		{
			cmbSkills.Items.Clear();
			lstExperience.Items.Clear();
			lstItems.Items.Clear();

			txtHealth.Clear();
			txtAction.Clear();
			txtStamina.Clear();
			txtWillpower.Clear();
			txtConstitution.Clear();
			txtMind.Clear();
			txtPVP.Clear();
			txtBankCredits.Clear();
			txtFaction.Clear();
			txtCredits.Clear();
			txtLotsAvail.Clear();
			txtCharacterId.Clear();
			txtAccount.Clear();
		}


		// TODO:  Change the following three methods to just use an associate() call.
		public void populateSkills( StringDictionary data )
		{
			foreach( string key in data.Keys )
			{
				if( key.StartsWith( "skill-" ) )
				{
					cmbSkills.Items.Add( data[ key ] );
				}
			}
		}

		public void populateExperience( StringDictionary data )
		{
			foreach( string key in data.Keys )
			{
				if( key.StartsWith( "experience-" ) )
				{
					string final = key.Substring( 11, key.Length- 11 );
					final = final + ":" + data[ key ];
					lstExperience.Items.Add( final );
				}
			}
		}
		
		public void populateItems( StringDictionary data )
		{
			foreach( string key in data.Keys )
			{
				if( key.StartsWith( "item-" ) || key.StartsWith( "equipment-" ) )
				{
                    bool b_found = false;
                    foreach(string s in m_nonDisplayItems)
                    {
                        if (data[key].Contains(s))
                        {
                            b_found = true;
                            break;  // out of the foreach.
                        }
                    }
                    if (!b_found)
					    lstItems.Items.Add( data[ key ] );
				}
			}
		}

		public void handleCharacterList( string response )
		{
			// create dialog
			CharSelectForm newform = new CharSelectForm();

			// populate it
			string[] lines = response.Split( "\r".ToCharArray() );
				
			for( int i = 1; i < lines.Length; ++i )
			{
				string[] parts = lines[ i ].Split( ":".ToCharArray() );
				if( parts.Length < 2 )
					continue; // bad format!
				newform.addChar( parts[ 1 ], Convert.ToInt32( parts[ 0 ] ) );
			}
			// display it.
			newform.ShowDialog();
		}

		public void handleDeletedItems( string response )
		{
			// create dialog, if needed.
            if (m_deletedItemsForm == null)
            {
                m_deletedItemsForm = new DeletedItemsForm(txtCharacterId.Text);
            }
            m_deletedItemsForm.clearItems();

			// populate it
			string[] lines = response.Split( "\r".ToCharArray() );
				
			for( int i = 1; i < lines.Length; ++i )
			{
				string[] parts = lines[ i ].Split( ":".ToCharArray() );
				if( parts.Length < 2 )
					continue; // bad format!
				m_deletedItemsForm.addItem( parts[ 1 ] + ":" + parts[ 2 ], Convert.ToInt64( parts[ 0 ] ) );
			}

            // update the next and previous buttons.
            m_deletedItemsForm.updateButtons();

            // display it.
            // we will delete the form after the ShowDialog() call returns.
            // that will ensure that any updates go to this form, however
            // after returning (via Cancel or OK), we will delete the form so that
            // the next one initializes to a default condition.
            if (!m_deletedItemsForm.Visible)
            {
                m_deletedItemsForm.ShowDialog();
                m_deletedItemsForm = null;
            }

		}

		public void handleDumpResponse( string response )
		{
			// get a generic text form.
			string text_to_show = response.Substring( response.IndexOf( "\r\n" ) +1 );
			GenericTextForm newform = new GenericTextForm( text_to_show );
			// show it.
			newform.ShowDialog();
			// that's it.
		}

		public override void handleResponse( string response )
		{
			if( response.StartsWith( "Character search" ) )
			{
				handleCharacterList( response );
			}
			else if( response.StartsWith( "Deleted items" ) )
			{
				handleDeletedItems( response );
			}
			else if( response.StartsWith( "Objvar list" ) )
			{
				handleObjvarList( response );
			}
			else if( response.StartsWith( "Character info dump" ) )
			{
				handleDumpResponse( response );
			}
			base.handleResponse( response );

		}

		public override void handleResponse( string type, StringDictionary data )
		{
			if( type.StartsWith(  "Stats for player " ) )
			{				
				// make sure we clear any old data, in case we don't receive a field.
				clearData();

                // PVP type
                // directly from src\engine\server\library\serverGame\src\shared\pvp\Pvp.h
                const int PvpType_Neutral = 0;
                const int PvpType_Covert = 1;
                const int PvpType_Declared = 2;

                switch( Convert.ToInt32( data["pvp type"] ) )
                {
                    case PvpType_Neutral:
                    {
                        txtPVP.Text = "Neutral";
                    };
                    break;
                    case PvpType_Declared:
                    {
                        txtPVP.Text = "Declared";
                    };
                    break;
                    case PvpType_Covert:
                    {
                        txtPVP.Text = "Covert";
                    };
                    break;
                }
                
                // stolen from SwgCuiCharacterSheet.cpp
                const int imperialFaction = -615855020;
                const int rebelFaction = 370444368;
                const int neutralFaction = 0;

                switch( Convert.ToInt32( data["faction"]))
                {
                    case imperialFaction:
                    {
                        txtFaction.Text = "Imperial";
                    }
                    break;
                    case rebelFaction:
                    {
                        txtFaction.Text = "Rebel";
                    }
                    break;
                    case neutralFaction:
                    {
                        txtFaction.Text = "Neutral";
                    }
                    break;
                }

                // handle skills.
				populateSkills( data );
				populateExperience( data );
				populateItems( data );
				
                base.handleResponse( type, data );

			}
			else if( type.StartsWith( "Successfully modified bank" ) )
			{
				// refresh data
				CsToolClass.getInstance().sendMessage( "get_pc_info " + txtPlayerName.Text );
			}
			else
			{
				base.handleResponse( type, data );
			}
			
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
            this.btnLookup = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.txtCredits = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtLotsAvail = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtFaction = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtBankCredits = new System.Windows.Forms.TextBox();
            this.btnBankUpdate = new System.Windows.Forms.Button();
            this.grpAttribs = new System.Windows.Forms.GroupBox();
            this.label11 = new System.Windows.Forms.Label();
            this.txtWillpower = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.txtStamina = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.txtMind = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.txtHealth = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.txtConstitution = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.txtAction = new System.Windows.Forms.TextBox();
            this.txtPVP = new System.Windows.Forms.TextBox();
            this.cmbSkills = new System.Windows.Forms.ListBox();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.lstExperience = new System.Windows.Forms.ListBox();
            this.label14 = new System.Windows.Forms.Label();
            this.contextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.mnuDeleteItem = new System.Windows.Forms.MenuItem();
            this.ContextItemInfo = new System.Windows.Forms.MenuItem();
            this.ContextMenuItemObjvars = new System.Windows.Forms.MenuItem();
            this.lstItems = new System.Windows.Forms.ListBox();
            this.label15 = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel4 = new System.Windows.Forms.Panel();
            this.splitter2 = new System.Windows.Forms.Splitter();
            this.panel3 = new System.Windows.Forms.Panel();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.panel2 = new System.Windows.Forms.Panel();
            this.txtAccount = new System.Windows.Forms.TextBox();
            this.label16 = new System.Windows.Forms.Label();
            this.btnLookupChars = new System.Windows.Forms.Button();
            this.label17 = new System.Windows.Forms.Label();
            this.txtCharacterId = new System.Windows.Forms.TextBox();
            this.btnDeletedItems = new System.Windows.Forms.Button();
            this.btnObjVars = new System.Windows.Forms.Button();
            this.btnDump = new System.Windows.Forms.Button();
            this.btnLookupID = new System.Windows.Forms.Button();
            this.cmbItems = new System.Windows.Forms.ComboBox();
            this.btnGiveItem = new System.Windows.Forms.Button();
            this.btnLotsAdjust = new System.Windows.Forms.Button();
            this.btnLoadCharacter = new System.Windows.Forms.Button();
            this.grpAttribs.SuspendLayout();
            this.panel1.SuspendLayout();
            this.panel4.SuspendLayout();
            this.panel3.SuspendLayout();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // txtPlayerName
            // 
            this.txtPlayerName.Location = new System.Drawing.Point(80, 8);
            this.txtPlayerName.MaxLength = 39;
            this.txtPlayerName.Name = "txtPlayerName";
            this.txtPlayerName.Size = new System.Drawing.Size(64, 20);
            this.txtPlayerName.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(56, 24);
            this.label1.TabIndex = 1;
            this.label1.Text = "PC Name";
            // 
            // btnLookup
            // 
            this.btnLookup.Location = new System.Drawing.Point(152, 8);
            this.btnLookup.Name = "btnLookup";
            this.btnLookup.Size = new System.Drawing.Size(75, 23);
            this.btnLookup.TabIndex = 2;
            this.btnLookup.Text = "Lookup!";
            this.btnLookup.Click += new System.EventHandler(this.btnLookup_Click);
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(8, 40);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 24);
            this.label2.TabIndex = 4;
            this.label2.Text = "Credits";
            // 
            // txtCredits
            // 
            this.txtCredits.Location = new System.Drawing.Point(80, 40);
            this.txtCredits.Name = "txtCredits";
            this.txtCredits.ReadOnly = true;
            this.txtCredits.Size = new System.Drawing.Size(64, 20);
            this.txtCredits.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 112);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(56, 24);
            this.label3.TabIndex = 6;
            this.label3.Text = "Lots Avail.";
            // 
            // txtLotsAvail
            // 
            this.txtLotsAvail.Location = new System.Drawing.Point(80, 112);
            this.txtLotsAvail.Name = "txtLotsAvail";
            this.txtLotsAvail.ReadOnly = true;
            this.txtLotsAvail.Size = new System.Drawing.Size(64, 20);
            this.txtLotsAvail.TabIndex = 5;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(8, 144);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(56, 24);
            this.label4.TabIndex = 8;
            this.label4.Text = "Faction";
            // 
            // txtFaction
            // 
            this.txtFaction.Location = new System.Drawing.Point(80, 144);
            this.txtFaction.Name = "txtFaction";
            this.txtFaction.ReadOnly = true;
            this.txtFaction.Size = new System.Drawing.Size(64, 20);
            this.txtFaction.TabIndex = 7;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 72);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(56, 24);
            this.label5.TabIndex = 10;
            this.label5.Text = "Bank Credits";
            // 
            // txtBankCredits
            // 
            this.txtBankCredits.Location = new System.Drawing.Point(80, 72);
            this.txtBankCredits.Name = "txtBankCredits";
            this.txtBankCredits.ReadOnly = true;
            this.txtBankCredits.Size = new System.Drawing.Size(64, 20);
            this.txtBankCredits.TabIndex = 9;
            // 
            // btnBankUpdate
            // 
            this.btnBankUpdate.Location = new System.Drawing.Point(152, 72);
            this.btnBankUpdate.Name = "btnBankUpdate";
            this.btnBankUpdate.Size = new System.Drawing.Size(75, 23);
            this.btnBankUpdate.TabIndex = 11;
            this.btnBankUpdate.Text = "Update";
            this.btnBankUpdate.Click += new System.EventHandler(this.btnBankUpdate_Click);
            // 
            // grpAttribs
            // 
            this.grpAttribs.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.grpAttribs.Controls.Add(this.label11);
            this.grpAttribs.Controls.Add(this.txtWillpower);
            this.grpAttribs.Controls.Add(this.label10);
            this.grpAttribs.Controls.Add(this.txtStamina);
            this.grpAttribs.Controls.Add(this.label9);
            this.grpAttribs.Controls.Add(this.txtMind);
            this.grpAttribs.Controls.Add(this.label8);
            this.grpAttribs.Controls.Add(this.txtHealth);
            this.grpAttribs.Controls.Add(this.label7);
            this.grpAttribs.Controls.Add(this.txtConstitution);
            this.grpAttribs.Controls.Add(this.label6);
            this.grpAttribs.Controls.Add(this.txtAction);
            this.grpAttribs.Location = new System.Drawing.Point(524, 8);
            this.grpAttribs.Name = "grpAttribs";
            this.grpAttribs.Size = new System.Drawing.Size(176, 176);
            this.grpAttribs.TabIndex = 12;
            this.grpAttribs.TabStop = false;
            this.grpAttribs.Text = "Attributes";
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(8, 144);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(96, 23);
            this.label11.TabIndex = 11;
            this.label11.Text = "Willpower";
            // 
            // txtWillpower
            // 
            this.txtWillpower.Location = new System.Drawing.Point(104, 144);
            this.txtWillpower.Name = "txtWillpower";
            this.txtWillpower.ReadOnly = true;
            this.txtWillpower.Size = new System.Drawing.Size(56, 20);
            this.txtWillpower.TabIndex = 10;
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(8, 120);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(96, 23);
            this.label10.TabIndex = 9;
            this.label10.Text = "Stamina";
            // 
            // txtStamina
            // 
            this.txtStamina.Location = new System.Drawing.Point(104, 120);
            this.txtStamina.Name = "txtStamina";
            this.txtStamina.ReadOnly = true;
            this.txtStamina.Size = new System.Drawing.Size(56, 20);
            this.txtStamina.TabIndex = 8;
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(8, 96);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(96, 23);
            this.label9.TabIndex = 7;
            this.label9.Text = "Mind";
            // 
            // txtMind
            // 
            this.txtMind.Location = new System.Drawing.Point(104, 96);
            this.txtMind.Name = "txtMind";
            this.txtMind.ReadOnly = true;
            this.txtMind.Size = new System.Drawing.Size(56, 20);
            this.txtMind.TabIndex = 6;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(8, 72);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(96, 23);
            this.label8.TabIndex = 5;
            this.label8.Text = "Health";
            // 
            // txtHealth
            // 
            this.txtHealth.Location = new System.Drawing.Point(104, 72);
            this.txtHealth.Name = "txtHealth";
            this.txtHealth.ReadOnly = true;
            this.txtHealth.Size = new System.Drawing.Size(56, 20);
            this.txtHealth.TabIndex = 4;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(8, 48);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(96, 23);
            this.label7.TabIndex = 3;
            this.label7.Text = "Constitution";
            // 
            // txtConstitution
            // 
            this.txtConstitution.Location = new System.Drawing.Point(104, 48);
            this.txtConstitution.Name = "txtConstitution";
            this.txtConstitution.ReadOnly = true;
            this.txtConstitution.Size = new System.Drawing.Size(56, 20);
            this.txtConstitution.TabIndex = 2;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(8, 24);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(96, 23);
            this.label6.TabIndex = 1;
            this.label6.Text = "Action";
            // 
            // txtAction
            // 
            this.txtAction.Location = new System.Drawing.Point(104, 24);
            this.txtAction.Name = "txtAction";
            this.txtAction.ReadOnly = true;
            this.txtAction.Size = new System.Drawing.Size(56, 20);
            this.txtAction.TabIndex = 0;
            // 
            // txtPVP
            // 
            this.txtPVP.Location = new System.Drawing.Point(80, 176);
            this.txtPVP.Name = "txtPVP";
            this.txtPVP.ReadOnly = true;
            this.txtPVP.Size = new System.Drawing.Size(64, 20);
            this.txtPVP.TabIndex = 15;
            // 
            // cmbSkills
            // 
            this.cmbSkills.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cmbSkills.Location = new System.Drawing.Point(0, 33);
            this.cmbSkills.Name = "cmbSkills";
            this.cmbSkills.Size = new System.Drawing.Size(300, 95);
            this.cmbSkills.TabIndex = 16;
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(8, 176);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(56, 24);
            this.label12.TabIndex = 14;
            this.label12.Text = "PvP type";
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(8, 8);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(88, 24);
            this.label13.TabIndex = 17;
            this.label13.Text = "Skills";
            // 
            // lstExperience
            // 
            this.lstExperience.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lstExperience.Location = new System.Drawing.Point(0, 33);
            this.lstExperience.Name = "lstExperience";
            this.lstExperience.Size = new System.Drawing.Size(200, 95);
            this.lstExperience.TabIndex = 18;
            // 
            // label14
            // 
            this.label14.Location = new System.Drawing.Point(8, 8);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(72, 24);
            this.label14.TabIndex = 19;
            this.label14.Text = "Experience";
            // 
            // contextMenu
            // 
            this.contextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.mnuDeleteItem,
            this.ContextItemInfo,
            this.ContextMenuItemObjvars});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.Text = "Transfer";
            // 
            // mnuDeleteItem
            // 
            this.mnuDeleteItem.Index = 1;
            this.mnuDeleteItem.Text = "Delete";
            this.mnuDeleteItem.Click += new System.EventHandler(this.mnuDeleteItem_Click);
            // 
            // ContextItemInfo
            // 
            this.ContextItemInfo.Index = 2;
            this.ContextItemInfo.Text = "Info";
            // 
            // ContextMenuItemObjvars
            // 
            this.ContextMenuItemObjvars.Index = 3;
            this.ContextMenuItemObjvars.Text = "Objvars";
            this.ContextMenuItemObjvars.Click += new System.EventHandler(this.ContextMenuItemObjvars_Click);
            // 
            // lstItems
            // 
            this.lstItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lstItems.ContextMenu = this.contextMenu;
            this.lstItems.Location = new System.Drawing.Point(0, 33);
            this.lstItems.Name = "lstItems";
            this.lstItems.Size = new System.Drawing.Size(200, 95);
            this.lstItems.TabIndex = 20;
            // 
            // label15
            // 
            this.label15.Location = new System.Drawing.Point(8, 8);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(152, 24);
            this.label15.TabIndex = 21;
            this.label15.Text = "Items";
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.Controls.Add(this.panel4);
            this.panel1.Controls.Add(this.splitter2);
            this.panel1.Controls.Add(this.panel3);
            this.panel1.Controls.Add(this.splitter1);
            this.panel1.Controls.Add(this.panel2);
            this.panel1.Location = new System.Drawing.Point(0, 264);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(708, 138);
            this.panel1.TabIndex = 22;
            // 
            // panel4
            // 
            this.panel4.Controls.Add(this.cmbSkills);
            this.panel4.Controls.Add(this.label13);
            this.panel4.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel4.Location = new System.Drawing.Point(406, 0);
            this.panel4.Name = "panel4";
            this.panel4.Size = new System.Drawing.Size(302, 138);
            this.panel4.TabIndex = 4;
            // 
            // splitter2
            // 
            this.splitter2.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.splitter2.Location = new System.Drawing.Point(403, 0);
            this.splitter2.Name = "splitter2";
            this.splitter2.Size = new System.Drawing.Size(3, 138);
            this.splitter2.TabIndex = 3;
            this.splitter2.TabStop = false;
            // 
            // panel3
            // 
            this.panel3.Controls.Add(this.label15);
            this.panel3.Controls.Add(this.lstItems);
            this.panel3.Dock = System.Windows.Forms.DockStyle.Left;
            this.panel3.Location = new System.Drawing.Point(203, 0);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(200, 138);
            this.panel3.TabIndex = 2;
            // 
            // splitter1
            // 
            this.splitter1.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.splitter1.Location = new System.Drawing.Point(200, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 138);
            this.splitter1.TabIndex = 1;
            this.splitter1.TabStop = false;
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.lstExperience);
            this.panel2.Controls.Add(this.label14);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Left;
            this.panel2.Location = new System.Drawing.Point(0, 0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(200, 138);
            this.panel2.TabIndex = 0;
            // 
            // txtAccount
            // 
            this.txtAccount.Location = new System.Drawing.Point(384, 8);
            this.txtAccount.Name = "txtAccount";
            this.txtAccount.ReadOnly = true;
            this.txtAccount.Size = new System.Drawing.Size(72, 20);
            this.txtAccount.TabIndex = 23;
            // 
            // label16
            // 
            this.label16.Location = new System.Drawing.Point(328, 8);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(48, 24);
            this.label16.TabIndex = 24;
            this.label16.Text = "Account";
            // 
            // btnLookupChars
            // 
            this.btnLookupChars.Location = new System.Drawing.Point(368, 32);
            this.btnLookupChars.Name = "btnLookupChars";
            this.btnLookupChars.Size = new System.Drawing.Size(88, 23);
            this.btnLookupChars.TabIndex = 25;
            this.btnLookupChars.Text = "Lookup Chars";
            this.btnLookupChars.Click += new System.EventHandler(this.btnLookupChars_Click);
            // 
            // label17
            // 
            this.label17.Location = new System.Drawing.Point(240, 8);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(24, 24);
            this.label17.TabIndex = 27;
            this.label17.Text = "ID";
            // 
            // txtCharacterId
            // 
            this.txtCharacterId.Location = new System.Drawing.Point(264, 8);
            this.txtCharacterId.Name = "txtCharacterId";
            this.txtCharacterId.ReadOnly = true;
            this.txtCharacterId.Size = new System.Drawing.Size(56, 20);
            this.txtCharacterId.TabIndex = 26;
            // 
            // btnDeletedItems
            // 
            this.btnDeletedItems.Location = new System.Drawing.Point(368, 91);
            this.btnDeletedItems.Name = "btnDeletedItems";
            this.btnDeletedItems.Size = new System.Drawing.Size(88, 23);
            this.btnDeletedItems.TabIndex = 28;
            this.btnDeletedItems.Text = "Deleted Items";
            this.btnDeletedItems.Click += new System.EventHandler(this.btnDeletedItems_Click);
            // 
            // btnObjVars
            // 
            this.btnObjVars.Location = new System.Drawing.Point(376, 120);
            this.btnObjVars.Name = "btnObjVars";
            this.btnObjVars.Size = new System.Drawing.Size(75, 23);
            this.btnObjVars.TabIndex = 29;
            this.btnObjVars.Text = "ObjVars";
            this.btnObjVars.Click += new System.EventHandler(this.btnObjVars_Click);
            // 
            // btnDump
            // 
            this.btnDump.Location = new System.Drawing.Point(368, 61);
            this.btnDump.Name = "btnDump";
            this.btnDump.Size = new System.Drawing.Size(88, 23);
            this.btnDump.TabIndex = 30;
            this.btnDump.Text = "Get Dump Info";
            this.btnDump.Click += new System.EventHandler(this.btnDump_Click);
            // 
            // btnLookupID
            // 
            this.btnLookupID.Location = new System.Drawing.Point(92, 217);
            this.btnLookupID.Name = "btnLookupID";
            this.btnLookupID.Size = new System.Drawing.Size(75, 23);
            this.btnLookupID.TabIndex = 31;
            this.btnLookupID.Text = "lookup ID";
            this.btnLookupID.Click += new System.EventHandler(this.btnLookupID_Click);
            // 
            // cmbItems
            // 
            this.cmbItems.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Append;
            this.cmbItems.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.cmbItems.FormattingEnabled = true;
            this.cmbItems.Location = new System.Drawing.Point(200, 237);
            this.cmbItems.Name = "cmbItems";
            this.cmbItems.Size = new System.Drawing.Size(313, 21);
            this.cmbItems.TabIndex = 32;
            // 
            // btnGiveItem
            // 
            this.btnGiveItem.Location = new System.Drawing.Point(532, 235);
            this.btnGiveItem.Name = "btnGiveItem";
            this.btnGiveItem.Size = new System.Drawing.Size(75, 23);
            this.btnGiveItem.TabIndex = 33;
            this.btnGiveItem.Text = "Give Item";
            this.btnGiveItem.UseVisualStyleBackColor = true;
            this.btnGiveItem.Click += new System.EventHandler(this.btnGiveItem_Click);
            // 
            // btnLotsAdjust
            // 
            this.btnLotsAdjust.Location = new System.Drawing.Point(152, 110);
            this.btnLotsAdjust.Name = "btnLotsAdjust";
            this.btnLotsAdjust.Size = new System.Drawing.Size(75, 23);
            this.btnLotsAdjust.TabIndex = 34;
            this.btnLotsAdjust.Text = "Adjust";
            this.btnLotsAdjust.UseVisualStyleBackColor = true;
            this.btnLotsAdjust.Click += new System.EventHandler(this.btnLotsAdjust_Click);
            // 
            // btnLoadCharacter
            // 
            this.btnLoadCharacter.Location = new System.Drawing.Point(11, 217);
            this.btnLoadCharacter.Name = "btnLoadCharacter";
            this.btnLoadCharacter.Size = new System.Drawing.Size(75, 23);
            this.btnLoadCharacter.TabIndex = 35;
            this.btnLoadCharacter.Text = "Load";
            this.btnLoadCharacter.UseVisualStyleBackColor = true;
            this.btnLoadCharacter.Click += new System.EventHandler(this.btnLoadCharacter_Click);
            // 
            // PlayerTab
            // 
            this.Controls.Add(this.btnLoadCharacter);
            this.Controls.Add(this.btnLotsAdjust);
            this.Controls.Add(this.btnGiveItem);
            this.Controls.Add(this.cmbItems);
            this.Controls.Add(this.btnLookupID);
            this.Controls.Add(this.btnDump);
            this.Controls.Add(this.btnObjVars);
            this.Controls.Add(this.btnDeletedItems);
            this.Controls.Add(this.label17);
            this.Controls.Add(this.txtCharacterId);
            this.Controls.Add(this.btnLookupChars);
            this.Controls.Add(this.label16);
            this.Controls.Add(this.txtAccount);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.txtPVP);
            this.Controls.Add(this.grpAttribs);
            this.Controls.Add(this.btnBankUpdate);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.txtBankCredits);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtFaction);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txtLotsAvail);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtCredits);
            this.Controls.Add(this.btnLookup);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtPlayerName);
            this.Name = "PlayerTab";
            this.Size = new System.Drawing.Size(708, 402);
            this.grpAttribs.ResumeLayout(false);
            this.grpAttribs.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel4.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region events
		private void btnLookup_Click(object sender, System.EventArgs e)
		{
            getCharacterData();
		}

		private void btnBankUpdate_Click(object sender, System.EventArgs e)
		{
			SimpleTextForm request = new SimpleTextForm( "Modify Bank Cash", "Please enter the new bank amount", true );
			DialogResult result = request.ShowDialog();
			if( result == DialogResult.OK )
			{
				// todo:  Make sure they REALLY want to do this in case they put in a large difference.
				CsToolClass.getInstance().sendMessage( "set_bank_credits " +  txtPlayerName.Text + " " + request.getText() );
			}
            getCharacterData();
		}

		private void btnLookupChars_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "lookup_characters " + txtAccount.Text );
		}

		private void btnDeletedItems_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "get_deleted_items " + txtCharacterId.Text + " 0" );
		}

		private void btnObjVars_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "list_objvars " + txtCharacterId.Text );
		}

		private void btnDump_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "dump_info " + txtCharacterId.Text );
		}

		private void btnLookupID_Click(object sender, System.EventArgs e)
		{
			CsToolClass.getInstance().sendMessage( "get_player_id " + txtPlayerName.Text );
			clearData();
		}
        
        private void mnuDeleteItem_Click(object sender, EventArgs e)
        {
            // get the line from the window
            if (lstItems.Items.Count == 0)
                return; // no items.
            if (lstItems.SelectedIndex == -1)
                return; // nada selected.
            string line = lstItems.GetItemText(lstItems.Items[lstItems.SelectedIndex]);

            // parse out the id
            if (line.IndexOf("(") >= 0)
            {
                if (line.IndexOf(")") >= 0) // this check is probably unnecessary.
                {
                    int beginning = line.LastIndexOf("(") + 1;
                    int end = line.LastIndexOf(")");
                    line = line.Substring(beginning, end - beginning);

                    // send the objvars command.
                    CsToolClass.getInstance().sendMessage("delete_object " + line);
                    // and update.
                    CsToolClass.getInstance().sendMessage("get_pc_info " + txtPlayerName.Text);

                }
            }
            getCharacterData();
        }

		private void ContextMenuItemObjvars_Click(object sender, System.EventArgs e)
		{
			// objvars....

			// get the line from the window
			if( lstItems.Items.Count == 0 )
				return; // no items.
			if( lstItems.SelectedIndex == -1 )
				return; // nada selected.

			string line = lstItems.GetItemText( lstItems.Items[ lstItems.SelectedIndex ] );

			// parse out the id
			if( line.IndexOf( "(" ) >= 0 )
			{
				if( line.IndexOf( ")" ) >= 0 ) // this check is probably unnecessary.
				{
					int beginning = line.LastIndexOf( "(" ) + 1;
					int end = line.LastIndexOf( ")" );
					line = line.Substring( beginning, end - beginning );

					// send the objvars command.
					CsToolClass.getInstance().sendMessage( "list_objvars " + line );
				}
			}
        }

        private void btnGiveItem_Click(object sender, EventArgs e)
        {
            CsToolClass.getInstance().sendMessage("create_crafted_object " + txtCharacterId.Text + " " + cmbItems.Text);
            getCharacterData();
        }

        private void btnLotsAdjust_Click(object sender, EventArgs e)
        {
            SimpleTextForm request = new SimpleTextForm("Adjust lots", "Please enter the amount to adjust lots by (relative value)", true);
            DialogResult result = request.ShowDialog();
            if (result == DialogResult.OK)
            {
                // todo:  Make sure they REALLY want to do this in case they put in a large difference.
                CsToolClass.getInstance().sendMessage("adjust_lots " + txtCharacterId.Text + " " + request.getText());
            }
            getCharacterData();
        }

        private void btnLoadCharacter_Click(object sender, EventArgs e)
        {
            if (txtAccount.Text != "" && txtCharacterId.Text != "")
            {
                CsToolClass.getInstance().sendMessage("login_character " +
                                                      txtCharacterId.Text + " " +
                                                      txtAccount.Text);
                getCharacterData();
            }
        }

		#endregion

		#region members

        private System.Collections.Generic.List<string> m_nonDisplayItems;
        DeletedItemsForm m_deletedItemsForm;
		#endregion
	}
}

