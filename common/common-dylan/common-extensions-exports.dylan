module: dylan-user

define library common-extensions
  use dylan;
  use melange-support;
  use format;
  use standard-io;

  // Only import transcendentals if we have them.
#if (~compiled-for-solaris)
  use transcendental,
     import: { transcendental => transcendentals },
     export: all;
#endif

  export
    common-extensions,
    finalization,
    simple-io,
    simple-random,
    simple-profiling,
    simple-debugging,
    byte-vectors,
    machine-words;
end library;

define module c-support
  use dylan;
  use extensions;
  use melange-support;

  export
    application-argc,
    application-argv;
end module c-support;

define module common-extensions
  use dylan;
  use extensions,
    exclude: { assert },
    rename: {$not-supplied => $unsupplied,
	     on-exit => register-exit-application-function},
    export: {$unsupplied,
             integer-length,
	     false-or,
	     one-of,
	     subclass,
	     <format-string-condition>, <simple-condition>,
	     ignore,
	     key-exists?,
	     difference,
	     register-exit-application-function,
	     <stretchy-sequence>,
	     <object-deque>,
	     <stretchy-object-vector>,
             <byte-character>};
  use %Hash-Tables,
    export: {remove-all-keys!};
  use c-support;
  use format;
  use standard-io;

  export
    /* Numerics */
    //integer-length,

    /* Unsupplied, unfound */
    //$unsupplied,
    supplied?,
    unsupplied?,
    unsupplied,
    $unfound,
    found?,
    unfound?,
    unfound,

    /* Collections */
    //<object-deque>,
    //<stretchy-sequence>,
    //<stretchy-object-vector>,
    concatenate!,
    position,
    //remove-all-keys!,
    //difference,
    fill-table!,
    find-element,
    //key-exists?,

    /* Conditions */
    //<format-string-condition>,
    condition-to-string,

    /* Debugging */
    \assert,
    debug-message,
    \debug-assert,

    /* Types */
    //false-or,
    //one-of,
    //subclass,

    /* Ignoring */
    //ignore,
    ignorable,

    /* Converting to and from numbers */
    float-to-string,
    string-to-float,
    integer-to-string,
    string-to-integer,
    // Not part of common? number-to-string,

    /* Appliation runtime environment */
    application-name,
    application-filename,
    application-arguments,
    exit-application;
    //register-exit-application-function,

#if (~mindy)
  export
    \table-definer,
    \iterate,
    \when;

  export
    \%iterate-aux,
    \%iterate-param-helper,
    \%iterate-value-helper;
#endif
end module;

define module finalization
  // XXX - Needs definition. No-op stubs OK.
end module;

define module simple-io
  // XXX - Needs definition.
end module;

define module simple-random
  // XXX - Needs definition.
end module;

define module simple-profiling
  // XXX - Needs definition.
end module;

define module simple-debugging
  // XXX - Needs definition.
end module;

define module byte-vectors
  use extensions,
    export: {<byte>,
	     <byte-vector>};
end module;

define module machine-words
  // XXX - Needs definition.
end module;

/*
  Stream protocol.
*/

/*
  // Locators.
  export
    <locator>,
    supports-open-locator?,
    open-locator,
    supports-list-locator?,
    list-locator;
*/
