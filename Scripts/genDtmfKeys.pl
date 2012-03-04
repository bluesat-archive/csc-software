#!/usr/bin/perl

print "/*\n\tEncryption keys for XXTEA encryption used in DTMF transmission\n";
print "\tGenerated using random number perl program\n*/\n\n";
print "long dtmfEncryptionKeys[64][4]={\n";

srand();

foreach $i (1..64) {
	print "\t{";
	foreach $j (1..4) {
		my $r=int(rand(2147483647))-int(rand(2147483648));
		print "$r";
		print "," if $j<4;
	}
	print "}";
	print "," if $i<64;
	print "\n";
}
print "};\n";
