using System;
using System.Collections;
using System.Collections.Specialized;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms;

namespace SwgCsTool
{
	/// <summary>
	/// Base class for user tabs.  Provides interface and base functionality for
	/// actual working tabs for the application.
	/// </summary>
	public class BaseTab : System.Windows.Forms.UserControl
	{
		#region automembers
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		#region constructors
		// make sure in all derived controls that the controls are properly
		// associated with their fields, if you plan on using the autopopulate
		// functionality.  While it's only necessary to have this done before
		// text is received, the constructor is the most appropriate place to put this.
		public BaseTab()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			m_controlMap = new HybridDictionary();
            m_controlRequirements = new Dictionary<string, List<Control>>();
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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			components = new System.ComponentModel.Container();
		}
		#endregion

		#region public interface
		/// <summary>
		/// Take a string (possibly multiline), and parse it into a type field (the first line)
		/// and key/value pairs based on the rest of the line.
		/// </summary>
		/// <param name="response">the response that was sent to the CS Tool</param>
		public void parse( string response )
		{
			string[] data = response.Split( '\n' );
			StringDictionary dict = new StringDictionary();
			string type = "";
			if( data.Length > 0 )
			{
				type = data[ 0 ];
				type.Trim();
				for( int i = 1; i < data.Length; ++i )
				{
					string[] vals = data[ i ].Split( ':' );
					if( vals.Length >= 2 ) // it should!
					{
						// undestroy any extra :s.
						vals[ 1 ] = System.String.Join( ":", vals, 1, vals.Length - 1 );
						vals[ 1 ] = vals[ 1 ].Trim(); // we should have a space at the beginning, and a cr at the end
						dict.Add( vals[ 0 ], vals[ 1 ] );
					}
				}
				handleResponse( type, dict );
			}
		}

        /// <summary>
        /// set the requirements for a control to be enabled or not.
        /// </summary>
        /// <param name="requirement">The name of the text box that must not be null, or "oneserver" to require that exactly one server be selected for the control to work.</param>
        /// <param name="toEnable">The control to enable/disable</param>
        public void activationRequirement( string requirement, Control toEnable )
        {
            List<Control> controlList;
            if(m_controlRequirements.ContainsKey(requirement))
            {
                controlList = m_controlRequirements[requirement];
            }
            else
            {
                controlList = new List<Control>();                
            }
            if (!controlList.Contains(toEnable))
                controlList.Add(toEnable);
            m_controlRequirements[requirement] = controlList;
        }

        /// <summary>
        /// this is called after every message to the tab, to give the tab a chance to properly update
        /// the enabled/disabled status of all of its controls
        /// </summary>
        virtual public void updateEnabledControls()
        {
            // create a dictionary of controls to their new state.  Since a control
            // can have multiple requirements to be activated, we use this to track
            // each one as we iterate over the lists.
            Dictionary<Control, bool> controls = new Dictionary<Control, bool>();

            foreach(string key in m_controlRequirements.Keys)
            {
                List<Control>currentList = m_controlRequirements[key];
                
                // If the list associated with this key is null, there's no
                // need to continue, as there's no controls to modify.
                // This shouldn't happen.
                if (currentList == null)
                    continue;
                
                // determine the new state for our controls
                bool enable = false;
                // special case for dealing with being connected to one server.
                if(key=="oneserver")
                {
                    enable = ((MainForm)this.ParentForm).getNumberOfSelectedServers() == 1 ? true : false;
                }
                else // generic case for anything matching by name.
                {
                    Control[] current = this.Controls.Find(key, true);
                    foreach (Control matchingControl in current)
                    {
                        // check for exact matches only.. I'm not sure if
                        // this is necessary.  The documentation
                        // is a little unclear on whether or not doing a find
                        // on "partial" will pick up "partial" as well as
                        // "partialName"
                        if (matchingControl.Name == key && matchingControl.Text != "")
                            enable = true;
                    }
                }

                // a control can have multiple requirements for it to be activated.
                // update our master dictionary with the info from this requirement.
                foreach (Control oneControl in currentList)
                {
                    if(controls.ContainsKey(oneControl))
                    {
                        // there's a requirement we've already checked,
                        // so the new state should be the previous
                        // state in the dictionary && the new one.
                        controls[oneControl] = controls[oneControl] && enable;
                    }
                    else
                    {
                        // add to the dictionary.
                        controls[oneControl] = enable;
                    }
                }

            }
            // iterate through the dictionary, setting the enabled status properly.
            foreach(Control oneControl in controls.Keys)
            {
                oneControl.Enabled = controls[oneControl];
            }
        }

		/// <summary>
		/// default behavior here is to pass the message along to the parser.
		/// if the tab needs to do pre-processing, then override this, and either
		/// manually call the parser or the parent (safer!) when you want default
		/// handling to occur.
		/// </summary>
		/// <param name="response">response sent to the CSTool.</param>
		virtual public void handleResponse( string response )
		{
			parse( response );

		}
	
		/// <summary>
		/// autopopulation of fields based on the map.  Override this at the 
		/// instantiated class level to make sure that you only do this for
		/// the messages you want it done for.  Just call the parent's handleResponse()
		/// call when you want to autopopulate.
		/// </summary>
		/// <param name="type">the first line of received data</param>
		/// <param name="data">a StringDictionary containing the key/value pairs</param>
		virtual public void handleResponse( string type, StringDictionary data )
		{
			// ignore the type, we don't care.  We assume that if we're here, we
			// should be doing this
			foreach( DictionaryEntry entry in data )
			{
				string key = entry.Key.ToString();
				string value = entry.Value.ToString();
				
				// find the control.
				// If it exists...
				if( m_controlMap.Contains( key ) )
				{
					// populate it with our new data.
					System.Windows.Forms.Control control = m_controlMap[ key ] as Control;
					if( control != null )
					{
						control.Text = value;
					}
				}
				else
				{
					// if the raw key wasn't there, search for the key without a -xxx extension.
					// in those cases, see if we have a listbox that wants the input, and if so,
					// insert the item into the listbox.

					// TODO:  Add other types if necessary.
					if( key.IndexOf( "-" ) > -1 )
					{
						string subkey = key.Substring( 0, key.IndexOf( "-" ) );
						if( m_controlMap.Contains( subkey ) )
						{
							ListBox control = m_controlMap[ subkey ] as ListBox;
							if( control != null )
							{
								control.Items.Add( value );
							}
						}
					}
				}				
			}
		}

        /// <summary>
        /// Displays an objvar form when we get an objvar response.
        /// While this isn't really generic functionality, it's relatively
        /// common in multiple situations
        /// </summary>
        /// <param name="response">the obvars sent for parsing</param>
        public void handleObjvarList(string response)
        {
            // If we already have a form, it means that we're updating it.
            // In that case, we don't want to keep making more forms, as
            // that would look really, really bad.
            bool bShow = false;
            // build the form
            if (m_objvarform == null)
            {
                bShow = true;
                m_objvarform = new ObjVarsForm();
            }

            // get the item number.
            int end = response.IndexOf("\r");

            //TODO:  Add const for the actual text, instead of magic number
            string string_id = response.Substring(16, end - 16);


            m_objvarform.populate(string_id, response);

            // display it.
            if (bShow)
            {
                m_objvarform.ShowDialog();
                m_objvarform = null;
            }
        }

		/// <summary>
		/// associates a control with a key.  use this to autopopulate tab pages.
		/// NOTE: keys should be in lower case.
		/// </summary>
		/// <param name="key">LOWER CASE version of the text you expect as the key</param>
		/// <param name="control">the control that should be populated</param>
		public void associate( string key, Control control )
		{
			m_controlMap[ key ] = control;
		}
		#endregion

		#region members
		/// <summary>
		/// map of keys to controls that they care about.  Used by autopopulation system.
		/// </summary>
		protected HybridDictionary m_controlMap;
        protected ObjVarsForm m_objvarform;

        protected Dictionary<string, List<Control>> m_controlRequirements;
		#endregion
	}
}
