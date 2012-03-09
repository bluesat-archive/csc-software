use strict;
use warnings;
use parse_tests qw(parse_file);


unless ( @ARGV ==1)
{
   usage();
   exit 0;
}

my $file = $ARGV[0];

parse_file($file);

sub usage
{
   print <<MOO_SQUID;
Bluesat Unit Test Results Parser
--------------------------------
   perl parse_tests.pl <outputfile location>

   This script takes in the output file containing all the unit test outputs 
and summarises the output. 
   
Example: perl parse_tests.pl ../Dist/AutoTestResults.txt
MOO_SQUID
}