using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Collections.Specialized;
using System.Text.RegularExpressions;

namespace SwgCsTool
{
    /// <summary>
    /// Tab to view and query structure data.
    /// </summary>
    public partial class StructuresListTab : BaseTab
    {
        #region constructors
        public StructuresListTab()
        {
            InitializeComponent();
            associate("character id", txtCharacterId);

            activationRequirement("oneserver", txtCharacterName);
            activationRequirement("oneserver", btnGetStructures);
            activationRequirement("txtCharacterId", btnGetStructures);
        }
        #endregion

        #region override methods

        // we need to use the raw override for when we get
        // an objvar list message
        public override void handleResponse(string response)
        {
            if (response.StartsWith("Objvar list"))
            {
                handleObjvarList(response);
            }
            base.handleResponse(response);
        }

        // handle a structure list message
        public override void handleResponse(string type, StringDictionary data)
        {

            // use this override, since we're expecting the data to be properly formatted
            if (type.StartsWith("structurelist"))
            {
                if (data["character_id"] != txtCharacterId.Text)
                    return; // bad character id.
                handleStructureList(data);
            }
            base.handleResponse(type, data);
        }

        #endregion

        #region events
        // get the character ID
        private void btnLookup_Click(object sender, EventArgs e)
        {
            txtCharacterId.Clear();
            if( txtCharacterName.Text != "" )
                CsToolClass.getInstance().sendMessage("get_player_id " + txtCharacterName.Text);
        }

        // retrieve our list of structures
        private void btnGetStructures_Click(object sender, EventArgs e)
        {
            if( txtCharacterId.Text != "" )
                CsToolClass.getInstance().sendMessage("list_structures " + txtCharacterId.Text);
        }

        private void mnuViewObjvars_Click(object sender, EventArgs e)
        {
            if (grdItems.SelectedRows.Count != 1)
                return; // one at a time
            string id = ((StructureData)grdItems.SelectedRows[0].DataBoundItem).Id;
            CsToolClass.getInstance().sendMessage("list_objvars " + id);
        }

        #endregion

        #region protected interface
        protected void handleStructureList( StringDictionary data)
        {
            List<StructureData> structureList = new List<StructureData>();

            foreach (String key in data.Keys)
            {
                // only add actual structures
                if (!key.StartsWith("structure"))
                    continue;

                StructureData item = new StructureData( data[key] );
                if (item.Id != "" && !(item.deleted && chkHideDeleted.Checked ))
                    structureList.Add(item);
            }
            grdItems.DataSource = structureList;

        }
        #endregion
    }

    /// <summary>
    /// Data class to hold information on a structure.  Used to populate the
    /// data grid.
    /// </summary>
    public class StructureData
    {
        #region constructor
        public StructureData(string rawData)
        {
            if (m_regex == null)
            {
                m_regex = new Regex(@"\((.*)\)(.*) (.*)\((.*)\)(\d+)");
            }
            System.Text.RegularExpressions.Match match = m_regex.Match(rawData);
            if (match.Success)
            {
                Id = match.Groups[1].Value;
                template = match.Groups[2].Value;
                scene = match.Groups[3].Value;
                location = match.Groups[4].Value;
                deleted = match.Groups[5].Value == "0" ? false : true;
            }
        }
        #endregion

        #region properties
        private bool m_deleted;

        /// <summary>
        /// is this a deleted structure?
        /// </summary>
        public bool deleted
        {
            get { return m_deleted; }
            set { m_deleted = value; }
        }

        /// <summary>
        /// the coordinates of the structure
        /// </summary>
        public string location
        {
            get { return m_location; }
            set { m_location = value; }
        }

        /// <summary>
        /// The ID of the structure... we store this as string since we
        /// don't actually need it numerically for anything
        /// </summary>
        public string Id
        {
            get { return m_id; }
            set { m_id = value; }
        }

        /// <summary>
        /// template name of the structure
        /// </summary>
        public string template
        {
            get { return m_template; }
            set { m_template = value; }
        }

        /// <summary>
        /// the scene(planet) that this structure is on.
        /// </summary>
        public string scene
        {
            get { return m_scene; }
            set { m_scene = value; }
        }
        #endregion

        #region private members
        /// <summary>
        /// the regular expression to parse out data.
        /// </summary>
        private static Regex m_regex;

        private string m_location;
        private string m_id;
        private string m_template;
        private string m_scene;
        #endregion
    };
}
