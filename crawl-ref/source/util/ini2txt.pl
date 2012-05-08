#!/usr/bin/env perl
# Generate or update a .txt file (database or description) from a .ini file
# (normally downloaded from transifex). If the .txt file already exists,
# the values of its keys found in the .ini will be updated, and the rest of the
# file will be preserved (comments and keys not in the .ini).
# This behaviour is desired for updating the original files in english.
# For translated files, use the -o option (overwrite). This way, keys deleted
# in the original files will also be deleted in translated files.
# Use the -w option to automatically wrap text at 80 columns (for description
# files, not databasa ones).

use strict;
use warnings;
use File::Basename;
use Getopt::Std;
use Text::WrapI18N qw(wrap);

if ($^O ne 'msys') {
    use open ':encoding(utf8)';
}

die "Usage: $0 ini_files\n" unless (@ARGV);

getopts('ow');
our($opt_o, $opt_w);

sub clean_value {
    $_ = shift;

    # transifex returns html encoded quotes
    s/&quot;/"/mg;

    # convert literal \n to actual newlines
    s/\\n/\n/g;

    # In case we have some windows newline format, remove them
    tr/\r//d;

    # If using -w, wrap the text
    if ($opt_w) {
        $_ = wrap("", "", $_) ;

        # WrapIl8N adds spaces at the end of lines and doesn't support the
        # unexpand option.
        s/ +$//mg;
        s/\t/ {8}/mg;
    }

    return $_;
}

foreach my $file (@ARGV) {
    unless (-e $file) {
        print "$file not found\n";
        next;
    }
    my ($basename,$path) = fileparse($file, '.ini');
    my $out_file = "$path/$basename.txt";
    open IN, $file;
    if (-e $out_file and not $opt_o) {
        my $original_file = $out_file . "~";
        rename $out_file, $original_file;
        my %Text;
        while (<IN>) {
            chomp;
            next if (/^#/);
            my ($key, $value) = /(.*?)=(.*)$/;
            $Text{$key} = clean_value($value);
        }
        close IN;

        my $key;
        open IN, $original_file;
        open OUT, ">$out_file";
        while (<IN>) {
            chomp;
            my $new_text = $key && $Text{$key};
            next if (!/^%+$/ and $new_text);
            if (/^%+$/) {
                print OUT "\n", $Text{$key}, "\n" if ($new_text);
                $key = "";
            }
            elsif (!$key and !/^#/) {
                chomp ($key = $_);
            }
            print OUT "$_\n";
        }
        close IN;
        close OUT;
        unlink $original_file;
    } else {
        my $empty = 1;
        open OUT, ">$out_file";
        while (<IN>) {
            next if (/^#/);
            $empty = 0;
            my ($key, $value) = /(.*?)=(.*)/;
            print OUT "%%%%\n";
            print OUT "$key\n\n";
            print OUT clean_value($value), "\n";
        }
        close IN;
        if ($empty) {
            close OUT;
            unlink $out_file;
        } else {
            print OUT "%%%%\n";
            close OUT;
        }
    }
}