sub foo {
	print "in sub foo\n";
	bar();
}

sub bar {
	print "in sub bar\n";
}

sub baz {
	print "in sub baz\n";
	bar();
	bar();
	bar();
	foo();
}

bar();

fork;

bar();
baz();
foo();
