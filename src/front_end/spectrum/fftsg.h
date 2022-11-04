//
// Function Prototypes
//
void cdft(int n, int isgn, double *a, int *ip, double *w);
void rdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);
void dfct(int n, double *a, double *t, int *ip, double *w);
void dfst(int n, double *a, double *t, int *ip, double *w);
void makewt(int nw, int *ip, double *w);
void makeipt(int nw, int *ip);
void makect(int nc, int *ip, double *c);
void cftfsub(int n, double *a, int *ip, int nw, double *w);
void cftbsub(int n, double *a, int *ip, int nw, double *w);
void bitrv2(int n, int *ip, double *a);
void bitrv2conj(int n, int *ip, double *a);
void bitrv216(double *a);
void bitrv216neg(double *a);
void bitrv208(double *a);
void bitrv208neg(double *a);
void cftf1st(int n, double *a, double *w);
void cftb1st(int n, double *a, double *w);




