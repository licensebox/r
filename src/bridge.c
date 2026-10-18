#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <licensebox_json.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef struct {lb_client *p; SEXP callbacks; int busy; int failure;} client;
static const char *str(SEXP s){if(TYPEOF(s)!=STRSXP||XLENGTH(s)!=1||STRING_ELT(s,0)==NA_STRING) Rf_error("expected one nonmissing string");return Rf_translateCharUTF8(STRING_ELT(s,0));}
static SEXP invoke(client*c,int n,SEXP a,SEXP b){SEXP fn=VECTOR_ELT(c->callbacks,n);if(fn==R_NilValue)return R_NilValue;SEXP call=PROTECT(n==0?Rf_lang3(fn,a,b):Rf_lang1(fn));int error=0;SEXP r=R_tryEval(call,R_GlobalEnv,&error);UNPROTECT(1);if(error)c->failure=1;return error?R_NilValue:r;}
static void signin(void*x,const char*u,const char*v){client*c=x;SEXP a=PROTECT(Rf_mkString(u)),b=PROTECT(Rf_mkString(v));invoke(c,0,a,b);UNPROTECT(2);}
static double clock_cb(void*x){client*c=x;SEXP v=PROTECT(invoke(c,1,R_NilValue,R_NilValue));double n=(TYPEOF(v)==REALSXP||TYPEOF(v)==INTSXP)&&XLENGTH(v)==1?Rf_asReal(v):NAN;if(!isfinite(n))c->failure=1;UNPROTECT(1);return isfinite(n)?n:0;}
static int binding(void*x,char*out,size_t cap){client*c=x;SEXP v=PROTECT(invoke(c,2,R_NilValue,R_NilValue));int ok=TYPEOF(v)==STRSXP&&XLENGTH(v)==1&&STRING_ELT(v,0)!=NA_STRING;if(ok){const char*s=Rf_translateCharUTF8(STRING_ELT(v,0));ok=strlen(s)<cap;if(ok)memcpy(out,s,strlen(s)+1);}if(!ok)c->failure=1;UNPROTECT(1);return ok?0:1;}
static int cancel(void*x){client*c=x;if(c->failure)return 1;if(VECTOR_ELT(c->callbacks,3)==R_NilValue)return 0;SEXP v=PROTECT(invoke(c,3,R_NilValue,R_NilValue));int n=TYPEOF(v)==LGLSXP&&XLENGTH(v)==1?LOGICAL(v)[0]:NA_LOGICAL;if(n==NA_LOGICAL)c->failure=1;UNPROTECT(1);return c->failure||n==1;}
static void finalize(SEXP xp){client*c=R_ExternalPtrAddr(xp);if(c&&!c->busy){lb_client_destroy(c->p);R_ReleaseObject(c->callbacks);free(c);R_ClearExternalPtr(xp);}}
static client* get(SEXP xp){if(TYPEOF(xp)!=EXTPTRSXP||!R_ExternalPtrAddr(xp))Rf_error("client is closed");client*c=R_ExternalPtrAddr(xp);if(c->busy)Rf_error("callbacks must not reenter the client");return c;}
static SEXP failure(lb_result rc,client*c){const char*code=c?lb_error_code(c->p):lb_result_code(rc);if(!code||!strcmp(code,"ok")||rc==LB_INVALID_ARGUMENT)code=lb_result_code(rc);SEXP e=PROTECT(Rf_allocVector(VECSXP,2));SET_VECTOR_ELT(e,0,R_NilValue);SET_VECTOR_ELT(e,1,Rf_mkString(code));UNPROTECT(1);return e;}
static SEXP output(lb_result rc,char*p,client*c){if(c&&c->failure){lb_free(p);c->failure=0;Rf_error("Licensebox callback failed");}if(rc){lb_free(p);return failure(rc,c);}SEXP e=PROTECT(Rf_allocVector(VECSXP,2));SET_VECTOR_ELT(e,0,Rf_mkString(p?p:"null"));lb_free(p);SET_VECTOR_ELT(e,1,R_NilValue);UNPROTECT(1);return e;}
SEXP lbR_create(SEXP json,SEXP callbacks){const char*text=str(json);if(TYPEOF(callbacks)!=VECSXP||XLENGTH(callbacks)!=4)Rf_error("invalid callbacks");client*c=calloc(1,sizeof(*c));if(!c)Rf_error("allocation failed");c->callbacks=callbacks;R_PreserveObject(callbacks);lb_callbacks cb={c,VECTOR_ELT(callbacks,0)==R_NilValue?NULL:signin,VECTOR_ELT(callbacks,1)==R_NilValue?NULL:clock_cb,VECTOR_ELT(callbacks,2)==R_NilValue?NULL:binding,cancel};lb_result rc=lb_client_create_json(text,&cb,&c->p);if(rc||c->failure){lb_client_destroy(c->p);R_ReleaseObject(callbacks);free(c);return failure(rc?rc:LB_CANCELLED,NULL);}SEXP xp=PROTECT(R_MakeExternalPtr(c,R_NilValue,R_NilValue));R_RegisterCFinalizerEx(xp,finalize,TRUE);UNPROTECT(1);return xp;}
SEXP lbR_close(SEXP xp){if(TYPEOF(xp)!=EXTPTRSXP)Rf_error("invalid client handle");client*c=R_ExternalPtrAddr(xp);if(c&&c->busy)Rf_error("callbacks must not reenter the client");finalize(xp);return R_NilValue;}
SEXP lbR_call(SEXP xp,SEXP op,SEXP args){client*c=get(xp);const char*o=str(op),*a=str(args);char*p=NULL;c->busy=1;lb_result rc=lb_client_call_json(c->p,o,a,&p);c->busy=0;return output(rc,p,c);}
SEXP lbR_derive(SEXP product,SEXP source,SEXP raw){char out[128];lb_result rc=lb_derive_binding(str(product),str(source),str(raw),out,sizeof(out));if(rc)return failure(rc,NULL);SEXP e=PROTECT(Rf_allocVector(VECSXP,2));SET_VECTOR_ELT(e,0,Rf_mkString(out));SET_VECTOR_ELT(e,1,R_NilValue);UNPROTECT(1);return e;}
SEXP lbR_native(SEXP product){char out[128];lb_result rc=lb_native_binding(str(product),out,sizeof(out));if(rc)return failure(rc,NULL);SEXP e=PROTECT(Rf_allocVector(VECSXP,2));SET_VECTOR_ELT(e,0,Rf_mkString(out));SET_VECTOR_ELT(e,1,R_NilValue);UNPROTECT(1);return e;}
SEXP lbR_verify(SEXP token,SEXP product,SEXP keys,SEXP now){char*p=NULL;lb_result rc=lb_verify_lease_json(str(token),str(product),str(keys),Rf_asReal(now),&p);return output(rc,p,NULL);}
static const R_CallMethodDef methods[]={{"lbR_create",(DL_FUNC)&lbR_create,2},{"lbR_close",(DL_FUNC)&lbR_close,1},{"lbR_call",(DL_FUNC)&lbR_call,3},{"lbR_derive",(DL_FUNC)&lbR_derive,3},{"lbR_native",(DL_FUNC)&lbR_native,1},{"lbR_verify",(DL_FUNC)&lbR_verify,4},{NULL,NULL,0}};
void R_init_licensebox(DllInfo*d){R_registerRoutines(d,NULL,methods,NULL,NULL);R_useDynamicSymbols(d,FALSE);}
