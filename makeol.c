#include <stdio.h>
#include <assert.h>

int
main(int argc, char **argv)
{
  assert(argc == 2);
  FILE *fasl = fopen(argv[1], "r");
  int c;

  printf("static const unsigned char heap[] = {\n");

  while (1) {
    c = fgetc(fasl);
    printf("%d", c);
    if (!feof(fasl))
      printf(", ");
    else
      break;
  }

  printf("};\n");

  fclose(fasl);

  return 0;
}
