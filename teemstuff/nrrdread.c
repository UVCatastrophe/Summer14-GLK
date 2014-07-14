
/*

to compile:
gcc -Wall -o nrrdread nrrdread.c -I/$TEEM/include -L/$TEEM/lib -Wl,-rpath,$TEEM/lib -lteem -lpng

where $TEEM is the path into your teem-install directory
(with bin, lib, and include subdirectories)

 */

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>

int
main(int argc, char **argv) {
  char *err, *me, *filename;
  Nrrd *nin;
  unsigned int axi;

  me = argv[0];
  if (2 != argc) {
    fprintf(stderr, "usage: %s <filename>\n", me);
    return 1;
  }

  filename = argv[1];

  /* create a nrrd; at this point this is just an empty container */
  nin = nrrdNew();

  /* read in the nrrd from file */
  if (nrrdLoad(nin, filename, NULL)) {
    err = biffGetDone(NRRD);
    fprintf(stderr, "%s: trouble reading \"%s\":\n%s", me, filename, err);
    free(err);
    return 1;
  }

  /* say something about the array */
  printf("%s: \"%s\" is a %d-dimensional nrrd of type %s (%d)\n",
         me, filename, nin->dim,
         airEnumStr(nrrdType, nin->type),
         nin->type);
  for (axi=0; axi<nin->dim; axi++) {
    printf(" axis[%d] size = %u\n", axi, (unsigned int)nin->axis[axi].size);
  }
  printf("%s: the array contains %d elements, each %d bytes in size\n",
         me, (int)nrrdElementNumber(nin), (int)nrrdElementSize(nin));

  /* blow away both the Nrrd struct *and* the memory at nin->data
     (nrrdNix() frees the struct but not the data,
     nrrdEmpty() frees the data but not the struct) */
  nrrdNuke(nin);


  return 0;
}
