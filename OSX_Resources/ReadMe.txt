

$Id: README $

This is the OPeNDAP Gateway Module. It is used along with Hyrax.

For information about building the OPeNDAP Gateway Module, see theINSTALL file.

This module is a component of the OPeNDAP DAP Server; the server basesoftware is designed to allow any number of handlers to be configured easily.See the DAP Server README and INSTALL files for information aboutconfiguration, including how to use this handler.

Special options supported by the handler: * ShowSharedDimensions: Include shared dimensions as separate variables. Accepted values: true,yes|false,no, defaults to false.Example: "NC.ShowSharedDimensions=false"

To use this, make sure that you first install the bes, and thatdap-server gets installed too.  Finally, every time you install orreinstall handlers, make sure to restart the BES and OLFS.

