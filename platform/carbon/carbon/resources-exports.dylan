module: dylan-user

/*
	resources
*/

define module resources

	use Dylan;
	//use Extensions;						// <extended-integer>
	use melange-support;
	use mac-types;
	use memory;
	
	export	// Resource Manager.
		GetResource, ReleaseResource;
		
end module resources;








