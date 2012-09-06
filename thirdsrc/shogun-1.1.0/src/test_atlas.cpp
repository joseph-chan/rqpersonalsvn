extern "C"
{
#include <cblas.h>
#include <atlas/clapack.h>
}
int main(int argc, char** argv)
{
	    int skip=argc;
		    int n=argc; double* a=0; double* v1=0; double* v2=0;
			    int result=clapack_dpotrf(CblasRowMajor, CblasUpper, n, a, n);
				    int r = cblas_ddot(n, v1, skip, v2, skip);
					    return 0;
}
