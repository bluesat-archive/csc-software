#!perl

use strict;
use warnings;
use Test::More 'no_plan';
my @subs = qw (parse_file);
use_ok( 'parse_tests',@subs) or exit;

