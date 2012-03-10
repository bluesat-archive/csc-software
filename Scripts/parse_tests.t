#!perl

use strict;
use warnings;
use Test::More 'no_plan';
my @subs = qw (parse_file generate_summary);
use_ok( 'parse_tests',@subs) or exit;

my $input =<<MOO_SQUID;
Running test: test_fake_app.c.exe
...........................F.....

There was 1 failure:
1) TestCuStringNew: ../BLUEsat-CSC/Applications/Fake_app/test/test_fake_app.c:50: expected <1> but was <>

!!!FAILURES!!!
Runs: 33 Passes: 32 Fails: 1


MOO_SQUID

my $expected_hash = {
                     total => 33,
                     pass  => 32,
                     fail  => 1,
                     errors => ['TestCuStringNew: ../BLUEsat-CSC/Applications/Fake_app/test/test_fake_app.c:50: expected <1> but was <>']
                     };

my $expected_key = 'test_fake_app.c.exe';                    

my ($out_key, $out_hash) = parse_tests::process_test($input);

is_deeply ($out_hash, $expected_hash, 'process_test: See if hash is generated correctly.');
ok ($out_key eq $expected_key,'process_test: Verify that key was extracted correctly' );

$input =<<MOO_SQUID;
Running test: test_fake_app.c.exe
...........................F.....

There was 1 failure:
1) TestCuStringNew: ../BLUEsat-CSC/Applications/Fake_app/test/test_fake_app.c:50: expected <1> but was <>

!!!FAILURES!!!
Runs: 33 Passes: 32 Fails: 1


<Next Test>
Running test: test_fake_app2.c.exe
.................................

OK (33 tests)


<Next Test>

MOO_SQUID
$expected_hash = {
          'test_fake_app2.c.exe' => {
                                      'pass' => '33',
                                      'errors' => [],
                                      'fail' => 0,
                                      'total' => '33'
                                    },
          'test_fake_app.c.exe' => {
                                     'pass' => '32',
                                     'errors' => [
                                                   'TestCuStringNew: ../BLUEsat-CSC/Applications/Fake_app/test/test_fake_app.c:50: expected <1> but was <>'
                                                 ],
                                     'fail' => '1',
                                     'total' => '33'
                                   }
        }; 
my $input_file='testin.txt';
open (my $fh,'>',$input_file);
print $fh $input;
close $fh;
parse_file($input_file);
$out_hash = parse_tests::get_results_hash();
is_deeply($out_hash, $expected_hash, 'parse_file: Parse results file and verify that all expected data was collected.');

unlink $input_file

