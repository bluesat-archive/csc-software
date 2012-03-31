#!/usr/bin/perl

use strict;
use warnings;  
use compiler_exclusion qw(process);


my $argcnt = $#ARGV + 1;
usage() unless $argcnt == 3;


my $exc_file   = dir_slash_fix( shift @ARGV);  
my $build_dir  = dir_slash_fix( shift @ARGV);
my $out_file   = dir_slash_fix( shift @ARGV);


process($exc_file,$build_dir,$out_file);
exit 0;

sub dir_slash_fix
{
   my $temp = shift;
   $temp =~s/\\/\//g;
   return $temp;
}

sub usage
{
   print <<'MOO_SQUID';
Error: Misformed input variables.

Example: perl pl preprocess.pl '..\BLUEsat-CSC\SysBootAgent\include\AppExcList.xml' '..\Builds'  '..\BLUEsat-CSC\SysBootAgent\include\ActiveModuleIncList.h sysBootAgent.i'


MOO_SQUID
   exit 0;
}

