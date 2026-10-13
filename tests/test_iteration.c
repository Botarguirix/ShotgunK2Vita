#include "runtime.h"
#include <stdio.h>
int main(int argc,char**argv){
 if(argc<2||!sg_boot(argv[1],stdout))return 2;
 int ok=sg_host_eval(
  "local a={1,1,2,3};local n=0;for e in all(a) do n=n+1;del(a,e) end;assert(n==4 and #a==0);"
  "a={1,2,3,4};local seen={};for e in all(a) do add(seen,e);if e==2 then del(a,1);del(a,3) end end;assert(#seen==3 and seen[3]==4);"
  "a={1,2,3};n=0;for e in all(a) do n=n+1;if e==1 then add(a,9,1) end end;assert(n==3);"
  "a={1,2,3};local outer=0;for e in all(a) do outer=outer+1;for f in all(a) do if f==2 then del(a,f) end end end;assert(outer==2);"
  "for i=1,200 do for e in all({1,2,3}) do break end end;collectgarbage('collect');"
  "a={1,2,3};n=0;for e in all(a) do n=n+1;del(a,e) end;assert(n==3 and #a==0)"
 );if(!ok)puts(sg_error());printf("MUTATION_SAFE_ITERATION=%d\n",ok);sg_close();return !ok;
}
