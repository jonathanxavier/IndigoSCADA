#!/usr/bin/perl
# Copyright 2009
# Kaz Kylheku <kkylheku@gmail.com>
# Vancouver, Canada
# All rights reserved.
#
# BSD License:
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
#   3. The name of the author may not be used to endorse or promote
#      products derived from this software without specific prior
#      written permission.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# This is a program whose output can be piped to the test drivers for
# hash.c and dict.c. It inserts a bunch of data and then deletes it all.
#
# The $modulus should be a prime number. This ensures that the $modulus - 1
# generated keys are all distinct.  The $factor_i and $factor_d values need not
# be prime, but it should not be a multiple of $modulus (including zero),
# otherwise a sequence of duplicate keys will be generated: choose numbers
# in the range [1, $modulus - 1]. Choosing 1 means that
# insertions (or deletions) will take place in order.
# The purpose of using the prime modulus number is to generate a repeatable
# sequence of unique keys that is (possibly) not in sorted order.
#

# $modulus = 200003;
# $factor_i = 100;
# $factor_d = 301;

$modulus = 6113;
$factor_i = 1669;
$factor_d = 2036;

for ($i = 1; $i < $modulus; $i++) {
    printf("a %d %d\n", ($i * $factor_i) % $modulus, $i);
}

for ($i = 1; $i < $modulus; $i++) {
    printf("d %d\n", ($i * $factor_d) % $modulus);
}

print "t\nq\n"
