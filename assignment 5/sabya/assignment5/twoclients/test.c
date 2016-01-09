#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	FILE *fstream = fopen("shared/abc.txt", "r");
	char* line = NULL, buf[4], *p;
	int c, status, i, len = 0;
	/*while((c = fgetc(fstream)) != EOF) {
		printf("%c", (unsigned char)c);
	}*/
	while((status = getline(&line, (unsigned long*)&len, fstream)) > 0){
		printf("%s", line);
		line = 0;
		len = 0;
	}
	fclose(fstream);
	return 0;
}