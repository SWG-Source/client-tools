
/**************************************************************************
 *
 * Java-side Datatable writing. 
 *
 * copyright 2001 Sony Online Entertainment
 *
 *
 *************************************************************************/

package script;

import script.location;
import script.obj_id;
import java.io.*;;


public class datatable_writer
{
/*
public static class combatant_data
	{
		public obj_id   id;                 // the combatant's id
		public location pos;				// where the combatant is in local coordinates
		public location worldPos;			// where the combatant is in world-space coordinates
		public float    radius;			    // the bounding-radius of the combatant
		public boolean  isCreature;			// flag that the combatant is a creature
		public int      posture;			// the combatant's posture (if it is a creature)
		public int      locomotion;         // the combatant's locomotion (if it is a creature)
		public int      scriptMod = 0;		// generic param that will be added to the combatant's roll
	} EXAMPLE
*/
	public static String makeDataTable(String strFileName, String strHeader, String strHeaderTypes)
	{
		try
		{
			FileWriter dataTable = new FileWriter(strFileName);
			dataTable.write(strHeader);
			dataTable.write(strHeaderTypes);			
			dataTable.close();
			
		}
		catch (IOException e)
		{
			System.out.println("Error is "+e.getMessage());
			System.out.println("Error, file passed in was "+strFileName+"\n");
			System.out.println("Header is "+strHeader+"\n");
			System.out.println("headerTypes is "+strHeaderTypes+"\n");
			return null;
		}	
		return strFileName;

	}


}
