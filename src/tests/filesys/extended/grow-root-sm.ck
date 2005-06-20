# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(grow-root-sm) begin
(grow-root-sm) creating and checking "file0"
(grow-root-sm) creating and checking "file1"
(grow-root-sm) creating and checking "file2"
(grow-root-sm) creating and checking "file3"
(grow-root-sm) creating and checking "file4"
(grow-root-sm) creating and checking "file5"
(grow-root-sm) creating and checking "file6"
(grow-root-sm) creating and checking "file7"
(grow-root-sm) creating and checking "file8"
(grow-root-sm) creating and checking "file9"
(grow-root-sm) end
EOF