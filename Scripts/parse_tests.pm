package parse_tests;

use strict;
use Exporter;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);

$VERSION     = 1.00;
@ISA         = qw(Exporter);
@EXPORT      = ();
@EXPORT_OK   = qw( parse_file);


sub fatal_error
{
   my $msg = shift;
   
   print <<MOO_SQUID;
Fatal Error
-----------
$msg
MOO_SQUID
   exit 1;
}


sub parse_file 
{
   my $filename = shift;
   fatal_error("File: $filename not found. Please provide fully qualified file location.") unless (-e $filename);
   
}


1;