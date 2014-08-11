
/*

to compile:
gcc -Wall -o isodemo isodemo.c -I/$TEEM/include -L/$TEEM/lib -Wl,-rpath,$TEEM/lib -lteem -lpng

where $TEEM is the path into your teem-install directory
(with bin, lib, and include subdirectories)

example run: ./isodemo 

 */

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/limn.h>
#include <teem/seek.h>

char *progInfo=("Program to demo isosurface extraction with seek");

int
main(int argc, const char **argv) {
  const char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  Nrrd *nin;
  char *err;
  float *ival;
  unsigned int ivalIdx, ivalNum;
  limnPolyData *lpd;
  seekContext *sctx;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "volume from which to extract isosurfaces", NULL, NULL,
             nrrdHestNrrd);
  hestOptAdd(&hopt, "v", "v0 v1", airTypeFloat, 1, -1, &ival, "0.5",
             "one or more isovalues to extract at", &ivalNum);
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, progInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);


  sctx = seekContextNew();
  seekVerboseSet(sctx, 0);
  seekNormalsFindSet(sctx, AIR_TRUE);
  airMopAdd(mop, sctx, (airMopper)seekContextNix, airMopAlways);

  lpd = limnPolyDataNew();
  airMopAdd(mop, lpd, (airMopper)limnPolyDataNix, airMopAlways);

  if (seekDataSet(sctx, nin, NULL, 0)
      || seekTypeSet(sctx, seekTypeIsocontour)) {
    err = biffGetDone(SEEK);
    fprintf(stderr, "%s: trouble with set-up:\n%s", me, err);
    free(err);
  }

  for (ivalIdx=0; ivalIdx<ivalNum; ivalIdx++) {
    if (seekIsovalueSet(sctx, ival[ivalIdx])
        || seekUpdate(sctx)
        || seekExtract(sctx, lpd)) {
      err = biffGetDone(SEEK);
      fprintf(stderr, "%s: trouble getting isovalue:\n%s", me, err);
      free(err);
    }

    /* do something with the isosurface */
    printf("isoval %g -> xyzwNum = %u\n", ival[ivalIdx], lpd->xyzwNum);
  }

  airMopOkay(mop);
  exit(0);
}
