#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DELIM '\n'

int main()
{
	int n;
	int count;
	printf("Enter how many lines from last needs to be printed:\n");
	scanf("%d", &n);
	count = n;
	char *str = "This is a new line\n"
		"This is a part of docs that are of no use\n"
		"And then blah blah\n"
		"And then blah blah 2\n"
		"And then blah blah 3\n"
		"And then blah blah 4\n"
		"And then blah blah 5\n";
	char *last_ptr = strrchr(str, DELIM);
	while (n >= 0) {
		if (*last_ptr == '\n') {
			last_ptr--;
			n--;
		}
		last_ptr--;
	}
	last_ptr += 2;
	printf("Last %d lines:", count);
	printf("%s\n", last_ptr);
}
