#!/usr/bin/env perl -w

use strict;

my ($infile, $outfile);

if (-e "enum.h")
{
    $infile = "enum.h";
    $outfile = "cmd-name.h";
}
elsif (-e "../enum.h")
{
    $infile = "../enum.h";
    $outfile = "../cmd-name.h";
}
else {
    die "Can't find 'enum.h'";
}

unless (open(INFILE, "<$infile")) {
    die "Couldn't open '$infile' for reading: $!\n";
}

unless (open(OUTFILE, ">$outfile")) {
    die "Couldn't open '$outfile' for writing: $!\n";
}

# All set, now get to first command
while (<INFILE>) {
    last if (/^ *CMD_NO_CMD/);
}

unless (/^ *CMD_NO_CMD/) {
    die "Couldn't find CMD_NO_CMD in enum.h\n";
}

print OUTFILE "// Generated by util/cmd-name.pl\n\n";

while (<INFILE>) {
    # Pass through pre-processor directives
    if (/^#/) {
        print OUTFILE $_;
        next;
    }

    s|//.*||; # Strip comments
    s/=.*//;  # Strip enum asignments
    s/\s//g;  # Strip whitespace
    s/,$//;   # Strip comma

    next if (/^$/); # Skip blank lines

    my $cmd = $_;

    unless ($cmd =~ /^CMD_/) {
        die "'$cmd' doesn't start with CMD_\n";
    }

    # Don't include synthetic keys
    last if ($cmd eq "CMD_DISABLE_MORE");

    # Skip MIN or MAX enums, since they aren't commands
    next if ($cmd =~ /^CMD_(MIN|MAX)_/);

    print OUTFILE "{$cmd, \"$cmd\"},\n";
}

# End of array sentinel
print OUTFILE "\n";
print OUTFILE "{CMD_NO_CMD, NULL}\n";

close (INFILE);
close (OUTFILE);

exit (0);
