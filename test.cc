#include <stdlib.h>

int
foo(int a)
{
  int b = a;
  for (int i = 0; i < 3; i++)
    b++;
  if (b > 5)
    return b;
      
  return b+4;
}

int
main(int argc, char **argv)
{
  int a = 0;
  int b = 0;
  
  if (argc < 2)
    return(-1);
  
  a = atoi(argv[1]);

  for (int j = 0; j < 3; j++)
    a++;

  if (a < 5)
    {
      b = foo(a);
    }

  switch (a)
    {
    case 8:
      a+=b;
      break;
    case 12:
      a+=100;
      break;
    default:
      a+=b+50;
    }

  return a;
}