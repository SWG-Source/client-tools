package script;

import java.io.Serializable;

/**
* variables
*
* Full Names (first + last)
*
* %TU           user
* %TT           target
* %TO           other object
*
* %NU/NT/NO     short name (first name only)
*
* %SU/ST/SO     personal subjective pronoun:    he/she/it
* %OU/OT/OO     personal objective pronoun:     him/her/it
* %PU/PT/PO     possessive pronoun :            his/her/its
* %FU/FT/FO     associated faction
* %RU/RT/RO     associated species
* %HU/HT/HO     honorific title (sir, madame, lady, etc)
* %MU/MT/MO     military rank (if any) (private, commander, captain)
* %KU/KT/KO     title (if any) (master weaponsmith, journeyman chef, medical assistant)
* %DI           digit integer
* %DF           digit float
*
*/

/**
* Provide encapsulation for data involved in a ProsePackage
*/

final public class prose_package implements Serializable
{
	private final static long serialVersionUID = -3334262904982229749L;

	public prose_package ()
	{
		complexGrammar = false;
	}
	public prose_package (boolean _complexGrammar)
	{
		complexGrammar = _complexGrammar;
	}

	/**
	* A participant's data members take the following precedence when displaying:
	*
	* id
	* nameId
	* name
	*
	*/

	final public static class participant_info implements Serializable
	{
		private final static long serialVersionUID = -3334262904982229749L;
		public obj_id    id;
		public string_id nameId;
		public String    name;

		public void set (obj_id _id)
		{
			String    _name   = base_class.getAssignedName (_id);
			string_id _nameId = base_class.getNameStringId  (_id);

			set (_id, _nameId, _name);
		}

		public void set (string_id _nameId)
		{
			set (null, _nameId, null);
		}

		public void set (String _name)
		{
			set (null, null, _name);
		}

		public void set (obj_id _id, string_id _nameId, String _name)
		{
			id     = _id;
			nameId = _nameId;
			name   = _name;
		}
	};

	public string_id stringId            = new string_id ();

	final public participant_info actor  = new participant_info ();
	final public participant_info target = new participant_info ();
	final public participant_info other  = new participant_info ();

	public int       digitInteger;
	public float     digitFloat;
	public boolean   complexGrammar;
};
