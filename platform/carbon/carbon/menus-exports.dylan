module: dylan-user

/*
	menus
*/

define module menus

	use Dylan;
	//use Extensions;						// <extended-integer>
	use melange-support;
	use mac-types;
	use memory;
	use quickdraw;
	
	export	// Menu Manager.
		<MenuBarHandle>, <MenuHandle>, <MenuRef>,
		GetNewMBar, SetMenuBar, DrawMenuBar, HiliteMenu,
		MenuSelect, MenuKey,
		GetMenuHandle, GetMenuItemText, EnableMenuItem, DisableMenuItem, CountMenuItems,
		DeleteMenu,
		AppendResMenu,
    GetMBarHeight;
		
		// Desk Accessories.
		//OpenDeskAcc;
		
end module menus;








