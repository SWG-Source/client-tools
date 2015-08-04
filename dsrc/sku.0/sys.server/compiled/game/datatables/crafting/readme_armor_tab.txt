Most of the data for the armor.tab file is self-explanitory, if you know how the armor system works. However, there are some settings that may need further explanation:


***************************************************************

Columns attrib_inc, attrib_dec:

	The damage type protections modified by armor layers. These only apply to layer rows. If you add a new damage type the armor.scriptlib SID_SPECIAL_PROTECTIONS and DATATABLE_SPECIAL_PROTECTIONS arrays must have that damage type name added to them.


***************************************************************

Columns resource_X_percent:

	These are used to define how much of the attribute the column represents comes from resources, with the remainder coming from components. The reason this is needed is because the attribute ranges are defined in the draft schematics as being between 0 and 1. This means that just with resources the crafter can make a perfect item, and any components that contribute to the attribute will make it even easier to make a perfect item. An example:

	The min/max for the encumbrance of segments is 30/85. However, segments can have up to 4 layers, each of which may contribute up to 8 encumbrance. So a crafter only needs resources that contribute 23 [(85 - 30) - 8 * 4] encumbrance, or 42% [23 / (85 - 30)], to make a perfect item (please ignore that low encumbrance is better for now). This means that in order to make sure the crafter can only make a perfect item with perfect resources, we need to multiply the encumbrance amount contributed by the resource by 42%. So the resource_encumbrance_percent entry for the segment row should be set to 42.


***************************************************************

Rows layer#:

	We define armor special protection by which layers the armor has, instead of per damage type. For each special protection combination needed, there needs to be a new layer# entry. Feel free to add new layers to the table. If the layer is for a special purpose (such as layer13, used for PSGs), please note it in the comments column.


***************************************************************

Row layer_reference:

	This is used internally by the armor.scriptlib, and in general shouldn't need to be modified. Its data should only be changed if the min/max values of one of the layer# rows is smaller/larger than the values in the layer_reference row. Ex: If the max_special_protection value of layer7 was changed to 200, the max_special_protection value of the layer_reference row should be changed to be >= 200. Please do not just set the values to a really large/negative number, in order avoid rounding errors.


***************************************************************

Rows core#, final#, psg#:

	The # in these rows refers to the basic(0), standard(1), and advanced(2) models of these armor types. Due to the way the armor system works, there must always be the same number of core and final entries.


