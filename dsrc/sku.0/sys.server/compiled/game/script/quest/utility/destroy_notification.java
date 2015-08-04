package script.quest.utility;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


public class destroy_notification extends script.base_script
{
	public destroy_notification()
	{
	}
	
	public int addNotification(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("newquests", "destroy_notification - addNotification()");
		if (params != null)
		{
			obj_id target = params.getObjId("target");
			LOG("newquests", "destroy_notification - addNotification() - target="+ target);
			if (target != null)
			{
				Vector notificationTargets = null;
				if (hasObjVar(self, "destroy_notification_targets"))
				{
					notificationTargets = getResizeableObjIdArrayObjVar(self, "destroy_notification_targets");
				}
				else
				{
					notificationTargets = new Vector();
				}
				
				if (notificationTargets != null)
				{
					notificationTargets.add(target);
					setObjVar(self, "destroy_notification_targets", notificationTargets);
					LOG("newquests", "destroy_notification - addNotification() - added "+ target + " to notification obj_id array");
					String parameter = params.getString("parameter");
					LOG("newquests", "destroy_notification - addNotification() - parameter="+ parameter);
					if (parameter != null && parameter.length() > 0)
					{
						String objvarname = target.toString();
						setObjVar(self, objvarname, parameter);
						LOG("newquests", "destroy_notification - addNotification() - set objvar "+ objvarname + " = "+ parameter);
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void sendNotifications(obj_id self) throws InterruptedException
	{
		LOG("newquests", "destroy_notification - sendNotifications()");
		if (hasObjVar(self, "destroy_notification_targets"))
		{
			Vector notificationTargets = getResizeableObjIdArrayObjVar(self, "destroy_notification_targets");
			int iter = 0;
			for (iter = 0; iter < notificationTargets.size(); ++iter)
			{
				testAbortScript();
				dictionary destroyNotificationParameters = new dictionary();
				destroyNotificationParameters.put("source", self);
				LOG("newquests", "destroy_notification - addNotification() - putting "+ self + " into dictionary");
				String objvarname = ((obj_id)(notificationTargets.get(iter))).toString();
				if (hasObjVar(self, objvarname))
				{
					String parameter = getStringObjVar(self, objvarname);
					destroyNotificationParameters.put("parameter", parameter);
					LOG("newquests", "destroy_notification - addNotification() - putting "+ parameter + " into dictionary");
				}
				else
				{
					LOG("newquests", "destroy_notification - addNotification() - could not retrieve parameter from objvar "+ self);
				}
				LOG("newquests", "destroy_notification - addNotification() - sending notification to "+ ((obj_id)(notificationTargets.get(iter))));
				messageTo(((obj_id)(notificationTargets.get(iter))), "destroyNotification", destroyNotificationParameters, 0.0f, false);
			}
		}
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		LOG("newquests", "destroy_notification - OnIncapacitated()");
		sendNotifications(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		LOG("newquests", "destroy_notification - OnDestroy()");
		sendNotifications(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDisabled(obj_id self, obj_id killer) throws InterruptedException
	{
		LOG("newquests", "destroy_notification: OnObjectDisabled()");
		sendNotifications(self);
		return SCRIPT_CONTINUE;
	}
}
