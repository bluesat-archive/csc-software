package parse_tests;

use strict;
use Exporter;
use Data::Dumper;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);
use constant {
        NEXT_TEST_TAG   => '<Next Test>'
    };
$VERSION     = 1.00;
@ISA         = qw(Exporter);
@EXPORT      = ();
@EXPORT_OK   = qw( parse_file generate_summary);


my $allResults = {};

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

sub process_test
{
   my $test = shift;
   my $file = ($test =~ /Running test: (.*)\s/)?$1:'';
   return unless ($file ne '');
   my ($tot, $pass, $fail);
   if ($test =~/Runs: (\d+) Passes: (\d+) Fails: (\d+)/)
   {
      ($tot, $pass, $fail) = ($1,$2,$3);
   }
   if ($test=~/OK \((\d+) tests?\)/)
   {
      ($tot, $pass, $fail) = ($1,$1,0);
   }
   my @errors;
   while ($test =~ /\d+\)\s*(.*)\n/g) 
   { 
      push (@errors, $1);
   }
   my $entry = {
                  total => $tot,
                  pass  => $pass,
                  fail  => $fail,
                  errors => [@errors]
               };
               
   return ($file, $entry);
}

sub parse_file 
{
   my $filename = shift;
   fatal_error("File: $filename not found. Please provide fully qualified file location.") unless (-e $filename);
   open (my $fh, '<', $filename) or fatal_error("Unable to read file. Error: $!");
   my $holdTerminator = $/;
   undef $/;
   my $tests = <$fh>;
   $/ = $holdTerminator;
   close $fh;
   foreach my $test (split(NEXT_TEST_TAG, $tests))
   {
      my ($key, $value) = process_test($test);
      if ($key=~/\w+/)
      {
         $allResults->{$key}= $value ; 
      }
   }
}

sub get_results_hash
{
   return $allResults;
}

sub generate_summary
{
my $result =<<MOO_SQUID;
Bluesat Test Summary
--------------------
Total Tests Run   : TOTALTESTS
Total Tests Failed: TOTALFAILED
Total Tests Passed: TOTALPASSED

Suites Run
----------
SUITESRUN

MOO_SQUID

my $resultDetails =<<MOO_SQUID;   
Tests Failed
------------
TESTSFAILED

MOO_SQUID

   my ($totTest, $totPass, $totFail, $suites, $failed);
   foreach my $tempKey (keys %$allResults)
   {
      my $tempTest = $allResults->{$tempKey};
      $totTest += $tempTest->{'total'};
      $totPass += $tempTest->{'pass'};
      $totFail += $tempTest->{'fail'};
      $suites  .= " - ".$tempKey."\n";
      $failed  .= ($tempTest->{'fail'}>0)? " - ".join("\n - ",@{$tempTest->{'errors'}})."\n":'';
   }
   $result =~s/TOTALTESTS/$totTest/;
   $result =~s/TOTALFAILED/$totFail/;
   $result =~s/TOTALPASSED/$totPass/;
   $result =~s/SUITESRUN/$suites/;
   if ($totFail>0)
   {
      $result .= $resultDetails;
      $result =~s/TESTSFAILED/$failed/;
   }
  
   print $result;
}


1;